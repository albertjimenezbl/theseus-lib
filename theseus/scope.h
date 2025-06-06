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
    /**
     * @brief Struct representing a range of values
     *
     */
    struct range {
        int32_t start;
        int32_t end;
    };

    // TODO: Prefer this?
    using RangeVector = Vector<range, true>;
    // using JumpsPos = ManualCapacityVector<int32_t>;

    /**
     * @brief Construct a new Scope object
     *
     * @param nscores Number of scores in the scope
     */
    Scope(int nscores) {
        _squeue.realloc(nscores);

        for (int i = 0; i < nscores; i++) {
            constexpr int init_capacity = 1024;
            ScoreData sd(init_capacity);
            _squeue.push_back(std::move(sd));
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
     * @return Cell::CellVector&
     */
    Cell::CellVector &i_wf(int score) {
        return _squeue[score%_squeue.size()]._i_wf;
    }

    /**
     * @brief Get the data from the wavefront D of score "score".
     *
     * @param score
     * @return Cell::CellVector&
     */
    Cell::CellVector &d_wf(int score) {
        return _squeue[score%_squeue.size()]._d_wf;
    }

    /**
     * @brief Get the data from the wavefront I2 of score "score".
     *
     * @param score
     * @return Cell::CellVector&
     */
    Cell::CellVector &i2_wf(int score) {
        return _squeue[score%_squeue.size()]._i2_wf;
    }

    /**
     * @brief Get the data from the wavefront D2 of score "score".
     *
     * @param score
     * @return Cell::CellVector&
     */
    Cell::CellVector &d2_wf(int score) {
        return _squeue[score%_squeue.size()]._d2_wf;
    }

    /**
     * @brief Get the data from the vector of M positions at score "score".
     *
     * @param score
     * @return RangeVector&
     */
    RangeVector &m_pos(int score) {
        return _squeue[score%_squeue.size()]._m_pos;
    }

    /**
     * @brief Get the data from the vector of I positions at score "score".
     *
     * @param score
     * @return RangeVector&
     */
    RangeVector &i_pos(int score) {
        return _squeue[score%_squeue.size()]._i_pos;
    }

    /**
     * @brief Get the data from the vector of I2 positions at score "score".
     *
     * @param score
     * @return RangeVector&
     */
    RangeVector &i2_pos(int score) {
        return _squeue[score%_squeue.size()]._i2_pos;
    }

    /**
     * @brief Get the data from the vector of D positions at score "score".
     *
     * @param score
     * @return RangeVector&
     */
    RangeVector &d_pos(int score) {
        return _squeue[score%_squeue.size()]._d_pos;
    }

    /**
     * @brief Get the data from the vector of D2 positions at score "score".
     *
     * @param score
     * @return RangeVector&
     */
    RangeVector &d2_pos(int score) {
        return _squeue[score%_squeue.size()]._d2_pos;
    }

private:
    struct ScoreData {
        static constexpr std::ptrdiff_t realloc_policy(std::ptrdiff_t capacity,
                                                       std::ptrdiff_t required_size) {
            return required_size * 2;
        };

        Cell::CellVector _i_wf;
        Cell::CellVector _d_wf;

        Cell::CellVector _i2_wf;
        Cell::CellVector _d2_wf;

        RangeVector _m_pos;

        RangeVector _i_pos;
        RangeVector _i2_pos;

        RangeVector _d_pos;
        RangeVector _d2_pos;

        ScoreData(int capacity) {
            _i_wf.realloc(capacity);
            _d_wf.realloc(capacity);
            _i2_wf.realloc(capacity);
            _d2_wf.realloc(capacity);

            _m_pos.realloc(capacity);
            _i_pos.realloc(capacity);
            _i2_pos.realloc(capacity);
            _d_pos.realloc(capacity);
            _d2_pos.realloc(capacity);

            _i_wf.set_realloc_policy(realloc_policy);
            _d_wf.set_realloc_policy(realloc_policy);
            _i2_wf.set_realloc_policy(realloc_policy);
            _d2_wf.set_realloc_policy(realloc_policy);

            _m_pos.set_realloc_policy(realloc_policy);
            _i_pos.set_realloc_policy(realloc_policy);
            _i2_pos.set_realloc_policy(realloc_policy);
            _d_pos.set_realloc_policy(realloc_policy);
            _d2_pos.set_realloc_policy(realloc_policy);
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

    Vector<ScoreData> _squeue;
};

} // namespace theseus