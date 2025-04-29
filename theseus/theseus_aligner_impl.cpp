#include "theseus_aligner_impl.h"

namespace theseus {

TheseusAlignerImpl::TheseusAlignerImpl(const Penalties &penalties,
                                       Graph &&graph,
                                       bool msa,
                                       bool score_only) : _penalties(penalties),
                                                          _graph(std::move(graph)),
                                                          _is_msa(msa),
                                                          _is_score_only(score_only) {
    // TODO: Gap-linear, gap-affine and dual affine-gap.
    const auto n_scores = std::max({penalties.gapo() +_penalties.gape(),
                                  _penalties.gapo() +_penalties.gape(),
                                  _penalties.mism()}) + 1;

    if (_is_msa) {
      _poa_graph = std::make_unique<POAGraph>();
      _poa_graph->create_initial_graph(_graph);
    }

    _scope = std::make_unique<Scope>(n_scores);
    _beyond_scope = std::make_unique<BeyondScope>();
    constexpr int expected_nvertices = std::max(1024, 0); // TODO: Set the expected number of vertices
    _vertices_data = std::make_unique<VerticesData>(penalties, n_scores, expected_nvertices);
    _scratchpad = std::make_unique<ScratchPad>(-1024, 1024);
}

void TheseusAlignerImpl::new_alignment(int start_vtx) {
    // TODO: Avoid recomputing the max_diag if possible.
    int max_diag = 0, v_n;
    for (int l = 0; l < _graph._vertices.size(); ++l) {
      v_n = _graph._vertices[l].value.size();
      max_diag = std::max(max_diag, v_n);
    }
    const int min_diag = -_seq.size();

    if (_scratchpad->max_diag() < max_diag ||
        _scratchpad->min_diag() > min_diag) {
        // TODO: Compute the max and min with a factor.
        _scratchpad = std::make_unique<ScratchPad>(min_diag, max_diag);
    }

    // Set data for first score
    _beyond_scope->new_score();
    _scope->new_score(_score);

    // TODO: Allow for different initial conditions. Now only global alignment.
    // Initial condition Global Alignment
    Cell init_condition;
    init_condition.offset = 0;
    init_condition.vertex_id = 0;
    init_condition.diag = 0;
    init_condition.score_diff = 0;
    // Initial vertex data
    _beyond_scope->m_jumps(_score).push_back(init_condition);
    _vertices_data->activate_vertex(start_vtx);
    _vertices_data->get_vertex_data(start_vtx)._m_jumps_positions[0].push_back(0);

    // Alignment data
    _alignment.cigar.path.clear();
    _alignment.cigar.edit_op.clear();
}


// Process a given vertex with a given _score
void TheseusAlignerImpl::process_vertex(Graph::vertex* curr_v,
                                        int v) {

  // Perform the next operation
  int upper_bound = curr_v->value.size();
  next_I(curr_v, upper_bound, v);
  _scratchpad->reset();
  next_D(upper_bound, v);
  _scratchpad->reset();
  next_M(upper_bound, v);
  _scratchpad->reset();

  // Perform the extend operations
  int v_pos = _vertices_data->get_id(v);
  int start_idx, end_idx = _scope->m_pos(_score)[v_pos];
  if (v_pos == 0) start_idx = 0;
  else start_idx = _scope->m_pos(_score)[v_pos - 1];
  for (int idx = start_idx; idx < end_idx; ++idx) {
    extend_diagonal(curr_v, _beyond_scope->m_wf(_score)[idx], v, _beyond_scope->m_wf(_score)[idx], idx, Cell::Matrix::M);
  }
}


void TheseusAlignerImpl::compute_new_wave() {

  // Update invalid segments
  _vertices_data->expand();
  _vertices_data->compact();

  // Process all active vertices
  int num_active_vertices = _vertices_data->num_active_vertices(), v;
  for (int l = 0; l < num_active_vertices; ++l) {
    v = _vertices_data->get_vertex_id(l);
    Graph::vertex* curr_v = &_graph._vertices[v];
    process_vertex(curr_v, v);
  }
}

Alignment TheseusAlignerImpl::align(std::string seq)
{
  _scope->new_alignment();
  _beyond_scope->new_alignment();
  _vertices_data->new_alignment();
  _seq = seq;

  new_alignment(0);

  // TODO: Set initial conditions
  _score = 0;
  _end_vertex = _graph._vertices.size() - 1; // TODO: Set the end vertex
  _end = false;
  // _graph.print_code_graphviz();

  // Find the optimal _score and an optimal alignment
  while (!_end)
  {
    // Compute the values of the new wave
    if (_score == 0) extend_diagonal(&_graph._vertices[0], _beyond_scope->m_jumps(0)[0], 0, _beyond_scope->m_jumps(0)[0], 0, Cell::Matrix::MJumps);
    compute_new_wave();

    // Update _score
    _score = _score + 1;

    // Clear the corresponding waves and metadata from the scope
    _beyond_scope->new_score();
    _scope->new_score(_score);
    _vertices_data->new_score(_score);
  }
  _score -= 1;
  _alignment.score = _score;

  // Update the graph in case of MSA
  if (_is_msa) {
      _seq_ID += 1;
      backtrace(0);
      _poa_graph->add_alignment_poa(_graph, _alignment.cigar, _seq, _seq_ID);
      _graph._vertices.clear();
      _poa_graph->create_compacted_graph(_graph, _poa_graph->_end_vtx_poa);
  }

  return _alignment;
}

