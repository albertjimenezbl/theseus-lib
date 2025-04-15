#include <getopt.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "../dp_aligner.h"
#include "../../include/theseus/alignment.h"
#include "../../include/theseus/penalties.h"
#include "../../include/theseus/graph.h"
#include "../../include/theseus/theseus_aligner.h"
#include "../../include/theseus/msa.hpp" // TODO: Add this to theseus?

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
};

/**
 * @brief Create an initial graph for the msa problem
 *
 * @param G
 * @param POAObject
 * @param first_sequence
 */
void create_initial_graph(theseus::Graph &G,
                          theseus::POAGraph &poa_graph,
                          std::string &first_sequence)
{
    // Create the initial graph
    theseus::POAVertex start_v;
    start_v.out_edges.push_back(0);
    start_v.associated_vtx_compact = 0;
    poa_graph._poa_vertices.push_back(start_v);
    theseus::POAEdge start_edge;
    start_edge.source = 0;
    start_edge.destination = 1;
    start_edge.weight = 1;
    start_edge.sequence_IDs.push_back(0);
    poa_graph._poa_edges.push_back(start_edge);
    for (int l = 0; l < first_sequence.size(); ++l) {
        theseus::POAVertex new_v;
        new_v.in_edges.push_back(l);           // New vertex
        new_v.value = first_sequence[l];
        new_v.associated_vtx_compact = 1;
        new_v.out_edges.push_back(l + 1);      // Add as out edge
        _poa_graph._poa_vertices.push_back(new_v);

        theseus::POAEdge new_edge;
        new_edge.destination = l + 2;          // New edge
        new_edge.source = l + 1;
        new_edge.weight = 1;
        new_edge.sequence_IDs.push_back(0);
        poa_graph._poa_edges.push_back(new_edge);
    }
    theseus::POAVertex end_v;
    end_v.in_edges.push_back(poa_graph._poa_edges.size() - 1);
    end_v.associated_vtx_compact = 2;
    poa_graph._poa_vertices.push_back(end_v);
    int end_vtx_poa = poa_graph._poa_vertices.size() - 1;
    poa_graph.create_compacted_graph(G, end_vtx_poa);
}

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
    while (getline(sequence_file, line))
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

void help() {
    std::cout << "Usage: benchmark [OPTIONS]\n"
                 "Options:\n"
                 "  -m, --match <int>       The match penalty [default=0]\n"
                 "  -x, --mismatch <int>    The mismatch penalty [default=2]\n"
                 "  -o, --gapo <int>        The gap open penalty [default=3]\n"
                 "  -e, --gape <int>        The gap extension penalty [default=1]\n"
                 "  -s, --sequences <file>  Dataset file\n";
}

CMDArgs parse_args(int argc, char *const *argv) {
    static const option long_options[] = {{"match", required_argument, 0, 'm'},
                                          {"mismatch", required_argument, 0, 'x'},
                                          {"gapo", required_argument, 0, 'o'},
                                          {"gape", required_argument, 0, 'e'},
                                          {"sequences", required_argument, 0, 's'},
                                          {0, 0, 0, 0}};

    CMDArgs args;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:x:o:e:s:", long_options, &option_index)) != -1) {
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

    theseus::TheseusAligner Aligner();

    // Read the sequences for the MSA
    std::vector<std::string> sequences;
    read_sequences(sequences, args);

    // Prepare the data
        // Graph
        theseus::POAGraph poa_graph;
        theseus::Graph G;
        create_initial_graph(G, poa_graph, sequences[0]);
        int end_vtx_poa = poa_graph.vertices.size() - 1;

        // Other data
        std::vector<theseus::Alignment> alignments(sequences.size());
        int end_vertex = 2;

    // Alignment with Theseus
    for (int j = 1; j < sequences.size(); ++j) {
        alignments[j - 1] = Aligner.align(sequences[j]);

        // Add the alignment to the graph (implies changing the graph G)
        POAObject.add_alignment_poa(G, alignments[j-1].cigar, sequences[j], j);
        G._vertices.clear();
        POAObject.create_compacted_graph(G, end_vtx_poa);
        end_vertex = G._vertices.size() - 1;
    }

    return 0;
}