#include "theseus/theseus_aligner.h"

#include "theseus_aligner_impl.h"

namespace theseus {

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               std::string_view seq,
                               bool score_only) {

    // Create the initial graph
    theseus::Graph G;
    theseus::Graph::vertex source_v, central_v, sink_v;

    // Source vertex
    source_v.out_vertices.push_back(1);
    source_v.first_poa_vtx = 0;
    G._vertices.push_back(source_v);

    // Central vertex (initial sequence)
    central_v.in_vertices.push_back(0);
    central_v.out_vertices.push_back(2);
    central_v.first_poa_vtx = 1;
    central_v.value = seq;
    G._vertices.push_back(central_v);

    // Sink vertex
    sink_v.in_vertices.push_back(1);
    sink_v.first_poa_vtx = seq.size() + 1;
    G._vertices.push_back(sink_v);

    _aligner_impl = std::make_unique<TheseusAlignerImpl>(penalties, std::move(G), true, score_only);
}

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               const Graph &graph,
                               bool msa,
                               bool score_only) {
    Graph graph_copy = graph;
    _aligner_impl = std::make_unique<TheseusAlignerImpl>(penalties, std::move(graph_copy), msa, score_only);
}

TheseusAligner::TheseusAligner(const Penalties &penalties,
                               Graph &&graph,
                               bool msa,
                               bool score_only) {
    _aligner_impl = std::make_unique<TheseusAlignerImpl>(penalties, std::move(graph), msa, score_only);
}

TheseusAligner::~TheseusAligner() {}

// TODO:
Alignment TheseusAligner::align(std::string seq) {
    return _aligner_impl->align(seq);
}

} // namespace theseus