  // Sparsify M data
  void TheseusAlignerImpl::sparsify_M_data(Cell::Wavefront & dense_wf,
                                           int offset_increase,
                                           int shift_factor,
                                           int start_idx,
                                           int end_idx,
                                           int m,
                                           int upper_bound,
                                           int new_score_diff)
  {

    int len = end_idx - start_idx, new_col;
    Cell new_cell;

    // Sparsify the active diagonals
    for (int l = 0; l < len; ++l)
    {
      new_cell = dense_wf[start_idx + l];
      new_cell.diag += shift_factor;
      new_cell.offset += offset_increase;
      new_cell.score_diff = new_score_diff;
      new_cell.from_matrix = Cell::Matrix::M;
      new_cell.prev_pos = start_idx + l;
      new_col = new_cell.offset + new_cell.diag; // d = j - i -> j = d + i

      // Check validity
      if (new_cell.offset <= m && new_col <= upper_bound)
      { // If in bounds
        // Branchless push_back
        auto &cell = _scratchpad->access_alloc(new_cell.diag);

        // If better offset
        const bool cmp = cell.offset < new_cell.offset;
        cell = (cmp) ? new_cell : cell;
      }
    }
  }

  // Sparsify jumps data
  void TheseusAlignerImpl::sparsify_jumps_data(Cell::Wavefront & dense_wf,
                                               std::vector<int> & jumps_positions,
                                               int offset_increase,
                                               int shift_factor,
                                               int m,
                                               int upper_bound,
                                               int new_score_diff,
                                               Cell::Matrix from_matrix)
  {
    int len = jumps_positions.size(), new_col, pos;
    Cell new_cell;

    // Sparsify the active diagonals
    for (int l = 0; l < len; ++l)
    {
      pos = jumps_positions[l];
      new_cell = dense_wf[pos];
      new_cell.prev_pos = pos;
      new_cell.diag += shift_factor;
      new_cell.offset += offset_increase;
      new_cell.score_diff = new_score_diff;
      new_cell.from_matrix = from_matrix;
      new_col = new_cell.offset + new_cell.diag; // d = j - i -> j = d + i

      // Check validity
      if (new_cell.offset <= m && new_col <= upper_bound)
      { // If in bounds
        // Branchless push_back
        auto &cell = _scratchpad->access_alloc(new_cell.diag);

        // If better offset
        const bool cmp = cell.offset < new_cell.offset;
        cell = (cmp) ? new_cell : cell;
      }
    }
  }

