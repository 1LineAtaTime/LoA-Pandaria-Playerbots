#ifndef LOA_CURATED_PLAYER_NAMES_H
#define LOA_CURATED_PLAYER_NAMES_H
#include "Define.h"
#include <string>

// Reservations cover only imported candidates. Ordinary player names retain
// their existing rules; imported names share an atomic claim with bot creation.
namespace CuratedPlayerNames
{
    bool Enabled();
    bool Claim(std::string const& name, uint32 guid, uint32 account, bool requireCandidate = false);
    void Release(std::string const& name, uint32 guid, uint32 account);
    std::string Pick(uint32 guid, uint32 account, uint8 race, uint8 cls, uint8 gender);
    bool Exhausted();
    uint32 HintFallbacks();
}
#endif
