#pragma once

#include <vector>
#include <string>

#include "../include/theseus/graph.h"
#include "../include/theseus/alignment.h"

namespace theseus {

    class POAVertex {
        public:
            std::vector<int> associated_vtxs;   // Associated vertexes
            std::vector<int> in_edges;          // In-going vertices
            std::vector<int> out_edges;         // Out-going vertices
            char value;                         // Base pair in this vertex
            int associated_vtx_compact;         // Corresponding vertex in the compact G graph
    };

    class POAEdge {
        public:
            std::vector<int> sequence_IDs;      // Sequence IDs
            int source;                         // Source vertex
            int destination;                    // Destination vertex
            int weight;                         // Weight of the edge
    };

    class POAGraph {
        public:

        std::vector<POAVertex> _poa_vertices;
        std::vector<POAEdge> _poa_edges;
        int _end_vtx_poa;

        /**
         * @brief Update a vertex in the POA graph.
         *
         * @param poa_v Vertex to update
         * @param value
         */
        void update_poa_vertex(
            int &poa_v,
            char value)
        {

            // Check if the vertex already exists
            bool already_exists = false;
            int vtx;
            char vtx_value;
            for (int l = 0; l < _poa_vertices[poa_v].associated_vtxs.size(); ++l)
            {
                vtx = _poa_vertices[poa_v].associated_vtxs[l];
                vtx_value = _poa_vertices[vtx].value;
                if (value == vtx_value)
                {
                    already_exists = true;
                    poa_v = vtx; // poa_v is the vertex that will be used when adding an edge
                }
            }

            // Create it if it doesn't
            if (!already_exists)
            {
                POAVertex new_vertex;
                new_vertex.value = value;
                new_vertex.associated_vtxs = _poa_vertices[poa_v].associated_vtxs; // Associate it the necessary vertices
                new_vertex.associated_vtxs.push_back(poa_v);                            // Add the vtx poa_v, as it is missing
                _poa_vertices.push_back(new_vertex);
                poa_v = _poa_vertices.size() - 1; // poa_v is the vertex that will be used when adding an edge

                // Update the other vertices
                for (int l = 0; l < new_vertex.associated_vtxs.size(); ++l)
                {
                    vtx = new_vertex.associated_vtxs[l];
                    _poa_vertices[vtx].associated_vtxs.push_back(poa_v);
                }
            }
        }

        /**
         * @brief Update a POA edge.
         *
         * @param source
         * @param destination
         * @param added_weight
         * @param seq_ID
         */
        void update_poa_edge(
            int source,
            int destination,
            int added_weight,
            int seq_ID) {

        // Check if the edge already exists
        bool already_exists = false;
        int curr_edge;
        for (int l = 0; l < _poa_vertices[source].out_edges.size(); ++l) {
            curr_edge = _poa_vertices[source].out_edges[l];
            if (_poa_edges[curr_edge].source == source && _poa_edges[curr_edge].destination == destination) {
            _poa_edges[curr_edge].sequence_IDs.push_back(seq_ID);
            _poa_edges[curr_edge].weight += added_weight;
            already_exists = true;
            }
        }

        // It doesn't, so you should create it
        if (!already_exists) {
            POAEdge new_edge;
            new_edge.source = source;
            new_edge.destination = destination;
            new_edge.weight = added_weight;
            new_edge.sequence_IDs.push_back(seq_ID);

            // Update the data in the poa graph
            _poa_vertices[source].out_edges.push_back(_poa_edges.size());
            _poa_vertices[destination].in_edges.push_back(_poa_edges.size());
            _poa_edges.push_back(new_edge);
        }
        }


