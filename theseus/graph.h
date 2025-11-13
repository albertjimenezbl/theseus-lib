/*
 *                             The MIT License
 *
 * Copyright (c) 2024 by Albert Jimenez-Blanco
 *
 * This file is part of #################### Theseus Library ####################.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#pragma once

#include<vector>
#include<string>
#include<iostream>
#include<fstream>

#include"gfa_graph.h"

/**
 * Internal representation of a directed graph.
 *
 */

namespace theseus {

class Graph {
    public:
        struct edge {
            int from_vertex;     // from vertex
            int to_vertex;       // to vertex
            size_t overlap = 0;  // overlap length
        };

        struct vertex
        {
            std::vector<edge> in_edges;     // in-going vertices
            std::vector<edge> out_edges;    // out-going vertices
            std::string value;              // sequence associated to the vtx
            std::string name;               // name of the vertex
            int first_poa_vtx;              // starting point in the poa graph
        };

        std::vector<vertex> _vertices;
        std::unordered_map<std::string, size_t> name_to_id_;    // Mapping from node names to their ids.

        // Return the vertices of the graph
        std::vector<vertex> &vertices() { return _vertices; }

        Graph() = default;

        /**
         * @brief Construct a new Graph object from a GfaGraph object.
         *
         * @param gfa_graph
         */
        Graph(std::istream &gfa_stream);

        /**
         * @brief Visualize the graph in Graphviz format.
         *
         * @param G
         */
        void print_code_graphviz(std::ofstream &out_stream)
        {
            out_stream << "digraph G {" << std::endl;
            // Set all sequences to the nodes
            for (int i = 0; i < _vertices.size(); ++i)
            {
                out_stream << i << " [label=\"";
                for (int j = 0; j < _vertices[i].value.size(); ++j)
                {
                    out_stream << _vertices[i].value[j];
                }
                out_stream << "\"]" << std::endl;
            }

            // Set edges
            for (int i = 0; i < _vertices.size(); ++i)
            {
                for (int j = 0; j < _vertices[i].out_edges.size(); ++j)
                {
                    int out_v = _vertices[i].out_edges[j].to_vertex;
                    out_stream << i << "->" << out_v << std::endl;
                }
            }

            out_stream << "}" << std::endl;
        }


        void print_as_gfa(std::ofstream &gfa_output) {
            if (!gfa_output.is_open())
            {
                throw std::runtime_error("Could not open output file");
            }

            // Print all nodes as segments
            for (const auto &vtx : _vertices)
            {
                gfa_output << "S\t" << vtx.name << "\t" << vtx.value << "\n";
            }

            // Print all edges as links
            for (const auto &vtx : _vertices)
            {
                // Go through all incoming vertices (with this you cover all possible edges,
                // since the graph is directed)
                for (const auto &edge : vtx.in_edges)
                {
                    gfa_output << "L\t" << _vertices[edge.from_vertex].name << "\t+\t"
                        << vtx.name << "\t+\t"
                        << edge.overlap << "M\n";
                }
            }

            gfa_output.close();
        }

        // Get the id of a vertex given its name
        size_t get_id(const std::string &name) {
            return name_to_id_.at(name);
        }

    private:
};

} // namespace theseus
