/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OCTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Unit.h"

// AC Ai/Dungeon/OC/OCTriggers.cpp:13
bool DrakosUnstableSphereTrigger::IsActive()
{
    // Doesn't seem to be much point trying to get melee to dodge this,
    // they get hit anyway and it just causes a lot of running around and chaos
    // if (botAI->IsMelee(bot)) { return false; }
    // 5.4.8 ADAPTATION: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank (AI/PlayerbotSpec.h:11)
    if (PlayerBotSpec::IsTank(bot)) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_UNSTABLE_SPHERE)
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:32
bool DrakeMountTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }

    return master->GetVehicleBase() && !bot->GetVehicleBase();
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:40
bool DrakeDismountTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }

    return !master->GetVehicleBase() && bot->GetVehicleBase();
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:48
bool GroupFlyingTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }

    return master->GetVehicleBase() && bot->GetVehicleBase();
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:56
bool DrakeCombatTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    return !targets.empty();
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:62
bool VarosCloudstriderTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "varos cloudstrider");
    if (!boss) { return false; }

    return true;
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:70
bool UromArcaneExplosionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "mage-lord urom");
    if (!boss) { return false; }

    return bool(boss->FindCurrentSpellBySpellId(SPELL_EMPOWERED_ARCANE_EXPLOSION));
}

// AC Ai/Dungeon/OC/OCTriggers.cpp:78
bool UromTimeBombTrigger::IsActive()
{
    return bot->HasAura(SPELL_TIME_BOMB);
}
