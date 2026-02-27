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

#include <vector>
#include "theseus/penalties.h"

/**
 * Class containing the alignment information (CIGAR and path) and a custom conversion
 * function to compute the alignment score according to user defined penalties.
 *
 */

namespace theseus {

class Alignment {
    public:
    /**
     * @brief Backtrace objects: similar to the CIGAR in sequence alignment. It consists
     *        of the set of edit operations of the alignment and the path of the alignment
     *        through the reference graph.
     */
      std::vector<char> edit_op; // Edit operations
      std::vector<int> path;     // Path of the alignment
      int start_offset;          // Start offset in the first vertex of the path
      int end_offset;            // End offset in the last vertex of the path


      // Compute the affine gap score of the CIGAR,
      /**
       * @brief Compute the affine gap score of the CIGAR. This is due to the fact
       * that we use equivalent internal penalties during the alignment stage that
       * might differ from user defined penalties.
       *
       * @param user_penalties
       * @return int Alignment score according to user penalties and computed CIGAR
       */
      int compute_affine_gap_score(Penalties &user_penalties) {
          int score = 0;
          bool insertion_open = false, deletion_open = false;
          for (const auto &op : edit_op) {
              if (op == 'X') {
                  insertion_open = false;
                  deletion_open = false;
                  score += user_penalties.mism(); // Mismatch score
              }
              else if (op == 'I') {
                  deletion_open = false;
                  if (!insertion_open) {
                      insertion_open = true;
                      score += user_penalties.gapo() + user_penalties.gape(); // Gap open penalty for insertion
                  }
                  else {
                      score += user_penalties.gape(); // Gap extend penalty for insertion
                  }
              }
              else if (op == 'D') {
                  insertion_open = false;
                  if (!deletion_open) {
                      deletion_open = true;
                      score += user_penalties.gapo() + user_penalties.gape(); // Gap open penalty for deletion
                  }
                  else {
                      score += user_penalties.gape(); // Gap extend penalty for deletion
                  }
              }
              else if (op == 'M') {
                  insertion_open = false;
                  deletion_open = false;
                  score += user_penalties.match(); // Match score
              }
          }
          return score;
      }

    private:
};

} // namespace theseus