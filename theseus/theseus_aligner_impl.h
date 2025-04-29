#pragma once

#include <memory>
#include <string>

#include "theseus/alignment.h"
#include "theseus/penalties.h"

#include "graph.h"
#include "beyond_scope.h"
#include "cell.h"
#include "scope.h"
#include "scratchpad.h"
#include "vertices_data.h"
#include "wavefront.h"
#include "internal_penalties.h"
#include "msa.h"

namespace theseus {

class TheseusAlignerImpl {
public:
    TheseusAlignerImpl(const Penalties &penalties,
                       Graph &&graph,
                       bool msa,
                       bool score_only);

    // TODO:
    Alignment align(std::string seq);

private:
    /**
     * @brief Initialize the data for a new alignment.
     *
     * @param start_vtx
     */
    void new_alignment(int start_vtx);

    /**
     * @brief Process a given vertex with a given _score. This means performing
     * the next and extend operations.
     *
     * @param curr_v
     * @param v
     */
    void process_vertex(Graph::vertex *curr_v, int v);

    /**
     * @brief Compute the wave for a given score for all active vertices.
     *
     */
    void compute_new_wave();

    /**
     * @brief Sparsify the M data. This means storing the data in the scratchpad
     * to be later processed.
     *
     * @param curr_v
     * @param dense_wf
     * @param offset_increase
     * @param shift_factor
     * @param start_idx
     * @param end_idx
     * @param m
     * @param upper_bound
     * @param vertex_id
     * @param new_score_diff
     * @param prev_matrix
     */
    void sparsify_M_data(Cell::Wavefront &dense_wf,
                         int offset_increase,
                         int shift_factor,
                         int start_idx,
                         int end_idx,
                         int m,
                         int upper_bound,
                         int new_score_diff);

    /**
     * @brief Sparsify the jumps data. This means storing the data in the scratchpad
     * to be later processed.
     *
     * @param curr_v
     * @param dense_wf
     * @param offset_increase
     * @param shift_factor
     * @param start_idx
     * @param end_idx
     * @param m
     * @param upper_bound
     * @param vertex_id
     * @param new_score_diff
     * @param prev_matrix
     */
    void sparsify_jumps_data(Cell::Wavefront &dense_wf,
                             std::vector<int> &jumps_positions,
                             int offset_increase,
                             int shift_factor,
                             int m,
                             int upper_bound,
                             int new_score_diff,
                             Cell::Matrix prev_matrix);

    /**
     * @brief Sparsify the indel (coming from I or D) data. This means storing
     * the data in the scratchpad to be later processed.
     *
     * @param curr_v
     * @param dense_wf
     * @param offset_increase
     * @param shift_factor
     * @param start_idx
     * @param end_idx
     * @param m
     * @param upper_bound
     * @param vertex_id
     * @param new_score_diff
     * @param prev_matrix
     */
    void sparsify_indel_data(std::vector<Cell> &dense_wf,
                             int offset_increase,
                             int shift_factor,
                             int start_idx,
                             int end_idx,
                             int m,
                             int upper_bound,
                             int added_score_diff);

    /**
     * @brief Compute the next I matrix for a vertex v. This implies both sparsifying
     * the data in the scratchpad and storing it back on the new wavefront, once the
     * corresponding maximums and checks have been done.
     *
     * @param curr_v
     * @param upper_bound // Maximum value of the diagonal
     * @param v
     */
    void next_I(Graph::vertex *curr_v, int upper_bound, int v);


    /**
     * @brief Compute the next D matrix for a vertex v. This implies both sparsifying
     * the data in the scratchpad and storing it back on the new wavefront, once the
     * corresponding maximums and checks have been done.
     *
     * @param curr_v
     * @param upper_bound // Maximum value of the diagonal
     * @param v
     */
    void next_D(int upper_bound, int v);


