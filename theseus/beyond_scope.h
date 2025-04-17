#pragma once

#include <vector>
#include "cell.h"

/**
 * TODO:
 *
 */

namespace theseus {

class BeyondScope {
public:
    // TODO: Prefer this?
    // using Wavefront = ManualCapacityVector<Cell>;
    // using Jumps = ManualCapacityVector<Cell>;

    /**
     * @brief Construct a new Beyond Scope object
     *
     */
    BeyondScope()
    {
        constexpr int expected_nscores = 1024;
        _sdata.reserve(expected_nscores);
    }

    /**
     * @brief Reinitialize the beyond the scope object each time that a new
     * alignment is called.
     *
     */
    void new_alignment() {
        _sdata.resize(0);
    }


    /**
     * @brief Add a new score to the beyond scope object.
     *
     */
    void new_score() {
        if (_sdata.size() == _sdata.capacity()) {
            _sdata.reserve(_sdata.capacity() * 2);
        }
        _sdata.resize(_sdata.size() + 1);
    }

    /**
     * @brief Access the M wavefront associated to score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &m_wf(int score) {
        return _sdata[score]._m_wf;
    }

    /**
     * @brief Access the M jumps wavefront associated to score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &m_jumps(int score) {
        return _sdata[score]._m_jumps;
    }

    /**
     * @brief Access the I jumps wavefront associated to score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &i_jumps(int score) {
        return _sdata[score]._i_jumps;
    }

    /**
     * @brief Access the I2 jumps wavefront associated to score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &i2_jumps(int score) {
        return _sdata[score]._i2_jumps;
    }

private:
    struct ScoreData {
        std::vector<Cell> _m_wf;

        std::vector<Cell> _m_jumps;

        std::vector<Cell> _i_jumps;
        std::vector<Cell> _i2_jumps;
    };

    std::vector<ScoreData> _sdata;
};

} // namespace theseus