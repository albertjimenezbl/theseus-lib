/*
 *                             The MIT License
 *
 * Copyright (c) 2024 by Albert Jimenez-Blanco
 *
 * This file is part of #################### Theseus Library ####################.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>
// #include <type_traits>

#include "cell.h"
#include "theseus/graph.h"
#include "internal_penalties.h"


namespace theseus {

using NodeId = Graph::NodeId;

/**
 * @brief Class vertices data. It stores the data related to the active vertices,
 * their indexes, the invalid diagonals and some jumping data.
 *
 */
class VerticesData {
public:
    using pos_t = int64_t;

    struct Segment {
        int32_t start_d;   // Start diagonal of the segment (inclusive)
        int32_t end_d;     // End diagonal of the segment (inclusive)
    };

    struct InvalidData {
        Segment seg;        // Segment of invalid diagonals
        int32_t rem_up;     // Remaining scores to grow the segment one diagonal
                            // up
        int32_t rem_down;   // Remaining scores to grow the segment one diagonal
                            // down
    };


    /**
     * @brief Vertex dat structure. It contains:
     * - Invalid data for the M, I (I2) and D (D2) matrices.
     * - Positions of the jumps in the MJ and IJ (I2) matrices.
     */
    struct VertexData {
        NodeId vertex_id;

        std::vector<InvalidData> _m_invalid;

        std::vector<InvalidData> _i_invalid;
        // std::vector<InvalidData> _i2_invalid;

        std::vector<InvalidData> _d_invalid;
        // std::vector<InvalidData> _d2_invalid;

        // Scope with the positions of M jumps in the scope previous waves
        std::vector<std::vector<pos_t>> _m_jumps_positions;

        // Scope with the positions of I jumps in the scope previous waves
        std::vector<std::vector<pos_t>> _i_jumps_positions;

        // Scope with the positions of I2s jumps in the scope previous waves
        // std::vector<std::vector<pos_t>> _i2_jumps_positions;
    };

    int _nscores;

    /**
     * @brief Construct a new Vertices Data object
     *
     * @param penalties Set of used penalties.
     * @param nscores   Number of scores of the scope.
     * @param nexpected_vertices    Number of expected vertices.
     */
    VerticesData(const InternalPenalties &penalties, int nscores, int nexpected_vertices) :
        _nscores(nscores), _penalties(penalties) {
        _active_vertices.reserve(nexpected_vertices);
        _vertex_to_idx.reserve(nexpected_vertices);
    }

    /**
     * @brief Clear the previously stored values in the scope to write the
     * newly relevant data.
     *
     * @param score  Current score
     */
    void new_score(int score) {
        int len = _active_vertices.size();
        int pos_curr_score = get_pos(score);

        for (int l = 0; l < len; ++l) {
            auto &vdata = _active_vertices[l];

            // Clear the jumps (they work as a scope)
            vdata._m_jumps_positions[pos_curr_score].clear();
            vdata._i_jumps_positions[pos_curr_score].clear();
        }
    }

    /**
     * @brief Reinitialize the vertices data object each time that a new
     * alignment is called.
     *
     */
    void new_alignment() {
        _active_vertices.clear();
        _vertex_to_idx.clear();
    }

    /**
     * @brief Get the id of a given vertex.
     *
     * @param vtx   Vertex id
     * @return Cell::vertex_t  Index of the vertex in the active vertices
     */
    int get_id(int vtx) {
        return _vertex_to_idx[vtx];
    }

    /**
     * @brief Get the vertex id of a given vertex.
     *
     * @param idx   Index of the vertex in the active vertices
     * @return Cell::vertex_t  Vertex id
     */
    Cell::vertex_t get_vertex_id(int idx) {
        return _active_vertices[idx].vertex_id;
    }

    /**
     * @brief Get the vertex data of a given vertex.
     *
     * @param vtx  Vertex id
     * @return VertexData&  Data of the vertex
     */
    VertexData &get_vertex_data(int vtx) {
        return _active_vertices[_vertex_to_idx[vtx]];
    }

    /**
     * @brief Get the position in the scope of a given score.
     *
     * @param score
     * @return int  Position in the scope (it ranges [0, _nscores-1])
     */
    int get_pos(int score) {
        return score%_nscores;
    }

    /**
     * @brief Get the number of scores.
     *
     * @return int Number of scores
     */
    int get_n_scores() {
        return _nscores;
    }

