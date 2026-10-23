/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericMageStrategy.h"

#include "AiFactory.h"
#include "Playerbots.h"
#include "RangedCombatStrategy.h"

namespace
{
// #408 (AC parity, GenericMageStrategy.cpp:12-20). AC's WotLK-only constants are kept where the
// mechanic still exists on 5.4.8 and restated where it does not.
// #385 lane M correction: Frostfire Bolt (44614) IS a 5.4.8 mage spell (SkillLineAbility 25211, skill 904, level 1;
// the MoP mage rotations cast it). The frostfire branch below is inert because AC's second gate, the
// Ice Shards talent aura, is not a 5.4.8 talent (cls.py mage 'ice shards' -> NOT-LEARNABLE), so HasSpell && HasAura
// is never true. Still ported 1:1; "co +frostfire" runs FrostFireMageStrategy, whose Frostfire Bolt resolves.
// AC's Ice Shards constant is 15047, the WotLK rank. 5.4.8's Ice Shards talent is 93527
// (src/server/game/Spells/SpellMgr.cpp:4423), so the debuff check uses the 5.4.8 id.
constexpr uint32 SPELL_FROSTFIRE_BOLT = 44614;
constexpr uint32 SPELL_ICE_SHARDS = 93527;
// #385 (AC GenericMageStrategy.cpp:108-121): AC picks the best of six WotLK conjure-gem spells with
// HasSpell(). 5.4.8 has one Conjure Mana Gem (759, level 47; cls.py mage -> YES 759/skill/L47) that
// creates the single Mana Gem item 36799 (Wowhead mop-classic tooltip spell 759 / item 36799, 2026-10-04).
constexpr uint32 SPELL_CONJURE_MANA_GEM = 759;
}  // namespace

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["frost nova"] = &frost_nova;
        creators["fire blast"] = &fire_blast;
        creators["counterspell"] = &counterspell;
        creators["arcane explosion"] = &arcane_explosion;
        creators["blizzard"] = &blizzard;
        creators["ice lance"] = &ice_lance;
        creators["arcane missiles"] = &arcane_missiles;
        creators["cone of cold"] = &cone_of_cold;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["evocation"] = &evocation;
        creators["deep freeze"] = &deep_freeze;
        creators["blink"] = &blink;

        // talents generic define
        creators["rune of power"] = &rune_of_power;
        creators["ice barrier"] = &ice_barrier;
        creators["flameglow"] = &flameglow;
        creators["frostjaw"] = &frostjaw;
    }