  // Sparsify indel
  void TheseusAlignerImpl::sparsify_indel_data(std::vector<Cell> & dense_wf,
                                               int offset_increase,
                                               int shift_factor,
                                               int start_idx,
                                               int end_idx,
                                               int m,
                                               int upper_bound,
                                               int added_score_diff)
  {

    int len = end_idx - start_idx, new_col;
    Cell new_cell;

    // Sparsify the active diagonals
    for (int l = 0; l < len; ++l)
    {
      // Vertex_id and previous matrix are the same as before
      new_cell = dense_wf[start_idx + l];
      new_cell.diag += shift_factor;
      new_cell.offset += offset_increase;
      new_cell.score_diff += added_score_diff;
      new_col = new_cell.offset + new_cell.diag; // d = j - i -> j = d + i

      // Check validity
      if (new_cell.offset <= m && new_col <= upper_bound)
      { // If in bounds
        // Branchless push_back
        auto &cell = _scratchpad->access_alloc(new_cell.diag);

        // If better offset
        const bool cmp = cell.offset < new_cell.offset;
        cell = (cmp) ? new_cell : cell;
      }
    }
  }

  // Compute next I matrix
  void TheseusAlignerImpl::next_I(Graph::vertex * curr_v,
                                  int upper_bound,
                                  int v)
  {

    // Sparsify data (put it in the scratch pad)
    int pos_prev_M = _score - (_penalties.gapo() + _penalties.gape()), pos_prev_I = _score - _penalties.gape(), start_idx, end_idx, pos_prev_M_scope = _vertices_data->get_pos(pos_prev_M);
    int pos_prev_I_scope = _vertices_data->get_pos(pos_prev_I);

    // Come from an Insertion
    if (pos_prev_I >= 0) {
      if (_scope->i_pos(pos_prev_I).size() > _vertices_data->get_id(v))
      {
        if (_vertices_data->get_id(v) > 0)
          start_idx = _scope->i_pos(pos_prev_I)[_vertices_data->get_id(v) - 1];
        else
          start_idx = 0;
        end_idx = _scope->i_pos(pos_prev_I)[_vertices_data->get_id(v)];
        sparsify_indel_data(_scope->i_wf(pos_prev_I), 0, 1, start_idx, end_idx, _seq.size(), upper_bound, _penalties.gape()); // Sparsify I data
      }
      sparsify_jumps_data(_beyond_scope->i_jumps(pos_prev_I), _vertices_data->get_vertex_data(v)._i_jumps_positions[pos_prev_I_scope], 0, 1, _seq.size(), upper_bound, _penalties.gape(), Cell::Matrix::IJumps);
    }

    // Come from M
    if (pos_prev_M >= 0) {
      if (_scope->m_pos(pos_prev_M).size() > _vertices_data->get_id(v)) {
        if (_vertices_data->get_id(v) > 0)
          start_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v) - 1];
        else
          start_idx = 0;
        end_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v)];
        sparsify_M_data(_beyond_scope->m_wf(pos_prev_M), 0, 1, start_idx, end_idx, _seq.size(), upper_bound, _penalties.gapo() + _penalties.gape()); // Sparsify M data
      }
      sparsify_jumps_data(_beyond_scope->m_jumps(pos_prev_M), _vertices_data->get_vertex_data(v)._m_jumps_positions[pos_prev_M_scope], 0, 1, _seq.size(), upper_bound, _penalties.gapo() + _penalties.gape(), Cell::Matrix::MJumps);
    }

    // Densify data (store it in the big wavefront)
    for (auto diag : _scratchpad->active_diags()) {
      if (_vertices_data->valid_diagonal<Cell::Matrix::I>(v, diag)) {
        _scope->i_wf(_score).push_back((*_scratchpad)[diag]);     // Store Cell
      }
    }
    int new_end = _scope->i_wf(_score).size();
    _scope->i_pos(_score).push_back(new_end);

    // Check, store and invalidate new I jumps
    if (_vertices_data->get_id(v) == 0) start_idx = 0;
    else start_idx = _scope->i_pos(_score)[_vertices_data->get_id(v) - 1];
    if (curr_v->out_edges.size() > 0) {
      check_and_store_jumps(curr_v, _scope->i_wf(_score), start_idx, new_end);
    }
}


