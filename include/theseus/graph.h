#pragma once

#include<vector>
#include<string>
#include<iostream>

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
                for (int j = 0; j < _vertices[i].out_vertices.size(); ++j)
                {
                    int out_v = _vertices[i].out_vertices[j];
                    std::cout << i << "->" << out_v << std::endl;
                }
            }
        }

    private:
};

} // namespace theseus
