/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBTriggers.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

// AC Ai/Dungeon/UB/UBTriggers.cpp:13
bool UBFoulSporesTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen") /* 5.4.8: "find target" supplied by lead plumbing */;
    return boss && boss->HasAura(SPELL_FOUL_SPORES);
}

// AC Ai/Dungeon/UB/UBTriggers.cpp:19
bool UBSporeCloudDangerTrigger::IsActive()
{
    auto const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    return GetNearestDangerousMushroom(bot, mushrooms, MushroomDangerRange(bot)) != nullptr;
}

// AC Ai/Dungeon/UB/UBTriggers.cpp:25
bool UBUnderbatLashTrigger::IsActive()
{
    if (PlayerBotSpec::IsTank(bot) /* 5.4.8: static spec helper, cf. strategy/actions/PullActions.cpp:51 */)
        return false;

    auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    return GetNearestUnderbatInLashRange(bot, attackers) != nullptr;
}
