/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MeleeShamanStrategy.h"

#include "Playerbots.h"

class MeleeShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    MeleeShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["magma totem"] = &magma_totem;
    }

private:
    static ActionNode* stormstrike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("stormstrike",
                              /*P*/ nullptr,
                              // /*A*/ NextAction::array(0, new NextAction("lava lash"), nullptr),
                              nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lava lash",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("magma totem",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("searing totem"), NULL),
                              /*C*/ NULL);
    }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new MeleeShamanStrategyActionNodeFactory());
}

NextAction** MeleeShamanStrategy::getDefaultActions()
{
    // #382 (AC EnhancementShamanStrategy.cpp:46-55): AC's defaults and order exactly: stormstrike 5.5, feral spirit
    // 5.4, earth shock 5.3, lava lash 5.2, melee 5.0. The fork-only "fire nova" default is dropped (AC casts Fire Nova
    // from the "aoe" strategy, "enemy within melee" 5.1, GenericShamanStrategy.cpp ShamanAoeStrategy).
    // Rotation lane 3 (#858): Elemental Blast (0/12 top of the spec list) and
    // Unleash Elements in front of AC's list; Primal Strike (levels 1-9, replaced by Stormstrike
    // later so unknown above that) just before the melee fallback.
    return NextAction::array(
        0, new NextAction("elemental blast", ACTION_DEFAULT + 0.6f),
        new NextAction("unleash elements", ACTION_DEFAULT + 0.55f),
        new NextAction("stormstrike", ACTION_DEFAULT + 0.5f), new NextAction("feral spirit", ACTION_DEFAULT + 0.4f),
        new NextAction("earth shock", ACTION_DEFAULT + 0.3f), new NextAction("lava lash", ACTION_DEFAULT + 0.2f),
        new NextAction("primal strike", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT), NULL);
}

void MeleeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // #382 (AC EnhancementShamanStrategy.cpp:54-61 "call of the elements and enemy within melee" 60): DIVERGENCE,
    // edge removed - AC's spell 66842 places the Totem Bar totems, the bar and the spell are gone for players in
    // 5.4.8 (wave 9: has=0 why=not-known x11; Wowhead mop-classic tooltip 66842 / 108285, 2026-10-05; the name now
    // resolves to the unrelated 5.x talent 108285, a cooldown reset). Trigger/action stay registered.
    // #427 (AC parity, AC EnhancementShamanStrategy.cpp:63-71): "spirit walk" edge.
    triggers.push_back(
        new TriggerNode("spirit walk ready", NextAction::array(0, new NextAction("spirit walk", 50.0f), nullptr)));
    // Rotation fix C (#858, wave 73): Elemental Mastery is a class-wide 5.4.8 haste cooldown, the MoP rotation casts it whenever
    // ready (enhancement and restoration, condition "always"); AC has it only in the elemental strategy.
    triggers.push_back(new TriggerNode("elemental mastery",
                                       NextAction::array(0, new NextAction("elemental mastery", 29.0f), nullptr)));

    // #382 (AC EnhancementShamanStrategy.cpp:46-130 has no imbue edges): the fork-only combat copies of
    // "main hand weapon no imbue -> windfury weapon main hand" / "off hand ... -> flametongue weapon off hand" are
    // removed; AC imbues out of combat only and ShamanNonCombatStrategy.cpp keeps both per-hand edges.
    // triggers.push_back(new TriggerNode("searing totem", NextAction::array(0, new NextAction("reach melee", 22.0f),
    // new NextAction("searing totem", 22.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 19.0f), nullptr)));  // #382 (AC EnhancementShamanStrategy.cpp:103: 19)
    // #427 (AC parity, AC EnhancementShamanStrategy.cpp:108-114).
    triggers.push_back(
        new TriggerNode("lightning shield", NextAction::array(0, new NextAction("lightning shield", 18.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("maelstrom weapon 4", NextAction::array(0, new NextAction("lightning bolt", 19.5f), nullptr)));  // #382 (AC EnhancementShamanStrategy.cpp:92: 19.5; was 25, above the 5-stack edge)
    // #427 (AC parity, AC EnhancementShamanStrategy.cpp:83-98): the 5-stack overload edge was missing.
    triggers.push_back(
        new TriggerNode("maelstrom weapon 5", NextAction::array(0, new NextAction("lightning bolt", 20.0f), nullptr)));

    // #382: the "medium aoe -> strength of earth totem" edge is removed - Strength of Earth Totem is NOT-LEARNABLE
    // in 5.4.8 (cls.py shaman, removed 5.0.4) and the earth slot has no same-role successor.
    // #382 (AC EnhancementShamanStrategy.cpp:73-79): reach melee 40.
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("reach melee", 40.0f), nullptr)));

    // #382: fork-only "no fire totem -> reach melee, magma totem" and "fire elemental totem -> fire elemental totem
    // melee 32" are removed (not in AC's enhancement strategy; AC casts Fire Elemental from the "boost" strategy at 24
    // and drops fire totems via the "totems"-style strategies, here "totems").

    // #382: the "no air totem -> windfury totem" edge is removed - Windfury Totem is NOT-LEARNABLE in 5.4.8
    // (removed 5.0.4; the Windfury effect is the Windfury Weapon imbue above) and there is no standing air buff totem.

    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("shamanistic rage", 23.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("shamanistic rage", 23.0f), nullptr)));

    // Rotation lane 3 (#858), MoP defensives at health <= 50 % (low health = config lowHealth): Healing Tide Totem,
    // Ancestral Guidance; Healing Surge on self at <= 30 % (critical health).
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("healing tide totem", 39.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("ancestral guidance", 38.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("healing surge", 37.0f), nullptr)));
}

void MeleeAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // triggers.push_back(new TriggerNode("magma totem", NextAction::array(0, new NextAction("magma totem", 26.0f),
    // nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fire nova", 25.0f), nullptr)));
}
