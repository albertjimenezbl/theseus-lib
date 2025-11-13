/*
 * 									MIT License
 *
 * Copyright (c) 2018 Mikko Rautiainen
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * -------------------------------------------------------------------------
 * Modifications copyright (c) 2024 Albert Jimenez-Blanco
 *
 * This file has been modified from the original version by Mikko Rautiainen
 * (https://github.com/maickrau/GraphAligner) under the MIT License.
 *
 * Simplification and some minor changes.
 * -------------------------------------------------------------------------
 */


#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include "gfa_graph.h"

namespace theseus {
	GfaGraph::GfaGraph(std::istream &gfa_stream) // TODO: Necessary?
	{
		load_from_stream(gfa_stream);
	}

	/**
	 * @brief Load a GFA graph from a stream. Currently, only segments (S) and
	 * links (L) are supported.
	 *
	 * @param gfa_file Input stream containing the graph in GFA format
	 */
	void GfaGraph::load_from_stream(std::istream &gfa_file)
	{
		std::string line;
		while (gfa_file.good())
		{
			std::getline(gfa_file, line);
			if (line.size() == 0 && !gfa_file.good())
				break;

			// Only Segments and Links are supported
			if (line.size() == 0 || (line[0] != 'S' && line[0] != 'L'))
				continue;

			// Parse segment data
			if (line[0] == 'S')
			{
				std::stringstream sstr{line};
				std::string type, name, dna_seq;
				sstr >> type;
				assert(type == "S");
				sstr >> name >> dna_seq;

				// We consider the forward orientation. If the segment appears reversed
				// in some link, we will add the reversed version as well.
				name = name + "+";
				size_t id = node_name_to_id(name);

				// Warning on empty nodes
				if (dna_seq == "*")
					std::cerr << std::string{"Nodes without sequence (*) are not currently supported (nodeid " + std::to_string(id) + ")"};
				assert(dna_seq.size() >= 1);
				gfa_nodes[id].seq = dna_seq; // Store the DNA sequence
			}

			// Parse link data
			if (line[0] == 'L')
			{
				std::stringstream sstr{line};
				std::string type, fromstr, tostr, fromstart, toend, overlapstr;
				int overlap = 0;
				sstr >> type;
				sstr >> fromstr >> fromstart >> tostr >> toend >> overlapstr;

				// Assess if the read data is consistent with the format
				assert(type == "L");
				assert(fromstart == "+" || fromstart == "-");
				assert(toend == "+" || toend == "-");

				// Check if the corresponding node or its reverse exist and add them if not
				fromstr = fromstr + fromstart;
				tostr = tostr + toend;
				size_t from = node_name_to_id(fromstr);
				size_t to = node_name_to_id(tostr);

				// Check overlap
				assert(overlapstr.size() >= 1);
				if (overlapstr == "*")
				{
					std::cerr << "Unspecified edge overlaps (*) are not supported" << std::endl;
				}
				if (overlapstr == "")
				{
					std::cerr << "Edge overlap missing between edges " + fromstr + " and " + tostr << std::endl;
				}
				size_t charAfterIndex = 0;
				overlap = std::stol(overlapstr, &charAfterIndex, 10);

				// Check if the format is valid: (number)M
				if (charAfterIndex != overlapstr.size() - 1 || overlapstr.back() != 'M')
				{
					std::cerr << "Edge overlaps other than exact match are not supported (non supported overlap: " + overlapstr + ")" << std::endl;
				}
				if (overlap < 0)
					std::cerr << std::string{"Edge overlap between nodes " + std::to_string(from) + " and " + std::to_string(to) + " is negative"} << std::endl;

				// Store the edge
				int frompos = (int)from;
				int topos = (int)to;
				gfa_edges.emplace_back(frompos, topos, overlap);
			}
		}

		// Add the value (DNA string) for the reversed nodes (if the + segment exists)
		for (int i = 0; i < gfa_nodes.size(); i++)
		{
			if (gfa_nodes[i].seq.size() > 0)
				continue;
			std::string name = gfa_nodes[i].name;
			if (name.back() == '+')
			{
				std::cerr << std::string{"Node " + name + " is present in edges but missing in nodes"} << std::endl;
				;
			}
			assert(name.back() == '-');
			std::string rev_name = name.substr(0, name.size() - 1) + "+";
			auto rev_ptr = name_to_id_.find(rev_name);
			if (rev_ptr != name_to_id_.end())
			{
				size_t rev_id = rev_ptr->second;
				assert(rev_id < gfa_nodes.size());
				assert(gfa_nodes[rev_id].seq.size() > 0);
				std::string dna_string = gfa_nodes[rev_id].seq;
				std::reverse(dna_string.begin(), dna_string.end()); // Reverse the DNA string
				gfa_nodes[i].seq = dna_string;
			}
		}

		// Validate that all edges connect existing nodes
		for (const auto &edge : gfa_edges)
		{
			if (edge.from_node >= gfa_nodes.size() || gfa_nodes[edge.from_node].seq.size() == 0)
			{
				std::cerr << std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name} << std::endl;
			}
			if (edge.to_node >= gfa_nodes.size() || gfa_nodes[edge.to_node].seq.size() == 0)
			{
				std::cerr << std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name} << std::endl;
			}
		}

	}

	size_t GfaGraph::node_name_to_id(const std::string &name)
	{
		auto seq_ptr = name_to_id_.find(name);

		// If the name is not found, add it
		if (seq_ptr == name_to_id_.end())
		{
			// Check that lengths are consistent
			assert(name_to_id_.size() == gfa_nodes.size());
			int result = name_to_id_.size();

			// Add the new name
			name_to_id_[name] = result;
			theseus::GfaGraph::GfaNode new_node;
			new_node.name = name;
			gfa_nodes.emplace_back(new_node);
			return result;
		}

		// Check consistency
		assert(seq_ptr->second < gfa_nodes.size());
		assert(name == gfa_nodes[seq_ptr->second].name);
		return seq_ptr->second; // Second is the value of the key-value pair
	}
}