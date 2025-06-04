#pragma once

#include <span>

#include "wavefront.h"
#include "cell.h"
#include "vector.h"

// TODO:

namespace theseus {

class ScratchPad {
public:
    using size_type = ptrdiff_t;
    using diag_type = Cell::idx2d_t;

    /**
     * @brief Construct a new Scratch Pad object
     *
     * @param min_diag
     * @param max_diag
     */
    ScratchPad(diag_type min_diag, diag_type max_diag) :
        _wf(min_diag, max_diag, Cell{-1, -1, -1, -1, Cell::Matrix::None}) {

        _diags.realloc(_wf.size());
    }

    // TODO:
    Cell& access_alloc(diag_type diag) {
        // If the diagonal was not yet in the wavefront (offset is -1), add the
        // diagonal to _diags.
        auto size = _diags.size();

        // We are writing out of boundaries but inside capacity.
        // This is okay with a theseus::vector of trivial types (this is the case).
        _diags[size] = diag;

        size += _wf[diag].offset == -1;

        _diags.resize_unsafe(size);

        return _wf[diag];
    }

    // TODO:
    Cell& operator[](diag_type diag) { return _wf[diag]; }
    const Cell& operator[](diag_type diag) const { return _wf[diag]; }

    // TODO:
    size_type nactive_diags() {
        return _diags.size();
    }

    /**
     * @brief Return the minimum diagonal of the wavefront.
     *
     * @return diag_type
     */
    diag_type min_diag() const {
        return _wf.min_diag();
    }

    /**
     * @brief Return the maximum diagonal of the wavefront.
     *
     * @return diag_type
     */
    diag_type max_diag() const {
        return _wf.max_diag();
    }

    // TODO:
    std::span<const diag_type> active_diags() {
        return std::span<diag_type>(_diags.data(), _diags.data() + _diags.size());
    }

    /**
     * @brief Reset the scratchpad. This means setting the offsets to a default
     * value (-1) and resizing the vector of diagonals to 0 (none have been changed
     * yet).
     *
     */
    void reset() {
        for (const auto diag : _diags) {
            _wf[diag].offset = -1;
        }
        _diags.resize(0);
    }

private:
    Wavefront<Cell> _wf;
    Vector<diag_type, true> _diags;
};

} // namespace theseus