// Compute next D matrix
void TheseusAlignerImpl::next_D(int upper_bound,
                                int v)
{

  // Sparsify data (put it in the scratch pad)
  int pos_prev_M = _score - (_penalties.gapo() + _penalties.gape()), pos_prev_D = _score - _penalties.gape(), start_idx, end_idx, pos_prev_M_scope = _vertices_data->get_pos(pos_prev_M);

  // Come from a Deletion
  if (pos_prev_D >= 0 && _scope->d_pos(pos_prev_D).size() > _vertices_data->get_id(v))
  {
    if (_vertices_data->get_id(v) > 0)
      start_idx = _scope->d_pos(pos_prev_D)[_vertices_data->get_id(v) - 1];
    else
      start_idx = 0;
    end_idx = _scope->d_pos(pos_prev_D)[_vertices_data->get_id(v)];
    sparsify_indel_data(_scope->d_wf(pos_prev_D), 1, -1, start_idx, end_idx, _seq.size(), upper_bound, _penalties.gape()); // Sparsify D data
  }

  // Come from M
  if (pos_prev_M >= 0) {
    if (_scope->m_pos(pos_prev_M).size() > _vertices_data->get_id(v))
    {
      if (_vertices_data->get_id(v) > 0)
        start_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v) - 1];
      else
        start_idx = 0;
      end_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v)];
      sparsify_M_data(_beyond_scope->m_wf(pos_prev_M), 1, -1, start_idx, end_idx, _seq.size(), upper_bound, _penalties.gapo() + _penalties.gape()); // Sparsify M data
    }
    sparsify_jumps_data(_beyond_scope->m_jumps(pos_prev_M), _vertices_data->get_vertex_data(v)._m_jumps_positions[pos_prev_M_scope], 1, -1, _seq.size(), upper_bound, _penalties.gapo() + _penalties.gape(), Cell::Matrix::MJumps);
  }

  // Densify data (store it in the big wavefront)
  for (auto diag : _scratchpad->active_diags()) {
    if (_vertices_data->valid_diagonal<Cell::Matrix::D>(v, diag)) {
      _scope->d_wf(_score).push_back((*_scratchpad)[diag]); // Store Cell
    }
  }
  int new_end = _scope->d_wf(_score).size();
  _scope->d_pos(_score).push_back(new_end);
}

