/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace (map 994) strategy "mop-msp" (epic #373, issue #654). Sources: see MopMspShared.h.

#include "MopMspStrategy.h"
#include "MopMspMultipliers.h"
#include "Playerbots.h"

void MopMspStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // --- Trial of the King (Ming the Cunning, Kuai the Brute, Haiyan the Unstoppable; one after the other) ---

    // Ming: interrupt Lightning Bolt (3 s cast on the tank); stay out of the roaming Whirling Dervish.
    triggers.push_back(new TriggerNode("mop msp ming interrupt",
        NextAction::array(0, new NextAction("mop msp ming interrupt", ACTION_INTERRUPT + 5), nullptr)));
    triggers.push_back(new TriggerNode("mop msp dervish",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_EMERGENCY + 3), nullptr)));

    // Kuai: leave the Shockwave cone (the picked player's line); Mu'Shiba is excluded as a target (see below).
    triggers.push_back(new TriggerNode("mop msp shockwave",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_EMERGENCY + 4), nullptr)));

    // Haiyan: the Conflagrate carrier and its neighbours spread; everybody stacks on the Meteor target.
    triggers.push_back(new TriggerNode("mop msp conflagrate",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("mop msp meteor stack",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_RAID + 5), nullptr)));

    // --- Gekkan and his entourage ---
    // Interrupt Cleansing Flame (Oracle) and Hex of Lethargy (Hexxer); kill the protectors in order.
    // #781: ACTION_INTERRUPT + 5 as AC MgTStrategy.cpp:23; the ready interrupters take turns
    triggers.push_back(new TriggerNode("mop msp gekkan interrupt",
        NextAction::array(0, new NextAction("mop msp gekkan interrupt", ACTION_INTERRUPT + 5), nullptr)));
    triggers.push_back(new TriggerNode("mop msp focus target",
        NextAction::array(0, new NextAction("mop msp focus target", ACTION_RAID + 2), nullptr)));

    // --- Xin the Weaponmaster ---
    // Ground Slam cone (everybody but the tank), the whirling axes and the Ring of Fire.
    triggers.push_back(new TriggerNode("mop msp ground slam",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop msp axe",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("mop msp fire ring",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_EMERGENCY + 4), nullptr)));
    // #654 w47: the healer walks back into range + LOS of Xin's victim (the dais hides the tank from the floor); same
    // priority as the class "reach party member to heal" (HolyPriestStrategy.cpp:112), which the hazard multiplier holds.
    triggers.push_back(new TriggerNode("mop msp healer leash",
        NextAction::array(0, new NextAction("mop msp escape", ACTION_CRITICAL_HEAL + 10), nullptr)));
}

void MopMspStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopMspHazardMultiplier(botAI));
    multipliers.push_back(new MopMspFocusMultiplier(botAI));
}

// Wowpedia "Trial of the King": "Ignore Mu'Shiba throughout and focus on Kuai." (shape of UB/UBStrategy.cpp:30)
void MopMspStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& pets = AI_VALUE_REF(GuidVector, "mop msp exclusions");
    exclusions.insert(pets.begin(), pets.end());
}
