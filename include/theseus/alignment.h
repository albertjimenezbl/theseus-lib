#pragma once

#include <vector>

/**
 * TODO:
 *
 */

namespace theseus {

class Alignment {
    public:
    /**
     * @brief Backtrace struct. Similarly to the CIGAR in sequence alignment it
     *        contains the edit operations of the alignment. It also includes
     *        the path of the alignment.
     */
      struct Cigar {
        std::vector<char> edit_op; // Edit operations
        std::vector<int> path;      // Path of the alignment
      };

      int score;
      Cigar cigar;

      /**
       * @brief Reset the alignment data.
       *
       */
      void clear() {
        cigar.edit_op.clear();
        cigar.path.clear();
      }

    private:
};

} // namespace theseus