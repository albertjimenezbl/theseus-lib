#pragma once

#include "cell.h"
#include <vector>

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

    /**
     * @brief Struct representing a range of values
     *
     */
    struct range {
        int32_t start;
        int32_t end;
    };


    /**
     * @brief Construct a new Scope object
     *
     * @param nscores Number of scores in the scope
     */
    Scope(int nscores)
        : _squeue(nscores) {

        for (int i = 0; i < nscores; i++) {
            _squeue[i].reserve(1024);
        }
    }

    /**
     * @brief Restore data for a new alignment.
     *
     */
    void new_alignment() {
        for (int i = 0; i < _squeue.size(); ++i) {
            _squeue[i].resize(0);
        }
    }

    /**
     * @brief Reinitialize data for a new score. As the scope works as a circular
     * queue, the previously stored data in a position of the scope is not relevant
     * anymore and its contents should be reset to be reused again.
     *
     */
    void new_score(int score) {
        _squeue[score%_squeue.size()].resize(0);
    }

    /**
     * @brief Get the size of the scope.
     *
     * @return int
     */
    int size() {
        return _squeue.size();
    }


    /**
     * @brief Get the data from the wavefront I of score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &i_wf(int score) {
        return _squeue[score%_squeue.size()]._i_wf;
    }

    /**
     * @brief Get the data from the wavefront D of score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &d_wf(int score) {
        return _squeue[score%_squeue.size()]._d_wf;
    }

    /**
     * @brief Get the data from the wavefront I2 of score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &i2_wf(int score) {
        return _squeue[score%_squeue.size()]._i2_wf;
    }

    /**
     * @brief Get the data from the wavefront D2 of score "score".
     *
     * @param score
     * @return std::vector<Cell>&
     */
    std::vector<Cell> &d2_wf(int score) {
        return _squeue[score%_squeue.size()]._d2_wf;
    }

    /**
     * @brief Get the data from the vector of M positions at score "score".
     *
     * @param score
     * @return std::vector<range>&
     */
    std::vector<range> &m_pos(int score) {
        return _squeue[score%_squeue.size()]._m_pos;
    }

    /**
     * @brief Get the data from the vector of I positions at score "score".
     *
     * @param score
     * @return std::vector<range>&
     */
    std::vector<range> &i_pos(int score) {
        return _squeue[score%_squeue.size()]._i_pos;
    }

    /**
     * @brief Get the data from the vector of I2 positions at score "score".
     *
     * @param score
     * @return std::vector<range>&
     */
    std::vector<range> &i2_pos(int score) {
        return _squeue[score%_squeue.size()]._i2_pos;
    }

    /**
     * @brief Get the data from the vector of D positions at score "score".
     *
     * @param score
     * @return std::vector<range>&
     */
    std::vector<range> &d_pos(int score) {
        return _squeue[score%_squeue.size()]._d_pos;
    }

    /**
     * @brief Get the data from the vector of D2 positions at score "score".
     *
     * @param score
     * @return std::vector<range>&
     */
    std::vector<range> &d2_pos(int score) {
        return _squeue[score%_squeue.size()]._d2_pos;
    }

private:
    struct ScoreData {
        std::vector<Cell> _i_wf;
        std::vector<Cell> _d_wf;

        std::vector<Cell> _i2_wf;
        std::vector<Cell> _d2_wf;

        std::vector<range> _m_pos;

        std::vector<range> _i_pos;
        std::vector<range> _i2_pos;

        std::vector<range> _d_pos;
        std::vector<range> _d2_pos;

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

    std::vector<ScoreData> _squeue;
};

} // namespace theseus