        // Add alignment data in the _poa_graph
        void add_alignment_poa(
            Graph &G,
            Alignment::Cigar &backtrace,
            std::string &new_seq,
            int seq_ID) {

        int i = 0, j = 0, l = 0, k = 0, prev_v = backtrace.path[0], j_end = G._vertices[prev_v].value.size(), prev_v_poa = 0, new_v_poa;
        while (k < backtrace.edit_op.size()) {
            if (backtrace.edit_op[k] == 'M') {  // Match
            if (j == j_end) {
                l += 1;
                prev_v = backtrace.path[l];
                j_end = G._vertices[prev_v].value.size();
                j = 0;
            }
            else {
                new_v_poa = G._vertices[prev_v].first_poa_vtx + j;
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID);
                prev_v_poa = new_v_poa;
                new_v_poa = G._vertices[prev_v].first_poa_vtx + j + 1;
                i += 1;
                j += 1;
                k += 1;
            }
            }
            else if (backtrace.edit_op[k] == 'X') { // Mismatch
            if (j == j_end) {
                l += 1;
                prev_v = backtrace.path[l];
                j_end = G._vertices[prev_v].value.size();
                j = 0;
            }
            else {
                new_v_poa = G._vertices[backtrace.path[l]].first_poa_vtx + j;
                update_poa_vertex(new_v_poa, new_seq[i]);
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID);
                prev_v_poa = new_v_poa;
                new_v_poa = G._vertices[prev_v].first_poa_vtx + j + 1;
                i += 1;
                j += 1;
                k += 1;
            }
            }
            else if (backtrace.edit_op[k] == 'D') { // Deletion
                // Add the new vertex
                POAVertex new_vertex;
                new_vertex.value = new_seq[i];
                _poa_vertices.push_back(new_vertex);

                // Add the new edge
                new_v_poa = _poa_vertices.size() - 1;
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID);
                prev_v_poa = new_v_poa;
                i += 1;
                k += 1;
            }
            else {
                if (j == j_end) {
                    l += 1;
                    prev_v = backtrace.path[l];
                    j_end = G._vertices[prev_v].value.size();
                    j = 0;
                }
                else {
                    j += 1;
                    k += 1;
                }
                }
            }

            if (j >= j_end) {
                l += 1;
                prev_v = backtrace.path[l];
                j_end = G._vertices[prev_v].value.size();
                j = 0;
            }
            update_poa_edge(prev_v_poa, G._vertices[prev_v].first_poa_vtx + j, 1, seq_ID); // Add the edge to the sink node
        }

        /**
         * @brief Compact the POA Graph into a compacted graph "compacted_G"
         *
         * @param compacted_G
         * @param end_vertex_poa
         */
        void create_compacted_graph(
            Graph &compacted_G,
            int end_vertex_poa)
        {

            // First pass to determine the vertices in the compacted graph
            Graph::vertex first_vertex;
            first_vertex.first_poa_vtx = 0;
            compacted_G._vertices.push_back(first_vertex);
            _poa_vertices[0].associated_vtx_compact = 0;
            for (int l = 1; l < end_vertex_poa; ++l) {
                Graph::vertex new_vertex;
                new_vertex.first_poa_vtx = l;
                new_vertex.value.push_back(_poa_vertices[l].value);
                _poa_vertices[l].associated_vtx_compact = compacted_G._vertices.size();
                bool out_branching = _poa_vertices[l].out_edges.size() == 1;
                while (out_branching && l < _poa_vertices.size() - 1) {
                ++l;
                bool is_connected = false;
                for (int s = 0; s < _poa_vertices[l].in_edges.size(); ++s) {
                    if (_poa_edges[_poa_vertices[l].in_edges[s]].source == l-1) is_connected = true;
                }
                if (_poa_vertices[l].in_edges.size() > 1 || !is_connected) {
                    --l;
                    break;
                }
                else if (l < end_vertex_poa) {
                    new_vertex.value.push_back(_poa_vertices[l].value);
                    _poa_vertices[l].associated_vtx_compact = compacted_G._vertices.size(); // To which vertex in the compacted graph are you associated?
                    out_branching = _poa_vertices[l].out_edges.size() == 1;
                }
                }

                // Save the vertex
                compacted_G._vertices.push_back(new_vertex);
            }
            for (int l = end_vertex_poa + 1; l < _poa_vertices.size(); ++l) {
                Graph::vertex new_vertex;
                new_vertex.first_poa_vtx = l;
                new_vertex.value.push_back(_poa_vertices[l].value);
                _poa_vertices[l].associated_vtx_compact = compacted_G._vertices.size();
                bool out_branching = _poa_vertices[l].out_edges.size() == 1;
                while (out_branching && l < _poa_vertices.size() - 1) {
                    ++l;
                    bool is_connected = false;
                    for (int s = 0; s < _poa_vertices[l].in_edges.size(); ++s) {
                        if (_poa_edges[_poa_vertices[l].in_edges[s]].source == l-1) is_connected = true;
                    }
                    if (_poa_vertices[l].in_edges.size() > 1 || !is_connected) {
                        --l;
                        break;
                    }
                    else {
                        new_vertex.value.push_back(_poa_vertices[l].value);
                        _poa_vertices[l].associated_vtx_compact = compacted_G._vertices.size(); // To which vertex in the compacted graph are you associated?
                        out_branching = _poa_vertices[l].out_edges.size() == 1;
                    }
                }

                // Save the vertex
                compacted_G._vertices.push_back(new_vertex);
            }
            Graph::vertex last_vertex;
            last_vertex.first_poa_vtx = end_vertex_poa;
            compacted_G._vertices.push_back(last_vertex);
            _poa_vertices[end_vertex_poa].associated_vtx_compact = compacted_G._vertices.size() - 1;

            // Second pass to determine the edges
            for (int l = 0; l < compacted_G._vertices.size(); ++l) {
                // Get incoming vertices
                for (int k = 0; k < _poa_vertices[compacted_G._vertices[l].first_poa_vtx].in_edges.size(); ++k) {
                int new_in_vtx_poa = _poa_edges[_poa_vertices[compacted_G._vertices[l].first_poa_vtx].in_edges[k]].source;
                int new_in_vtx_compacted = _poa_vertices[new_in_vtx_poa].associated_vtx_compact;
                compacted_G._vertices[l].in_vertices.push_back(new_in_vtx_compacted);
                }

                // Get outgoing vertices
                int last_poa_vtx = compacted_G._vertices[l].first_poa_vtx;
                while (last_poa_vtx < _poa_vertices.size() - 1) { // Find last poa vtx
                    ++last_poa_vtx;
                    if (_poa_vertices[last_poa_vtx].associated_vtx_compact != l) {
                        --last_poa_vtx;
                        break;
                    }
                    }
                    for (int k = 0; k < _poa_vertices[last_poa_vtx].out_edges.size(); ++k) {
                    int new_out_vtx_poa = _poa_edges[_poa_vertices[last_poa_vtx].out_edges[k]].destination;
                    int new_out_vtx_compacted = _poa_vertices[new_out_vtx_poa].associated_vtx_compact;
                    compacted_G._vertices[l].out_vertices.push_back(new_out_vtx_compacted);
                }

            }

            // Set start and end nodes as not having any value
            compacted_G._vertices[0].value.clear();
            compacted_G._vertices[compacted_G._vertices.size() - 1].value.clear();

            // for (int l = 0; l < compacted_G._vertices.size(); ++l) compacted_G._vertices[l].pos = -INT_MAX; // Initialize to -INT_MAX
        }

        /**
         * @brief TODO:
         *
         */
        void create_initial_graph(theseus::Graph &G)
        {
            // Source vertex
            theseus::POAVertex source_v;
            source_v.out_edges.push_back(0);
            source_v.associated_vtx_compact = 0;
            _poa_vertices.push_back(source_v);
            theseus::POAEdge source_edge;
            source_edge.source = 0;
            source_edge.destination = 1;
            source_edge.weight = 1;
            _poa_edges.push_back(source_edge);

            // Central vertices
            for (int l = 0; l < G._vertices[1].value.size(); ++l) {
                theseus::POAVertex new_v;
                new_v.in_edges.push_back(_poa_edges.size() - 1);
                new_v.out_edges.push_back(_poa_edges.size());
                new_v.value = G._vertices[1].value[l];
                new_v.associated_vtx_compact = 1;
                _poa_vertices.push_back(new_v);
                theseus::POAEdge new_edge;
                new_edge.source = _poa_vertices.size() - 1;
                new_edge.destination = _poa_vertices.size();
                new_edge.weight = 1;
                _poa_edges.push_back(new_edge);
            }

            // Sink vertex
            theseus::POAVertex sink_v;
            sink_v.in_edges.push_back(_poa_edges.size() - 1);
            sink_v.associated_vtx_compact = 2;
            _poa_vertices.push_back(sink_v);

            _end_vtx_poa = _poa_vertices.size() - 1; // Set the end vertex
        }
    };
}