/*
 *                             The MIT License
 *
 * Copyright (c) 2024 by Albert Jimenez-Blanco
 *
 * This file is part of #################### Theseus Library ####################.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once

#include <iostream>
#include <vector>

#include "theseus/alignment.h"

/**
 * Class containing the data related to heuristics
 *
 */

namespace theseus {

    class Heuristics {
    public:
        /**
         * @brief Construct a new Heuristics object with parameters
         *
         */
        Heuristics() {}


        // INITIALIZER //
        void new_alignment(int gape, int seq_len, bool density_drop_active, bool lag_pruning_active) {
            // General
            _max_offset = 0;
            _lag_pruning  = lag_pruning_active;
            _density_drop = density_drop_active;

            // Drop heuristic parameters (depending on sequence length and penalties)
            // Consider for now that you want to capture gaps of length max(min(500, seq_len/20), 10)
            _s_min               = std::max(std::min(1000, seq_len/10), 100)*gape;
            // std::cout << seq_len << " smin= " << _s_min << std::endl;
            int min_offsets      = std::max(std::min(1000, seq_len/10), 100);
            _offsets_to_drop     = min_offsets*2; // (TODO: Too loose?)
            // General _last_offsets initialization
            _K                   = _s_min + 1;
            _pruning_allowed     = false;
            _last_max_offsets.clear();
            _last_max_offsets.resize(_K, 0);
            // Initialization of lag pruning
            int min_offsets_to_prune   = std::max(std::min(500, seq_len/20), 50);
            _min_off_increase_to_prune = min_offsets_to_prune*3; // Advance 2 matches per error (33% error rate?)
            _lookback_lag              = min_offsets_to_prune*gape;
        }


        // ------------------------ HEURISTICS ---------------------------------

        // LOCAL heuristics: Heuristics that are checked on all wavefront cells
        bool check_local_heuristics(int &curr_offset) {
            // Update the max value
            _max_offset = (curr_offset > _max_offset)? curr_offset : _max_offset;
            // Check lag pruning
            if (check_lag_pruning(curr_offset)) {
                return true;
            }
            else return false;
        }

        /**
         * @brief Lag behind heuristic
         *
         * Let "max_offset" indicate the maximum offset reached so far by Theseus
         * and let "curr_offset" indicate the offset of a currently analysed Theseus'
         * cell. Also let "lag_threshold" indicate a maximum allowed offset difference
         * between "max_offset" and "curr_offset".
         * -----------
         * Definition: The lag behing heuristic checks, for a given cell with offset
         * curr_offset whether
         *                      max_offset - curr_offset > lag_threshold
         * That is, whether the current cell if too far behind from the currently
         * most promising cell (the one with offset max_offset).
         * -----------
         * We can use this heuristic to prune unpromising cells and paths.
         */
        // inline bool check_lag_behind(int &curr_offset) {
        //     if (!_lag_behind) return false;
        //     _max_offset = (curr_offset > _max_offset)? curr_offset : _max_offset;
        //     return (_max_offset - curr_offset > _lag_threshold);
        // }

        /**
         * @brief  Lag pruning heuristic.
         *
         * This heuristic combines is a refined version of the lag behind heuristic,
         * that only prunes cells when they are too far behind AND the most promising
         * path has advanced enough in the last K steps. That is, you want to avoid
         * pruning cells when the most promising path has not advanced in the last K
         * steps, as it may be stuck in a zone with a high error rate in the alignment.
         * -----------
         * Definition: We define the density lag heuristic as a heuristic that checks,
         * for a given cell with offset curr_offset if two conditions hold:
         *      1. max_offset - curr_offset > lag_threshold
         *      2. The most promising path has advanced enough in the last K steps
         *         (that is, the increase in the maximum offset in the last K steps
         *         is greater than a given threshold, indicating a "promising advancement
         *         density").
         *            max_offset - _last_max_offsets[score%K] > min_increase
         * -----------
         * We can use this heuristic to prune unpromising cells and paths, while avoiding
         * pruning cells when there is no clear primising path.
         */
        inline bool check_lag_pruning(int &curr_offset) {
            if (!_lag_pruning) return false;
            // Condition 1
            bool condition_1 = _max_offset - curr_offset > _min_off_increase_to_prune; // TODO: Check
            // Condition 2 is _pruning_allowed value (updated once per score)
            return (condition_1 && _pruning_allowed);
        }


