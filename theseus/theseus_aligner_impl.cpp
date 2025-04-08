#include "theseus/theseus_aligner_impl.h"

TheseusAlignerImpl::TheseusAlignerImpl(Penalties &penalties,
                                       Graph &graph,
                                       bool msa,
                                       bool score_only) : _orig_penalties(penalties),
                                                          _penalties(penalties),
                                                          _graph(graph),
                                                          _is_msa(msa),
                                                          _is_score_only(score_only) {
    // TODO: Gap-linear and gap-affine.
    const auto nscores = std::max({penalties.gapo() + penalties.ins(),
                                   penalties.gapo() + penalties.del(),
                                   penalties.mismatch()}) + 1;

    _scope = std::make_unique<Scope>(nscores);
    _beyond_scope = std::make_unique<BeyondScope>();
    // constexpr int expected_nvertices = std::max()
    constexpr int expected_nvertices = std::max(1024, /* TODO: */ 0);
    _vertices_data = std::make_unique<VerticesData>(expected_nvertices);

}

TheseusAlignerImpl::Alignment align(std::string_view seq) {
    _scope->new_alignment();
    _beyond_scope->new_alignment();
    _vertices_data->new_alignment();

    // TODO: Set initial conditions

    _score = -1;

    bool end = false;

    // Find the optimal score and an optimal alignment
    while (!end) {
        // Update score
        _score = _score + 1;

        // Clear the corresponding waves and metadata from the scope
        _scope->new_score();
        _beyond_scope->new_score();
        _vertices_data->new_score();

        // Compute the values of the new wave
        if (_score == 0) extend_diagonal(G, data, &G.vertices[0], seq, data.M_jumps[0].wf_data[0], score, 0, end_vertex, end, start_pos, data.M_jumps[0].wf_data[0], 0, 'J');
        compute_new_wave(G, seq, data, start_pos, score, end_vertex, end);
    }

}


// Compute wavefronts
int compute_wavefronts(
    graph &G,
    std::string &seq,
    align_data &data,
    data_cell &start_pos,
    int end_vertex) {

  int score = 0;
  bool end = false;

  // Find the optimal score and an optimal alignment
  while (end == false) {
    // Clear the corresponding waves and metadata from the scope
    int pos_curr_score = score%(INDEL_OPEN + INDEL_EXTEND + 1);
    data.I[pos_curr_score].wf_data.clear();
    data.D[pos_curr_score].wf_data.clear();
    data.D_positions[pos_curr_score].clear();
    data.I_positions[pos_curr_score].clear();
    data.M_positions[pos_curr_score].clear();

    // Clear M_jumps_positions and I_jumps_positions
    for (int l = 0; l < data.active_vertices.size(); ++l) {
      data.vertices_data[l].M_jumps_positions[pos_curr_score].clear();
      data.vertices_data[l].I_jumps_positions[pos_curr_score].clear();
    }

    // Compute the values of the new wave
    if (score == 0) extend_diagonal(G, data, &G.vertices[0], seq, data.M_jumps[0].wf_data[0], score, 0, end_vertex, end, start_pos, data.M_jumps[0].wf_data[0], 0, 'J');
    compute_new_wave(G, seq, data, start_pos, score, end_vertex, end);

    // Update score
    score = score + 1;

    // Create waves of the new score s
    data.M.resize(score + 1);
    data.M_jumps.resize(score + 1);
    data.I_jumps.resize(score + 1);
  }

  return score - 1;
}

// Initialize a the align_data object
void initialize_data(
    align_data &data,
    graph &G,
    GrowingAllocator<int> &growing_alloc,
    int m) {

  // Create the scratch pad
  int max_n = 0, v_n;
  for (int l = 0; l < G.vertices.size(); ++l) {
    v_n = G.vertices[l].value.size();
    max_n = std::max(max_n, v_n);
  }
  data.m = m;
  data.scratch_pad = std::make_unique<ScratchPad>(-m , max_n);

  // Initialize scope of I and D matrices (and their X_positions vectors)
  int score_scope = INDEL_OPEN + INDEL_EXTEND + 1;
  data.I.resize(score_scope);
  data.D.resize(score_scope);
  data.I_positions.resize(score_scope);
  data.D_positions.resize(score_scope);
  data.M_positions.resize(score_scope);

  // Create M and I_jumps waves for score 0
  data.M.resize(1);
  data.M_jumps.resize(1);
  data.I_jumps.resize(1);

  // Initial condition Global Alignment
  data_cell init_condition;
  init_condition.offset = 0;
  init_condition.vertex_id = 0;
  init_condition.diagonal = 0;
  init_condition.score_diff = 0;
  // Initial vertex data
  data.M_jumps[0].wf_data.push_back(init_condition);
  data.active_vertices.push_back(0);
  G.vertices[0].pos = 0;
  data.vertices_data.resize(1);
  data.vertices_data[0].M_jumps_positions.resize(score_scope);
  data.vertices_data[0].I_jumps_positions.resize(score_scope);
  data.vertices_data[0].M_jumps_positions[0].push_back(0);
}
