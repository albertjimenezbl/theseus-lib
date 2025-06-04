#include "theseus/theseus_aligner.h"

#include "theseus_aligner_impl.h"

namespace theseus {

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               std::string_view seq,
                               bool score_only) {

    // Create the initial graph
    theseus::Graph G;
    theseus::Graph::vertex source_v, central_v, sink_v;
    Graph::edge source_edge, central_edge;

    // Source vertex
    source_edge.from_vertex = 0;
    source_edge.to_vertex = 1;
    source_v.out_edges.push_back(source_edge);
    source_v.first_poa_vtx = 0;
    G._vertices.push_back(source_v);

    // Central vertex (initial sequence)
    central_edge.from_vertex = 1;
    central_edge.to_vertex = 2;
    central_v.in_edges.push_back(source_edge);
    central_v.out_edges.push_back(central_edge);
    central_v.first_poa_vtx = 1;
    central_v.value = seq;
    G._vertices.push_back(central_v);

    // Sink vertex
    sink_v.in_edges.push_back(central_edge);
    sink_v.first_poa_vtx = seq.size() + 1;
    G._vertices.push_back(sink_v);

    _aligner_impl = std::make_unique<TheseusAlignerImpl>(penalties, std::move(G), true, score_only);
}

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               const GfaGraph &gfa_graph,
                               bool msa,
                               bool score_only)
{
    Graph graph(gfa_graph);
    _aligner_impl = std::make_unique<TheseusAlignerImpl>(penalties, std::move(graph), msa, score_only);
}

TheseusAligner::~TheseusAligner() {}

// TODO:
Alignment TheseusAligner::align(std::string seq, int start_node, int start_offset) {
    return _aligner_impl->align(seq, start_node, start_offset);
}

// TODO:
void TheseusAligner::output_msa_as_fasta(const std::string &output_file) {
    _aligner_impl->output_msa_as_fasta(output_file);
}

} // namespace theseus