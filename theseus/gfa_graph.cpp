// This parser has been adapted from GraphAligner's GFA parser. https://github.com/maickrau/GraphAligner


#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include "theseus/gfa_graph.h"
#include "utils.h"

namespace theseus {
	/**
	 * @brief Loads a GFA graph from a file.
	 *
	 * @param filename
	 * @return GfaGraph
	 */
	GfaGraph::GfaGraph(std::string_view filename)
	{
		/**TODO: Add support for compressed .gz files */
		assert(filename.substr(filename.size() - 4) == ".gfa");
		std::string filename_str{filename};
		std::ifstream file(filename_str);
		LoadFromStream(file);
	}

	GfaGraph::GfaGraph(std::istream &gfa_stream)
	{
		LoadFromStream(gfa_stream);
	}

	GfaGraph::GfaGraph(std::istream &gfa_stream, std::string &sequence, int &start_node, int &start_offset)
	{
		LoadGraphandSeqFromStream(gfa_stream, sequence, start_node, start_offset);
	}

	void GfaGraph::LoadGraphandSeqFromStream(
		std::istream &gfa_file,
		std::string &sequence,
		int &start_node,
		int &start_offset)
	{
		std::string line;
		while (gfa_file.good() && std::getline(gfa_file, line) && line[0] != '-')
		{
			if (line.size() == 0 && !gfa_file.good())
				break;

			// Only Segments and Links are supported
			if (line.size() == 0)
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
				size_t id = getNameId(name);

				// Warning on empty nodes
				if (dna_seq == "*")
					throw Utils::InvalidGraphException{std::string{"Nodes without sequence (*) are not currently supported (nodeid " + std::to_string(id) + ")"}};
				assert(dna_seq.size() >= 1);
				gfa_nodes[id].seq = dna_seq; // Store the DNA sequence
			}
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
				size_t from = getNameId(fromstr);
				size_t to = getNameId(tostr);

				// Check overlap
				assert(overlapstr.size() >= 1);
				if (overlapstr == "*")
				{
					throw Utils::InvalidGraphException{"Unspecified edge overlaps (*) are not supported"};
				}
				if (overlapstr == "")
				{
					throw Utils::InvalidGraphException{"Edge overlap missing between edges " + fromstr + " and " + tostr};
				}
				size_t charAfterIndex = 0;
				overlap = std::stol(overlapstr, &charAfterIndex, 10);

				// Check if the format is valid: (number)M
				if (charAfterIndex != overlapstr.size() - 1 || overlapstr.back() != 'M')
				{
					throw Utils::InvalidGraphException{"Edge overlaps other than exact match are not supported (non supported overlap: " + overlapstr + ")"};
				}
				if (overlap < 0)
					throw Utils::InvalidGraphException{std::string{"Edge overlap between nodes " + std::to_string(from) + " and " + std::to_string(to) + " is negative"}};

				// Store the edge
				int frompos = (int)from;
				int topos = (int)to;
				gfa_edges.emplace_back(frompos, topos, overlap);
			}
			if (line[0] == 's')
			{
				std::stringstream sstr{line};
				std::string type;
				sstr >> type >> sequence;
			}
			if (line[0] == 'p')
			{
				std::stringstream sstr{line};
				std::string start_node_str, start_offset_str, type;
				sstr >> type >> start_node_str >> start_offset_str;

				// Convert the strings to integers
				start_node = getNameId(start_node_str + "+"); // Assume the start node is in the forward orientation
				start_offset = std::stoi(start_offset_str);
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
				throw Utils::InvalidGraphException{std::string{"Node " + name + " is present in edges but missing in nodes"}};
				;
			}
			assert(name.back() == '-');
			std::string rev_name = name.substr(0, name.size() - 1) + "+";
			auto rev_ptr = name_to_id.find(rev_name);
			if (rev_ptr != name_to_id.end())
			{
				size_t rev_id = rev_ptr->second;
				assert(rev_id < gfa_nodes.size());
				assert(gfa_nodes[rev_id].seq.size() > 0);
				std::string dna_string = gfa_nodes[rev_id].seq;
				std::reverse(dna_string.begin(), dna_string.end()); // Reverse the DNA string
				gfa_nodes[i].seq = dna_string;
			}
		}

		for (auto edge : gfa_edges)
		{
			auto from = edge.from_node;
			auto to = edge.to_node;
			auto overlap = edge.overlap;
			assert(from < gfa_nodes.size());
			assert(to < gfa_nodes.size());
			if (gfa_nodes[from].seq.size() <= overlap || gfa_nodes[to].seq.size() <= overlap)
			{
				throw Utils::InvalidGraphException{std::string{"Overlap between nodes " + gfa_nodes[from].name + " and " + gfa_nodes[to].seq + " fully contains one of the nodes. Fix the overlap to be strictly smaller than both nodes"}};
			}
		}
		for (const auto &edge : gfa_edges)
		{
			if (edge.from_node >= gfa_nodes.size() || gfa_nodes[edge.from_node].seq.size() == 0)
			{
				throw Utils::InvalidGraphException{std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name}};
			}
			if (edge.to_node >= gfa_nodes.size() || gfa_nodes[edge.to_node].seq.size() == 0)
			{
				throw Utils::InvalidGraphException{std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name}};
			}
		}
	}

	void GfaGraph::LoadFromStream(std::istream &gfa_file)
	{
		std::string line;
		while (gfa_file.good())
		{
			std::getline(gfa_file, line);
			if (line.size() == 0 && !gfa_file.good())
				break;

			// Only Segments and Links are supported
			if (line.size() == 0 || line[0] != 'S' && line[0] != 'L')
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
				size_t id = getNameId(name);

				// Warning on empty nodes
				if (dna_seq == "*")
					throw Utils::InvalidGraphException{std::string{"Nodes without sequence (*) are not currently supported (nodeid " + std::to_string(id) + ")"}};
				assert(dna_seq.size() >= 1);
				gfa_nodes[id].seq = dna_seq; // Store the DNA sequence
			}
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
				size_t from = getNameId(fromstr);
				size_t to = getNameId(tostr);

				// Check overlap
				assert(overlapstr.size() >= 1);
				if (overlapstr == "*")
				{
					throw Utils::InvalidGraphException{"Unspecified edge overlaps (*) are not supported"};
				}
				if (overlapstr == "")
				{
					throw Utils::InvalidGraphException{"Edge overlap missing between edges " + fromstr + " and " + tostr};
				}
				size_t charAfterIndex = 0;
				overlap = std::stol(overlapstr, &charAfterIndex, 10);

				// Check if the format is valid: (number)M
				if (charAfterIndex != overlapstr.size() - 1 || overlapstr.back() != 'M')
				{
					throw Utils::InvalidGraphException{"Edge overlaps other than exact match are not supported (non supported overlap: " + overlapstr + ")"};
				}
				if (overlap < 0)
					throw Utils::InvalidGraphException{std::string{"Edge overlap between nodes " + std::to_string(from) + " and " + std::to_string(to) + " is negative"}};

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
				throw Utils::InvalidGraphException{std::string{"Node " + name + " is present in edges but missing in nodes"}};
				;
			}
			assert(name.back() == '-');
			std::string rev_name = name.substr(0, name.size() - 1) + "+";
			auto rev_ptr = name_to_id.find(rev_name);
			if (rev_ptr != name_to_id.end())
			{
				size_t rev_id = rev_ptr->second;
				assert(rev_id < gfa_nodes.size());
				assert(gfa_nodes[rev_id].seq.size() > 0);
				std::string dna_string = gfa_nodes[rev_id].seq;
				std::reverse(dna_string.begin(), dna_string.end()); // Reverse the DNA string
				gfa_nodes[i].seq = dna_string;
			}
		}

		for (auto edge : gfa_edges)
		{
			auto from = edge.from_node;
			auto to = edge.to_node;
			auto overlap = edge.overlap;
			assert(from < gfa_nodes.size());
			assert(to < gfa_nodes.size());
			if (gfa_nodes[from].seq.size() <= overlap || gfa_nodes[to].seq.size() <= overlap)
			{
				throw Utils::InvalidGraphException{std::string{"Overlap between nodes " + gfa_nodes[from].name + " and " + gfa_nodes[to].seq + " fully contains one of the nodes. Fix the overlap to be strictly smaller than both nodes"}};
			}
		}
		for (const auto &edge : gfa_edges)
		{
			if (edge.from_node >= gfa_nodes.size() || gfa_nodes[edge.from_node].seq.size() == 0)
			{
				throw Utils::InvalidGraphException{std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name}};
			}
			if (edge.to_node >= gfa_nodes.size() || gfa_nodes[edge.to_node].seq.size() == 0)
			{
				throw Utils::InvalidGraphException{std::string{"The graph has an edge between non-existant node(s) " + gfa_nodes[edge.from_node].name + " and " + gfa_nodes[edge.to_node].name}};
			}
		}

	}


	/**
	 * @brief Get the id of a given vertex (or segment)
	 *
	 * @param assigned          Unordered map of stored data
	 * @param name              Name of the segment to check or add
	 * @param nodeSeqs          Vector of segment sequences
	 * @param originalNodeName  Vector of segment names
	 * @return size_t           Id of the vertex (or segment)
	 */
	size_t GfaGraph::getNameId(const std::string &name)
	{
		auto seq_ptr = name_to_id.find(name);
		if (seq_ptr == name_to_id.end())
		{
			// Check that lengths are consistent
			assert(name_to_id.size() == gfa_nodes.size());
			int result = name_to_id.size();

			// Add the new name
			name_to_id[name] = result;
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

	std::string theseus::GfaGraph::OriginalNodeName(int nodeId) const
	{
		assert(nodeId < gfa_nodes.size());
		return gfa_nodes[nodeId].name;
	}

	// if (graphFile.substr(graphFile.size() - 4) == ".gfa" || (graphFile.size() > 7 && graphFile.substr(graphFile.size() - 7) == ".gfa.gz"))
	// {
	//     auto graph = GfaGraph::LoadFromFile(graphFile);
	//     if (loadMxmSeeder)
	//     {
	//         std::cout << "Build MUM/MEM seeder from the graph" << std::endl;
	//         *mxmSeeder = new MEMSeeder { graph, params.seederCachePrefix, params.uniqueMemBonusFactor, params.lowMemoryMEMIndexConstruction, params.MEMindexUsesWaveletTree, params.MEMwindowsize };
	//     }
	//     std::cout << "Build alignment graph" << std::endl;
	//     auto result = DirectedGraph::BuildFromGFA(graph);
	//     return result;
	// }
	// else
	// {
	//     std::cerr << "Unknown graph type (" << graphFile << ")" << std::endl;
	//     std::exit(0);
	// }
}