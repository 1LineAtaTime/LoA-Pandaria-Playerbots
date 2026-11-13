#ifndef IMMEDIATE_EVENT_SUMMONS_H
#define IMMEDIATE_EVENT_SUMMONS_H
#include <cstdint>
#include <map>
#include <set>
#include <vector>

namespace ImmediateEventSummons
{
    struct Node
    {
        std::uint32_t id;
        std::uint32_t link;
        bool linked;
        std::uint32_t chance;
        bool supported;
    };

    // Validate complete synchronous graphs; never execute half an encounter.
    inline bool Validate(std::vector<Node> const& nodes)
    {
        std::map<std::uint32_t, Node const*> indexed;
        for (Node const& node : nodes)
            if (!node.supported || node.chance > 100 || !indexed.emplace(node.id, &node).second)
                return false;
        bool root = false;
        std::set<std::uint32_t> reached;
        for (Node const& node : nodes)
        {
            if (node.linked)
                continue;
            root = true;
            reached.insert(node.id);
            std::set<std::uint32_t> chain;
            Node const* current = &node;
            while (current->link)
            {
                auto next = indexed.find(current->link);
                if (next == indexed.end() || !next->second->linked || !chain.insert(current->link).second)
                    return false;
                current = next->second;
                reached.insert(current->id);
            }
        }
        return root && reached.size() == nodes.size();
    }

    inline bool PassesChance(std::uint32_t chance, std::uint32_t sample)
    {
        // Preserve SmartAI's zero-as-unconditional convention. Draws in [0,99]
        // give an authored 50% root exactly 50 possible successes.
        return chance == 0 || chance >= 100 || sample < chance;
    }
}
#endif
