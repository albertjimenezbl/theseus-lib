#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "cell.h"
#include "circular_queue.h"
#include "internal_penalties.h"

/**
 * TODO:
 *
 */

namespace theseus {

class VerticesData { // TODO: Other name?
public:
    struct Segment {
        int32_t start_d;
        int32_t end_d;
    };

    struct InvalidData {
        Segment seg;        // Invalid diagonals
        int32_t rem_up;     // Remaining scores to grow the segment one diagonal up
        int32_t rem_down;   // Remaining scores to grow the segment one diagonal down
    };

    VerticesData(int nvertices) {

    }

    // TODO: Compact the invalid vectors.
    void compact(std::vector<InvalidData> &invalid_v,
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
                invalid_v[k].seg.end_d = std::max(invalid_v[l].seg.end_d, invalid_v[k].seg.end_d);

                // Update remaining down scores
                invalid_v[k].rem_down = std::min(invalid_v[k].rem_down,
                                                 invalid_v[l].rem_down +
                                                     (invalid_v[l].seg.start_d -
                                                      invalid_v[k].seg.start_d) *
                                                         INDEL_EXTEND);

                // Update remaining up scores
                if (invalid_v[l].seg.end_d > invalid_v[k].seg.end_d) {
                    invalid_v[k].rem_up = std::min(invalid_v[l].rem_up,
                                                   invalid_v[k].rem_up +
                                                       (invalid_v[l].seg.end_d -
                                                        invalid_v[k].seg.end_d) *
                                                           INDEL_EXTEND);
                }
                else {
                    invalid_v[k].rem_up = std::min(invalid_v[k].rem_up,
                                                   ordered_v[l].rem_up +
                                                       (invalid_v[k].seg.end_d -
                                                        invalid_v[l].seg.end_d) *
                                                           INDEL_EXTEND);
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
            expand_invalid_vector(vdata._m_invalid, TODO, TODO);
            expand_invalid_vector(vdata._i_invalid, TODO, TODO);
            // TODO:
            // expand_invalid_vector(vdata._i2_invalid, TODO, TODO);
            expand_invalid_vector(vdata._d_invalid, TODO, TODO);
            // TODO:
            // expand_invalid_vector(vdata._d2_invalid, TODO, TODO);
        }
    }
    
    // TODO: Compact the invalid vectors.
    void compact() {
        for (int l = 0; l < _active_vertices.size(); ++l) {
            compact_invalid_vector(_active_vertices[l]._m_invalid, TODO, TODO);
            compact_invalid_vector(_active_vertices[l]._i_invalid, TODO, TODO);
            // TODO:
            // compact_invalid_vector(_active_vertices[l]._i2_invalid, TODO, TODO);
            compact_invalid_vector(_active_vertices[l]._d_invalid, TODO, TODO);
            // TODO:
            // compact_invalid_vector(_active_vertices[l]._d2_invalid, TODO, TODO);
        }
    }


private:
    InternalPenalties &_penalties;

    struct VertexData {
        Cell::vertex_id vertex_id;

        std::vector<InvalidData> _m_invalid;

        std::vector<InvalidData> _i_invalid;
        std::vector<InvalidData> _i2_invalid;

        std::vector<InvalidData> _d_invalid;
        std::vector<InvalidData> _d2_invalid;
    };

    std::vector<VertexData> _active_vertices;

    struct JumpsPos {
        // Scope with the positions of M jumps in the scope previous waves
        std::vector<std::vector<int32_t>> m_jumps_positions;
        // Scope with the positions of I jumps in the scope previous waves
        std::vector<std::vector<int32_t>> i_jumps_positions;
    };

    CircularQueue<JumpsPos> _jumps_pos;
};

} // namespace theseus