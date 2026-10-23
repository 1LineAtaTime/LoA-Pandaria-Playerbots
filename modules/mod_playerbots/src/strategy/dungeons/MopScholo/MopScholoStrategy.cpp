/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.
// TriggerNode syntax per the fork's UBStrategy.cpp:13-21.

#include "MopScholoStrategy.h"
#include "MopDungeonShared.h"
#include "MopScholoMultipliers.h"
#include "MopScholoShared.h"
#include "Playerbots.h"

void MopDungeonScholomanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Instructor Chillheart
    // #659 Ice Wave: instant death, so above every other Chillheart edge (sources in MopScholoShared.h).
    triggers.push_back(new TriggerNode("scholo ice wall",
        NextAction::array(0, new NextAction("scholo outrun ice wall", ACTION_EMERGENCY + 8), nullptr)));

    // #659: the tank fights Chillheart on the landing (then the corridor mouth) and Jandice at the passage mouth
    // (MopScholoShared.h geometry). AC tank-position wiring (AC Ai/Raid/ICC/ICCStrategy.cpp:14), below the Ice Wave
    // outrun and the Wondrous Rapidity edges (ACTION_MOVE + 5), whose multiplier vetoes it during the cone.
    triggers.push_back(new TriggerNode("scholo tank spot",
        NextAction::array(0, new NextAction("scholo tank spot", ACTION_MOVE + 4), nullptr)));

    triggers.push_back(new TriggerNode("scholo frigid grasp",
        NextAction::array(0, new NextAction("scholo leave frigid grasp", ACTION_EMERGENCY + 7), nullptr)));

    triggers.push_back(new TriggerNode("scholo ice wrath",
        NextAction::array(0, new NextAction("scholo spread ice wrath", ACTION_EMERGENCY + 3), nullptr)));

    triggers.push_back(new TriggerNode("scholo arcane bomb",
        NextAction::array(0, new NextAction("scholo leave arcane bomb", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("scholo fire tome",
        NextAction::array(0, new NextAction("scholo leave fire tome", ACTION_EMERGENCY + 5), nullptr)));

    // Chillheart (Phylactery), Jandice (real Jandice), Gandling (Failed Students)
    triggers.push_back(new TriggerNode("scholo focus target",
        NextAction::array(0, new NextAction("scholo focus target", ACTION_RAID + 3), nullptr)));

    // Jandice Barov
    triggers.push_back(new TriggerNode("scholo gravity flux",
        NextAction::array(0, new NextAction("scholo leave gravity flux", ACTION_EMERGENCY + 7), nullptr)));

    // Jandice: Wondrous Rapidity is a frontal cone the tank steps out of (Wowpedia "Jandice Barov" and the Wowhead MoP
    // tooltip of 114062, both 2026-10-12; sources in MopScholoShared.h). Same wiring and relevance as AC's Ingvar
    // smash dodge (AC Ai/Dungeon/UK/UKStrategy.cpp:27-30, ACTION_MOVE + 5).
    triggers.push_back(new TriggerNode("scholo rapidity tank",
        NextAction::array(0, new NextAction("scholo dodge rapidity", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("scholo rapidity tank return",
        NextAction::array(0, new NextAction("scholo rapidity return", ACTION_MOVE + 5), nullptr)));

    // #781: no sourced priority interrupt is left in Scholomance (see MopScholoShared.h). The edge stays registered
    // (context + InterruptGate) but undriven, the way AC keeps dormant code:
    // triggers.push_back(new TriggerNode("scholo priority interrupt",
    //     NextAction::array(0, new NextAction("scholo priority interrupt", ACTION_INTERRUPT + 5), nullptr)));
    // #781 add wave (AC AKStrategy.cpp:13 shape): the tank picks up the Failed Students of Rise!
    triggers.push_back(new TriggerNode("scholo tank students",
        NextAction::array(0, new NextAction("scholo tank students", ACTION_RAID + 5), nullptr)));

    // #806 Gandling Harsh Lesson (Wowpedia "Darkmaster Gandling"): inside a classroom the healer dispels Explosive Pain off
    // the Fresh Test Subjects, everyone else kills the Expired ones (FindFocusTarget); once the room is clear walk back.
    triggers.push_back(new TriggerNode("scholo harsh lesson",
        NextAction::array(0, new NextAction("scholo harsh lesson", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("scholo lesson exit",
        NextAction::array(0, new NextAction("scholo lesson exit", ACTION_MOVE + 6), nullptr)));

    // Rattlegore
    triggers.push_back(new TriggerNode("scholo bone armor",
        NextAction::array(0, new NextAction("scholo get bone armor", ACTION_MOVE + 5), nullptr)));

    triggers.push_back(new TriggerNode("scholo soulflame",
        NextAction::array(0, new NextAction("scholo leave soulflame", ACTION_EMERGENCY + 5), nullptr)));

    // Lilian Voss
    triggers.push_back(new TriggerNode("scholo dark blaze",
        NextAction::array(0, new NextAction("scholo leave dark blaze", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("scholo lilian spread",
        NextAction::array(0, new NextAction("scholo spread from group", ACTION_MOVE + 7), nullptr)));

    triggers.push_back(new TriggerNode("scholo fixated",
        NextAction::array(0, new NextAction("scholo run from soul", ACTION_EMERGENCY + 6), nullptr)));
}

void MopDungeonScholomanceStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopScholoFocusMultiplier(botAI));
    multipliers.push_back(new MopScholoFixateMultiplier(botAI));
    multipliers.push_back(new MopScholoRapidityMultiplier(botAI));
    multipliers.push_back(new MopScholoIceWallMultiplier(botAI));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "scholo tank students",
                                                            MopScholomance::GandlingTankWave()));
}
