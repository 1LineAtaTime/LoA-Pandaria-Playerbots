/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericShamanStrategy.h"
#include "AiFactory.h"
#include "HealShamanStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

class GenericShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericShamanStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["windfury weapon"] = &windfury_weapon;
        // #382 (MoP-only): Lesser Healing Wave is NOT-LEARNABLE in 5.4.8 (cls.py shaman); Healing Surge (8004,
        // Wowhead mop-classic tooltip 8004) is the 5.x fast heal. The old Lesser nodes are kept registered but unused.
        creators["lesser healing wave"] = &lesser_healing_wave;
        creators["lesser healing wave on party"] = &lesser_healing_wave_on_party;
        creators["healing surge"] = &healing_surge;
        creators["healing surge on party"] = &healing_surge_on_party;
        // #382 (AC GenericShamanStrategy.cpp:12-60 per-hand imbue nodes, same fallbacks as AC).
        creators["flametongue weapon main hand"] = &flametongue_weapon_main_hand;
        creators["windfury weapon main hand"] = &windfury_weapon_main_hand;
        creators["earthliving weapon main hand"] = &earthliving_weapon_main_hand;
        creators["chain heal on party"] = &chain_heal;
        creators["riptide"] = &riptide;
        creators["riptide on party"] = &riptide_on_party;
        creators["earth shock"] = &earth_shock;
    }

private:
    static ActionNode* earth_shock([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("earth shock",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flame shock"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* flametongue_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("rockbiter weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* frostbrand_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbrand weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* windfury_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* lesser_healing_wave([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser healing wave",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* lesser_healing_wave_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser healing wave on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave on party"), nullptr),
                              /*C*/ nullptr);
    }

    // #382 (AC GenericShamanStrategy.cpp: LHW -> HW fallbacks, AC lines 14-32): Healing Surge replaces Lesser
    // Healing Wave. Healing Wave is a Restoration-only spell in 5.4.8 (cls.py shaman 'healing wave' spec264 only),
    // so the Healing Wave fallback only helps resto; it is kept to mirror AC.
    static ActionNode* healing_surge([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing surge",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* healing_surge_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing surge on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave on party"), nullptr),
                              /*C*/ nullptr);
    }

    // #382 (AC GenericShamanStrategy.cpp: per-hand imbue fallback nodes).
    static ActionNode* flametongue_weapon_main_hand([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue weapon main hand",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("rockbiter weapon main hand"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* windfury_weapon_main_hand([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury weapon main hand",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon main hand"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* earthliving_weapon_main_hand([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("earthliving weapon main hand",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon main hand"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* chain_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        // #382: AC falls back to lesser healing wave on party; that spell is gone in 5.4.8, Healing Surge replaces it.
        return new ActionNode("chain heal on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing surge on party"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* riptide([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("riptide",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* riptide_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("riptide on party",
                              /*P*/ nullptr,
                              // /*A*/ NextAction::array(0, new NextAction("healing wave on party"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

GenericShamanStrategy::GenericShamanStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericShamanStrategyActionNodeFactory());
}

void GenericShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, GenericShamanStrategy.cpp:121): apply the configured stance to a
    // freshly summoned elemental (fire/earth elemental totem, spirit wolves).
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 65.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("wind shear", NextAction::array(0, new NextAction("wind shear", 23.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("wind shear on enemy healer",
                        NextAction::array(0, new NextAction("wind shear on enemy healer", 23.0f), nullptr)));
    // Rotation lane 3 (#858), Wind Shear (on the victim and on ANY casting enemy in
    // range, priority group 10): beyond AC, battleground / arena only (trigger gated on "in pvp instance"), same 23 as
    // AC's target / enemy-healer edges above.
    triggers.push_back(new TriggerNode("pvp enemy caster in range",
                                       NextAction::array(0, new NextAction("wind shear on enemy caster", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("purge", NextAction::array(0, new NextAction("purge", ACTION_DISPEL), nullptr)));
    // triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("lesser
    // healing wave on party", 25.0f), nullptr))); triggers.push_back(new TriggerNode("party member low health",
    // NextAction::array(0, new NextAction("riptide on party", 25.0f), nullptr))); triggers.push_back(new
    // TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("chain heal on party", 27.0f), nullptr)));
    // triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("lesser healing
    // wave", 26.0f), nullptr))); triggers.push_back(new TriggerNode("low health", NextAction::array(0, new
    // NextAction("riptide", 26.0f), nullptr)));
    // #382 (AC GenericShamanStrategy.cpp:109-114): AC's generic shaman strategy has no heroism/bloodlust edge; both
    // are cast from the "boost" strategy at 30 (ShamanBoostStrategy below, AC :132-133). The fork-only copies at
    // 31/30 here are removed. Heroism 32182 / Bloodlust 2825 both exist in 5.4.8 (cls.py shaman, L70).
}

void ShamanBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("lightning shield", NextAction::array(0, new NextAction("lightning shield", 22.0f), nullptr)));
}

void ShamanBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 22.0f), nullptr)));
}

void ShamanCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #382 (AC GenericShamanStrategy.cpp:120-127): AC's cure strategy has poison/disease/curse edges on Cleanse
    // Spirit. In 5.4.8 Cleanse Spirit removes Curse effects only (Wowhead mop-classic tooltip 51886, 2026-10-04) and
    // Cure Poison / Cure Disease are NOT-LEARNABLE (cls.py shaman), so the poison and disease edges (and the MoP
    // "cure poison"/"cure disease" edges) are removed; only the curse edges remain.
    triggers.push_back(new TriggerNode("cleanse spirit curse",
                                       NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member cleanse spirit curse",
                        NextAction::array(0, new NextAction("cleanse spirit curse on party", 23.0f), nullptr)));

    // #388/#382 (MoP-only, AC pattern: AC keeps the self-cure edges in "cure" (AC GenericShamanStrategy.cpp:124-132),
    // which both engines carry, and gates spec-only edges on the tab (AC GenericShamanStrategy.cpp:139-146)):
    // Restoration's Purify Spirit (77130, SpecializationSpells spec 264) also removes Magic. HealShamanStrategy.cpp:94-97
    // has the magic edges too, but only in the combat "resto" strategy, which an ungrouped random resto bot runs as
    // "caster" (AiFactory, AC AiFactory.cpp:440-443), and never out of combat - w33 magic_resto: no purify trigger at all.
    if (AiFactory::GetPlayerSpecTab(botAI->GetBot()) == Specializations::SPEC_SHAMAN_RESTORATION)
    {
        triggers.push_back(
            new TriggerNode("purify spirit magic", NextAction::array(0, new NextAction("purify spirit", 24.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("party member purify spirit magic",
                            NextAction::array(0, new NextAction("purify spirit magic on party", 23.0f), nullptr)));
    }
}

void ShamanHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        NextAction::array(0, 
                            new NextAction("flame shock", ACTION_DEFAULT + 0.2f),
                            new NextAction("lava burst", ACTION_DEFAULT + 0.1f),
                            // Rotation lane 3 (#858), Resto Elemental Blast (0/3,
                            // between Lava Burst 0/2... and Lightning Bolt).
                            new NextAction("elemental blast", ACTION_DEFAULT + 0.05f),
                            new NextAction("lightning bolt", ACTION_DEFAULT), nullptr)));
    
    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        NextAction::array(0,
                            new NextAction("chain lightning", ACTION_DEFAULT + 0.3f), nullptr)));
}

