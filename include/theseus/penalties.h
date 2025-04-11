#pragma once

#include <span>
#include <string>
#include <vector>

/**
 * A class containing the penalties for the alignment algorithm. The objective
 * function is to minimize the score.
 *
 */

namespace theseus {

class Penalties {
public:
    using penalty_t = int;

    enum Type {
        Linear,
        Affine,
        DualAffine
    };

    /**
     * Create a Gap-Linear penalty object.
     *
     * @param match The match score.
     * @param mismatch The mismatch score.
     * @param gape The gap extension penalty.
     */
    Penalties(penalty_t match,
              penalty_t mismatch,
              penalty_t gape);

    /**
     * Create a Gap-Affine penalty object.
     *
     * @param match The match score.
     * @param mismatch The mismatch score.
     * @param gapo The gap open penalty.
     * @param gape The gap extension penalty.
     */
    Penalties(penalty_t match,
              penalty_t mismatch,
              penalty_t gapo,
              penalty_t gape);

    /**
     * Create a Dual Gap-Affine penalty object.
     *
     * @param match The match score.
     * @param mismatch The mismatch score.
     * @param gapo The first gap open penalty.
     * @param gape The first gap extension penalty.
     * @param gapo2 The second gap open penalty.
     * @param gape2 The second gap extension penalty.
     */
    Penalties(penalty_t match,
              penalty_t mismatch,
              penalty_t gapo,
              penalty_t gape,
              penalty_t gapo2,
              penalty_t gape2);

    /**
     * Get the gap type.
     *
     * @return The gap type.
     */
    Type type() const { return _type; }

    /**
     * Get the match score.
     *
     * @return The match score.
     */
    penalty_t match() const { return _match; }

    /**
     * Get the mismatch score.
     *
     * @return The mismatch score.
     */
    penalty_t mism() const { return _mismatch; }

    /**
     * Get the gap open penalty if the gap type is affine or dual affine.
     * Otherwise, return 0.
     *
     * @return The gap open penalty if the gap type is affine or dual affine.
     * Otherwise, return 0.
     */
    penalty_t gapo() const { return _gapo; }

    /**
     * Get the gap extension penalty.
     *
     * @return The gap extension penalty.
     */
    penalty_t gape() const { return _gape; }

    /**
     * Get the second gap open penalty if the gap type is dual affine.
     * Otherwise, return 0.
     *
     * @return The gap open penalty if the gap type is dual affine. Otherwise,
     * return 0.
     */
    penalty_t gapo2() const { return _gapo2; }

    /**
     * Get the second gap extension penalty if the gap type is dual affine.
     * Otherwise, return 0.
     *
     * @return The gap extension penalty if the gap type is dual affine.
     * Otherwise, return 0.
     */
    penalty_t gape2() const { return _gape; }
protected:
    Type _type;

    penalty_t _match;
    penalty_t _mismatch;

    penalty_t _gapo; // Gap open.
    penalty_t _gape; // Gap extension.

    penalty_t _gapo2; // Gap open for dual gap-affine.
    penalty_t _gape2; // Gap extension for dual gap-affine.
};

} // namespace theseus
