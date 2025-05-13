#pragma once

#include <vector>
#include <string>

#include "graph.h"
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
         * @brief Update the compacted edges once a vertex has been split
         *
         * @param orig_from
         * @param new_from
         * @param compacted_G
         */
        void update_compact_out_edges(
            int orig_from,
            int new_from,
            Graph &compacted_G) {

            // The source is changed
            for (int l = 0; l < compacted_G._vertices[new_from].out_edges.size(); ++l) {
                // Update the out-edge
                compacted_G._vertices[new_from].out_edges[l].from_vertex = new_from;
                int to_vtx = compacted_G._vertices[new_from].out_edges[l].to_vertex;

                // Update the in-edge
                for (int k = 0; k < compacted_G._vertices[to_vtx].in_edges.size(); ++k) {
                    if (compacted_G._vertices[to_vtx].in_edges[k].from_vertex == orig_from) {
                        compacted_G._vertices[to_vtx].in_edges[k].from_vertex = new_from;
                    }
                }
            }
        }


        /**
         * @brief Split vertices in compacted_G if an edge splitting them is added.
         *
         * @param poa_source
         * @param poa_destination
         * @param compacted_G
         */
        void split_vertices(
            int poa_source,
            int poa_destination,
            Graph &compacted_G) {

            // Update the data in the compacted graph
            int source_v_compact = _poa_vertices[poa_source].associated_vtx_compact;
            int destination_v_compact = _poa_vertices[poa_destination].associated_vtx_compact;
            int pos_source = poa_source - compacted_G._vertices[source_v_compact].first_poa_vtx;
            bool split_source = pos_source < (int)compacted_G._vertices[source_v_compact].value.size() - 1;
            int pos_destination = poa_destination - compacted_G._vertices[destination_v_compact].first_poa_vtx;
            bool split_destination = pos_destination > 0;

            if (source_v_compact == destination_v_compact && poa_source + 1 == poa_destination) {
                return; // No need to split
            }

            // Update source vertex
            if (split_source) { // Split the source vertex
                // SECOND PART
                Graph::vertex new_vertex_compacted = compacted_G._vertices[source_v_compact];
                Graph::edge new_edge;
                new_vertex_compacted.in_edges.clear();
                new_edge.from_vertex = source_v_compact;
                new_edge.to_vertex = compacted_G._vertices.size();
                new_vertex_compacted.in_edges.push_back(new_edge);
                new_vertex_compacted.first_poa_vtx = poa_source + 1;
                // Update the value
                new_vertex_compacted.value.clear();
                for (int l = pos_source + 1; l <= compacted_G._vertices[source_v_compact].value.size() - 1; ++l) {
                    new_vertex_compacted.value.push_back(compacted_G._vertices[source_v_compact].value[l]);
                    _poa_vertices[(poa_source - pos_source) + l].associated_vtx_compact = compacted_G._vertices.size();
                }
                compacted_G._vertices.push_back(new_vertex_compacted);
                // Update the edges
                update_compact_out_edges(source_v_compact, compacted_G._vertices.size() - 1, compacted_G);

                // FIRST PART
                compacted_G._vertices[source_v_compact].out_edges.clear();
                compacted_G._vertices[source_v_compact].out_edges.push_back(new_edge);
                int size_seg = compacted_G._vertices[source_v_compact].value.size() - 1;
                for (int l = size_seg; l > pos_source; --l) {
                    compacted_G._vertices[source_v_compact].value.pop_back();
                }

                if (source_v_compact == destination_v_compact) {
                    destination_v_compact = compacted_G._vertices.size() - 1;
                    pos_destination = poa_destination - compacted_G._vertices[destination_v_compact].first_poa_vtx;
                }
            }

            // Update destination vertex
            if (split_destination) { // Split the destination vertex
                // Second part
                Graph::vertex new_vertex_compacted = compacted_G._vertices[destination_v_compact];
                new_vertex_compacted.in_edges.clear();
                Graph::edge new_edge;
                new_edge.from_vertex = destination_v_compact;
                new_edge.to_vertex = compacted_G._vertices.size();
                new_vertex_compacted.in_edges.push_back(new_edge);
                new_vertex_compacted.first_poa_vtx = poa_destination;
                new_vertex_compacted.value.clear();
                for (int l = pos_destination; l <= compacted_G._vertices[destination_v_compact].value.size() - 1; ++l) {
                    new_vertex_compacted.value.push_back(compacted_G._vertices[destination_v_compact].value[l]);
                    _poa_vertices[(poa_destination - pos_destination) + l].associated_vtx_compact = compacted_G._vertices.size();
                }
                compacted_G._vertices.push_back(new_vertex_compacted);
                // Update the edges
                update_compact_out_edges(destination_v_compact, compacted_G._vertices.size() - 1, compacted_G);

                // First part
                compacted_G._vertices[destination_v_compact].out_edges.clear();
                compacted_G._vertices[destination_v_compact].out_edges.push_back(new_edge);
                int size_seg = compacted_G._vertices[destination_v_compact].value.size() - 1;
                for (int l = size_seg; l >= pos_destination; --l) {
                    compacted_G._vertices[destination_v_compact].value.pop_back();
                }

                // New destination vertex
                destination_v_compact = compacted_G._vertices.size() - 1;
            }

            // Add the new edge
            Graph::edge new_edge;
            new_edge.from_vertex = source_v_compact;
            new_edge.to_vertex = destination_v_compact;
            compacted_G._vertices[source_v_compact].out_edges.push_back(new_edge);
            compacted_G._vertices[destination_v_compact].in_edges.push_back(new_edge);
        }


        /**
         * @brief Update a vertex in the POA graph.
         *
         * @param poa_v Vertex to update
         * @param value
         * @param new_vertex_exists
         * @param G
         */
        void update_poa_vertex(
            int &poa_v,
            int &pos_new_vtx,
            char value,
            bool &new_vertex_exists,
            Graph &compacted_G)
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
                    new_vertex_exists = false;
                    poa_v = vtx; // poa_v is the vertex that will be used when adding an edge
                }
            }

            // Create it if it doesn't
            if (!already_exists)
            {
                POAVertex new_vertex;
                new_vertex.value = value;
                new_vertex.associated_vtxs = _poa_vertices[poa_v].associated_vtxs;      // Associate it the necessary vertices
                new_vertex.associated_vtxs.push_back(poa_v);                            // Add the vtx poa_v, as it is missing
                _poa_vertices.push_back(new_vertex);
                poa_v = _poa_vertices.size() - 1; // poa_v is the vertex that will be used when adding an edge

                // Update the other vertices
                for (int l = 0; l < new_vertex.associated_vtxs.size(); ++l)
                {
                    vtx = new_vertex.associated_vtxs[l];
                    _poa_vertices[vtx].associated_vtxs.push_back(poa_v);
                }

                // Add a new vertex to the compacted graph
                if (new_vertex_exists) {
                    // Add a character to the existing vertex (the last one in compacted_G)
                    compacted_G._vertices[pos_new_vtx].value.push_back(value);
                    _poa_vertices[poa_v].associated_vtx_compact = pos_new_vtx;
                }
                else {
                    // Create a new vertex (the last one in compacted_G)
                    Graph::vertex new_vertex_compacted;
                    new_vertex_compacted.first_poa_vtx = poa_v;
                    new_vertex_compacted.value.push_back(value);
                    compacted_G._vertices.push_back(new_vertex_compacted);
                    pos_new_vtx = compacted_G._vertices.size() - 1;
                    _poa_vertices[poa_v].associated_vtx_compact = pos_new_vtx;
                    new_vertex_exists = true;
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
         * @param G
         */
        void update_poa_edge(
            int source,
            int destination,
            int added_weight,
            int seq_ID,
            Graph &compacted_G) {

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

            // Update the data in the compacted graph
            split_vertices(source, destination, compacted_G);
        }
        }

        void convert_path(
            Alignment::Cigar &backtrace,
            std::vector<int> &poa_path,
            Graph &compacted_G
        ) {

            // First vertex
            poa_path.push_back(compacted_G._vertices[backtrace.path[0]].first_poa_vtx);

            // Convert the path to a path in the poa graph
            for (int l = 0; l < backtrace.path.size(); ++l) {
                int first_poa_vtx = compacted_G._vertices[backtrace.path[l]].first_poa_vtx;

                for (int k = 0; k < compacted_G._vertices[backtrace.path[l]].value.size(); ++k) {
                    poa_path.push_back(first_poa_vtx + k);
                }
            }

            // Last vertex
            poa_path.push_back(compacted_G._vertices[backtrace.path[backtrace.path.size() - 1]].first_poa_vtx);
        }


        // Add alignment data in the _poa_graph
        void add_alignment_poa(
            Graph &compacted_G,
            Alignment::Cigar &backtrace,
            std::string &new_seq,
            int seq_ID) {

        // Convert the path to the corresponding path in the poa graph
        std::vector<int> poa_path;
        convert_path(backtrace, poa_path, compacted_G);

        bool new_vertex_exists = false;
        int pos_new_vertex;
        int i = 0, l = 0, k = 0, prev_v_poa = 0, new_v_poa = 0;
        while (k < backtrace.edit_op.size()) {
            // std::cout << "k: " << k << std::endl;
            if (backtrace.edit_op[k] == 'M') {  // Match
                prev_v_poa = new_v_poa;
                new_v_poa = poa_path[l + 1];
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID, compacted_G);
                i += 1;
                l += 1;
                new_vertex_exists = false;
            }
            else if (backtrace.edit_op[k] == 'X') { // Mismatch
                prev_v_poa = new_v_poa;
                new_v_poa = poa_path[l + 1];
                update_poa_vertex(new_v_poa, pos_new_vertex, new_seq[i], new_vertex_exists, compacted_G);
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID, compacted_G);
                i += 1;
                l += 1;
            }
            else if (backtrace.edit_op[k] == 'D') { // Deletion
                // Add the new vertex
                POAVertex new_vertex;
                new_vertex.value = new_seq[i];
                _poa_vertices.push_back(new_vertex);

                // Add a new compacted vertex
                if (new_vertex_exists) {
                    // Add a character to the existing vertex (the last one in compacted_G)
                    compacted_G._vertices[pos_new_vertex].value.push_back(new_seq[i]);
                    _poa_vertices[_poa_vertices.size() - 1].associated_vtx_compact = pos_new_vertex;
                }
                else {
                    Graph::vertex new_vertex_compacted;
                    new_vertex_compacted.first_poa_vtx = _poa_vertices.size() - 1;
                    new_vertex_compacted.value.push_back(new_seq[i]);

                    // The new vertex now exists
                    compacted_G._vertices.push_back(new_vertex_compacted);
                    pos_new_vertex = compacted_G._vertices.size() - 1;
                    _poa_vertices[_poa_vertices.size() - 1].associated_vtx_compact = pos_new_vertex;
                    new_vertex_exists = true;
                }

                // Add the new edge
                prev_v_poa = new_v_poa;
                new_v_poa = _poa_vertices.size() - 1;
                update_poa_edge(prev_v_poa, new_v_poa, 1, seq_ID, compacted_G);
                i += 1;
            }
            else {
                l += 1;
            }

            k += 1;
        }

            prev_v_poa = new_v_poa;
            update_poa_edge(prev_v_poa, poa_path[poa_path.size() - 1], 1, seq_ID, compacted_G); // Add the edge to the sink node
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