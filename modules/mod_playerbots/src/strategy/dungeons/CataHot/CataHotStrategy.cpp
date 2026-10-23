/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight (map 940) strategy "cata-hot" - issue #673, epic #373. AC has no Hour of Twilight strategy; this is AC's
// dungeon-strategy pattern (Ai/Dungeon/HoS/HoSStrategy.cpp:9-33, Ai/Dungeon/UB/UBStrategy.cpp:30-34 for the target
// exclusions) with edges for the mechanics that wipe a pull. Sources: see CataHotShared.h.
//
// Boss x edges:
//   Arcurion   : kill the Icy Tomb that holds Thrall / ignore the Frozen Servitors on the canyon rim (target exclusion) /
//                leave an Icy Boulder glyph (#696)
//   Asira      : leave the Choking Smoke Bomb cloud (the tank drags her out, nobody can target into or out of the smoke) /
//                a Mark of Silence carrier stands behind the tank on Asira's line (#696)
//   Benedictus : avoid the Purifying Light / Corrupting Twilight orbs / phase 1: stack in Thrall's Water Shell for the Wave of
//                Virtue (#696), phase 2: step out of the Wave of Twilight / spread while carrying Righteous or Twilight Shear
//                (healers dispel it with the generic dispel strategy)

#include "CataHotStrategy.h"
#include "CataHotMultipliers.h"
#include "CataHotShared.h"
#include "Creature.h"
#include "Playerbots.h"
#include <vector>

void CataDungeonHourOfTwilightStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Arcurion
    triggers.push_back(new TriggerNode("hot icy tomb",
        NextAction::array(0, new NextAction("hot kill icy tomb", ACTION_RAID + 6), nullptr)));

    // #696: step out of a Frozen Servitor's Icy Boulder glyph before the 3 s cast lands
    triggers.push_back(new TriggerNode("hot icy boulder",
        NextAction::array(0, new NextAction("hot leave icy boulder", ACTION_EMERGENCY + 6), nullptr)));

    // Thrall's escorts (#673): kill the ambush that fights Thrall, so his escort moves on (he waits while in combat)
    triggers.push_back(new TriggerNode("hot thrall ambush",
        NextAction::array(0, new NextAction("hot assist thrall", ACTION_RAID + 1), nullptr)));

    // Asira Dawnslayer
    triggers.push_back(new TriggerNode("hot smoke bomb",
        NextAction::array(0, new NextAction("hot leave smoke bomb", ACTION_EMERGENCY + 6), nullptr)));
    // #696: a marked caster keeps the tank between itself and Asira (Throw Knife hits the first player on the line)
    triggers.push_back(new TriggerNode("hot mark of silence",
        NextAction::array(0, new NextAction("hot knife cover", ACTION_EMERGENCY + 1), nullptr)));

    // Archbishop Benedictus
    // #696: phase 1 Wave of Virtue - into Thrall's Water Shell (above the sideways dodge, which InWavePath then leaves out)
    triggers.push_back(new TriggerNode("hot water shell",
        NextAction::array(0, new NextAction("hot enter water shell", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(new TriggerNode("hot light orbs",
        NextAction::array(0, new NextAction("hot leave light orb", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("hot wave",
        NextAction::array(0, new NextAction("hot dodge wave", ACTION_EMERGENCY + 8), nullptr)));
    triggers.push_back(new TriggerNode("hot shear",
        NextAction::array(0, new NextAction("hot spread shear", ACTION_EMERGENCY + 2), nullptr)));
}

void CataDungeonHourOfTwilightStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataHotHazardMultiplier(botAI));
    multipliers.push_back(new CataHotIcyTombMultiplier(botAI));
}

// The Frozen Servitors (14 DB rows plus the script's own summons) stand 26-65 yd above the arena; chasing them leaves melee
// bots running at a cliff. Nobody targets them, whatever the target value type.
// #673 (wave 61 lane, 2026-10-14): only the canyon-rim servitors are excluded. The same entry 54600 is also the ambush trash of
// Thrall's escort (hour_of_twilight.cpp:1542-1546: summoned at the 55389 / 55384 / 54560 stalkers on the path, z 97-100, the
// same level as the group), which attacks the group and holds Thrall's escort (npc_thrall_intro EVENT_ESCORT waits while he is in
// combat, hour_of_twilight.cpp:676-678) until it is killed ("Another ambush. Watch your back!", Wowpedia "Arcurion", MediaWiki
// API 2026-10-14). The rim ones stand 26-65 yd above the arena (z 92-132 against ~66), so a height test separates the two.
constexpr float HOT_SERVITOR_RIM_HEIGHT = 10.0f;

void CataDungeonHourOfTwilightStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type == TargetValueExclusionType::None)
        return;

    Player* bot = botAI->GetBot();
    std::vector<Creature*> servitors;
    HourOfTwilight::CollectServitors(bot, servitors);
    for (Creature* servitor : servitors)
        if (servitor->GetPositionZ() - bot->GetPositionZ() > HOT_SERVITOR_RIM_HEIGHT)
            exclusions.insert(servitor->GetGUID());
}
