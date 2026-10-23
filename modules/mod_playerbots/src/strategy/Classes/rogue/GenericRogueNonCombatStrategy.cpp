/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericRogueNonCombatStrategy.h"

#include "Playerbots.h"

// 5.4.8: AC's item-based poison-on-weapon mechanic (TEMP_ENCHANTMENT_SLOT, "use deadly/instant poison
// on main/off hand" via UseItemAction) has no MoP equivalent -- rogue poisons here are self-cast buff
// spells with no main-hand/off-hand distinction. The old ActionNode chain that hopped between the two
// item actions is dead with them (#149); see GenericRogueNonCombatStrategy::InitTriggers below for the
// replacement "often" -> "deadly poison" self-buff trigger.
class GenericRogueNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericRogueNonCombatStrategyActionNodeFactory()
    {
        // 5.4.8: "use deadly poison on off hand" was the old item-based off-hand poison action; removed (#149).
        // creators["use deadly poison on off hand"] = &use_deadly_poison_on_off_hand;
    }

private:
    // static ActionNode* use_deadly_poison_on_off_hand(PlayerbotAI* botAI)
    // {
    //     return new ActionNode("use deadly poison on off hand",
    //                           /*P*/ nullptr,
    //                           /*A*/ NextAction::array(0, new NextAction("use instant poison on off hand"), nullptr),
    //                           /*C*/ nullptr);
    // }
};

GenericRogueNonCombatStrategy::GenericRogueNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericRogueNonCombatStrategyActionNodeFactory());
}

void GenericRogueNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("player has flag",
                                       NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 2), nullptr)));
    // triggers.push_back(new TriggerNode("unstealth", NextAction::array(0, new NextAction("unstealth", 1.0f),
    // nullptr)));

    // 5.4.8: AC's poison mechanic is item-based weapon coatings applied per hand via TEMP_ENCHANTMENT_SLOT
    // ("main/off hand weapon no enchant" triggers -> "use instant/deadly poison on main/off hand" item
    // actions). MoP rogues instead keep poisons up as self-cast buff spells with no per-hand distinction,
    // so that whole mechanic is removed here (#149) and replaced by the single "often" -> "deadly poison"
    // trigger below.
    // triggers.push_back(
    //     new TriggerNode("main hand weapon no enchant",
    //                     NextAction::array(0, new NextAction("use instant poison on main hand", 20.0f), NULL)));

    // triggers.push_back(
    //     new TriggerNode("off hand weapon no enchant",
    //                     NextAction::array(0, new NextAction("use deadly poison on off hand", 19.0f), NULL)));

    // triggers.push_back(new TriggerNode(
    // 	"off hand weapon no enchant",
    // 	NextAction::array(0, new NextAction("use instant poison", 18.0f), NULL)));

    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("deadly poison", 20.0f), nullptr)));
    // #386 (AC GenericRogueNonCombatStrategy.cpp:29-37, second poison at 19.0f): 5.x non-lethal poison, Crippling 3408.
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("crippling poison", 19.0f), nullptr)));

    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("unstealth", 30.0f), NULL)));
}