// Compute next M matrix
void TheseusAlignerImpl::next_M(int upper_bound,
                                int v) {

  // Sparsify data (put it in the scratch pad)
  int pos_prev_M = _score - _penalties.mism(), pos_prev_D = _score, pos_prev_I = _score, start_idx, end_idx, pos_prev_M_scope = _vertices_data->get_pos(pos_prev_M);

  // Come from a Deletion
  if (_scope->d_pos(pos_prev_D).size() > _vertices_data->get_id(v))  {
    if (_vertices_data->get_id(v) > 0) start_idx = _scope->d_pos(pos_prev_D)[_vertices_data->get_id(v) - 1];
    else start_idx = 0;
    end_idx = _scope->d_pos(pos_prev_D)[_vertices_data->get_id(v)];
    sparsify_indel_data(_scope->d_wf(pos_prev_D), 0, 0, start_idx, end_idx, _seq.size(), upper_bound, 0);  // Sparsify D data
  }

  // Come from an Insertion
  if (_scope->i_pos(pos_prev_I).size() > _vertices_data->get_id(v))  {
    if (_vertices_data->get_id(v) > 0) start_idx = _scope->i_pos(pos_prev_I)[_vertices_data->get_id(v) - 1];
    else start_idx = 0;
    end_idx = _scope->i_pos(pos_prev_I)[_vertices_data->get_id(v)];
    sparsify_indel_data(_scope->i_wf(pos_prev_I), 0, 0, start_idx, end_idx, _seq.size(), upper_bound, 0);  // Sparsify I data
  }

  // Come from M
  if (pos_prev_M >= 0) {
    if (_scope->m_pos(pos_prev_M).size() > _vertices_data->get_id(v))  {
      if (_vertices_data->get_id(v) > 0) start_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v) - 1];
      else start_idx = 0;
      end_idx = _scope->m_pos(pos_prev_M)[_vertices_data->get_id(v)];
      sparsify_M_data(_beyond_scope->m_wf(pos_prev_M), 1, 0, start_idx, end_idx,  _seq.size(), upper_bound, _penalties.mism());  // Sparsify M data
    }
    sparsify_jumps_data(_beyond_scope->m_jumps(pos_prev_M), _vertices_data->get_vertex_data(v)._m_jumps_positions[pos_prev_M_scope], 1, 0, _seq.size(), upper_bound, _penalties.mism(), Cell::Matrix::MJumps);
  }

  // Densify data (store it in the big wavefront)
  for (auto diag : _scratchpad->active_diags()) {
    if (_vertices_data->valid_diagonal<Cell::Matrix::M>(v, diag)) {
      _beyond_scope->m_wf(_score).push_back((*_scratchpad)[diag]);     // Store Cell
    }
  }
  int new_end =_beyond_scope->m_wf(_score).size();
  _scope->m_pos(_score).push_back(new_end);
}



// Store the jump in neighbours
void TheseusAlignerImpl::store_M_jump(Graph::vertex* curr_v,
                                      Cell &prev_cell,
                                      int prev_pos,
                                      Cell::Matrix from_matrix,
                                      int score_diff) {

  // Invalidate the jumping diagonal
  _vertices_data->invalidate_m_jump(_vertices_data->get_id(prev_cell.vertex_id), prev_cell.diag);
  int pos_score = _vertices_data->get_pos(_score);
  int new_diag = -prev_cell.offset;
  int num_out_v = curr_v->out_edges.size();
  Cell new_cell = prev_cell;
  new_cell.from_matrix = from_matrix;
  new_cell.prev_pos = prev_pos;
  new_cell.score_diff = score_diff;

  for (int l = 0; l < num_out_v; ++l) {
    new_cell.vertex_id = curr_v->out_edges[l].to_vertex;
    new_cell.diag = new_diag + curr_v->out_edges[l].overlap;
    Graph::vertex* new_v = &_graph._vertices[new_cell.vertex_id];
    _vertices_data->activate_vertex(new_cell.vertex_id);

    // Store jump and metadata
    bool valid_diag = _vertices_data->valid_diagonal<Cell::Matrix::M>(new_cell.vertex_id, new_cell.diag); // Extend only if it has not yet been visited
    if (valid_diag) { // Extend only if it has not yet been visited
      int pos_new_cell = _beyond_scope->m_jumps(_score).size();
      _beyond_scope->m_jumps(_score).push_back(new_cell);
      _vertices_data->get_vertex_data(new_cell.vertex_id)._m_jumps_positions[pos_score].push_back(pos_new_cell);
      extend_diagonal(new_v, _beyond_scope->m_jumps(_score)[pos_new_cell], new_cell.vertex_id, _beyond_scope->m_jumps(_score)[pos_new_cell], pos_new_cell, Cell::Matrix::MJumps);
    }
  }
}


