#pragma once

#include <cstdint>
#include <type_traits>
#include <typeinfo>

#include "mem_pool_allocator.h"
#include "vector.h"

/**
 * TODO:
 *
 */

namespace theseus {

constexpr ptrdiff_t realloc_wavefront_policy(std::ptrdiff_t capacity,
                                             std::ptrdiff_t required_size)
{
    return required_size * 1.5;
};

// WARNING: We want Cell to be a simple struct so it is standard layout and
// trivial. This way, resizes of Vector<Cell> are free.
struct Cell {
    using CellVector = Vector<Cell, true>;

    using vertex_t = int32_t; // TODO: This should be here?
    using idx2d_t = int32_t;
    using pos_t = int64_t;
    using score_t = int32_t;

    enum class edit_t : int8_t {
        None,
        M,
        Ins,
        Del};

    enum class Matrix : int8_t {
        None,
        M,
        MJumps,
        I,
        IJumps,
        D,
        I2,
        I2Jumps,
        D2
    };

    pos_t prev_pos;
    vertex_t vertex_id;
    idx2d_t offset;
    idx2d_t diag;
    Matrix from_matrix;
};

}   // namespace theseus
