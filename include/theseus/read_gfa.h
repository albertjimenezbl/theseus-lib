#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "graph.h"

/**
 * @brief Read a GFA file and populate the graph.
 *
 * @param g The graph to populate.
 * @param gfa_file The path to the GFA file.
 */

void read_gfa_graph(
    theseus::Graph &g,
    const std::string &gfa_file)
{
    std::ifstream gfa(gfa_file);
    if (!gfa.is_open())
    {
        std::cerr << "Error opening GFA file: " << gfa_file << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> remaining_lines;
    while (std::getline(gfa, line))
    {
        if (line.empty() || line[0] != 'H' || line[0] != 'S' || line[0] != 'L')
            continue; // Skip empty lines and information not related to headers, segments, or links

        // Parse the GFA line
        char type = line[0];
        std::istringstream line_stream(line);
        if (type == 'H')            // Header
        {
            /*TODO:*/
        }
        else if (type == 'S')       // Segment
        {
            std::string name, seq;
            line_stream >> name >> seq;

            theseus::Graph::vertex vtx;
            vtx.value = seq;
            g._vertices.push_back(vtx);
        }
        else if (type == 'L')      // Link
        {
            int from, to;
            char orientation_from, orientation_to;
            std::string overlap;
            line_stream >> from >> orientation_from >> to >> orientation_to >> overlap;

            // Two cases: the orientation is normal (both +) or it has at least one reversed segment
            if (orientation_from == '+' && orientation_to == '+')
            {
                g._vertices[from].out_vertices.push_back(to);
                g._vertices[to].in_vertices.push_back(from);
            }
            else
                remaining_lines.push_back(line); // Store the line and process it later
        }
    }

    // Process the reversed links
    ...

    gfa.close();
}