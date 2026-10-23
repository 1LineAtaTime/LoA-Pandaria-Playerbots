/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UKTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// AC Ai/Dungeon/UK/UKTriggers.cpp (KelesethFrostTombTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool KelesethFrostTombTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_FROST_TOMB))
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/UK/UKTriggers.cpp (DalronnDpsTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool DalronnDpsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!boss || !boss->isTargetableForAttack()) { return false; }

    // This doesn't cause issues with healers currently and they will continue to heal even when included here
    return !PlayerBotSpec::IsTank(bot);
}

// AC Ai/Dungeon/UK/UKTriggers.cpp (IngvarDreadfulRoarTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool IngvarDreadfulRoarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    if (boss->FindCurrentSpellBySpellId(SPELL_DREADFUL_ROAR))
    {
        return true;
    }
    return false;
}

// AC Ai/Dungeon/UK/UKTriggers.cpp (IngvarSmashTankTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool IngvarSmashTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss || !PlayerBotSpec::IsTank(bot)) { return false; }

    if (boss->FindCurrentSpellBySpellId(SPELL_SMASH) ||
        boss->FindCurrentSpellBySpellId(SPELL_DARK_SMASH))
        {
            return true;
        }
    return false;
}

// AC Ai/Dungeon/UK/UKTriggers.cpp (IngvarSmashTankReturnTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool IngvarSmashTankReturnTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    // if (!boss || !botAI->IsTank(bot) || boss->HasUnitState(UNIT_STATE_CASTING))
    // Ignore casting state as Ingvar will sometimes chain-cast a roar after a smash..
    // We don't want this to prevent our tank from repositioning properly.
    if (!boss || !PlayerBotSpec::IsTank(bot)) { return false; }

    return true;
}

// AC Ai/Dungeon/UK/UKTriggers.cpp (NotBehindIngvarTrigger::IsActive)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11; precedent strategy/actions/PullActions.cpp:51)
bool NotBehindIngvarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss || PlayerBotSpec::IsTank(bot)) { return false; }

    return AI_VALUE2(bool, "behind", "current target");
}
