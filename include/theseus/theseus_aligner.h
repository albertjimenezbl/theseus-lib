#pragma once

/**
 * TODO:
 *
 */

namespace theseus {

class TheseusAligner {
private:
    class TheseusAlignerImpl; // Forward declaration of the implementation class.
    std::unique_ptr<TheseusAlignerImpl> _aligner_impl;
};

} // namespace theseus