// #408 (AC parity, AC GenericShamanStrategy.cpp:134-150): Boost strategy.
void ShamanBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("heroism", NextAction::array(0, new NextAction("heroism", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("bloodlust", NextAction::array(0, new NextAction("bloodlust", 30.0f), nullptr)));

    Player* bot = botAI->GetBot();
    Specializations tab = AiFactory::GetPlayerSpecTab(bot);

    // Rotation lane 3 (#858), Ancestral Swiftness (every spec, group 5 prio 3).
    triggers.push_back(new TriggerNode("ancestral swiftness",
                                       NextAction::array(0, new NextAction("ancestral swiftness", 22.0f), nullptr)));

    if (tab == Specializations::SPEC_SHAMAN_ELEMENTAL)
    {
        triggers.push_back(
            new TriggerNode("fire elemental totem",
                            NextAction::array(0, new NextAction("fire elemental totem", 23.0f), nullptr)));
        // Ascendance (group 5 prio 1).
        triggers.push_back(
            new TriggerNode("ascendance", NextAction::array(0, new NextAction("ascendance", 22.5f), nullptr)));
    }
    else if (tab == Specializations::SPEC_SHAMAN_ENHANCEMENT)
    {
        triggers.push_back(
            new TriggerNode("fire elemental totem",
                            NextAction::array(0, new NextAction("fire elemental totem melee", 24.0f), nullptr)));
        // Ascendance (group 5 prio 1).
        triggers.push_back(
            new TriggerNode("ascendance", NextAction::array(0, new NextAction("ascendance", 22.5f), nullptr)));
    }
}

// #408 (AC parity, AC GenericShamanStrategy.cpp:152-171): AoE strategy.
void ShamanAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    Specializations tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == Specializations::SPEC_SHAMAN_ELEMENTAL)
    {
        triggers.push_back(
            new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fire nova", 23.0f), nullptr)));
        // #382 (AC GenericShamanStrategy.cpp:161 "chain lightning no cd" 5.6): DIVERGENCE. In 5.4.8 the Elemental
        // passive Shamanism 62099 "removes the cooldown from Chain Lightning" (Wowhead mop-classic tooltip 62099,
        // 2026-10-05), so "no cd" is true every tick: wave 11 cast Chain Lightning x18 in 40 s on one dummy and never
        // reached Flame Shock 5.3 / Lava Burst 5.2 / Lightning Bolt 5.1. AC's WotLK 6 s cooldown is what made the edge
        // safe there. Gated on 2+ attackers (light aoe) instead - same priority 5.6, 5.x Chain Lightning hits 3 targets.
        triggers.push_back(new TriggerNode("light aoe",
                                           NextAction::array(0, new NextAction("chain lightning", 5.6f), nullptr)));
    }
    else if (tab == Specializations::SPEC_SHAMAN_ENHANCEMENT)
    {
        triggers.push_back(
            new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fire nova", 23.0f), nullptr)));
        triggers.push_back(new TriggerNode("maelstrom weapon 5 and medium aoe",
                                           NextAction::array(0, new NextAction("chain lightning", 22.0f), nullptr)));
        triggers.push_back(new TriggerNode("maelstrom weapon 4 and medium aoe",
                                           NextAction::array(0, new NextAction("chain lightning", 21.0f), nullptr)));
        triggers.push_back(new TriggerNode("enemy within melee",
                                           NextAction::array(0, new NextAction("fire nova", 5.1f), nullptr)));
    }
    // Resto AoE handled by "Healer Dps" Strategy
}
