#ifndef _PLAYERBOT_PARTYMEMBERSNAREDTARGETVALUE_H
#define _PLAYERBOT_PARTYMEMBERSNAREDTARGETVALUE_H

#include "PartyMemberValue.h"

class PlayerbotAI;
class Unit;

// #163 S8 (AC parity): the first group member (besides pets/charms) who is rooted or slowed and in
// range, for Paladin's Hand of Freedom on party (and any future "cure the snare" action).
class PartyMemberSnaredTargetValue : public PartyMemberValue
{
public:
    PartyMemberSnaredTargetValue(PlayerbotAI* botAI, std::string const name = "party member snared target")
        : PartyMemberValue(botAI, name)
    {
    }

protected:
    Unit* Calculate() override;
    bool Check(Unit* player) override;
};

#endif
