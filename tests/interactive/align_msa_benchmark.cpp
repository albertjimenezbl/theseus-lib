#include <getopt.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "../dp_aligner.h"
#include "theseus/alignment.h"
#include "theseus/penalties.h"
#include "theseus/theseus_aligner.h"
#include "../../theseus/msa.h"
#include "../../theseus/graph.h"

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
    std::string sequences_file;
    std::string output_file = "msa_output.fasta"; // Default output file for the MSA
};

/**
 * @brief Create an initial graph for the msa problem
 *
 * @param G
 * @param POAObject
 * @param first_sequence
 */


/**
 * @brief Read the sequences from a file.
 *
 * @param sequences Vector to store the sequences
 * @param args Arguments containing the file name
 */
void read_sequences(
    std::vector<std::string> &sequences,
    CMDArgs &args)
{

    // Read all sequences
    std::ifstream sequences_file(args.sequences_file);

    if (!sequences_file.is_open()) {
        std::cerr << "Could not open dataset file\n";
        return;
    }

    std::string sequence, line; // Value and metadata of the sequence

    // TODO: Allow for several sequence formats
    int num = 0;
    while (getline(sequences_file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '>')
        {
            if (num > 0) sequences.push_back(sequence);
            sequence.clear();
            num += 1;
        }
        else
        {
            sequence += line;
        }
    }

    // Store the last sequence
    if (num > 0) {
      sequences.push_back(sequence);
    }

    sequences_file.close();
}


/**
 * @brief Print the help message.
 */
void help() {
    std::cout << "Usage: benchmark [OPTIONS]\n"
                 "Options:\n"
                 "  -m, --match <int>       The match penalty [default=0]\n"
                 "  -x, --mismatch <int>    The mismatch penalty [default=2]\n"
                 "  -o, --gapo <int>        The gap open penalty [default=3]\n"
                 "  -e, --gape <int>        The gap extension penalty [default=1]\n"
                 "  -s, --sequences <file>  Dataset file\n"
                 "  -f, --output <file>     Output file for the MSA\n";
}

CMDArgs parse_args(int argc, char *const *argv) {
    static const option long_options[] = {{"match", required_argument, 0, 'm'},
                                          {"mismatch", required_argument, 0, 'x'},
                                          {"gapo", required_argument, 0, 'o'},
                                          {"gape", required_argument, 0, 'e'},
                                          {"sequences", required_argument, 0, 's'},
                                          {"output", required_argument, 0, 'f'},
                                          {0, 0, 0, 0}};

    CMDArgs args;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:x:o:e:s:f:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                args.gapo = std::stoi(optarg);
                break;
            case 'e':
                args.gape = std::stoi(optarg);
                break;
            case 'm':
                args.match = std::stoi(optarg);
                break;
            case 'x':
                args.mismatch = std::stoi(optarg);
                break;
            case 's':
                args.sequences_file = optarg;
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

    if (args.sequences_file.empty()) {
        std::cerr << "Missing required arguments\n";
        help();
        return 1;
    }

    theseus::Penalties penalties(args.match, args.mismatch, args.gapo, args.gape);

    // Read the sequences for the MSA
    std::vector<std::string> sequences;
    read_sequences(sequences, args);

    // Prepare the data
    std::vector<theseus::Alignment> alignments(sequences.size());
    theseus::TheseusAligner aligner(penalties, sequences[0], false);
    theseus::Alignment dummy_alg;

    // Alignment with Theseus
    for (int j = 1; j < sequences.size(); ++j) {
        std::cout << "Seq " << j << std::endl;
        alignments[j] = aligner.align(sequences[j]);
        // if (j == 224) {
        //      for (int l = 0; l < alignments[j].cigar.edit_op.size(); ++l) {
        //         std::cout << alignments[j].cigar.edit_op[l] << " ";
        //     }
        //     std::cout << std::endl;
        // }

        // Check if all sequences match perfectly
        // for (int l = std::max(0, j - 5); l <= j; ++l) {
        //     dummy_alg = aligner.align(sequences[j]);
        //     if (dummy_alg.score != 0) {
        //         // for (int l = 0; l < dummy_alg.cigar.edit_op.size(); ++l) {
        //         //     std::cout << dummy_alg.cigar.edit_op[l] << " ";
        //         // }
        //         // std::cout << std::endl;
        //         std::cerr << "Error: The alignment score in sequence " << j << " is: " << dummy_alg.score << "not zero." << std::endl;
        //         return 1;
        //     }
        // }
        std::cout << "Score = " << alignments[j].score << std::endl << std::endl;
    }

    // Output the MSA in fasta format
    aligner.output_msa_as_fasta(args.output_file);

    return 0;
}