private:
    static ActionNode* frostfire_bolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostfire bolt",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* ice_lance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice lance",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* deep_freeze([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("deep freeze",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("ice lance"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* fire_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fire blast",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* frost_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost nova",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* cone_of_cold([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cone of cold",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* evocation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("evocation",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* remove_curse([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("remove lesser curse"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("remove lesser curse on party"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* arcane_missiles([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("arcane missiles",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* blizzard([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("blizzard",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* counterspell([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("counterspell",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* arcane_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("arcane explosion",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* ice_barrier([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice barrier",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* flameglow([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flameglow",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* rune_of_power([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rune of power",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* blink([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("blink",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* frostjaw([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostjaw",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    // #385 (AC GenericMageStrategy.cpp:91-127 is the spec): two fork-only edges (upstream module dbcb9e52, not in AC)
    // are removed.
    // - "enemy is close" -> frost nova 25 / cone of cold 5 / dragon's breath 25 / blink 50: AC's only close-range edges
    //   are the firestarter-gated frost nova 50 / blink back 35 below. The fork edge fired the forward Blink
    //   (CastBlinkAction blinks toward the target) at 50 and kept frost nova live under "co +firestarter"
    //   (t521-rot-mage-edges w33: MoP fire_close/flee_melee "blink x1", AC none).
    // - "frost nova on target" -> deep freeze (ACTION_NORMAL + 1) for every spec: AC has it in the frost strategy only
    //   (AC FrostMageStrategy.cpp:101-108, MoP FrostMageStrategy.cpp:148-149 keeps it). Here the deep freeze node's
    //   ice lance alternative made fire/arcane mages cast Ice Lance (w33 fire_close: Ice Lance(30455) on MoP only).
    triggers.push_back(new TriggerNode("counterspell on enemy healer",NextAction::array(0, new NextAction("counterspell on enemy healer", 40.0f), nullptr)));
    // #385: AC GenericMageStrategy.cpp:100 ice block 90 (was 80), :97 invisibility 30 (was 60), :122 evocation 90
    // (was ACTION_EMERGENCY + 5 = 95). AC :96 "high threat" -> mirror image 60 is not ported: no "high threat" trigger is
    // registered on either tree (AC grep: only that line), so the edge is dead in AC too.
    // Lane 10 (#865): "mage ice block health" = the MoP rule (<= 10 % in a group) in BG/arena, AC's
    // "critical health" (this edge's original trigger) everywhere else.
    triggers.push_back(new TriggerNode("mage ice block health", NextAction::array(0, new NextAction("ice block", 90.0f), nullptr)));
    triggers.push_back(new TriggerNode("spellsteal", NextAction::array(0, new NextAction("spellsteal", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("mage medium threat", NextAction::array(0, new NextAction("invisibility", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("evocation", 90.0f), nullptr)));
    // #385: "fire ward" / "frost ward" edges removed. Fire Ward and Frost Ward do not
    // exist as learnable spells in 5.4.8 (cls.py mage -> NOT-LEARNABLE, DBC ids 15041/37844/83744 and
    // 15044/25641); the MoP "ward" talent Ice Ward (111264) is a Frost Nova effect, not a spell ward.
    // MoP-only (5.x level-90 talent, no AC counterpart): keep Rune of Power up.
    triggers.push_back(new TriggerNode("rune of power", NextAction::array(0, new NextAction("rune of power", 50.0f), nullptr)));
    // #385: the fork-only "mirror image" -> mirror image 41 edge is removed. AC casts Mirror Image from the "boost"
    // strategy only (AC GenericMageStrategy.cpp:144/152/157/164 -> MoP MageBoostStrategy below: arcane 28, fire 17.5,
    // frost 26), and "boost" is in every bot's default combat list (AiFactory.cpp:312).

    // #408 (AC parity, GenericMageStrategy.cpp:88-92). MoP registers the "no firestarter strategy"
    // trigger family in MageAiObjectContext.cpp but had no TriggerNode consuming it, so these AC
    // defensive nodes had nowhere to land.
    // #385 (AC GenericMageStrategy.cpp:99 "low health -> mana shield"): not ported. Mana Shield is not
    // learnable by a 5.4.8 mage (cls.py -> NOT-LEARNABLE) and no documented successor exists (Wowpedia
    // "Incanter's Ward" patch notes, read 2026-10-04: added in 5.0.4, no "replaces Mana Shield" note), so
    // re-mapping it would be invented behaviour.
    triggers.push_back(new TriggerNode("enemy is close and no firestarter strategy",
        NextAction::array(0, new NextAction("frost nova", 50.0f), nullptr)));
    triggers.push_back(new TriggerNode("enemy too close for spell and no firestarter strategy",
        NextAction::array(0, new NextAction("blink back", 35.0f), nullptr)));

    // Rotation lane 10 (#865). BG/arena ONLY, gated by the
    // class-local "mage pvp ..." triggers ("in pvp instance" + the lane-0 triggers); open world / dungeons keep the
    // AC edges above and the #385 forward-Blink removal stays (the kite uses AC's "blink back", never the forward blink).
    // - Temporal Shield (self, in combat; prio group 5, with Ice Barrier): on being attacked (all maps; MoP-only talent, no AC counterpart).
    triggers.push_back(new TriggerNode("being attacked", NextAction::array(0, new NextAction("temporal shield", 28.5f), nullptr)));
    // - Counterspell on each valid enemy around (group 10): any enemy caster, beside AC's healer edge (40).
    triggers.push_back(new TriggerNode("mage pvp enemy caster",
        NextAction::array(0, new NextAction("counterspell on enemy caster", ACTION_INTERRUPT + 1), nullptr)));
    // - Frost Nova (group 2, target <= 5 yd), then Blink (<= 3 yd, target is a player;
    //   AC "blink back" faces away first) and Blazing Speed (group 2): kite an enemy player in melee range.
    triggers.push_back(new TriggerNode("mage pvp melee enemy player", NextAction::array(0,
        new NextAction("frost nova", 50.0f),
        new NextAction("blink back", 35.0f),
        new NextAction("blazing speed", 34.0f), nullptr)));

    // #385 (AC Ai/Base/Strategy/RangedCombatStrategy.cpp:14-15, the base of AC's GenericMageStrategy):
    // "enemy too close for spell -> flee". MoP's generic RangedCombatStrategy.cpp:9 has this edge
    // commented out, so mages (arcane/frost/frostfire/fire) never got it; restored here, mage-only.
    // Rotation fix lane H (#868): "mage flee close" = this edge outside a pvp instance / duel (the MoP rotation never flees; it Novas, Blinks, casts).
    triggers.push_back(new TriggerNode("mage flee close",
        NextAction::array(0, new NextAction("flee", ACTION_MOVE + 4), nullptr)));

    // #385 (AC GenericMageStrategy.cpp:108-121): "high mana" -> use the mana gem. AC branches per gem
    // spell; 5.4.8 has the single Mana Gem (see SPELL_CONJURE_MANA_GEM above).
    if (botAI->GetBot()->HasSpell(SPELL_CONJURE_MANA_GEM))
        triggers.push_back(new TriggerNode("high mana", NextAction::array(0, new NextAction("use mana gem", 90.0f), nullptr)));
}

void MageCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("remove curse", NextAction::array(0, new NextAction("remove curse", 41.0f), nullptr)));
    triggers.push_back(new TriggerNode("remove curse on party", NextAction::array(0, new NextAction("remove curse on party", 40.0f), nullptr)));
}

void MageBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    Specializations tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == Specializations::SPEC_MAGE_ARCANE)
    {
        triggers.push_back(new TriggerNode("arcane power", NextAction::array(0, new NextAction("arcane power", 29.0f), nullptr)));
        // #602: AC's arcane "icy veins" 28.5 edge removed - Icy Veins 12472 is frost-only in 5.4.8 (cls.py mage -> spec64).
        triggers.push_back(new TriggerNode("mirror image", NextAction::array(0, new NextAction("mirror image", 28.0f), nullptr)));
    }
    else if (tab == Specializations::SPEC_MAGE_FIRE)
    {
        if (bot->HasSpell(SPELL_FROSTFIRE_BOLT) && bot->HasAura(SPELL_ICE_SHARDS))
        { // Frostfire
            triggers.push_back(new TriggerNode("combustion", NextAction::array(0, new NextAction("combustion", 18.0f), nullptr)));
            triggers.push_back(new TriggerNode("icy veins", NextAction::array(0, new NextAction("icy veins", 17.5f), nullptr)));
            triggers.push_back(new TriggerNode("mirror image", NextAction::array(0, new NextAction("mirror image", 17.0f), nullptr)));
        }
        else
        { // Fire
            triggers.push_back(new TriggerNode("combustion", NextAction::array(0, new NextAction("combustion", 18.0f), nullptr)));
            triggers.push_back(new TriggerNode("mirror image", NextAction::array(0, new NextAction("mirror image", 17.5f), nullptr)));
        }
    }
    else if (tab == Specializations::SPEC_MAGE_FROST)  // Frost
    {
        triggers.push_back(new TriggerNode("cold snap", NextAction::array(0, new NextAction("cold snap", 28.0f), nullptr)));
        triggers.push_back(new TriggerNode("icy veins", NextAction::array(0, new NextAction("icy veins", 27.5f), nullptr)));
        triggers.push_back(new TriggerNode("mirror image", NextAction::array(0, new NextAction("mirror image", 26.0f), nullptr)));
    }
}

void MageCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("polymorph", NextAction::array(0, new NextAction("polymorph", 30.0f), nullptr)));
}

void MageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("blizzard channel check", NextAction::array(0, new NextAction("cancel channel", 26.0f), nullptr)));

    Player* bot = botAI->GetBot();
    Specializations tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == Specializations::SPEC_MAGE_ARCANE)
    {
        triggers.push_back(
            new TriggerNode("flamestrike active and medium aoe", NextAction::array(0, new NextAction("blizzard", 24.0f), nullptr)));
        triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                             new NextAction("flamestrike", 23.0f),
                                                             new NextAction("blizzard", 22.0f), nullptr)));
    }
    else if (tab == Specializations::SPEC_MAGE_FIRE)
    {
        // #385 (AC GenericMageStrategy.cpp aoe fire "blast wave" 38.0): removed, Blast Wave is not a
        // learnable 5.4.8 mage spell (cls.py -> NOT-LEARNABLE; Wowhead mop-classic spell 11113 is only a
        // level-1 non-class entry, 2026-10-04).
        triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                   new NextAction("dragon's breath", 39.0f),
                                                   new NextAction("flamestrike", 23.0f),
                                                   new NextAction("blizzard", 22.0f), nullptr)));

        triggers.push_back(
            new TriggerNode("flamestrike active and medium aoe", NextAction::array(0, new NextAction("blizzard", 24.0f), nullptr)));
        triggers.push_back(new TriggerNode("firestarter", NextAction::array(0, new NextAction("flamestrike", 40.0f), nullptr)));
        triggers.push_back(new TriggerNode("living bomb on attackers",
            NextAction::array(0, new NextAction("living bomb on attackers", 21.0f), nullptr)));
    }
    else if (tab == Specializations::SPEC_MAGE_FROST)  // Frost
    {
        triggers.push_back(
            new TriggerNode("flamestrike active and medium aoe", NextAction::array(0, new NextAction("blizzard", 24.0f), nullptr)));
        triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                             new NextAction("flamestrike", 23.0f),
                                                             new NextAction("blizzard", 22.0f), nullptr)));
        triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("cone of cold", 21.0f), nullptr)));
    }
}