        // GLOBAL heuristics: Heuristics checked once per score
        int check_global_heuristics(int score) {
            // Update the value for the max offset in the current score
            _last_max_offsets[score%_K] = _max_offset;
            // Update pruning condition
            update_pruning_condition(score);
            // Check advancement density heuristic
            if (_density_drop) return check_density_drop(score);
            return THESEUS_STATUS_OK;
        }

        /**
         * @brief Density drop
         *
         * Let "max_offset" indicate the maximum offset reached so far by Theseus
         * and let "_offsets_to_drop" indicate the minimum number of offsets that
         * Theseus has to advance in the last "_score_increase scores". If the increase
         * in offsets is lower than "_offsets_to_drop", drop the alignment. Else,
         * continue processing the query.
         * -----------
         * Definition: We define the density drop heuristic as a heuristic that
         * checks whether the current offset_inc/score_inc ratio is lower than
         * a given threshold. That is, we check
         *
         *                      offset_inc/score_inc < threshold
         *                                  iff
         *           max_offset[s] - max_offset[s - score_inc] < _offsets_to_drop
         *
         * We further complement this by checking if the score is greater than
         * score_inc = "_s_min", to avoid stopping alignment with little context.
         * -----------
         * This heuristic serves as a global stopping criterion: if the heuristic
         * fails, Theseus stops alignment and returns an incomplete alignment.
         */
        inline int check_density_drop(int &score) {
            // Check if the heuristic is active
            if (!_density_drop) return THESEUS_STATUS_OK;
            // Check minimum score
            if (score <= _s_min) return THESEUS_STATUS_OK;
            // Offset values at score and score-score_inc
            int curr_offset = _last_max_offsets[score%_K];
            int prev_offset = _last_max_offsets[(score - _s_min + _K)%_K];
            int offset_inc  = curr_offset - prev_offset;
            // if (offset_inc < _offsets_to_drop) {
            //     std::cout << "Dropping because off_inc=" << offset_inc << " and offsets_to_drop=" << _offsets_to_drop << std::endl;
            // }
            // Return heuristic fail or success
            return (offset_inc < _offsets_to_drop) ?
                    THESEUS_STATUS_END_UNREACHABLE :
                    THESEUS_STATUS_OK;
        }

        /**
         * @brief Update density for the lag density heuristic
         *
         * @param score
         */
        void update_pruning_condition(int &score) {
            // Check if density is high enough
            // std::cout << "Pos: " << (score - _s_min + _K)%_K << " and size " << _last_max_offsets.size() << std::endl;
            int offset_inc = _max_offset - _last_max_offsets[(score - _lookback_lag + _K)%_K];
            if (offset_inc > _min_off_increase_to_prune) {
                _pruning_allowed = true;
                // std::cout << "Pruning allowed" << std::endl;
            }
            else {
                _pruning_allowed = false;
            }
        }

        /**
         * @brief Over steps limit heuristic
         *
         * Check if we are out of the step limit (the two sequences are too
         * similar)
         *
         */
        // int is_over_step_limit(int &score) {
        //     if (!_steps_limit) return THESEUS_STATUS_OK;
        //     else return score > _max_steps ? THESEUS_STATUS_MAX_STEPS_REACHED : THESEUS_STATUS_OK;
        // }


        //---------------------------- ACCESSORS -------------------------------

        /**
         * @brief Return whether the lag behind heuristic is active
         *
         */
        // bool is_lag_behind_active() {
        //     return _lag_behind;
        // }

        /**
         * @brief Return whether the lag pruning heuristic is active
         *
         */
        bool is_lag_pruning_active() {
            return _lag_pruning;
        }

        /**
         * @brief Return whether the density drop is active
         *
         */
        bool is_density_drop_active() {
            return _density_drop;
        }

        /**
         * @brief Return the _s_min value
         *
         */
        int s_min() {
            return _s_min;
        }

        /**
         * @brief Return maximum number of steps
         *
         */
        // int max_steps() {
        //     return _max_steps;
        // }

    private:
        // Used heuristics
        bool _lag_pruning;
        bool _density_drop;

        // General data
        int _max_offset;

        // Lag pruning
        int  _lookback_lag;
        int  _min_off_increase_to_prune;
        bool _pruning_allowed;
        int  _K;
        std::vector<int> _last_max_offsets; // Vector of the last maximum offsets

        // Density drop heuristic
        int _s_min;
        int _offsets_to_drop;
    };

} // namespace theseus