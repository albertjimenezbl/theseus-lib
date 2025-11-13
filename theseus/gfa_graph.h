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

#include <istream>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_map>
#include <algorithm>

/**
 * Gfa Graph representation and loading utilities. More information on the GFA
 * format in: https://gfa-spec.github.io/GFA-spec/GFA1.html
 *
 */

namespace theseus
{
    class GfaGraph
    {
    public:
        /**
         * @brief An edge in the GFA graph.
         *
         */
        struct GfaEdge
        {
            int from_node;  // Source node
            int to_node;    // Target node
            size_t overlap; // Overlap between the two nodes (Currently accept 0)
        };

        /**
         * @brief A node (or segment) in the GFA graph.
         *
         */
        struct GfaNode
        {
            std::string seq;  // DNA sequence of the segment
            std::string name; // Name of the segment
        };

        // The graph can be thought of as two vectors: one containing the node
        // information and another containing the edges connnecting its nodes.
        std::vector<GfaNode> gfa_nodes;
        std::vector<GfaEdge> gfa_edges;


        /**
         * @brief Loads a GFA graph from a stream. More information on the GFA
         * format in: https://gfa-spec.github.io/GFA-spec/GFA1.html
         *
         * Disclaimer: Currently, only segments (S) and links (L) are supported.
         *
         * @param gfa_stream Input stream containing the graph in GFA format
         */
        GfaGraph(std::istream &gfa_stream);

        /**
         * Get the name of a node (or segment) given its id.
         *
         * @param id
         */
        std::string_view id_to_node_name(int id) const
        {
            assert(id < gfa_nodes.size());
            return gfa_nodes[id].name;
        }

        /**
         * Get (or create) the id of a given vertex (or segment).
         *
         * @param name              Name of the segment to check or add.
         * @return size_t           Id of the vertex (or segment)
         */
        size_t node_name_to_id(const std::string &name);

    private:
        /**
         * Load the GFA graph from a file stream.
         *
         * @param gfa_stream
         */
        void load_from_stream(std::istream &gfa_stream);

        // Mapping from node names to their ids.
        std::unordered_map<std::string, size_t> name_to_id_;
    };

} // namespace theseus