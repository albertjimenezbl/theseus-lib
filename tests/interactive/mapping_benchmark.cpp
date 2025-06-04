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
    std::string data_file;
};


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
                 "  -d, --data_file <file>  Dataset file\n";
}

CMDArgs parse_args(int argc, char *const *argv) {
    static const option long_options[] = {{"match", required_argument, 0, 'm'},
                                          {"mismatch", required_argument, 0, 'x'},
                                          {"gapo", required_argument, 0, 'o'},
                                          {"gape", required_argument, 0, 'e'},
                                          {"data_file", required_argument, 0, 'd'},
                                          {0, 0, 0, 0}};

    CMDArgs args;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:x:o:e:d:", long_options, &option_index)) != -1) {
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
            case 'd':
                args.data_file = optarg;
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

    if (args.data_file.empty()) {
        std::cerr << "Missing required arguments\n";
        help();
        return 1;
    }

    theseus::Penalties penalties(args.match, args.mismatch, args.gapo, args.gape);

    // Main while loop
    std::ifstream data_file(args.data_file);

    if (!data_file.is_open()) {
        std::cerr << "Could not open dataset file\n";
        return 0;
    }

    std::string sequence, line; // Value and metadata of the sequence
    int num = 0, start_offset, start_node;
    while (data_file.good() && data_file.peek() != EOF)
    {
        theseus::GfaGraph graph(data_file, sequence, start_node, start_offset);
        ++num;

        // Prepare the data
        theseus::Alignment alignment;
        theseus::TheseusAligner aligner(penalties, graph, false, false);

        // Perform alignment
        std::cout << "Seq " << num << std::endl;
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        alignment = aligner.align(sequence, start_node, start_offset);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
        std::cout << "Alignment score: " << alignment.score << std::endl;
        for (int l = 0; l < alignment.cigar.edit_op.size(); ++l)
        {
            std::cout << alignment.cigar.edit_op[l] << " ";
        }
        std::cout << std::endl;
    }

    data_file.close();

    return 0;
}