    // FUNCTIONS
    /**
     * @brief Compact a set of ordered invalid objects to avoid redundant information.
     *
     */
    void compact_invalid_vector(std::vector<InvalidData> &invalid_v,
                                int default_rem_up,
                                int default_rem_down) {

        if (invalid_v.size() == 0) {
            return;
        }

        // Order the set of segments
        std::sort(invalid_v.begin(),
                  invalid_v.end(),
                  [](const InvalidData &s1, const InvalidData &s2) {
                      return s1.seg.start_d < s2.seg.start_d;
                  });

        // Iterate through the loop
        int k = 0;
        for (long unsigned int l = 1; l < invalid_v.size(); ++l) {
            // You should compact them
            if (invalid_v[k].seg.end_d + 1 >= invalid_v[l].seg.start_d) {
                // Update segment
                invalid_v[k].seg.end_d = std::max(invalid_v[l].seg.end_d,
                                                  invalid_v[k].seg.end_d);

                // Update remaining down scores
                invalid_v[k].rem_down = std::min(invalid_v[k].rem_down,
                                                 invalid_v[l].rem_down +
                                                     (invalid_v[l].seg.start_d -
                                                      invalid_v[k].seg.start_d) *
                                                         default_rem_down);

                // Update remaining up scores
                if (invalid_v[l].seg.end_d > invalid_v[k].seg.end_d) {
                    invalid_v[k].rem_up = std::min(invalid_v[l].rem_up,
                                                   invalid_v[k].rem_up +
                                                       (invalid_v[l].seg.end_d -
                                                        invalid_v[k].seg.end_d) *
                                                           default_rem_up);
                }
                else {
                    invalid_v[k].rem_up = std::min(invalid_v[k].rem_up,
                                                   invalid_v[l].rem_up +
                                                       (invalid_v[k].seg.end_d -
                                                        invalid_v[l].seg.end_d) *
                                                           default_rem_up);
                }
            }
            else {
                k += 1;
                invalid_v[k] = invalid_v[l];
            }
        }
        invalid_v.resize(k + 1);
    }

    /**
     * @brief Expand a set of invalid objects. This means reducing the counters
     * of remaining scores to grow and growing the segments if those counters get
     * to 0.
     *
     */
    void expand_invalid_vector(std::vector<InvalidData> &invalid_v,
                               int default_rem_up,
                               int default_rem_down) {

        for (long unsigned int l = 0; l < invalid_v.size(); ++l) {
            invalid_v[l].rem_down -= 1;
            invalid_v[l].rem_up -= 1;

            if (invalid_v[l].rem_up == 0) {
                invalid_v[l].rem_up = default_rem_up;
                invalid_v[l].seg.end_d += 1;
            }
            if (invalid_v[l].rem_down == 0) {
                invalid_v[l].rem_down = default_rem_down;
                invalid_v[l].seg.start_d -= 1;
            }
        }
    }

    /**
     * @brief Expand all invalid objects.
     *
     */
    void expand() {
        for (long unsigned int l = 0; l < _active_vertices.size(); ++l) {
            auto &vdata = _active_vertices[l];
            expand_invalid_vector(vdata._m_invalid, _penalties.gape(), _penalties.gape());
            expand_invalid_vector(vdata._i_invalid, _penalties.gape(), _penalties.gape());
            expand_invalid_vector(vdata._d_invalid, _penalties.gape(), _penalties.gape());

            // expand_invalid_vector(vdata._i2_invalid, TODO, TODO);
            // expand_invalid_vector(vdata._d2_invalid, TODO, TODO);
        }
    }

    /**
     * @brief Compact all invalid objects.
     *
     */
    void compact() {
        for (long unsigned int l = 0; l < _active_vertices.size(); ++l) {
            compact_invalid_vector(_active_vertices[l]._m_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());
            compact_invalid_vector(_active_vertices[l]._i_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());
            compact_invalid_vector(_active_vertices[l]._d_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());

            // compact_invalid_vector(_active_vertices[l]._i2_invalid, TODO,
            // TODO);
            // compact_invalid_vector(_active_vertices[l]._d2_invalid, TODO,
            // TODO);
        }
    }

