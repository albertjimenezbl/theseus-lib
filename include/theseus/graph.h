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

} // namespace theseus
