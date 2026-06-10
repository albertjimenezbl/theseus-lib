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


#include "theseus/theseus_msa_aligner.h"

#include "theseus_aligner_impl.h"

namespace theseus {

using NodeId = Graph::NodeId;

TheseusMSA::TheseusMSA(const Penalties &penalties,
                       const Heuristics &heuristics,
                       std::string_view seq,
                       int initial_weight,
                       bool is_ends_free) {

    // Error out if the sequence is ends_free. Ask for a backbone (end-to-end)
    if (is_ends_free) {
        throw std::invalid_argument("Sequence must be end-to-end for MSA initialization");
    }
    // Create the initial graph
    theseus::Graph G;
    // Add nodes
    NodeId source_node_id = G.add_node("");
    NodeId central_node_id = G.add_node(seq);
    NodeId sink_node_id = G.add_node("");
    // Add edges
    G.add_edge(source_node_id, central_node_id);
    G.add_edge(central_node_id, sink_node_id);
    // Construct the aligner implementation
    msa_aligner_impl_ = std::make_unique<TheseusAlignerImpl>(penalties, heuristics, std::move(G), initial_weight);
}

/**
 * @brief Destroy the Theseus Aligner:: Theseus Aligner object
 *
 */
TheseusMSA::~TheseusMSA() {}

/**
 * @brief Main alignment function for the Theseus aligner.
 *
 * @param seq
 * @param reverse_alignment
 * @param is_ends_free
 * @return Alignment
 */
Alignment TheseusMSA::align(
    std::string_view seq,
    int weight,
    bool reverse_alignment,
    bool is_ends_free,
    bool density_drop_active,
    bool lag_pruning_active
) {

    // Error out if you are adding a backbone sequence after partial sequences
    if (!still_end_to_end && !is_ends_free) {
        throw std::invalid_argument("Cannot add a backbone sequence after partial sequences");
    }
    // Update the still_end_to_end value
    still_end_to_end = !is_ends_free;
    return msa_aligner_impl_->align(seq, weight, reverse_alignment, is_ends_free, density_drop_active, lag_pruning_active);
}

/**
 * @brief Print the current POA graph in MSA format.
 *
 */
void TheseusMSA::print_as_gfa(std::ostream &out_stream) {
    msa_aligner_impl_->print_as_gfa(out_stream);
}

/**
 * @brief Print the current POA graph in MSA format.
 *
 */
void TheseusMSA::print_as_msa(std::ostream &out_stream) {
    msa_aligner_impl_->print_as_msa(out_stream);
}

/**
 * @brief Return consensus sequence.
 *
 */
std::string TheseusMSA::heaviest_bundle_consensus() {
    return msa_aligner_impl_->heaviest_bundle_consensus();
}

/**
 * @brief Print the weighted majority voting consensus.
 *
 */
void TheseusMSA::majority_voting_consensus(std::vector<int> &consensus_weights,
                                           std::string &consensus_sequence,
                                           std::string &consensus_sequence_gapped) {
    msa_aligner_impl_->majority_voting_consensus(consensus_weights, consensus_sequence, consensus_sequence_gapped);
}

/**
 * @brief Print in graphviz format.
 *
 */
void TheseusMSA::print_as_dot(std::ostream &out_stream) {
    msa_aligner_impl_->print_code_graphviz(out_stream);
}

} // namespace theseus