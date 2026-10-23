/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/Seth/SethStrategy.cpp (#565, epic #569)

#include "SethStrategy.h"
#include "SethMultipliers.h"
#include "SethTriggers.h"
#include "Playerbots.h"

void TbcDungeonSethekkHallsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("time-lost controller drops charming totem",
        NextAction::array(0, new NextAction("time-lost controller mark charming totem with skull", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("sethekk prophet casts fear",
        NextAction::array(0, new NextAction("sethekk prophet set tremor totem", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("darkweaver syth boss summons elementals",
        NextAction::array(0, new NextAction("darkweaver syth mark elementals with skull", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("anzu encounter has two phases",
        NextAction::array(0, new NextAction("anzu alternate marks on boss", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("anzu bird spirits provide buffs",
        NextAction::array(0, new NextAction("anzu cast heal over time spell on bird spirit", ACTION_HIGH), nullptr)));

    triggers.push_back(new TriggerNode("talon king ikiss boss engaged by tank",
        NextAction::array(0, new NextAction("talon king ikiss tank move boss to pillar position", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("talon king ikiss ranged prepare for arcane explosion",
        NextAction::array(0, new NextAction("talon king ikiss ranged stay near victim of boss", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("talon king ikiss boss casting arcane explosion",
        NextAction::array(0, new NextAction("talon king ikiss los arcane explosion", ACTION_EMERGENCY + 10), nullptr)));

    triggers.push_back(new TriggerNode("talon king ikiss boss out of los",
        NextAction::array(0, new NextAction("talon king ikiss move to within los", ACTION_RAID + 1), nullptr)));
}

void TbcDungeonSethekkHallsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SethekkProphetSetTremorTotemMultiplier(botAI));
    multipliers.push_back(new AnzuControlSpellCastingWithSpellBombMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissControlMovementMultiplier(botAI));
}