    /**
     * @brief Invalidate a diagonal "diag" in vertex located at index "idx" and
     * for matrix I. This happens when a jump is performed in the I matrix.
     *
     * @param idx
     * @param diag
     */
    void invalidate_i_jump(int idx, int diag) {
        VertexData &vdata = _active_vertices[idx];
        InvalidData new_invalid;

        // New invalid in M
        new_invalid.rem_down = _penalties.gapo() + _penalties.gape();
        new_invalid.rem_up = _penalties.gape();
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag;
        vdata._m_invalid.push_back(new_invalid);

        // New invalid in I
        new_invalid.rem_down = 2 * _penalties.gapo() + 3 * _penalties.gape();
        new_invalid.rem_up = _penalties.gape();
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag;
        vdata._i_invalid.push_back(new_invalid);

        // New invalid in D (initially empty)
        new_invalid.rem_down = _penalties.gapo() + _penalties.gape();
        new_invalid.rem_up = _penalties.gapo() + 2 * _penalties.gape();
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag - 1;
        vdata._d_invalid.push_back(new_invalid);
    }

    /**
     * @brief Invalidate a diagonal "diag" in vertex located at index "idx" and
     * for matrix M. This happens when a jump is performed in the M matrix.
     *
     * @param idx
     * @param diag
     */
    void invalidate_m_jump(int idx, int diag) {
        VertexData &vdata = _active_vertices[idx];
        InvalidData new_invalid;

        // New invalid in M
        new_invalid.rem_down = _penalties.gapo() + _penalties.gape();
        new_invalid.rem_up = _penalties.gapo() + _penalties.gape();
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag;
        vdata._m_invalid.push_back(new_invalid);

        // New invalid in I (initially empty)
        new_invalid.rem_down = 2 * (_penalties.gapo() + _penalties.gape());
        new_invalid.rem_up = _penalties.gapo() + _penalties.gape();
        new_invalid.seg.start_d = diag + 1;
        new_invalid.seg.end_d = diag;
        vdata._i_invalid.push_back(new_invalid);

        // New invalid in D (initially empty)
        new_invalid.rem_down = _penalties.gapo() + _penalties.gape();
        new_invalid.rem_up = 2 * (_penalties.gapo() + _penalties.gape());
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag - 1;
        vdata._d_invalid.push_back(new_invalid);
    }

    /**
     * @brief Check if a given diagonal "diag" is valid in the vertex located at
     * position "idx". This function allows to validate the M, I and D matrices.
     *
     * @tparam matrix
     * @param idx
     * @param diag
     * @return bool
     */
    template <Cell::Matrix matrix>
    bool valid_diagonal(int vtx, int diag) {
        std::vector<InvalidData> &invalid =
        [this, vtx]() -> std::vector<InvalidData>& {
            VertexData &vdata = _active_vertices[get_id(vtx)];
            if constexpr (matrix == Cell::Matrix::M) {
                return vdata._m_invalid;
            }
            else if constexpr (matrix == Cell::Matrix::I) {
                return vdata._i_invalid;
            }
            else if constexpr (matrix == Cell::Matrix::D) {
                return vdata._d_invalid;
            }
            // else if constexpr (matrix == Cell::Matrix::I2) {
            //     return vdata._i2_invalid;
            // }
            // else if constexpr (matrix == Cell::Matrix::D2) {
            //     return vdata._d2_invalid;
            // }
            else {
                static_assert([]{ return false; }(), "Unsupported matrix type");
            }
        }();

        for (size_t l = 0; l < invalid.size(); ++l) {
            if (invalid[l].seg.start_d <= diag &&
                diag <= invalid[l].seg.end_d) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Return the number of active vertices.
     *
     * @return int
     */
    size_t num_active_vertices() {
        return _active_vertices.size();
    }

    /**
     * @brief Activate a new vertex "vtx" in the active vertices list. Nothing is
     * done if the vertex is already active.
     *
     * @param vtx
     */
    void activate_vertex(NodeId vtx) {
        if (_vertex_to_idx.size() <= vtx) {
            _vertex_to_idx.resize(2*vtx + 1, -1);
        }
        if (_vertex_to_idx[vtx] == -1) {
            // Add the vertex to the active vertices
            _active_vertices.push_back(VertexData());
            _active_vertices[_active_vertices.size() - 1].vertex_id = vtx;
            _active_vertices[_active_vertices.size() - 1]._i_jumps_positions.resize(_nscores);
            _active_vertices[_active_vertices.size() - 1]._m_jumps_positions.resize(_nscores);

            // Determine the vertex id
            _vertex_to_idx[vtx] = _active_vertices.size() - 1;
        }
    }

private:
    const InternalPenalties &_penalties;

    std::vector<VertexData> _active_vertices;

    std::vector<int> _vertex_to_idx;
};

}   // namespace theseus