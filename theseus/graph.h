#pragma once

#include<vector>
#include<string>
#include<iostream>
#include"theseus/gfa_graph.h"

/**
 * TODO:
 *
 */

namespace theseus {

class Graph {
    public:
        struct edge {
            int from_vertex; // from vertex
            int to_vertex;   // to vertex
            size_t overlap = 0;  // overlap length
        };

        struct vertex
        {
            std::vector<edge> in_edges;     // in-going vertices
            std::vector<edge> out_edges;    // out-going vertices
            std::string value;              // sequence associated to the edge
            std::string name;               // name of the vertex
            int first_poa_vtx;              // starting point in the poa graph
        };

        std::vector<vertex> _vertices;

        // TODO:
        std::vector<vertex> &vertices() { return _vertices; }

        Graph() = default;

        /**
         * @brief Construct a new Graph object from a GfaGraph object.
         *
         * @param gfa_graph
         */
        Graph(const GfaGraph &gfa_graph);

        /**
         * @brief Visualize the graph in Graphviz format.
         *
         * @param G
         */
        void print_code_graphviz()
        {

            // Set all sequences to the nodes
            for (int i = 0; i < _vertices.size(); ++i)
            {
                std::cout << i << " [label=\"";
                for (int j = 0; j < _vertices[i].value.size(); ++j)
                {
                    std::cout << _vertices[i].value[j];
                }
                std::cout << "\"]" << std::endl;
            }

            // Set edges
            for (int i = 0; i < _vertices.size(); ++i)
            {
                for (int j = 0; j < _vertices[i].out_edges.size(); ++j)
                {
                    int out_v = _vertices[i].out_edges[j].to_vertex;
                    std::cout << i << "->" << out_v << std::endl;
                }
            }
        }

    private:
};

} // namespace theseus
