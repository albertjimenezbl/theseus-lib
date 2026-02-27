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


#include "../doctest.h"

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "../../theseus/graph.h"
#include "../../include/theseus/alignment.h"
#include "../../include/theseus/penalties.h"
#include "../../include/theseus/theseus_aligner.h"


TEST_CASE("Check sequence-to-graph aligner") {
    SUBCASE("Correct alignment of sequences against a graph with a cycle") {
        // Reference graph
        std::istringstream gfa_stream(
            "S\t1\tACTTAG\n"
            "S\t2\tACA\n"
            "S\t3\tT\n"
            "S\t4\tGTACTT\n"
            "L\t1\t+\t2\t+\t0M\n"
            "L\t1\t+\t3\t+\t0M\n"
            "L\t2\t+\t4\t+\t0M\n"
            "L\t3\t+\t4\t+\t0M\n"
            "L\t4\t+\t1\t+\t0M\n"
        );

        // Sequences for alignment
        std::vector<std::string> sequences = {
            "TAGACAGTACT",   // Perfect match
            "TAGACAGGACT",   // One mismatch
            "ACAGTACTTACT",  // Perfect match with a cycle
            "AACAGTACTTACT", // A deletion with a cycle
            "ACAGTATTACT"    // An insertion with a cycle
        };

        // Starting offsets and vertices
        std::vector<int> start_offsets = {3, 3, 0, 0, 0};
        std::vector<std::string> start_vertices = {"1+", "1+", "2+", "2+", "2+"};

        // Expected CIGARs
        std::vector<std::vector<char>> expected_cigars = {
            {'M','M','M','M','M','M','M','M','M','M','M'},
            {'M','M','M','M','M','M','M','X','M','M','M'},
            {'M','M','M','M','M','M','M','M','M','M','M','M'},
            {'M','D','M','M','M','M','M','M','M','M','M','M','M'},
            {'M','M','M','M','M','M','I','M','M','M','M','M'}
        };

        // Expected paths TODO: Should have names of vertices instead of internal indices
        std::vector<std::vector<int>> expected_paths = {
            {0, 2, 6},
            {0, 2, 6},
            {2, 6, 0},
            {2, 6, 0},
            {2, 6, 0},
        };

        // Expected scores
        std::vector<int> expected_scores = {0, 2, 0, 4, 4};

        // Set aligner's parameters
        theseus::Penalties penalties(0, 2, 3, 1);            // Create penalties object
        theseus::TheseusAligner aligner(penalties, gfa_stream, theseus::TheseusAligner::GfaStreamTag{}); // Create aligner

        // Align sequences and check results
        for (int i = 0; i < start_offsets.size(); ++i) {
            theseus::Alignment alignment = aligner.align(
                sequences[i], start_vertices[i], start_offsets[i]
            );

            CHECK(alignment.compute_affine_gap_score(penalties) == expected_scores[i]); // Check score
            CHECK(alignment.edit_op == expected_cigars[i]);        // Check CIGAR
            CHECK(alignment.path == expected_paths[i]); // Check path
        }
    }
}