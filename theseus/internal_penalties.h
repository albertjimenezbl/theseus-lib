#pragma once

#include "theseus/penalties.h"

/**
 * A class derived from Penalties that contains the actual penalties used
 * in the alignment algorithm.
 *
 */

namespace theseus {

class InternalPenalties {
// public:
//     InternalPenalties(Penalties penalties);

//     // Substitute gape() for ins() and del().
//     penalty_t ins() = delete;
//     penalty_t del() = delete;
//     // penalty_t gape() = delete;
//     penalty_t gapo2() = delete;

// private:
//     // Diagonal penalties
//     penalty_t match;
//     penalty_t mismatch;

//     // Gap penalties
//     penalty_t gapo;
//     penalty_t ins;
//     penalty_t del;
};

} // namespace theseus