// Store the jump in neighbours
void TheseusAlignerImpl::store_I_jump(Graph::vertex* curr_v,
                                      Cell& prev_cell,
                                      int prev_pos,
                                      Cell::Matrix from_matrix) {

  // Invalidate the jumping diagonal
  _vertices_data->invalidate_i_jump(_vertices_data->get_id(prev_cell.vertex_id), prev_cell.diag);

  std::vector<Cell> new_wave;
  int pos_score = _vertices_data->get_pos(_score);
  int new_diag = -prev_cell.offset;
  int len = curr_v->out_edges.size();
  Cell new_cell = prev_cell;
  new_cell.from_matrix = from_matrix;
  new_cell.prev_pos = prev_pos;
  for (int l = 0; l < len; ++l) {
    new_cell.vertex_id = curr_v->out_edges[l].to_vertex;
    new_cell.diag = new_diag + curr_v->out_edges[l].overlap;
    _vertices_data->activate_vertex(new_cell.vertex_id);

    // Store jump and metadata
    bool valid_diag = _vertices_data->valid_diagonal<Cell::Matrix::I>(new_cell.vertex_id, new_cell.diag);
    if (valid_diag) { // Extend only if it has not yet been visited
      int pos_new_cell = _beyond_scope->i_jumps(_score).size();
      _beyond_scope->i_jumps(_score).push_back(new_cell);
      _vertices_data->get_vertex_data(new_cell.vertex_id)._i_jumps_positions[pos_score].push_back(pos_new_cell);
    }
  }
}


// Check and store I jumps (that is, those diagonals that have reached the last column of a vertex)
void TheseusAlignerImpl::check_and_store_jumps(Graph::vertex *curr_v,
                                               std::vector<Cell> &curr_wavefront,
                                               int start_idx,
                                               int end_idx)
{

  int len = end_idx - start_idx, diag, offset, curr_j, n = curr_v->value.size(), prev_pos;
  Cell::Matrix from_matrix;

  for (int l = 0; l < len; ++l) {
    diag = curr_wavefront[start_idx + l].diag;
    offset = curr_wavefront[start_idx + l].offset;
    curr_j = diag + offset;
    if (curr_j == n && offset <= _seq.size()) {
      from_matrix = curr_wavefront[start_idx + l].from_matrix;
      prev_pos = curr_wavefront[start_idx + l].prev_pos;
      store_M_jump(curr_v, curr_wavefront[start_idx + l], prev_pos, from_matrix, curr_wavefront[start_idx + l].score_diff);
      store_I_jump(curr_v, curr_wavefront[start_idx + l], prev_pos, from_matrix);
    }
  }
}


// Compute the Longest Common Prefix between two given sequences
void TheseusAlignerImpl::LCP(std::string &seq_1,
                             std::string &seq_2,
                             int &offset,
                             int &j) {

    // Find LCP
    int len_seq_1 = seq_1.size();
    int len_seq_2 = seq_2.size();
    while (offset < len_seq_1 && j < len_seq_2 && seq_1[offset] == seq_2[j]) {
        offset = offset + 1;   // Update the f.r. of this diagonal
        j = j + 1;
    }
}


// TODO: Implement different end conditions as Global, Semi-Global...
void TheseusAlignerImpl::check_end_condition(Cell curr_data, // Offset and prev_index
                        int j,
                        int v) {

  int j_end = _graph._vertices[v].value.size(); // The last node is empty
  if (!_is_msa && curr_data.offset == _seq.size()) {
    _end = true;
    _start_pos = curr_data;
  }
  else if (_is_msa && curr_data.offset == _seq.size() && v == _end_vertex && j == j_end) { // End condition global alignment
    _end = true;
    _start_pos = curr_data;
  }
}


// Extend a particular diagonal
void TheseusAlignerImpl::extend_diagonal(
    Graph::vertex *curr_v,
    Cell &curr_cell,
    int v,
    Cell &prev_cell,
    int prev_pos,
    Cell::Matrix from_matrix) {

  // Longest Common prefix
  int j = curr_cell.diag + curr_cell.offset;
  LCP(_seq, curr_v->value, curr_cell.offset, j); // Find Longest Common Prefix

  // End condition
  check_end_condition(curr_cell, j, v); // Check end condition

  // Check jump
  if (j == curr_v->value.size() && curr_cell.offset <= _seq.size() && curr_v->out_edges.size() > 0) {
    store_M_jump(curr_v, prev_cell, prev_pos, from_matrix, 0);
  }
}


