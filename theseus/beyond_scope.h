#pragma once

#include <vector>

#include "cell.h"
#include "wavefront_mem_pool.h"
#include "growing_allocator.h"
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
        constexpr int expected_nscores = 1024;
        _sdata.reserve(expected_nscores);
    }

    /**
     * @brief Reinitialize the beyond the scope object each time that a new
     * alignment is called.
     *
     */
    void new_alignment() {
        _sdata.clear();

        _m_wf_mem_pool.clear();
        _m_jumps_mem_pool.clear();
        _i_jumps_mem_pool.clear();
        _i2_jumps_mem_pool.clear();
    }


    /**
     * @brief Add a new score to the beyond scope object.
     *
     */
    void new_score() {
        if (_sdata.size() == _sdata.capacity()) {
            _sdata.reserve(_sdata.capacity() * 2);
        }

        ScoreData sd(&_m_wf_mem_pool,
                     &_m_jumps_mem_pool,
                     &_i_jumps_mem_pool,
                     &_i2_jumps_mem_pool);

        if (_sdata.empty()) {
            // Size of the first wavefronts.
            constexpr int first_wf_size = 10;
            sd._m_wf.reserve(first_wf_size);
            sd._m_jumps.reserve(first_wf_size);
            sd._i_jumps.reserve(first_wf_size);
            sd._i2_jumps.reserve(first_wf_size);
        }
        else {
            ScoreData &prev_sd = _sdata.back();
            // Set the initial size based on the size of the previous wfs.
            constexpr double realloc_factor = 1.5;
            sd._m_wf.reserve(prev_sd._m_jumps.size() * realloc_factor);
            sd._m_jumps.reserve(prev_sd._m_jumps.size() * realloc_factor);
            sd._i_jumps.reserve(prev_sd._m_jumps.size() * realloc_factor);
            sd._i2_jumps.reserve(prev_sd._m_jumps.size() * realloc_factor);
        }

        _sdata.push_back(std::move(sd));

    }

    /**
     * @brief Access the M wavefront associated to score "score".
     *
     * @param score
     * @return Cell::Wavefront&
     */
     Cell::Wavefront &m_wf(int score) {
        return _sdata[score]._m_wf;
    }

    /**
     * @brief Access the M jumps wavefront associated to score "score".
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::Wavefront &m_jumps(int score) {
        return _sdata[score]._m_jumps;
    }

    /**
     * @brief Access the I jumps wavefront associated to score "score".
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::Wavefront &i_jumps(int score) {
        return _sdata[score]._i_jumps;
    }

    /**
     * @brief Access the I2 jumps wavefront associated to score "score".
     *
     * @param score
     * @return Cell::Wavefront&
     */
    Cell::Wavefront &i2_jumps(int score) {
        return _sdata[score]._i2_jumps;
    }

private:
    // Memory pools for faster realloc.
    WavefrontMemPool _m_wf_mem_pool;
    WavefrontMemPool _m_jumps_mem_pool;
    WavefrontMemPool _i_jumps_mem_pool;
    WavefrontMemPool _i2_jumps_mem_pool;

    struct ScoreData {
        Cell::Wavefront _m_wf;
        Cell::Wavefront _m_jumps;
        Cell::Wavefront _i_jumps;
        Cell::Wavefront _i2_jumps;

        ScoreData(WavefrontMemPool *const m_wf_mem_pool,
                  WavefrontMemPool *const m_jumps_mem_pool,
                  WavefrontMemPool *const i_jumps_mem_pool,
                  WavefrontMemPool *const i2_jumps_mem_pool) :
            _m_wf(GrowingAllocator<Cell>{m_wf_mem_pool}),
            _m_jumps(GrowingAllocator<Cell>{m_jumps_mem_pool}),
            _i_jumps(GrowingAllocator<Cell>{i_jumps_mem_pool}),
            _i2_jumps(GrowingAllocator<Cell>{i2_jumps_mem_pool}) {}
    };

    std::vector<ScoreData> _sdata;
};

} // namespace theseus