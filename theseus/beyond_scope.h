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
    using DenseWavefront = std::vector<Cell>;

    /**
     * @brief Construct a new Beyond Scope object
     *
     */
    BeyondScope() {
        // constexpr int expected_nscores = 1024;
        // _bt_wf.realloc(expected_nscores);

        // auto sdata_realloc_policy = [](std::ptrdiff_t capacity,
        //                                std::ptrdiff_t required_size) -> std::ptrdiff_t {
        //     return required_size * 2;
        // };

        // _sdata.set_realloc_policy(sdata_realloc_policy);
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
    DenseWavefront &i_jumps_wf() {
        return _i_jumps_wf;
    }

    /**
     * @brief Access the m_jumps wavefront
     *
     * @param score
     * @return Cell::Wavefront&
     */
    DenseWavefront &m_jumps_wf() {
        return _m_jumps_wf;
    }

    /**
     * @brief Access the m wavefront
     *
     * @param score
     * @return Cell::Wavefront&
     */
    DenseWavefront &m_wf() {
        return _m_wf;
    }


private:
    // struct ScoreData {
    //     Cell::Wavefront _bt_wf; // Backtrace wavefront

    //     ScoreData(MemPoolWavefront *const bt_wf_mem_pool)
    //         : _bt_wf(MemPoolAllocator<Cell>{bt_wf_mem_pool}) {

    //         auto wf_realloc_policy =
    //             [](std::ptrdiff_t capacity,
    //                std::ptrdiff_t required_size) -> std::ptrdiff_t {
    //             return required_size * 1.5;
    //         };

    //         _bt_wf.set_realloc_policy(wf_realloc_policy);
    //     }
    // };

    DenseWavefront _m_wf; // Backtrace wavefront
    DenseWavefront _m_jumps_wf; // Backtrace wavefront
    DenseWavefront _i_jumps_wf; // Backtrace wavefront
};

} // namespace theseus