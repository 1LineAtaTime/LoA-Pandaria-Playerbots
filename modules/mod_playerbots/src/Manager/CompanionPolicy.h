#pragma once
#include <algorithm>

namespace CompanionPolicy
{
inline bool Allowed(unsigned level, unsigned rank, bool autonomous, int minLevel, int minRank, bool gmBypass)
{
    return autonomous || (gmBypass && rank >= 2) ||
        (minLevel >= 0 && minRank >= 0 && level >= unsigned(minLevel) && rank >= unsigned(minRank));
}
inline unsigned Capacity(unsigned owned, unsigned pending, int configured)
{
    unsigned limit = unsigned(std::max(0, std::min(39, configured)));
    return owned >= limit || pending >= limit - owned ? 0 : limit - owned - pending;
}
}
