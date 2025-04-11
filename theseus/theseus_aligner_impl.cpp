#include "theseus_aligner_impl.h"

namespace theseus {

TheseusAlignerImpl::TheseusAlignerImpl(Penalties &penalties,
                                       Graph &graph,
                                       bool msa,
                                       bool _score_only) : _orig_penalties(penalties),
                                                          _graph(graph),
                                                          _is_msa(msa),
                                                          _is_score_only(_score_only) {
    // TODO: Gap-linear, gap-affine and dual affine-gap.
    const auto n_scores = std::max({penalties.gapo() + penalties.gape(),
                                   penalties.gapo() + penalties.gape(),
                                   penalties.mism()}) + 1;

    _scope = std::make_unique<Scope>(n_scores);
    _beyond_scope = std::make_unique<BeyondScope>();
    // constexpr int expected_nvertices = std::max()
    constexpr int expected_nvertices = std::max(1024, /* TODO: */ 0);
    _vertices_data = std::make_unique<VerticesData>(expected_nvertices);
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
    _scope->new_score();
    _beyond_scope->new_score();
    _vertices_data->new_score();

    // TODO: Allow for different initial conditions. Now only global alignment.
    // Initial condition Global Alignment
    Cell init_condition;
    init_condition.offset = 0;
    init_condition.vertex_id = 0;
    init_condition.diagonal = 0;
    init_condition._score_diff = 0;
    // Initial vertex data
    _beyond_scope->m_jumps(score).push_back(init_condition);
    _vertices_data->activate_vertex(start_vtx);
    _active_vertices[0]._m_jumps_positions[0].push_back(0);
}


// Process a given vertex with a given _score
void TheseusAlignerImpl::process_vertex(Graph::vertex* curr_v,
                                        int v) {

  // Perform the next operation
  int upper_bound = curr_v->value.size();
  next_I(curr_v, upper_bound, v);
  _scratchpad.scratch_pad->reset();
  next_D(curr_v, upper_bound, v);
  _scratchpad.scratch_pad->reset();
  next_M(curr_v, upper_bound, v);
  _scratchpad.scratch_pad->reset();

  // Perform the extend operations
  int start_idx, end_idx = _scope._squeue[_score]._m_pos[curr_v->pos];
  if (curr_v->pos == 0) start_idx = 0;
  else start_idx = _scope._squeue[_score]._m_pos[curr_v->pos - 1];
  for (int idx = start_idx; idx < end_idx; ++idx) {
    extend_diagonal(curr_v, data.M[_score].wf_data[idx], v, data.M[_score].wf_data[idx], idx, 'M');
  }
}


void TheseusAlignerImpl::compute_new_wave() {

  // Update invalid segments
  _vertices_data->expand();
  _vertices_data->compact();

  // Process all active vertices
  int num_active_vertices = _vertices_data._active_vertices.size(), v;
  for (int l = 0; l < num_active_vertices; ++l) {
    v = _vertices_data._active_vertices[l];
    Graph::vertex* curr_v = &_graph._vertices[v];
    process_vertex(curr_v, v);
  }
}


Alignment TheseusAlignerImpl::align(std::string_view seq) {
    _scope->new_alignment();
    _beyond_scope->new_alignment();
    _vertices_data->new_alignment();

    // TODO: Set initial conditions
    _score = 0;

    bool end = false;
    int end_vertex = ;
    Cell start_pos;

    // Find the optimal _score and an optimal alignment
    while (!end) {
        // Compute the values of the new wave
        if (_score == 0) extend_diagonal(&_graph._vertices[0], data.M_jumps[0].wf_data[0], 0, data.M_jumps[0].wf_data[0], 0, 'J');
        compute_new_wave();

        // Update _score
        _score = _score + 1;

        // Clear the corresponding waves and metadata from the scope
        _scope->new_score();
        _beyond_scope->new_score();
        _vertices_data->new_score();
    }

// Sparsify M data
void TheseusAlignerImpl::sparsify_M_data(Graph::vertex *curr_v,
                    wavefront &dense_wf,
                    int offset_increase,
                    int shift_factor,
                    int start_idx,
                    int end_idx,
                    int m,
                    int upper_bound,
                    int vertex_id,
                    int new_score_diff) {

  int len = end_idx - start_idx, new_col;
  Cell new_cell;

  // Sparsify the active diagonals
  for (int l = 0; l < len; ++l) {
    new_cell = dense_wf.wf_data[start_idx + l];
    new_cell.diagonal += shift_factor;
    new_cell.offset += offset_increase;
    new_cell._score_diff = new_score_diff;
    new_cell.prev_matrix = 'M';
    new_cell.prev_pos = start_idx + l;
    new_col = new_cell.offset + new_cell.diagonal; // d = j - i -> j = d + i

    // Check validity
    if (new_cell.offset <= m && new_col <= upper_bound) {     // If in bounds
      // Branchless push_back
      auto &cell = _scratchpad.access_alloc(new_cell.diagonal);

      // If better offset
      const bool cmp = cell.offset < new_cell.offset;
      cell = (cmp) ? new_cell :
                     cell;
    }
  }
}


// Sparsify jumps data
void TheseusAlignerImpl::sparsify_jumps_data(Graph::vertex *curr_v,
                        wavefront &dense_wf,
                        std::vector<int> &jumps_positions,
                        int offset_increase,
                        int shift_factor,
                        int m,
                        int upper_bound,
                        int vertex_id,
                        int new_score_diff,
                        char prev_matrix) {

  int len = jumps_positions.size(), new_col, pos;
  Cell new_cell;

  // Sparsify the active diagonals
  for (int l = 0; l < len; ++l) {
    pos = jumps_positions[l];
    new_cell = dense_wf.wf_data[pos];
    new_cell.prev_pos = pos;
    new_cell.diagonal += shift_factor;
    new_cell.offset += offset_increase;
    new_cell._score_diff = new_score_diff;
    new_cell.prev_matrix = prev_matrix;
    new_col = new_cell.offset + new_cell.diagonal; // d = j - i -> j = d + i

    // Check validity
    if (new_cell.offset <= m && new_col <= upper_bound) {     // If in bounds
      // Branchless push_back
      auto &cell = _scratchpad.access_alloc(new_cell.diagonal);

      // If better offset
      const bool cmp = cell.offset < new_cell.offset;
      cell = (cmp) ? new_cell :
                     cell;
    }
  }
}


// Sparsify indel
void TheseusAlignerImpl::sparsify_indel_data(Graph::vertex *curr_v,
                        wavefront &dense_wf,
                        int offset_increase,
                        int shift_factor,
                        int start_idx,
                        int end_idx,
                        int m,
                        int upper_bound,
                        int vertex_id,
                        int added_score_diff) {

  int len = end_idx - start_idx, new_col;
  Cell new_cell;

  // Sparsify the active diagonals
  for (int l = 0; l < len; ++l) {
    // Vertex_id and previous matrix are the same as before
    new_cell = dense_wf.wf_data[start_idx + l];
    new_cell.diagonal += shift_factor;
    new_cell.offset += offset_increase;
    new_cell._score_diff += added_score_diff;
    new_col = new_cell.offset + new_cell.diagonal; // d = j - i -> j = d + i

    // Check validity
    if (new_cell.offset <= m && new_col <= upper_bound) {     // If in bounds
      // Branchless push_back
      auto &cell = _scratchpad.access_alloc(new_cell.diagonal);

      // If better offset
      const bool cmp = cell.offset < new_cell.offset;
      cell = (cmp) ? new_cell :
                     cell;
    }
  }
}


// Compute next I matrix
void TheseusAlignerImpl::next_I(Graph::vertex* curr_v,
            int upper_bound,
            int v) {

    // Sparsify data (put it in the scratch pad)
    int pos_prev_M = _score - (_penalties.gapo() + _penalties.ins()), pos_prev_I = _score - penalties.ins(), start_idx, end_idx;

    // Come from an Insertion
    if (pos_prev_I >= 0 && _scope._squeue[pos_prev_I]._i_pos.size() > curr_v->pos)  {
      if (curr_v->pos > 0) start_idx = _scope._squeue[pos_prev_I]._i_pos[curr_v->pos - 1];
      else start_idx = 0;
      end_idx = _scope._squeue[pos_prev_I]._i_pos[curr_v->pos];
      sparsify_indel_data(curr_v, _scope._squeue[pos_prev_I]._i_wf, 0, 1, start_idx, end_idx, _seq.size(), upper_bound, v, _penalties.ins());  // Sparsify I data
      sparsify_jumps_data(curr_v, _beyond_scope[pos_prev_I]._i_jumps, _vertices_data._jumps_pos[curr_v->pos]._i_jumps_positions[pos_prev_I], 0, 1, _seq.size(), upper_bound, v, _penalties.ins(), 'I');
    }

    // Come from M
    if (pos_prev_M >= 0 && _scope._squeue[pos_prev_M]._m_pos.size() > curr_v->pos) {
      if (curr_v->pos > 0) start_idx = data.M_positions[pos_prev_M][curr_v->pos - 1];
      else start_idx = 0;
      end_idx = data.M_positions[pos_prev_M][curr_v->pos];
      sparsify_M_data(curr_v, _beyond_scope[pos_prev_M]._m_wf, 0, 1, start_idx, end_idx,  _seq.size(), upper_bound, v, _penalties.gapo() + _penalties.ins());  // Sparsify M data
      sparsify_jumps_data(curr_v, _beyond_scope[pos_prev_M]._m_jumps, _vertices_data._jumps_pos[curr_v->pos]._m_jumps_positions[pos_prev_M], 0, 1, _seq.size(), upper_bound, v, _penalties.gapo() + _penalties.ins(), 'J');
    }

    // Densify data (store it in the big wavefront)
    int len = data.scratch_pad->nactive_diags(), diag_pos, diag;
    for (auto diag : _scratchpad->active_diags()) {
      _vertices_data->valid_diagonal<Cell::Matrix::I>(curr_v->pos, diag);
      _scope._squeue[_score]._i_wf.push_back((*data.scratch_pad)[diag]);     // Store Cell
    }
    int new_end = _scope._squeue[_score]._i_wf.size();
    _scope._squeue[_score]._i_pos.push_back(new_end);

    // Check, store and invalidate new I jumps
    if (curr_v->pos == 0) start_idx = 0;
    else start_idx = _scope._squeue[_score]._i_pos[curr_v->pos - 1];
    if (curr_v->out_vertices.size() > 0) {
      check_and_store_jumps(curr_v, _scope._squeue[_score]._i_wf, start_idx, new_end, v, end_Graph::vertex, end, start_pos);
    }
}


// Compute next D matrix
void TheseusAlignerImpl::next_D(vertex* curr_v,
            int upper_bound,
            int v) {

    // Sparsify data (put it in the scratch pad)
    int pos_prev_M = _score - (_penalties.gapo() + _penalties.ins()), pos_prev_D = _score - penalties.ins(), start_idx, end_idx;

    // Come from a Deletion
    if (pos_prev_D >= 0 && _scope._squeue[pos_prev_D]._d_pos.size() > curr_v->pos)  {
      if (curr_v->pos > 0) start_idx = _scope._squeue[pos_prev_D]._d_pos[curr_v->pos - 1];
      else start_idx = 0;
      end_idx = _scope._squeue[pos_prev_D]._d_pos[curr_v->pos];
      sparsify_indel_data(curr_v, _scope._squeue[pos_prev_D]._d_wf, 1, -1, start_idx, end_idx, _seq.size(), upper_bound, v, _penalties.del());  // Sparsify D data
    }

    // Come from M
    if (pos_prev_M >= 0 && _scope._squeue[pos_prev_M]._m_pos.size() > curr_v->pos) {
      if (curr_v->pos > 0) start_idx = data.M_positions[pos_prev_M][curr_v->pos - 1];
      else start_idx = 0;
      end_idx = data.M_positions[pos_prev_M][curr_v->pos];
      sparsify_M_data(curr_v, _beyond_scope[pos_prev_M]._m_wf, 1, -1, start_idx, end_idx,  _seq.size(), upper_bound, v, _penalties.gapo() + _penalties.del());  // Sparsify M data
      sparsify_jumps_data(curr_v, _beyond_scope[pos_prev_M]._m_jumps, _vertices_data._jumps_pos[curr_v->pos]._m_jumps_positions[pos_prev_M], 1, -1, _seq.size(), upper_bound, v, _penalties.gapo() + _penalties.del(), 'J');
    }

    // Densify data (store it in the big wavefront)
    int len = data.scratch_pad->nactive_diags(), diag_pos, diag;
    for (auto diag : _scratchpad->active_diags()) {
      _vertices_data->valid_diagonal<Cell::Matrix::D>(curr_v->pos, diag);
      _scope._squeue[_score]._d_wf.push_back((*data.scratch_pad)[diag]);     // Store Cell
    }
    int new_end = _scope._squeue[_score]._d_wf.size();
    _scope._squeue[_score]._d_pos.push_back(new_end);
}


// Compute next M matrix
void TheseusAlignerImpl::next_M(Graph::vertex* curr_v,
                                int upper_bound,
                                int v) {

  // Sparsify data (put it in the scratch pad)
  int pos_prev_M = _score - _penalties.subs(), pos_prev_D = _score, pos_prev_I = _score, start_idx, end_idx;

  // Come from a Deletion
  if (_scope._squeue[pos_prev_D]._d_pos.size() > curr_v->pos)  {
    if (curr_v->pos > 0) start_idx = _scope._squeue[pos_prev_D]._d_pos[curr_v->pos - 1];
    else start_idx = 0;
    end_idx = _scope._squeue[pos_prev_D]._d_pos[curr_v->pos];
    sparsify_indel_data(curr_v, _scope._squeue[pos_prev_D]._d_wf, 0, 0, start_idx, end_idx, _seq.size(), upper_bound, v, 0);  // Sparsify D data
  }

  // Come from an Insertion
  if (_scope._squeue[pos_prev_I]._i_pos.size() > curr_v->pos)  {
    if (curr_v->pos > 0) start_idx = _scope._squeue[pos_prev_I]._i_pos[curr_v->pos - 1];
    else start_idx = 0;
    end_idx = _scope._squeue[pos_prev_I]._i_pos[curr_v->pos];
    sparsify_indel_data(curr_v, _scope._squeue[pos_prev_I]._d_wf, 0, 0, start_idx, end_idx, _seq.size(), upper_bound, v, 0);  // Sparsify I data
  }

  // Come from M
  if (pos_prev_M >= 0 && _scope._squeue[pos_prev_M]._m_pos.size() > curr_v->pos) {
    if (curr_v->pos > 0) start_idx = data.M_positions[pos_prev_M][curr_v->pos - 1];
    else start_idx = 0;
    end_idx = data.M_positions[pos_prev_M][curr_v->pos];
    sparsify_M_data(curr_v, _beyond_scope[pos_prev_M]._m_wf, 1, 0, start_idx, end_idx,  _seq.size(), upper_bound, v, _penalties.subs());  // Sparsify M data
    sparsify_jumps_data(curr_v, _beyond_scope[pos_prev_M]._m_jumps, _vertices_data._jumps_pos[curr_v->pos]._m_jumps_positions[pos_prev_M], 1, 0, _seq.size(), upper_bound, v, _penalties.subs(), 'J');
  }

  // Densify data (store it in the big wavefront)
  int len = data.scratch_pad->nactive_diags(), diag_pos, diag;
  for (auto diag : _scratchpad->active_diags()) {
    _vertices_data->valid_diagonal<Cell::Matrix::M>(curr_v->pos, diag);
    _beyond_scope[_score]._m_wf.push_back((*data.scratch_pad)[diag]);     // Store Cell
  }
  int new_end =_beyond_scope[_score]._m_wf.size();
  _scope._squeue[_score]._m_pos.push_back(new_end);
}



// Store the jump in neighbours
void TheseusAlignerImpl::store_M_jump(Graph::vertex* curr_v,
                                      Cell &prev_cell,
                                      int prev_pos,
                                      char prev_matrix,
                                      int _score_diff) {

  // Invalidate the jumping diagonal
  invalidate_M_jump(prev_cell.diagonal, data.vertices_data[curr_v->pos]);

  int num_out_v = curr_v->out_vertices.size();
  Cell new_cell = prev_cell;
  new_cell.diagonal = -new_cell.offset;
  new_cell.prev_matrix = prev_matrix;
  new_cell.prev_pos = prev_pos;
  new_cell._score_diff = _score_diff;

  for (int l = 0; l < num_out_v; ++l) {
    new_cell.vertex_id = curr_v->out_vertices[l];
    Graph::vertex* new_v = &_graph._vertices[curr_v->out_vertices[l]];
    activate_vertex(new_cell.vertex_id);

    // Store jump and metadata
    bool valid_diag = _vertices_data->valid_diagonal<Cell::Matrix::M>(new_cell.vertex_id, new_cell.diagonal); // Extend only if it has not yet been visited
    if (valid_diag) { // Extend only if it has not yet been visited
      int pos_new_cell = _beyond_scope[_score]._m_jumps[_score].size();
      _beyond_scope[_score]._m_jumps.push_back(new_cell);
      _vertices_data[new_cell.vertex_id]._m_jumps_positions[_score%_squeue.size()].push_back(pos_new_cell);
      extend_diagonal(new_v, data.M_jumps[_score].wf_data[pos_new_cell], new_cell.vertex_id, data.M_jumps[_score].wf_data[pos_new_cell], pos_new_cell, 'J');
    }
  }
}


// Store the jump in neighbours
void TheseusAlignerImpl::store_I_jump(Graph::vertex* curr_v,
                                      Cell& prev_cell,
                                      int prev_pos,
                                      char prev_matrix) {

  // Invalidate the jumping diagonal
  invalidate_I_jump(prev_cell.diagonal, data.vertices_data[curr_v->pos]);

  wavefront new_wave;
  int len = curr_v->out_vertices.size();
  Cell new_cell = prev_cell;
  new_cell.diagonal = -new_cell.offset;
  new_cell.prev_matrix = prev_matrix;
  new_cell.prev_pos = prev_pos;
  for (int l = 0; l < len; ++l) {
    new_cell.vertex_id = curr_v->out_vertices[l];
    Graph::vertex* new_v = &_graph._vertices[curr_v->out_vertices[l]];
    activate_vertex(new_cell.vertex_id);

    // Store jump and metadata
    bool valid_diag = _vertices_data->valid_diagonal<Cell::Matrix::I>(new_cell.vertex_id, new_cell.diagonal);
    if (valid_diag) { // Extend only if it has not yet been visited
      int pos_new_cell = _beyond_scope[_score]._m_jumps[_score].size();
      _beyond_scope[_score]._i_jumps.push_back(new_cell);
      _vertices_data[new_cell.vertex_id]._i_jumps_positions[_score%_squeue.size()].push_back(pos_new_cell);
    }
  }
}


// Check and store I jumps (that is, those diagonals that have reached the last column of a vertex)
void TheseusAlignerImpl::check_and_store_jumps(Graph::vertex* curr_v,
                          std::vector<Cell> &curr_wavefront,
                          int start_idx,
                          int end_idx,
                          int v) {

  int len = end_idx - start_idx, diag, offset, curr_j, n = curr_v->value.size(), prev_pos;
  char prev_matrix;

  for (int l = 0; l < len; ++l) {
    diag = curr_wavefront[start_idx + l].diagonal;
    offset = curr_wavefront[start_idx + l].offset;
    curr_j = diag + offset;
    if (curr_j == n && offset <= data.m) {
      prev_matrix = curr_wavefront.[start_idx + l].prev_matrix;
      prev_pos = curr_wavefront.[start_idx + l].prev_pos;
      store_M_jump(curr_v, curr_wavefront.[start_idx + l], prev_pos, prev_matrix, curr_wavefront[start_idx + l]._score_diff);
      store_I_jump(curr_v, curr_wavefront.[start_idx + l], prev_pos, prev_matrix);
    }
  }
}


// Compute the Longest Common Prefix between two given sequences
void TheseusAlignerImpl::LCP(std::string &seq_1,
                             std::string &seq_2,
                             int len_seq_1,
                             int len_seq_2,
                             int &offset,   // pointer to the offset (row value)
                                            // at the current diagonal
                             int &j) {

    // Find LCP
    while (offset < len_seq_1 && j < len_seq_2 && seq_1[offset] == seq_2[j]) {
        offset = offset + 1;   // Update the f.r. of this diagonal
        j = j + 1;
    }
}

// TODO: Implement different end conditions as Global, Semi-Global...
void TheseusAlignerImpl::check_end_condition(Cell curr_data, // Offset and prev_index
                        int j,
                        int v) {

  int j_end = 0; // The last node is empty
  if (curr_data.offset == data.m && v == end_vertex && j == j_end) { // End condition global alignment
    end = true;
    start_pos = curr_data;
  }
}


// Extend a particular diagonal
void TheseusAlignerImpl::extend_diagonal(
    Graph::vertex *curr_v,
    Cell &curr_cell,
    int v,
    Cell &prev_cell,
    int prev_pos,
    char prev_matrix) {

  int n = curr_v->value.size();

  // Longest Common prefix
  int diag = curr_cell.diagonal;
  int j = diag + curr_cell.offset;
  LCP(seq, curr_v->value, data.m, n, curr_cell.offset, j); // Find Longest Common Prefix

  // End condition
  check_end_condition(data, curr_cell, j, v); // Check end condition

  // Check jump
  if (j == n && curr_cell.offset <= data.m && curr_v->out_vertices.size() > 0) {
    store_M_jump(prev_cell, prev_pos, prev_matrix, 0);
  }
}


// Add matches to our backtracking vector
void TheseusAlignerImpl::add_matches(
    Cigar &back,
    int start_matches,
    int end_matches) {

  int size = end_matches - start_matches;
  for (int k = 0; k < size; ++k) {
    char character_to_add_seq = _seq[end_matches - k - 1];
    back.edit_op.push_back('M');
    back.rec_seq.push_back(character_to_add_seq);
  }
}


// Add a mismatch to our backtracking vector
void TheseusAlignerImpl::add_mismatch(
    Cigar &back,
    Cell curr_pos) {

  int j = curr_pos.offset + curr_pos.diagonal - 1;
  char character_to_add = _graph._vertices[curr_pos.vertex_id].value[j];     // Suposem que la posició
  back.edit_op.push_back('X');                                         // està ben guardada
  back.rec_seq.push_back(character_to_add);
}


// Add an insertion to our backtracking vector
void TheseusAlignerImpl::add_insertion(
    Cigar &back,
    Cell &curr_pos) {

  int j = curr_pos.offset + curr_pos.diagonal - 1;
  char character_to_add = _graph._vertices[curr_pos.vertex_id].value[j];
  back.edit_op.push_back('I');
  back.rec_seq.push_back(character_to_add);
}


// Add a deletion to our backtracking vector
void TheseusAlignerImpl::add_deletion(Cigar &back) {

  back.edit_op.push_back('D');
}


void TheseusAlignerImpl::one_backtrace_step(
    Cell &curr_cell,
    Cigar &back,
    Graph::vertex *curr_v) {

  // Get previous cell
  _score -= curr_cell.score_diff;
  Cell prev_cell;
  if (curr_cell.prev_matrix == 'M') prev_cell = data.M[score].wf_data[curr_cell.prev_pos];
  else if (curr_cell.prev_matrix == 'J') prev_cell = data.M_jumps[score].wf_data[curr_cell.prev_pos];
  else prev_cell = data.I_jumps[score].wf_data[curr_cell.prev_pos];

  // Inside the same vertex or jump
  int num_indels;
  if (curr_cell.vertex_id == prev_cell.vertex_id) { // Still in the same vertex
    if (curr_cell.diagonal == prev_cell.diagonal) {                                               // Mismatch
      if (curr_cell.offset > prev_cell.offset) {    // Consider 0 length vertices
        add_matches(back, seq, curr_v->value, prev_cell.offset + 1, curr_cell.offset);
        add_mismatch(G, back, curr_v->value, curr_cell);
      }
    }
    else { // Indel
      if (curr_cell.diagonal < prev_cell.diagonal) {                                              // Deletion
        num_indels = prev_cell.diagonal - curr_cell.diagonal;
        add_matches(back, seq, curr_v->value, prev_cell.offset + num_indels, curr_cell.offset);
        for (int l = 0; l < num_indels; ++l) add_deletion(back);
      }
      else {                                                                                      // Insertion
        num_indels = curr_cell.diagonal - prev_cell.diagonal;
        add_matches(back, seq, curr_v->value, prev_cell.offset, curr_cell.offset);
        for (int l = 0; l < num_indels; ++l) add_insertion(G, back, curr_cell);
      }
    }
  }
  else {                                            // Jump
    add_matches(back, seq, curr_v->value, prev_cell.offset, curr_cell.offset);                    // Add the necessary matches
    back.path.push_back(prev_cell.vertex_id); // Add the new vertex to the path
    int col_in_prev_v = prev_cell.diagonal + prev_cell.offset;
    int num_insertions = _graph._vertices[prev_cell.vertex_id].value.size() - col_in_prev_v;
    for (int l = 0; l < num_insertions; ++l) add_insertion(G, back, curr_cell);                   // Add the necessary insertions
  }

  curr_cell = prev_cell;
}


// Main function of the backtracking process
void TheseusAlignerImpl::backtrace(Cigar &back, Cell &start_pos, int initial_vertex) {

    Cell curr_pos = start_pos;
    Graph::vertex *curr_v = &_graph._vertices[start_pos.vertex_id];
    back.path.push_back(curr_pos.vertex_id);   // Guardo l'últim node?
    while (curr_pos.vertex_id != initial_vertex) {
        curr_v = &_graph._vertices[curr_pos.vertex_id];
        one_backtrace_step(curr_pos, back, curr_v);
    }

    int remaining_deletions = curr_pos.offset;
    for (int l = 0; l < remaining_deletions; ++l) {   // If the alignment
                                                      // started with some
                                                      // deletions, add them
        back.edit_op.push_back('D');
    }

    std::reverse(back.edit_op.begin(), back.edit_op.end());
    std::reverse(back.rec_seq.begin(), back.rec_seq.end());
    std::reverse(back.path.begin(), back.path.end());
}

} // namespace theseus