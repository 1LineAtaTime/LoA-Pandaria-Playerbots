/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#include "MopScholoTriggers.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include <cmath>

using namespace MopScholomance;

bool MopScholoHazardTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    float clearDistance = 0.0f;
    return FindNearestHazard(bot, _hazard, clearDistance) != nullptr;
}

bool MopScholoIceWrathTrigger::IsActive()
{
    return bot->HasAura(SPELL_ICE_WRATH) && FindNearestGroupMember(bot, ICE_WRATH_SPREAD) != nullptr;
}

bool MopScholoLilianSpreadTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    // Melee cannot spread around a boss that must be hit; the tank holds her. Ranged and healers can.
    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsMelee(bot))
        return false;

    Creature* lilian = FindBoss(bot, NPC_LILIAN_VOSS);
    if (!lilian || !lilian->IsInCombat())
        return false;

    return FindNearestGroupMember(bot, LILIAN_SPREAD) != nullptr;
}

bool MopScholoFixatedTrigger::IsActive()
{
    Creature* soul = nullptr;
    return FixatedBySoul(bot, soul);
}

bool MopScholoInterruptTrigger::IsActive()
{
    Unit* target = FindInterruptTarget(bot);
    return target && _gate.Allow(bot, target);
}

bool MopScholoFocusTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    return FindFocusTarget(bot) != nullptr;
}

// #806 (sources in MopScholoShared.h): healers dispel Explosive Pain off the Fresh Test Subjects.
bool MopScholoHarshLessonTrigger::IsActive()
{
    if (!PlayerBotSpec::IsHeal(bot) || !bot->IsInCombat() || !HasFriendlyDispel(botAI))
        return false;

    return InHarshLessonRoom(bot) && FindExplosivePainSubject(bot) != nullptr;
}

bool MopScholoLessonExitTrigger::IsActive()
{
    Creature* gandling = nullptr;
    return LessonExitNeeded(bot, gandling);
}

bool MopScholoBoneArmorTrigger::IsActive()
{
    return NeedsBoneArmor(bot) && FindBonePile(bot) != nullptr;
}

// AC Ai/Dungeon/UK/UKTriggers.cpp:46-57 (IngvarSmashTankTrigger): the tank, while the boss casts the frontal attack.
bool MopScholoRapidityTankTrigger::IsActive()
{
    if (!PlayerBotSpec::IsTank(bot))
        return false;

    Creature* boss = nullptr;
    return JandiceRapidityActive(bot, boss);
}

// AC Ai/Dungeon/UK/UKTriggers.cpp:59-68 (IngvarSmashTankReturnTrigger): the tank, while the boss is there; the action's
// isUseful() (behind the target) decides.
bool MopScholoRapidityTankReturnTrigger::IsActive()
{
    if (!PlayerBotSpec::IsTank(bot))
        return false;

    // #659: AC's "find target" (threat list), not an entry scan: Jandice is within 100 yd of Chillheart and Rattlegore.
    return FindEngagedBoss(bot, NPC_JANDICE_BAROV) != nullptr;
}

// #659 Ice Wave (sources in MopScholoShared.h): the bot stands inside the wall's reach (or will within one step).
bool MopScholoIceWallTrigger::IsActive()
{
    float shortfall = 0.0f;
    return IceWallSafeY(bot, shortfall) != 0.0f && shortfall > 0.0f;
}

// #659 (AC tank-position shape, AC Ai/Raid/ICC/ICCStrategy.cpp:14 "icc lm tank position"): off the spot by more than
// the slack.
bool MopScholoTankSpotTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!TankSpot(bot, x, y, z))
        return false;

    return bot->GetExactDist2d(x, y) > TANK_SPOT_SLACK || std::fabs(bot->GetPositionZ() - z) > 4.0f;
}
