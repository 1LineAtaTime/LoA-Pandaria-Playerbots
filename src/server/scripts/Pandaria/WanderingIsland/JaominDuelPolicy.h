#ifndef JAOMIN_DUEL_POLICY_H
#define JAOMIN_DUEL_POLICY_H
#include <cstdint>
namespace JaominDuelPolicy
{
inline bool IsDefeatingHit(std::uint32_t health, std::uint32_t maximum, std::uint32_t damage)
{
    if (!health || !maximum || !damage)
        return false;
    return damage >= health || std::uint64_t(health - damage) * 100 <= std::uint64_t(maximum) * 10;
}
}
#endif
