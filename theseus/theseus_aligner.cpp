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


#include "theseus/theseus_aligner.h"

#include "theseus_aligner_impl.h"

namespace theseus {

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               std::istream &gfa_stream,
                               TheseusAligner::GfaStreamTag)
{
    Graph graph(gfa_stream);
    aligner_impl_ = std::make_unique<TheseusAlignerImpl>(penalties, std::move(graph), false);
}


TheseusAligner::~TheseusAligner() {}

void TheseusAligner::print_alignment_as_gaf(
                theseus::Alignment &alignment,
                std::ostream &out_stream,
                std::string seq_name) {

    aligner_impl_->print_as_gaf(alignment, out_stream, seq_name);
}

/**
 * @brief Main alignment function for the Theseus aligner.
 *
 * @param seq
 * @param start_node
 * @param start_offset
 * @return Alignment
 */
Alignment TheseusAligner::align(
    std::string_view seq,
    std::string &start_node,
    int start_offset) {

    return aligner_impl_->align(seq, start_node, start_offset);
}

} // namespace theseus