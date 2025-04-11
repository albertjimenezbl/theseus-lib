#include <getopt.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "../dp_aligner.h"
#include "swfa/alignment.h"
#include "swfa/penalties.h"
#include "swfa/sparse_wfa.h"

// Control of the output.
#define AVOID_DP 0
#define AVOID_SWFA 0
#define PRINT_ALIGNMENTS 0

struct CMDArgs {
    int gapo = 0;
    int gape = -4;
    std::string smatrix = "";
    std::string dataset = "";
};

void help() {
    std::cout << "Usage: benchmark [OPTIONS]\n"
                 "Options:\n"
                 "  -o, --gapo <int>      The gap open penalty (negative or zero) [default=0]\n"
                 "  -e, --gape <int>      The gap extension penalty (negative) [default=-4]\n"
                 "  -m, --smatrix <file>  Substitution matrix file\n"
                 "  -d, --dataset <file>  Dataset file\n";
}

CMDArgs parse_args(int argc, char *const *argv) {
    static const option long_options[] = {{"gapo", required_argument, 0, 'o'},
                                          {"gape", required_argument, 0, 'e'},
                                          {"smatrix", required_argument, 0, 'm'},
                                          {"dataset", required_argument, 0, 'd'},
                                          {0, 0, 0, 0}};

    CMDArgs args;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "o:e:m:d:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                args.gapo = std::stoi(optarg);
                break;
            case 'e':
                args.gape = std::stoi(optarg);
                break;
            case 'm':
                args.smatrix = optarg;
                break;
            case 'd':
                args.dataset = optarg;
                break;
            default:
                std::cerr << "Invalid option" << std::endl;
                exit(1);
        }
    }

    return args;
}

int main(int argc, char *const *argv) {
    CMDArgs args = parse_args(argc, argv);

    if (args.smatrix.empty() || args.dataset.empty()) {
        std::cerr << "Missing required arguments\n";
        help();
        return 1;
    }

    swfa::Penalties penalties(args.gapo, args.gape, args.smatrix);

    // Open the file
    std::ifstream dataset(args.dataset);

    if (!dataset.is_open()) {
        std::cerr << "Could not open dataset file\n";
        return 1;
    }

    // TODO: Parameter with maxsize of target and query.
    constexpr int max_size = 15000;

    DPAligner dp_aligner(penalties, max_size, max_size);
    swfa::SparseWFA swfa_aligner(penalties, max_size, max_size);

    std::chrono::duration<double> dp_align_duration(0);
    std::chrono::duration<double> swfa_align_duration(0);

    long long total_dp_ncells = 0;

    std::string target;
    target.reserve(max_size);

    std::string query;
    query.reserve(max_size);

    while (!dataset.eof()) {
        std::getline(dataset, target);
        std::getline(dataset, query);

        if (target.empty() || query.empty()) {
            break;
        }

        std::string_view target_view(target.data() + 1, target.size() - 1);
        std::string_view query_view(query.data() + 1, query.size() - 1);

        int dp_score = -1;
        // Align with DPAligner
#if !AVOID_DP
        auto start_align_dp = std::chrono::high_resolution_clock::now();
        auto dp_cigar = dp_aligner.align(target_view, query_view);
        auto end_align_dp = std::chrono::high_resolution_clock::now();
        dp_align_duration += end_align_dp - start_align_dp;

        long long dp_ncells = (target_view.size() + 1) * (query_view.size() + 1);
        total_dp_ncells += dp_ncells;

        dp_score = swfa::alignment::cigar_score(penalties,
                                                dp_cigar,
                                                target_view,
                                                query_view);

#if PRINT_ALIGNMENTS
        std::string_view dp_cigar_view(dp_cigar.data(), dp_cigar.size());
        const auto dp_alignment = swfa::alignment::cigar_to_alignment(dp_cigar_view,
                                                                      target_view,
                                                                      query_view);
        std::cerr << "DP:\n";
        std::cerr << dp_alignment.first << "\n";
        std::cerr << dp_alignment.second << "\n";
#endif
#endif

        int swfa_score = -1;
        // Align with SparseWFA
#if !AVOID_SWFA
        auto start_align_swfa = std::chrono::high_resolution_clock::now();
        auto swfa_cigar = swfa_aligner.align(target_view, query_view);
        auto end_align_swfa = std::chrono::high_resolution_clock::now();
        swfa_align_duration += end_align_swfa - start_align_swfa;

        swfa_score = swfa::alignment::cigar_score(penalties,
                                                  swfa_cigar,
                                                  target_view,
                                                  query_view);

#if PRINT_ALIGNMENTS
        std::string_view swfa_cigar_view(swfa_cigar.data(), swfa_cigar.size());

        const auto swfa_alignment = swfa::alignment::cigar_to_alignment(swfa_cigar_view,
                                                                        target_view,
                                                                        query_view);
        std::cerr << "SWFA:\n";
        std::cerr << swfa_alignment.first << "\n";
        std::cerr << swfa_alignment.second << "\n";
#endif
#endif

#if !AVOID_DP && !AVOID_SWFA
        if (swfa_score != dp_score) {
            std::cerr << "WFA score: " << swfa_score << "\n";
            std::cerr << "DP score: " << dp_score << "\n";
            std::cerr << "Alignments do not match\n";
            return 1;
        }
#else
        std::cout << "**Not comparing scores**\n";
#endif
    }

    std::cout << "DPAligner time (s): " << dp_align_duration.count() << "\n";
    std::cout << "Sparse WFA time (s): " << swfa_align_duration.count() << "\n";

    std::cout << "DPAligner total cells: " << total_dp_ncells << "\n";

    return 0;
}