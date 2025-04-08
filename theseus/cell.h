#pragma once

#include <cstdint>
#include <type_traits>
#include <typeinfo>

/**
 * TODO:
 *
 */

namespace theseus {

// WARNING: We want Cell to be a simple struct so it is standard layout and
// trivial. This way, resizes of ManualCapacityVector<Cell> are free.
struct Cell {
    using vertex_t = int32_t; // TODO: This should be here?
    using idx2d_t = int32_t;
    using pos_t = int32_t;
    using score_t = int32_t;

    enum class FromMatrix : int8_t {
        None,
        M,
        MJump,
        IJump,
        I2Jump
    };

    vertex_t vertex_id;
    idx2d_t offset;
    idx2d_t diag;
    pos_t prev_pos;
    score_t score_diff;
    FromMatrix from_matrix;
};

}   // namespace theseus
