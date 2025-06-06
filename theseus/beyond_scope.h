#pragma once

#include <vector>

#include "cell.h"
#include "mem_pool_wavefront.h"
#include "mem_pool_allocator.h"
#include "vector.h"

/**
 * TODO:
 *
 */

namespace theseus {

class BeyondScope {
public:
    /**
     * @brief Construct a new Beyond Scope object
     *
     */
    BeyondScope() {
        constexpr int expected_ncells = 4096;

        _m_wf.realloc(expected_ncells);
        _m_jumps_wf.realloc(expected_ncells);
        _i_jumps_wf.realloc(expected_ncells);
        _i2_jumps_wf.realloc(expected_ncells);

        _m_wf.set_realloc_policy(dense_wf_realloc_policy);
        _m_jumps_wf.set_realloc_policy(dense_wf_realloc_policy);
        _i_jumps_wf.set_realloc_policy(dense_wf_realloc_policy);
        _i2_jumps_wf.set_realloc_policy(dense_wf_realloc_policy);
    }

    /**
     * @brief Reinitialize the beyond the scope object each time that a new
     * alignment is called.
     *
     */
    void new_alignment() {
        _m_wf.clear();
        _m_jumps_wf.clear();
        _i_jumps_wf.clear();
    }

    /**
     * @brief Access the i_jumps wavefront
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::CellVector &i_jumps_wf() {
        return _i_jumps_wf;
    }

    /**
     * @brief Access the m_jumps wavefront
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::CellVector &m_jumps_wf() {
        return _m_jumps_wf;
    }

    /**
     * @brief Access the m wavefront
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::CellVector &m_wf() {
        return _m_wf;
    }


private:
    static constexpr std::ptrdiff_t dense_wf_realloc_policy(std::ptrdiff_t capacity,
                                                            std::ptrdiff_t required_size) {
        return required_size * 2;
    };

    Cell::CellVector _m_wf; // Backtrace wavefront
    Cell::CellVector _m_jumps_wf;  // Backtrace wavefront
    Cell::CellVector _i_jumps_wf;  // Backtrace wavefront
    Cell::CellVector _i2_jumps_wf; // Backtrace wavefront
};

} // namespace theseus