#pragma once

#include "manual_capacity_vector.h"
#include "cell.h"
#include <vector>
#include "circular_queue.h"

/**
 * TODO:
 *
 */

// TODO: Create cpp.

namespace theseus {

class Scope {
public:
    // TODO: Prefer this?
    // using Wavefront = ManualCapacityVector<Cell>;
    // using Jumps = ManualCapacityVector<Cell>;
    // using JumpsPos = ManualCapacityVector<int32_t>;

    // TODO:
    Scope(int nscores)
        : _squeue(nscores) {

        for (int i = 0; i < nscores; i++) {
            _squeue[i].reserve(1024);
        }
    }

    // TODO:
    void new_alignment() {
        _squeue.reset();
        for (int score = 0; score < _squeue.nscores(); ++score) {
            _squeue[score].resize(0);
        }
    }

    // TODO:
    void new_score() {
        _squeue.new_score();
        _squeue[_squeue.score()].resize(0);
    }

    int size() {
        return _squeue.nscores();
    }

    // TODO:
    std::vector<Cell> &i_wf(int score) {
        return _squeue[score]._i_wf;
    }

    // TODO:
    std::vector<Cell> &d_wf(int score) {
        return _squeue[score]._d_wf;
    }

    // TODO:
    std::vector<Cell> &i2_wf(int score) {
        return _squeue[score]._i2_wf;
    }

    // TODO:
    std::vector<Cell> &d2_wf(int score) {
        return _squeue[score]._d2_wf;
    }

    // TODO:
    std::vector<int32_t> &m_pos(int score) {
        return _squeue[score]._m_pos;
    }

    // TODO:
    std::vector<int32_t> &i_pos(int score) {
        return _squeue[score]._i_pos;
    }

    // TODO:
    std::vector<int32_t> &i2_pos(int score) {
        return _squeue[score]._i2_pos;
    }

    // TODO:
    std::vector<int32_t> &d_pos(int score) {
        return _squeue[score]._d_pos;
    }

    // TODO:
    std::vector<int32_t> &d2_pos(int score) {
        return _squeue[score]._d2_pos;
    }

private:
    struct ScoreData {
        std::vector<Cell> _i_wf;
        std::vector<Cell> _d_wf;

        std::vector<Cell> _i2_wf;
        std::vector<Cell> _d2_wf;

        std::vector<int32_t> _m_pos;

        std::vector<int32_t> _i_pos;
        std::vector<int32_t> _i2_pos;

        std::vector<int32_t> _d_pos;
        std::vector<int32_t> _d2_pos;

        void reserve(int new_capacity) {
            _i_wf.reserve(new_capacity);
            _d_wf.reserve(new_capacity);
            _i2_wf.reserve(new_capacity);
            _d2_wf.reserve(new_capacity);
            _m_pos.reserve(new_capacity);
            _i_pos.reserve(new_capacity);
            _i2_pos.reserve(new_capacity);
            _d_pos.reserve(new_capacity);
            _d2_pos.reserve(new_capacity);
        }

        void resize(int new_size) {
            _i_wf.resize(new_size);
            _d_wf.resize(new_size);
            _i2_wf.resize(new_size);
            _d2_wf.resize(new_size);
            _m_pos.resize(new_size);
            _i_pos.resize(new_size);
            _i2_pos.resize(new_size);
            _d_pos.resize(new_size);
            _d2_pos.resize(new_size);
        }
    };

    ScoreCircularQueue<ScoreData> _squeue;
};

} // namespace theseus