// Add matches to our backtracking vector
void TheseusAlignerImpl::add_matches(
    int start_matches,
    int end_matches) {

  int size = end_matches - start_matches;
  for (int k = 0; k < size; ++k) {
    _alignment.cigar.edit_op.push_back('M');
  }
}


// Add a mismatch to our backtracking vector
void TheseusAlignerImpl::add_mismatch()
{
  _alignment.cigar.edit_op.push_back('X');
}


// Add an insertion to our backtracking vector
void TheseusAlignerImpl::add_insertion()
{
  _alignment.cigar.edit_op.push_back('I');
}


// Add a deletion to our backtracking vector
void TheseusAlignerImpl::add_deletion()
{
  _alignment.cigar.edit_op.push_back('D');
}


void TheseusAlignerImpl::one_backtrace_step(
    Cell &curr_cell) {

  // Get previous cell
  _score -= curr_cell.score_diff;
  Cell prev_cell;
  if (curr_cell.from_matrix == Cell::Matrix::M) prev_cell = _beyond_scope->m_wf(_score)[curr_cell.prev_pos];
  else if (curr_cell.from_matrix == Cell::Matrix::MJumps) prev_cell = _beyond_scope->m_jumps(_score)[curr_cell.prev_pos];
  else prev_cell = _beyond_scope->i_jumps(_score)[curr_cell.prev_pos];

  // Inside the same vertex or jump
  int num_indels;
  if (curr_cell.vertex_id == prev_cell.vertex_id) { // Still in the same vertex
    if (curr_cell.diag == prev_cell.diag) {                                               // Mismatch
      if (curr_cell.offset > prev_cell.offset) {    // Consider 0 length vertices
        add_matches(prev_cell.offset + 1, curr_cell.offset);
        add_mismatch();
      }
    }
    else { // Indel
      if (curr_cell.diag < prev_cell.diag) {                                              // Deletion
        num_indels = prev_cell.diag - curr_cell.diag;
        add_matches(prev_cell.offset + num_indels, curr_cell.offset);
        for (int l = 0; l < num_indels; ++l) add_deletion();
      }
      else {                                                                                      // Insertion
        num_indels = curr_cell.diag - prev_cell.diag;
        add_matches(prev_cell.offset, curr_cell.offset);
        for (int l = 0; l < num_indels; ++l) add_insertion();
      }
    }
  }
  else {                                            // Jump
    add_matches(prev_cell.offset, curr_cell.offset);                    // Add the necessary matches
    _alignment.cigar.path.push_back(prev_cell.vertex_id); // Add the new vertex to the path
    int col_in_prev_v = prev_cell.diag + prev_cell.offset;
    int num_insertions = _graph._vertices[prev_cell.vertex_id].value.size() - col_in_prev_v;
    for (int l = 0; l < num_insertions; ++l) add_insertion();                   // Add the necessary insertions
  }

  curr_cell = prev_cell;
}


// Main function of the backtracking process
void TheseusAlignerImpl::backtrace(int initial_vertex)
{

  Cell curr_pos = _start_pos;
  _alignment.cigar.path.push_back(curr_pos.vertex_id);
  while (curr_pos.vertex_id != initial_vertex)
  {
    one_backtrace_step(curr_pos);
  }

  int remaining_deletions = curr_pos.offset;
  for (int l = 0; l < remaining_deletions; ++l) add_deletion(); // Add the remaining deletions

  std::reverse(_alignment.cigar.edit_op.begin(), _alignment.cigar.edit_op.end());
  std::reverse(_alignment.cigar.path.begin(), _alignment.cigar.path.end());
}

} // namespace theseus