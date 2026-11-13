// Legacy npc_text rows need distinct BroadcastText hotfix records in build 18414.
// Keep the reserved IDs disjoint from Blizzard's BroadcastText.db2 IDs.
#ifndef LOA_GOSSIP_BROADCAST_ID_H
#define LOA_GOSSIP_BROADCAST_ID_H
#include <cstdint>
namespace GossipBroadcastId
{
    constexpr std::uint32_t Base = 0x10000000u;
    constexpr std::uint32_t Limit = 0x20000000u;
    constexpr std::uint32_t MaxTextId = 0x01ffffffu;
    inline std::uint32_t Encode(std::uint32_t text, unsigned option)
    {
        return text <= MaxTextId && option < 8 ? Base + text * 8 + option : 0;
    }
    inline bool Decode(std::uint32_t id, std::uint32_t& text, unsigned& option)
    {
        if (id < Base || id >= Limit)
            return false;
        text = (id - Base) / 8;
        option = (id - Base) % 8;
        return true;
    }
}
#endif