    /**
     * @brief Compute the next M matrix for a vertex v. This implies both sparsifying
     * the data in the scratchpad and storing it back on the new wavefront, once the
     * corresponding maximums and checks have been done.
     *
     * @param curr_v
     * @param upper_bound // Maximum value of the diagonal
     * @param v
     */
    void next_M(int upper_bound, int v);

    /**
     * @brief Invalidate the diagonal associated to a jump in M, activate the newly
     * discovered vertices and store the jump in the neighbours.
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
                      Cell::Matrix prev_matrix,
                      int _score_diff);

    /**
     * @brief Invalidate the diagonal associated to a jump in I, activate the newly
     * discovered vertices and store the jump in the neighbours.
     *
     * @param curr_v
     * @param prev_cell
     * @param prev_pos
     * @param prev_matrix
     */
    void store_I_jump(Graph::vertex *curr_v,
                      Cell &prev_cell,
                      int prev_pos,
                      Cell::Matrix prev_matrix);

    /**
     * @brief Check and store I jumps (that is, those diagonals that have reached
     * the last column of a vertex for matrix I).
     *
     * @param curr_v
     * @param curr_wavefront
     * @param start_idx
     * @param end_idx
     * @param v
     */
    void check_and_store_jumps(Graph::vertex *curr_v,
                               std::vector<Cell> &curr_wavefront,
                               int start_idx,
                               int end_idx);

    /**
     * @brief Longest Common Prefix of two sequences.
     *
     * @param seq_1
     * @param seq_2
     * @param offset
     * @param j
     */
    void LCP(std::string &seq_1,
             std::string &seq_2,
             int &offset,
             int &j);

    /**
     * @brief Check the end condition for the alignment.
     *
     * @param curr_data
     * @param j
     * @param v
     */
    void check_end_condition(Cell curr_data, int j, int v);

    /**
     * @brief Exyend a given diagonal for a given vertex and perform the necessary
     * jumps.
     *
     * @param curr_v
     * @param curr_cell
     * @param v
     * @param prev_cell
     * @param prev_pos
     * @param prev_matrix
     */
    void extend_diagonal(Graph::vertex *curr_v,
                         Cell &curr_cell,
                         int v,
                         Cell &prev_cell,
                         int prev_pos,
                         Cell::Matrix prev_matrix);

    /**
     * @brief Add matches to our backtracking vector.
     *
     * @param start_matches
     * @param end_matches
     */
    void add_matches(int start_matches, int end_matches);

    /**
     * @brief Add a mismatch to our backtracking vector.
     *
     */
    void add_mismatch();

    /**
     * @brief Add an insertion to our backtracking vector.
     *
     */
    void add_insertion();

    /**
     * @brief Add a deletion to our backtracking vector.
     *
     */
    void add_deletion();

    /**
     * @brief Perform a single step of the backtrace process.
     *
     * @param curr_cell
     * @param curr_v
     */
    void one_backtrace_step(Cell &curr_cell);

    /**
     * @brief Backtrace the alignment from the end vertex to the start vertex.
     *
     * @param initial_vertex
     */
    void backtrace(
        int initial_vertex);

    /**
     * @brief Add a sequence to the graph. This is done by adding the sequence to
     * the graph and then adding the corresponding edges.
     *
     * @param seq Sequence to add
     */
    void add_to_graph(std::string seq);

    int32_t _score = 0;

    Penalties _penalties;

    Graph _graph;   // TODO:

    std::unique_ptr<POAGraph> _poa_graph; // TODO: Add this to theseus?

    bool _is_msa;
    bool _is_score_only;
    bool _end = false;
    int _end_vertex;
    int _seq_ID = 0;
    Cell _start_pos;

    std::unique_ptr<ScratchPad> _scratchpad;   // TODO: Scratchpad inside scope?

    std::unique_ptr<Scope> _scope;
    std::unique_ptr<BeyondScope> _beyond_scope;

    std::unique_ptr<VerticesData> _vertices_data;

    std::string _seq;

    Alignment _alignment;
};

}   // namespace theseus