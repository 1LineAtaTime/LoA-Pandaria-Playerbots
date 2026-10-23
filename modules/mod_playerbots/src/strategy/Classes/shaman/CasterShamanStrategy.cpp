/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CasterShamanStrategy.h"

#include "Playerbots.h"

class CasterShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterShamanStrategyActionNodeFactory()
    {
        creators["magma totem"] = &magma_totem;
    }

private:
    static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("magma totem",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("fire nova"), nullptr));
    }
    // #382: the "totem of wrath" -> "flametongue totem" node is removed, both totems are NOT-LEARNABLE in 5.4.8
    // (cls.py shaman, removed 5.0.4); the fire-slot edge below now casts Searing Totem directly.
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new CasterShamanStrategyActionNodeFactory());
}

NextAction** CasterShamanStrategy::getDefaultActions()
{
    // Rotation lane 3 (#858): Unleash Elements (group 5 so before the filler) and
    // Elemental Blast (0/3, right below Lava Burst 0/4) in MoP order. Both are cooldown spells, so when
    // they are not castable the engine falls through to the next entry (AC defaults pattern).
    return NextAction::array(0, new NextAction("unleash elements", ACTION_DEFAULT + 0.25f),
                             new NextAction("lava burst", ACTION_DEFAULT + 0.2f),
                             new NextAction("elemental blast", ACTION_DEFAULT + 0.15f),
                             new NextAction("lightning bolt", ACTION_DEFAULT + 0.1f),
                            //  new NextAction("earth shock", ACTION_DEFAULT), // cast during movement
                             nullptr);
}

void CasterShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    
    // #382 (AC ElementalShamanStrategy.cpp:29-35 "call of the elements" 60): DIVERGENCE, edge removed. AC's spell is
    // 66842 "Simultaneously places up to 4 totems specified in the Totem Bar" (Wowhead mop-classic tooltip 66842,
    // 2026-10-05); the totem bar was removed in 5.0.4 and no player knows 66842 in 5.4.8 (wave 9: has=0 why=not-known
    // x7-11 per leg). The name now resolves to the 5.x talent 108285 (3 min cooldown reset of short totems, tooltip
    // 108285), which is a different effect, so nothing replaces it. The "call of the elements" trigger/action stay registered.
    // #382: Stoneclaw Totem no longer exists in 5.4.8 (cls.py shaman: NO-SUCH-NAME); Stone Bulwark Totem (108270,
    // Wowhead mop-classic tooltip 108270, 2026-10-04) is the earth-slot absorb-shield totem that replaces it.
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("stone bulwark totem", 40.0f), nullptr)));
    // Rotation lane 3 (#858), MoP defensives at health <= 50 % (low health = config lowHealth): Healing Tide Totem,
    // Ancestral Guidance; Healing Surge on self at <= 30 % (critical health).
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("healing tide totem", 39.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("ancestral guidance", 38.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("healing surge", 37.0f), nullptr)));
    // Rotation lane 3 (#858), Earthquake (the top of the spec list: >= 4 enemies
    // within 8 yd of the target). "high aoe" is AC's >= 4 attackers trigger; priority above Earth Shock execute 5.5.
    triggers.push_back(
        new TriggerNode("high aoe", NextAction::array(0, new NextAction("earthquake", 5.8f), nullptr)));
    // #427 (AC parity, AC ElementalShamanStrategy.cpp:57-65).
    triggers.push_back(
        new TriggerNode("earth shock execute", NextAction::array(0, new NextAction("earth shock", 5.5f), nullptr)));

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell",
    // ACTION_NORMAL + 9), nullptr))); triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new
    // NextAction("flametongue weapon", 23.0f), nullptr)));
    // #382 (AC ElementalShamanStrategy.cpp:21-103 has none of these, rotdiff "MoP-only triggers"): the fork-only
    // edges "enough mana -> chain lightning" (ACTION_DEFAULT+0.1 = 5.1 > Lightning Bolt 5.0, so single-target
    // Lightning Bolt could never be reached), "main hand weapon no imbue" (AC imbues out of combat only, see
    // ShamanNonCombatStrategy.cpp), "frost shock snare", "no fire totem -> searing totem" (AC has no totem edge in
    // the elemental strategy; the "totems" strategy owns the fire slot), "fire elemental totem" (AC casts it from the
    // "boost" strategy at 23, GenericShamanStrategy.cpp ShamanBoostStrategy), "medium mana -> thunderstorm" and
    // "enemy too close for spell -> flee" are removed. AC's chain lightning comes from the "aoe" strategy
    // ("chain lightning no cd" 5.6).

    // #382 (AC ElementalShamanStrategy.cpp:44-52, 57-65): elemental mastery 29, flame shock 5.3 (below earth shock
    // execute 5.5, above Lava Burst 5.2 / Lightning Bolt 5.0).
    triggers.push_back(
        new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 5.3f), nullptr)));
    triggers.push_back(new TriggerNode("elemental mastery",
                                       NextAction::array(0, new NextAction("elemental mastery", 29.0f), nullptr)));

    // #427 (AC parity, AC ElementalShamanStrategy.cpp:75-91).
    triggers.push_back(
        new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 19.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("high mana", NextAction::array(0, new NextAction("thunderstorm", 19.0f), nullptr)));

    // #382 (AC ElementalShamanStrategy.cpp:96-102): thunderstorm 19 (was ACTION_HIGH+1).
    triggers.push_back(new TriggerNode("enemy is close",
                                       NextAction::array(0, new NextAction("thunderstorm", 19.0f), nullptr)));
}

void CasterAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("light aoe", NextAction::array(0, new NextAction("chain lightning", 25.0f), nullptr)));
}
