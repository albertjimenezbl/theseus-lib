#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "cell.h"
#include "circular_queue.h"
#include "theseus/penalties.h"

/**
 * TODO:
 *
 */

namespace theseus {

class VerticesData {   // TODO: Other name?
public:
    struct Segment {
        int32_t start_d;
        int32_t end_d;
    };

    struct InvalidData {
        Segment seg;        // Invalid diagonals
        int32_t rem_up;     // Remaining scores to grow the segment one diagonal
                            // up
        int32_t rem_down;   // Remaining scores to grow the segment one diagonal
                            // down
    };

    // TODO:
    VerticesData(Penalties &penalties, int nscores, int nexpected_vertices) :
        _penalties(penalties), _nscores(nscores) {
        _active_vertices.reserve(nexpected_vertices);
        _vertex_to_idx.reserve(nexpected_vertices);
    }

private:
    Penalties &_penalties;
    int _nscores;

    struct VertexData {
        Cell::vertex_t vertex_id;

        std::vector<InvalidData> _m_invalid;

        std::vector<InvalidData> _i_invalid;
        std::vector<InvalidData> _i2_invalid;

        std::vector<InvalidData> _d_invalid;
        std::vector<InvalidData> _d2_invalid;

        // Scope with the positions of M jumps in the scope previous waves
        std::vector<std::vector<int32_t>> _m_jumps_positions;

        // Scope with the positions of I jumps in the scope previous waves
        std::vector<std::vector<int32_t>> _i_jumps_positions;
    };

    std::vector<VertexData> _active_vertices;
    std::vector<Cell::vertex_t> _vertex_to_idx;

    // FUNCTIONS
    // TODO: Compact the invalid vectors.
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
        for (int l = 1; l < invalid_v.size(); ++l) {
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

    void expand_invalid_vector(std::vector<InvalidData> &invalid_v,
                               int default_rem_up,
                               int default_rem_down) {

        for (int l = 0; l < invalid_v.size(); ++l) {
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

    // TODO: Expand the invalid vectors.
    void expand() {
        for (int l = 0; l < _active_vertices.size(); ++l) {
            auto &vdata = _active_vertices[l];
            expand_invalid_vector(vdata._m_invalid, _penalties.gape(), _penalties.gape());
            expand_invalid_vector(vdata._i_invalid, _penalties.gape(), _penalties.gape());
            // TODO:
            // expand_invalid_vector(vdata._i2_invalid, TODO, TODO);
            expand_invalid_vector(vdata._d_invalid, _penalties.gape(), _penalties.gape());
            // TODO:
            // expand_invalid_vector(vdata._d2_invalid, TODO, TODO);
        }
    }

    // TODO: Compact the invalid vectors.
    void compact() {
        for (int l = 0; l < _active_vertices.size(); ++l) {
            compact_invalid_vector(_active_vertices[l]._m_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());
            compact_invalid_vector(_active_vertices[l]._i_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());
            // TODO:
            // compact_invalid_vector(_active_vertices[l]._i2_invalid, TODO,
            // TODO);
            compact_invalid_vector(_active_vertices[l]._d_invalid,
                                   _penalties.gape(),
                                   _penalties.gape());
            // TODO:
            // compact_invalid_vector(_active_vertices[l]._d2_invalid, TODO,
            // TODO);
        }
    }

    void invalidate_i_jump(int idx, int diag) {
        VertexData &vdata = _active_vertices[idx];

        InvalidData new_invalid;

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

    void invalidate_m_jump(int idx, int diag) {
        VertexData &vdata = _active_vertices[idx];

        InvalidData new_invalid;

        new_invalid.rem_down = _penalties.gapo() + _penalties.gape();
        new_invalid.rem_up = _penalties.gapo() + _penalties.gape();
        new_invalid.seg.start_d = diag;
        new_invalid.seg.end_d = diag;

        vdata._m_invalid.push_back(new_invalid);

        // New invalid in I
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

    template <Cell::Matrix matrix>
    bool valid_diagonal(int idx, int diag) {
        VertexData &vdata = _active_vertices[idx];

        std::vector<InvalidData> *invalid = nullptr;

        if constexpr (matrix == Cell::Matrix::M) {
            invalid = &vdata._m_invalid;
        }
        else if constexpr (matrix == Cell::Matrix::I) {
            invalid = &vdata._i_invalid;
        }
        else if constexpr (matrix == Cell::Matrix::D) {
            invalid = &vdata._d_invalid;
        }
        else if constexpr (matrix == Cell::Matrix::I2) {
            invalid = &vdata._i2_invalid;
        }
        else if constexpr (matrix == Cell::Matrix::D2) {
            invalid = &vdata._d2_invalid;
        }
        else {
            static_assert(dependent_false<decltype(matrix)>,
                          "Invalid matrix type");
        }

        for (int l = 0; l < invalid->size(); ++l) {
            if ((*invalid)[l].seg.start_d <= diag &&
                diag <= (*invalid)[l].seg.end_d) {
                return false;
            }
        }
        return true;
    }
};

}   // namespace theseus