#pragma once

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename T>
class ScoreCircularQueue {
public:
    using value_type = T;
    using size_type = ptrdiff_t;

    // TODO:
    ScoreCircularQueue(size_type nscores)
        : _front_score(0), _front_idx(0), _queue(nscores) {}

    // TODO:
    void reset() {
        _front_score = 0;
        _front_idx = 0;
    }

    // TODO:
    void new_score() {
        _front_idx = (_front_idx + 1) % _queue.size();
        _front_score++;
    }

    int score() { return _front_score; }

    // TODO:
    size_type nscores() { return _queue.size(); }

    // TODO:
    T& operator[](const int score) {
        return _queue[score_to_idx(score)];
    }

    // TODO:
    const T& operator[](const int score) const {
        return _queue[score_to_idx(score)];
    }
private:
    int _front_idx;
    int _front_score;

    std::vector<T> _queue;

    // TODO:
    int score_to_idx(const int score) const {
        return (_front_idx + score - _front_score) % _queue.size();
    }
};