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


#include <getopt.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


#include "theseus/alignment.h"
#include "theseus/penalties.h"
#include "theseus/theseus_aligner.h"

#include <vector>

// Control of the output.
#define AVOID_DP 0
#define AVOID_THESEUS 0
#define PRINT_ALIGNMENTS 0

struct CMDArgs {
    int match = 0;
    int mismatch = 2;
    int gapo = 3;
    int gape = 1;
    std::string graph_file;
    std::string sequences_and_positions_file;
    std::string output_file;
};


// Read sequence data
void read_seq_pos_data(
    std::ifstream &sp_file,
    std::vector<std::string> &sequences,
    std::vector<std::string> &start_vertices,
    std::vector<int> &start_offsets)
{
    if (!sp_file.is_open()) {
        std::cerr << "Could not open dataset file\n";
        return;
    }

    std::string sequence, line; // Value and metadata of the sequence
    std::istringstream iss(line);
    std::string vertex, orientation;
    int offset;

    // Read sequences
    int num = 0;
    while (getline(sp_file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '>')
        {
            // Read positional data
            iss.clear();
            iss.str(line.substr(1)); // Skip the '>'
            if (!(iss >> vertex >> offset >> orientation)) {
                std::cerr << "Error reading position line: " << line << std::endl;
                continue;
            }
            if (orientation != "+" && orientation != "-") {
                std::cerr << "Invalid orientation in line: " << line << std::endl;
                continue;
            }

            vertex = vertex + orientation; // We store the orientation in the vertex name);
            start_vertices.push_back(vertex);
            start_offsets.push_back(offset);

            // Store previous sequence if any
            if (num > 0) sequences.push_back(sequence);
            sequence.clear();
            num += 1;
        }
        else
        {
            sequence += line;   // The sequnce may span several lines
        }
    }

    // Store the last sequence
    if (num > 0) {
      sequences.push_back(sequence);
    }

    // Close the file
    sp_file.close();
}


/**
 * @brief Print the help message.
 */
void help() {
    std::cout << "Usage: benchmark [OPTIONS]\n"
                 "Options:\n"
                 "  -m, --match <int>            The match penalty                                [default=0]\n"
                 "  -x, --mismatch <int>         The mismatch penalty                             [default=2]\n"
                 "  -o, --gapo <int>             The gap open penalty                             [default=3]\n"
                 "  -e, --gape <int>             The gap extension penalty                        [default=1]\n"
                 "  -g, --graph_file <file>      Graph file in .gfa format                        [Required]\n"
                 "  -s, --sequences_file <file>  Sequences and starting positons in .fasta format [Required]\n"
                 "  -f, --output_file <file>     Output file                                      [Required]\n";
}

CMDArgs parse_args(int argc, char *const *argv) {
    static const option long_options[] = {{"match", required_argument, 0, 'm'},
                                          {"mismatch", required_argument, 0, 'x'},
                                          {"gapo", required_argument, 0, 'o'},
                                          {"gape", required_argument, 0, 'e'},
                                          {"graph_file", required_argument, 0, 'g'},
                                          {"sequences_file", required_argument, 0, 's'},
                                          {"output_file", required_argument, 0, 'f'},
                                          {0, 0, 0, 0}};

    CMDArgs args;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:x:o:e:g:s:f:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                args.match = std::stoi(optarg);
                break;
            case 'x':
                args.mismatch = std::stoi(optarg);
                break;
            case 'o':
                args.gapo = std::stoi(optarg);
                break;
            case 'e':
                args.gape = std::stoi(optarg);
                break;
            case 'g':
                args.graph_file = optarg;
                break;
            case 's':
                args.sequences_and_positions_file = optarg;
                break;
            case 'f':
                args.output_file = optarg;
                break;
            default:
                std::cerr << "Invalid option" << std::endl;
                exit(1);
        }
    }

    return args;
}


int main(int argc, char *const *argv) {
    // Parsing
    CMDArgs args = parse_args(argc, argv);

    if (args.graph_file.empty() || args.sequences_and_positions_file.empty() || args.output_file.empty()) {
        std::cerr << "Missing required arguments\n";
        help();
        return 1;
    }

    theseus::Penalties penalties(args.match, args.mismatch, args.gapo, args.gape);

    // Manage input/output files
    std::ifstream graph_file(args.graph_file);
    std::ifstream sp_file(args.sequences_and_positions_file);
    std::ofstream output_file(args.output_file);

    // Prepare the aligner
    theseus::TheseusAligner aligner(penalties, graph_file, theseus::TheseusAligner::GfaStreamTag{});

    // Read queries data
    std::vector<std::string> sequences, start_vertices;
    std::vector<int> start_offsets;
    read_seq_pos_data(sp_file, sequences, start_vertices, start_offsets);

    // Align the sequences
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    int num_sequences = sequences.size();
    theseus::Alignment alignment;
    for (int i = 0; i < num_sequences; ++i) {
        // Perform alignment
        std::cout << "Seq " << i << std::endl;
        alignment = aligner.align(sequences[i], start_vertices[i], start_offsets[i]);
        aligner.print_alignment_as_gaf(alignment, output_file, "seq_" + std::to_string(i));
    }

    // End time measurement
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

    // Close files
    graph_file.close();
    sp_file.close();

    return 0;
}