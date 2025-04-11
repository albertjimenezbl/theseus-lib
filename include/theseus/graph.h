#pragma once

#include<vector>
#include<string>

/**
 * TODO:
 *
 */

namespace theseus {

class Graph {
    public:
        struct vertex
        {
            std::vector<int> in_vertices;   // in-going vertices
            std::vector<int> out_vertices;  // out-going vertices
            std::string value;              // sequence associated to the edge
            int first_poa_vtx;              // starting point in the poa graph
        };

        std::vector<vertex> _vertices;

        // TODO:
        std::vector<vertex> &vertices() { return _vertices; }

    private:
};


class POAGraph {
    public:
        struct POA_vertex
        {
            std::vector<int> associated_vtxs;   // Associated vertexes
            std::vector<int> in_edges;          // In-going vertices
            std::vector<int> out_edges;         // Out-going vertices
            char value;                         // Base pair in this vertex
            int associated_vtx_compact;         // Corresponding vertex in the compact G graph
        };

        struct POA_edge
        {
            std::vector<int> sequence_IDs;
            int source;
            int destination;
            int weight;
        };

        std::vector<POA_vertex> _poa_vertices;
        std::vector<POA_edge> _poa_edges;

    private:
};

} // namespace theseus
