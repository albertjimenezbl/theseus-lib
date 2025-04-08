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

    // TODO:
    BeyondScope() {
        constexpr int expected_nscores = 1024;
        _sdata.reserve(expected_nscores);
    }

    // TODO:
    void new_score() {
        if (_sdata.size() == _sdata.capacity()) {
            _sdata.reserve(_sdata.capacity() * 2);
        }
        _sdata.resize(_sdata.size() + 1);
    }

    // TODO:
    std::vector<Cell> &m_wf(int score) {
        return _sdata[score]._m_wf;
    }

    // TODO:
    std::vector<Cell> &m_jumps(int score) {
        return _sdata[score]._m_jumps;
    }

    // TODO:
    std::vector<Cell> &i_jumps(int score) {
        return _sdata[score]._i_jumps;
    }

    // TODO:
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