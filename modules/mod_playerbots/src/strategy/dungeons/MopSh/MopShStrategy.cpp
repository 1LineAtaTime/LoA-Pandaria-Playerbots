/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.
// TriggerNode syntax per the fork's UBStrategy.cpp:13-21.

#include "MopShStrategy.h"
#include "MopDungeonShared.h"
#include "MopShMultipliers.h"
#include "MopShShared.h"
#include "Playerbots.h"

void MopDungeonScarletHallsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Houndmaster Braun
    triggers.push_back(new TriggerNode("sh knife line",
        NextAction::array(0, new NextAction("sh leave knife line", ACTION_EMERGENCY + 4), nullptr)));
    // #781 add wave (AC AKStrategy.cpp:13 shape): each Obedient Hound dies before Braun; the tank picks it up.
    triggers.push_back(new TriggerNode("sh hounds",
        NextAction::array(0, new NextAction("sh hounds", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("sh tank hounds",
        NextAction::array(0, new NextAction("sh tank hounds", ACTION_RAID + 5), nullptr)));

    // Armsmaster Harlan
    triggers.push_back(new TriggerNode("sh blades of light",
        NextAction::array(0, new NextAction("sh flee blades of light", ACTION_EMERGENCY + 8), nullptr)));
    // #781 add wave: the Scarlet Defenders die first (single target - Heavy Armor shrugs off AoE); the tank picks them up.
    triggers.push_back(new TriggerNode("sh defenders",
        NextAction::array(0, new NextAction("sh defenders", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("sh tank defenders",
        NextAction::array(0, new NextAction("sh tank defenders", ACTION_RAID + 5), nullptr)));

    // Flameweaver Koegler
    triggers.push_back(new TriggerNode("sh koegler interrupt",
        NextAction::array(0, new NextAction("sh koegler interrupt", ACTION_INTERRUPT + 6), nullptr)));

    triggers.push_back(new TriggerNode("sh burning books",
        NextAction::array(0, new NextAction("sh leave burning books", ACTION_EMERGENCY + 6), nullptr)));

    // #683 Book Burner: one bot stands on the beam (Wowpedia "Flameweaver Koegler"): one person takes the bolt, the case stays dry.
    // w106: the bot nearest the beam (the tank when close enough), BookBurnerInterceptor.
    triggers.push_back(new TriggerNode("sh book burner intercept",
        NextAction::array(0, new NextAction("sh intercept book burner", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("sh dragon breath",
        NextAction::array(0, new NextAction("sh behind koegler", ACTION_EMERGENCY + 7), nullptr)));
}

void MopDungeonScarletHallsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopShBladesMultiplier(botAI));
    multipliers.push_back(new MopShDragonBreathMultiplier(botAI));
    multipliers.push_back(new MopShBookBurnerMultiplier(botAI));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sh hounds",
                                                            MopScarletHalls::HoundWave(MopDungeon::WaveRole::Dps)));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sh tank hounds",
                                                            MopScarletHalls::HoundWave(MopDungeon::WaveRole::Tank)));
    // Heavy Armor: 90% less area damage - the AoE is vetoed while a defender is up (AC ANMultipliers.cpp:54-57)
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sh defenders",
                                                            MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Dps), true));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sh tank defenders",
                                                            MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Tank)));
}
