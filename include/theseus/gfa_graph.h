#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace theseus {

    class GfaGraph
    {
    public:
        struct GfaEdge
        {
            int from_node;
            int to_node;
            size_t overlap;
        };

        struct GfaNode
        {
            std::string seq;
            std::string name;
        };

        GfaGraph(std::string_view filename);
        GfaGraph(std::istream& stream);
        void LoadFromStream(std::istream &gfa_stream);
        std::string OriginalNodeName(int nodeId) const;
        size_t getNameId(const std::string &name);

        std::unordered_map<std::string, size_t> name_to_id;
        std::vector<GfaNode> gfa_nodes;
        std::vector<GfaEdge> gfa_edges;
    private:
    };

} // namespace theseus