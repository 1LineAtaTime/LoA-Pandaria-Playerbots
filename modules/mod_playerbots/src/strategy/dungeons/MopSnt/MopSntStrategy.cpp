/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple (map 1011) strategy "mop-snt" (epic #373, issue #656). Sources: see MopSntShared.h.

#include "MopSntStrategy.h"
#include "MopDungeonShared.h"
#include "MopSntMultipliers.h"
#include "MopSntShared.h"
#include "Playerbots.h"

void MopSntStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // --- Vizier Jin'bak ---
    // The DPS kill the Sap Globules before they reach the Sap Puddle (a bigger puddle makes Detonate hit harder).
    triggers.push_back(new TriggerNode("mop snt focus target",
        NextAction::array(0, new NextAction("mop snt focus target", ACTION_RAID + 2), nullptr)));
    // #781 Sap Puddle: with no globule left, a DPS whose Sap Residue is low steps into the puddle to shrink it before
    // Detonate (the "mop snt puddle" hazard sends it out again at SNT_RESIDUE_MAX stacks).
    // MoP-only (no AC counterpart): intended design per Wowpedia "Vizier Jin'bak" (2026-10-11) and the Wowhead MoP
    // tooltips of 120001 / 119941 (2026-10-12) - see MopSntShared.h.
    triggers.push_back(new TriggerNode("mop snt soak puddle",
        NextAction::array(0, new NextAction("mop snt soak puddle", ACTION_RAID + 1), nullptr)));
    triggers.push_back(new TriggerNode("mop snt puddle",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 3), nullptr)));

    // --- Commander Vo'jak ---
    // Thousand Blades (4 s whirlwind): everybody leaves melee range; Dashing Strike: stay off the charge line.
    triggers.push_back(new TriggerNode("mop snt blades",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop snt dash",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 4), nullptr)));

    // #656 w54: the tar kegs are the defence of the waves (MoP-only; Wowpedia "Commander Vo'jak" - see MopSntShared.h).
    // w54: two Sik'thik Warriors (61701, 5.9 M hp each) were still up when Vo'jak reached the platform and the group wiped
    // with him at 22%. Below the siege-bomb priority, above the Sap Globule focus.
    triggers.push_back(new TriggerNode("mop snt tar keg",
        NextAction::array(0, new NextAction("mop snt tar keg", ACTION_RAID + 3), nullptr)));

    // --- General Pa'valak ---
    // Blade Rush: leave the sword's landing disc and the charge line; armed Siege Explosives are avoided and the
    // unarmed ones are used by the DPS to break the Bulwark.
    triggers.push_back(new TriggerNode("mop snt blade rush",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop snt bomb",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("mop snt siege bomb",
        NextAction::array(0, new NextAction("mop snt siege bomb", ACTION_RAID + 4), nullptr)));
    // #781 add wave (AC AKStrategy.cpp:13 shape): the tank picks up the Sik'thik Soldiers of the Reinforcements while
    // the DPS throw bombs at / hit the shielded Pa'valak ("mop snt focus target" names him during the Bulwark).
    triggers.push_back(new TriggerNode("mop snt tank reinforcements",
        NextAction::array(0, new NextAction("mop snt tank reinforcements", ACTION_RAID + 5), nullptr)));

    // --- Wing Leader Ner'onok ---
    // #687 Quick-Dry Resin: hop about once a second while the aura is up ("Jump to prevent the resin from hardening!",
    // Wowhead MoP tooltip 121447; Wowpedia "Wing Leader Ner'onok"). Above the escape actions (EMERGENCY + 3 / + 4): the
    // hop heads for the escape point itself when a Caustic Pitch has to be left.
    triggers.push_back(new TriggerNode("mop snt resin",
        NextAction::array(0, new NextAction("mop snt resin", ACTION_EMERGENCY + 5), nullptr)));
    // Caustic Pitch pools are left at once; Gusting Winds is interrupted.
    triggers.push_back(new TriggerNode("mop snt pitch",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_EMERGENCY + 3), nullptr)));
    // #781: ACTION_INTERRUPT + 5 as AC MgTStrategy.cpp:23; the ready interrupters take turns, nobody in range -> chase.
    triggers.push_back(new TriggerNode("mop snt neronok interrupt",
        NextAction::array(0, new NextAction("mop snt neronok interrupt", ACTION_INTERRUPT + 5), nullptr)));
    // #656: after each flight he lands at the other end of the bridge, beyond the 100 yd attackers range, and does not
    // walk back (no combat movement): everybody crosses the bridge to him (MoP-only; see MopSntShared.h).
    triggers.push_back(new TriggerNode("mop snt neronok far",
        NextAction::array(0, new NextAction("mop snt neronok approach", ACTION_RAID + 1), nullptr)));
    // #656 w48: the healer stays in range + LOS of Ner'onok's victim while the group crosses the bridge (same node as
    // MopMspStrategy.cpp "mop msp healer leash": the class "reach party member to heal" priority + 10).
    triggers.push_back(new TriggerNode("mop snt healer leash",
        NextAction::array(0, new NextAction("mop snt escape", ACTION_CRITICAL_HEAL + 10), nullptr)));
}

void MopSntStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopSntHazardMultiplier(botAI));
    multipliers.push_back(new MopSntFocusMultiplier(botAI));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop snt reinforcements", MopSnt::ReinforcementsWave()));
}
