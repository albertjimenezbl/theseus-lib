#pragma once

#include <memory>

#include "theseus/penalties.h"
#include "theseus/alignment.h"
#include "gfa_graph.h"

/**
 * TODO:
 *
 */

namespace theseus {

class TheseusAlignerImpl; // Forward declaration of the implementation class.

class TheseusAligner {
public:
    // Always MSA.
    TheseusAligner(const Penalties &penalties,
                   std::string_view seq,
                   bool score_only);

    TheseusAligner(const Penalties &penalties,
                   const GfaGraph &gfa_graph,
                   bool msa,
                   bool score_only);

    ~TheseusAligner();

    // TODO:
    Alignment align(std::string seq);

private:
    std::unique_ptr<TheseusAlignerImpl> _aligner_impl;
};

} // namespace theseus

