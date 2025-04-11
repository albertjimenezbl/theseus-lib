#pragma once

#include <memory>
#include <string_view>

#include "theseus/alignment.h"
#include "theseus/penalties.h"
#include "theseus/graph.h"

#include "beyond_scope.h"
#include "cell.h"
#include "scope.h"
#include "scratchpad.h"
#include "vertices_data.h"
#include "wavefront.h"
#include "internal_penalties.h"

namespace theseus {

class TheseusAlignerImpl {
public:
    TheseusAlignerImpl(Penalties penalties, Graph &graph, bool msa, bool score_only);

    // TODO:
    Alignment align(std::string_view seq);

private:
    /**
     * @brief Initialize the data for a new alignment.
     *
     * @param start_vtx
     */
    void new_alignment(int start_vtx);

    void process_vertex(Graph::vertex *curr_v, int v);

    void compute_new_wave();

    void sparsify_M_data(Graph::vertex *curr_v,
                         Wavefront<Cell> &dense_wf,
                         int offset_increase,
                         int shift_factor,
                         int start_idx,
                         int end_idx,
                         int m,
                         int upper_bound,
                         int vertex_id,
                         int new_score_diff);

    void sparsify_jumps_data(Graph::vertex *curr_v,
                             Wavefront<Cell> &dense_wf,
                             std::vector<int> &jumps_positions,
                             int offset_increase,
                             int shift_factor,
                             int m,
                             int upper_bound,
                             int vertex_id,
                             int new_score_diff,
                             char prev_matrix);

    void sparsify_indel_data(Graph::vertex *curr_v,
                             Wavefront<Cell> &dense_wf,
                             int offset_increase,
                             int shift_factor,
                             int start_idx,
                             int end_idx,
                             int m,
                             int upper_bound,
                             int vertex_id,
                             int added_score_diff);

    void next_I(Graph::vertex *curr_v, int upper_bound, int v);

    void next_D(Graph::vertex *curr_v, int upper_bound, int v);

    void next_M(Graph::vertex *curr_v, int upper_bound, int v);

    /**
     * @brief TODO:
     *
     * @param curr_v
     * @param prev_cell
     * @param prev_pos
     * @param prev_matrix
     * @param _score_diff
     */
    void store_M_jump(Graph::vertex *curr_v,
                      Cell &prev_cell,
                      int prev_pos,
                      char prev_matrix,
                      int _score_diff);

    /**
     * @brief TODO:
     *
     * @param curr_v
     * @param prev_cell
     * @param prev_pos
     * @param prev_matrix
     */
    void store_I_jump(Graph::vertex *curr_v,
                      Cell &prev_cell,
                      int prev_pos,
                      char prev_matrix);

    // TODO:
    void check_and_store_jumps(Graph::vertex *curr_v,
                               std::vector<Cell> &curr_wavefront,
                               int start_idx,
                               int end_idx,
                               int v);

    // TODO:
    void LCP(std::string &seq_1,
             std::string &seq_2,
             int len_seq_1,
             int len_seq_2,
             int &offset,   // pointer to the offset (row value) at the current
                            // diagonal
             int &j);

    // TODO:
    void check_end_condition(Cell curr_data, int j, int v);

    // TODO:
    void extend_diagonal(Graph::vertex *curr_v,
                         Cell &curr_cell,
                         int v,
                         Cell &prev_cell,
                         int prev_pos,
                         char prev_matrix);

    // TODO:
    void add_matches(Cigar &back, int start_matches, int end_matches);

    // TODO:
    void add_mismatch(Cigar &back, Cell curr_pos);

    // TODO:
    void add_insertion(Cigar &back, Cell &curr_pos);

    // TODO:
    void add_deletion(Cigar &back);

    // TODO:
    void one_backtrace_step(Cell &curr_cell, Cigar &back, Graph::vertex *curr_v);

    // TODO:
    void backtrace(
    Cigar &back,
    Cell &start_pos,
    int initial_vertex);

    // TODO:
    void add_to_graph(std::string_view seq);
    int32_t _score = 0;

    Penalties _orig_penalties;
    InternalPenalties _penalties;

    Graph _graph;   // TODO:

    bool _is_msa;
    bool _is_score_only;
    bool _end = false;
    int _end_vertex;
    Cell _start_pos;

    std::unique_ptr<ScratchPad> _scratchpad;   // TODO: Scratchpad inside scope?

    std::unique_ptr<Scope> _scope;
    std::unique_ptr<BeyondScope> _beyond_scope;

    std::unique_ptr<VerticesData> _vertices_data;

    std::string_view _seq;
};

}   // namespace theseus