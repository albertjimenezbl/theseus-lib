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

#include <memory>
#include <istream>

#include "theseus/penalties.h"
#include "theseus/alignment.h"

/**
 * @file theseus_aligner.h
 * @brief Header file for the TheseusAligner class. This class provides an interface
 * for aligning sequences to a graph given a starting position using the Theseus
 * algorithm.
 *
 *
 */

namespace theseus
{

    class TheseusAlignerImpl; // Forward declaration of the implementation class.

    class TheseusAligner
    {
    public:
        // Tag type for constructors that parse graph streams from files.
        struct GfaStreamTag {};

        /**
         * Constructor from gfa
         *
         * @param penalties User defined alignment penalties
         * @param gfa_stream Input stream containing the graph in GFA format
         */
        TheseusAligner(const Penalties &penalties, std::istream &gfa_stream, GfaStreamTag);

        /**
         * @brief Constructor from handlegraph graph
         *
         * @param penalties User defined alignment penalties
         * @param graph HandleGraph object representing the graph to be aligned to
         */ //TODO:
        // TheseusAligner(const Penalties &penalties, const handlegraph::HandleGraph &graph);

        /**
         * Class destructor
         *
         */
        ~TheseusAligner();

        /**
         * @brief Print the resulting alignment in GAF format.
         *
         * @param alignment Alignment to be printed
         * @param out_stream Output stream where the alignment will be printed
         */
        void print_alignment_as_gaf(
                theseus::Alignment &alignment,
                std::ostream &out_stream,
                std::string seq_name);

        /**
         * Main alignment function. Aligns the given sequence to the graph starting
         * from the specified node and offset.
         *
         * @param seq Sequence to be aligned
         * @param start_node Starting node in the graph
         * @param start_offset Starting offset within the starting node
         * @return Alignment
         */
        Alignment align(std::string_view seq,
                std::string &start_node,
                int start_offset = 0);

    private:
        std::unique_ptr<TheseusAlignerImpl> aligner_impl_;
    };

} // namespace theseus
