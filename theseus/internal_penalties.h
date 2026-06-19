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

#include "theseus/penalties.h"

/**
 * A class derived from Penalties that contains the internal penalties used
 * during alignment.
 *
 */

namespace theseus {

class InternalPenalties {
public:
    using penalty_t = Penalties::penalty_t;

    InternalPenalties(Penalties penalties) {
        // Transform penalties in an Eizenga fashion
        if (penalties.match() != 0) {
            _match = 0;
            _mismatch = 2*penalties.mism() - 2*penalties.match();
            _gapo = 2*penalties.gapo();
            _gape = 2*penalties.gape() - penalties.match();
            std::cout << _match  << " " << _mismatch << " " << _gapo << " " << _gape << std::endl;
        }
        else {
            _match = penalties.match();
            _mismatch = penalties.mism();
            _gapo = penalties.gapo();
            _gape = penalties.gape();
        }

        if (penalties.match() > penalties.mism()) {
            throw std::invalid_argument("The match penalty must be less than the mismatch penalty");
        }
        else if (penalties.match() > penalties.gapo()) {
            throw std::invalid_argument("The match penalty must be less than or equal to the gap open penalty.");
        }
        else if (penalties.match() > penalties.gape()) {
            throw std::invalid_argument("The match penalty must be less than or equal to the gap extend penalty.");
        }
        else if (penalties.gapo() < penalties.gape()) {
            throw std::invalid_argument("The gap open penalty must be greater than or equal to the gap extension penalty.");
        }

        // TODO: Control for dual affine penalties
    }

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
     *
     * @return The gap open penalty if the gap type is affine or dual affine.
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
     *
     * @return The gap open penalty if the gap type is dual affine.
     */
    penalty_t gapo2() const { return _gapo2; }

    /**
     * Get the second gap extension penalty if the gap type is dual affine.
     *
     * @return The gap extension penalty if the gap type is dual affine.
     */
    penalty_t gape2() const { return _gape2; }

private:
    // Diagonal penalties
    penalty_t _match;
    penalty_t _mismatch;

    // Affine gap penalties
    penalty_t _gapo;
    penalty_t _gape;

    // Dual affine gap penalties
    penalty_t _gapo2;
    penalty_t _gape2;
};

} // namespace theseus