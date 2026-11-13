#ifndef LOA_QUEST_DEPENDENCY_POLICY_H
#define LOA_QUEST_DEPENDENCY_POLICY_H

#include <cstdint>
#include <limits>

// Legacy SkyFire signed quest links: positive requires a rewarded predecessor;
// negative requires a taken predecessor. Preserve the sign in reverse links.
namespace QuestDependencyPolicy
{
    constexpr std::uint32_t QuestId(std::int32_t link)
    {
        return link < 0 ? std::uint32_t(-std::int64_t(link)) : std::uint32_t(link);
    }

    constexpr std::int32_t ReverseLink(std::int32_t next, std::uint32_t predecessor)
    {
        return !next || predecessor > std::uint32_t(std::numeric_limits<std::int32_t>::max())
            ? 0 : next < 0 ? -std::int32_t(predecessor) : std::int32_t(predecessor);
    }

    constexpr bool Satisfied(std::int32_t link, bool rewarded, bool taken)
    {
        return link > 0 ? rewarded : link < 0 && taken;
    }
}

#endif
