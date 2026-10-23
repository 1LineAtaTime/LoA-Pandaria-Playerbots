/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FrostMageStrategy.h"

#include "Playerbots.h"

class FrostMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["summon water elemental"] = &summon_water_elemental;
        creators["fingers of frost"] = &fingers_of_frost;
        creators["icy veins"] = &icy_veins;
        creators["frozen orb"] = &frozen_orb;
        creators["cold snap"] = &cold_snap;
        creators["deep freeze"] = &deep_freeze;
        creators["frost bomb"] = &frost_bomb;
        creators["frost bomb on attackers"] = &frost_bomb_on_attackers;
    }

private:
    static ActionNode* frostbolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbolt",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("frostfire bolt"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* summon_water_elemental([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon water elemental",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* fingers_of_frost([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fingers of frost",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* icy_veins([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy veins",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* frozen_orb([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frozen orb",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* cold_snap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cold snap",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* deep_freeze([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("deep freeze",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    // #790: Frost Bomb 112948, Nether Tempest 114923 and Living Bomb 44457 are the three choices of the same level-75
    // talent row ("Talent, Replaces Mage Bomb", Wowhead mop-classic tooltips, 2026-10-12), and Brain Freeze 44549 only
    // procs from "your most recently applied Nether Tempest, Living Bomb, or Frost Bomb". A frost bot that took Nether
    // Tempest or Living Bomb got no-spellid on every frost bomb edge and never applied a bomb (wave 35 SM Durand,
    // Haniano: 345 "frost bomb - IMPOSSIBLE", 0 Frostfire Bolt). Alternatives in AC's ActionNode style, like
    // "devastate" -> "sunder armor".
    static ActionNode* frost_bomb([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost bomb",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("nether tempest"), new NextAction("living bomb"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* frost_bomb_on_attackers([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost bomb on attackers",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("nether tempest on attackers"), new NextAction("living bomb on attackers"), nullptr),
            /*C*/ nullptr);
    }
};

FrostMageStrategy::FrostMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostMageStrategyActionNodeFactory());
}

NextAction** FrostMageStrategy::getDefaultActions()
{
    return NextAction::array(0,
        new NextAction("frostjaw", ACTION_DEFAULT + 0.10f),
        // Rotation fix lane B (#865): Frozen Orb (prio 0/11, "always", face target first).
        // Was only in the aoe strategy (0 casts at a single target, w73 frost leg); isUseful() 10 yd gate relaxed to 30 yd
        // in MageActions.cpp. Impossible on cooldown, so the chain falls through to Frostbolt.
        new NextAction("frozen orb", ACTION_DEFAULT + 0.95f),
        new NextAction("frostbolt", ACTION_DEFAULT + 0.9f),
        new NextAction("frost bomb", ACTION_DEFAULT + 0.8f),
        new NextAction("frostfire bolt", ACTION_DEFAULT + 0.7f),
        new NextAction("ice lance", ACTION_DEFAULT + 0.5f),
        new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void FrostMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("icy veins", NextAction::array(0, new NextAction("icy veins", 50.0f), nullptr)));

    // #163 S8 / #147 (AC parity, FrostMageStrategy.cpp:51): apply the configured stance to a freshly
    // summoned water elemental.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));

    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon water elemental", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("brain freeze", NextAction::array(0, new NextAction("frostfire bolt", ACTION_NORMAL + 3), nullptr)));
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("ice barrier", ACTION_NORMAL), nullptr)));
    // #385 (AC FrostMageStrategy.cpp:66-71): "being attacked" -> ice barrier. Ice Barrier is a 5.4.8
    // talent (11426, cls.py -> YES 11426/talent); the edge just fails when it is not talented.
    triggers.push_back(new TriggerNode("being attacked", NextAction::array(0, new NextAction("ice barrier", 29.0f), nullptr)));
    // Combo cast the last charge of fingers of frost for double crits.
    // Should only do this on the final charge of FoF.
    triggers.push_back(new TriggerNode("fingers of frost single",
        NextAction::array(0, new NextAction("deep freeze", ACTION_NORMAL + 2),
        new NextAction("ice lance", ACTION_NORMAL + 1), nullptr)));

    // Round 3 (#624, AC FrostMageStrategy.cpp:83-109): the AC proc/freeze edges the fork never had - "fingers of frost"
    // (any charge, aura 44544 / spec passive 112965 "Fingers of Frost", cls.py mage) -> Deep Freeze 44572 19, Frostbolt
    // 18; "frost nova on target" (Frost Nova 122 root applied) -> Deep Freeze 19, Frostbolt 18. AC "frostbite on target"
    // is not ported: Frostbite is NOT-LEARNABLE in 5.4.8 (cls.py mage, removed 5.0.4).
    // #790: AC's second action here is Frostbolt (WotLK FoF made the next 2 spells of any kind treat the target as
    // frozen). In 5.4.8 the effect "causes your next Ice Lance or Deep Freeze to act as if your target were frozen,
    // and increases Ice Lance damage by 25%" (Wowhead mop-classic tooltip 44544, 2026-10-12), and Frostbolt does not
    // consume it, so the bot sat on its procs and cast Frostbolt at 18 (wave 35 SM Thalnos, Haniano: 696 ticks with
    // FoF up, 2 Ice Lances). Mapped to the MoP consumer at AC's relevance.
    triggers.push_back(new TriggerNode("fingers of frost",
        NextAction::array(0, new NextAction("deep freeze", 19.0f), new NextAction("ice lance", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("frost nova on target",
        NextAction::array(0, new NextAction("deep freeze", 19.0f), new NextAction("frostbolt", 18.0f), nullptr)));

    // #521: "forst bomb" typo fixed (the registered key is now "frost bomb").
    triggers.push_back(new TriggerNode("frost bomb", NextAction::array(0, new NextAction("frost bomb", 19.0f), nullptr)));
}

void FrostMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
        new NextAction("frost bomb on attackers", 24.0f),
        new NextAction("frozen orb", 23.0f),
        new NextAction("blizzard", 22.0f), nullptr)));

    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0,
        new NextAction("frost bomb on attackers", 24.0f),
        new NextAction("cone of cold", 23.0f),
        new NextAction("frozen orb", 22.0f), nullptr)));
}
