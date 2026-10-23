/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"

#include "Playerbots.h"

// #431: AC's per-totem strategies test the bot's spellbook with bot->HasSpell(<WotLK rank-1 spell id>)
// (AC Strategy/TotemsShamanStrategy.cpp:13-19 SPELL_* constants). MoP re-numbered every shaman totem spell, so the
// same question is asked by name through the "spell id" value, which only resolves spells the bot actually knows.
static bool KnowsTotemSpell(PlayerbotAI* botAI, char const* spellName)
{
    return botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get() != 0;
}

// AC also registers "cleansing totem" as an ActionNode (Ai/Class/Shaman/Strategy/GenericShamanStrategy.cpp),
// falling back to Mana Spring Totem for low-level shamans who have not learned Cleansing Totem yet.
// #382: Mana Spring Totem was removed in 5.0.4 (cls.py shaman: NOT-LEARNABLE) and Cleansing Totem is not in 5.4.8
// at all (NO-SUCH-NAME); the water-slot totem a 5.4.8 shaman learns is Healing Stream Totem (5394, L30), so that is
// the fallback here.
// MoP has no shared/generic shaman ActionNodeFactory (each combat strategy file owns its own small one,
// e.g. Melee/Caster/HealShamanStrategy's magma/totem-of-wrath fallbacks), so this fallback lives here
// alongside the rest of the water-totem-slot logic.
class TotemsShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TotemsShamanStrategyActionNodeFactory() { creators["cleansing totem"] = &cleansing_totem; }

private:
    static ActionNode* cleansing_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cleansing totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing stream totem"), nullptr),
                              /*C*/ nullptr);
    }
};

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new TotemsShamanStrategyActionNodeFactory());
}

void TotemsShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // #382: the unified strategy's "no air totem -> wrath of air totem" edge is removed. Wrath of Air Totem (and
    // Windfury, Mana Spring, Strength of Earth below) were removed in 5.0.4 (cls.py shaman: NOT-LEARNABLE in
    // 5.4.8); the only air totems left are the utility ones (Grounding, Capacitor, Stormlash, Windwalk), none of
    // which is a standing buff totem, so nothing replaces it (use the "grounding" strategy to keep Grounding up).
    // Water slot: Mana Spring is gone, Healing Stream (5394) is the remaining baseline water totem.
    triggers.push_back(
        new TriggerNode("no water totem", NextAction::array(0, new NextAction("healing stream totem", 7.0f), nullptr)));

    // AC anchor: "no fire totem" trigger. Not previously wired up in this generic totem strategy (the
    // combat-spec strategies already have their own spec-appropriate "no fire totem" consumers).
    triggers.push_back(
        new TriggerNode("no fire totem", NextAction::array(0, new NextAction("searing totem", 7.0f),
                                                           new NextAction("magma totem", 6.0f), nullptr)));

    // #382: the "strength of earth totem" edge is removed - Strength of Earth Totem is NOT-LEARNABLE in 5.4.8
    // (removed 5.0.4) and the earth slot has no same-role successor (Wowhead mop-classic lists only Earthbind,
    // Tremor, Stone Bulwark, Earth Elemental there).
}


// #431 (AC parity, AC Strategy/TotemsShamanStrategy.cpp:27-183): per-totem combat strategies. Each one puts the
// totem on "set <totem>" (relevance 60) and casts it when its element slot is empty (relevance 55). AC keeps the
// totem bar for the first; MoP 5.4.8 has no totem bar (MAX_ACTION_BUTTONS = 132, Player.h:~414, so AC's bar slots
// 132-135 do not exist), so the "set" action recasts the totem into the element's summon slot instead
// (ShamanActions.cpp, SetTotemAction).
// Totem of Wrath, Cleansing, Wrath of Air and Windfury are learned late (or, on MoP, may not exist in the spellbook);
// until the bot knows them the strategy assigns the AC fallback totem so the trigger cannot fire continuously.

// Earth Totems
// #382: Strength of Earth and Stoneskin Totem are NOT-LEARNABLE in 5.4.8 (cls.py shaman, removed 5.0.4) and the earth
// slot has no same-role successor, so these two strategy names stay registered (#431) but add no totem edges.
StrengthOfEarthTotemStrategy::StrengthOfEarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StrengthOfEarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
}

StoneskinTotemStrategy::StoneskinTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StoneskinTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
}

EarthTotemStrategy::EarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set tremor totem", NextAction::array(0, new NextAction("set tremor totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("tremor totem", 55.0f), nullptr)));
}

EarthbindTotemStrategy::EarthbindTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthbindTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set earthbind totem", NextAction::array(0, new NextAction("set earthbind totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("earthbind totem", 55.0f), nullptr)));
}


// Fire Totems
SearingTotemStrategy::SearingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void SearingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set searing totem", NextAction::array(0, new NextAction("set searing totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("searing totem", 55.0f), nullptr)));
}

MagmaTotemStrategy::MagmaTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void MagmaTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set magma totem", NextAction::array(0, new NextAction("set magma totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("magma totem", 55.0f), nullptr)));
}

FlametongueTotemStrategy::FlametongueTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FlametongueTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Flametongue Totem is NOT-LEARNABLE in 5.4.8 (removed 5.0.4); the fire-slot baseline is Searing Totem
    // (3599, L16; Wowhead mop-classic tooltip 3599), which is also AC's own fire fallback.
    triggers.push_back(new TriggerNode("set searing totem", NextAction::array(0, new NextAction("set searing totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("searing totem", 55.0f), nullptr)));
}

TotemOfWrathStrategy::TotemOfWrathStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void TotemOfWrathStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Totem of Wrath and the Flametongue Totem fallback are both NOT-LEARNABLE in 5.4.8 (removed 5.0.4); the
    // fire-slot baseline is Searing Totem (3599, L16). AC's order is Totem of Wrath -> Flametongue (AC .cpp fallback).
    triggers.push_back(new TriggerNode("set searing totem", NextAction::array(0, new NextAction("set searing totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("searing totem", 55.0f), nullptr)));
}

FrostResistanceTotemStrategy::FrostResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FrostResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Frost Resistance Totem is NOT-LEARNABLE in 5.4.8 (removed 4.0.1, no successor): like Strength of Earth /
    // Stoneskin above, the strategy name stays registered (#431, AC key parity) but adds no totem edges (the action was only
    // rejected with no-spellid every tick; w76: 51 A: lines).
}


// Water Totems
HealingStreamTotemStrategy::HealingStreamTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void HealingStreamTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set healing stream totem", NextAction::array(0, new NextAction("set healing stream totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("healing stream totem", 55.0f), nullptr)));
}

ManaSpringTotemStrategy::ManaSpringTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void ManaSpringTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Mana Spring Totem is NOT-LEARNABLE in 5.4.8 (removed 5.0.4); the water-slot totem is Healing Stream (5394, L30).
    triggers.push_back(new TriggerNode("set healing stream totem", NextAction::array(0, new NextAction("set healing stream totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("healing stream totem", 55.0f), nullptr)));
}

CleansingTotemStrategy::CleansingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void CleansingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot has not learned Cleansing Totem yet, set the water-slot baseline instead (AC .cpp fallback).
    // #382: AC's baseline Mana Spring is NOT-LEARNABLE in 5.4.8, so the fallback is Healing Stream Totem.
    if (KnowsTotemSpell(botAI, "cleansing totem"))
        triggers.push_back(new TriggerNode("set cleansing totem", NextAction::array(0, new NextAction("set cleansing totem", 60.0f), nullptr)));
    else if (KnowsTotemSpell(botAI, "healing stream totem"))
        triggers.push_back(new TriggerNode("set healing stream totem", NextAction::array(0, new NextAction("set healing stream totem", 60.0f), nullptr)));
    // The action node (see the factory above) falls back to healing stream totem when cleansing totem is unknown.
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("cleansing totem", 55.0f), nullptr)));
}

FireResistanceTotemStrategy::FireResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FireResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Fire Resistance Totem is NOT-LEARNABLE in 5.4.8 (removed 4.0.1, no successor): like Strength of Earth /
    // Stoneskin above, the strategy name stays registered (#431, AC key parity) but adds no totem edges (the action was only
    // rejected with no-spellid every tick; w76: 51 A: lines).
}


// Air Totems
WrathOfAirTotemStrategy::WrathOfAirTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WrathOfAirTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot has not learned Wrath of Air Totem yet, set grounding totem instead (AC .cpp fallback).
    // #382: Wrath of Air Totem is NOT-LEARNABLE in 5.4.8 (removed 5.0.4), so the AC fallback is the only path.
    if (KnowsTotemSpell(botAI, "wrath of air totem"))
        triggers.push_back(new TriggerNode("set wrath of air totem", NextAction::array(0, new NextAction("set wrath of air totem", 60.0f), nullptr)));
    else if (KnowsTotemSpell(botAI, "grounding totem"))
        triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("grounding totem", 55.0f), nullptr)));
}

WindfuryTotemStrategy::WindfuryTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WindfuryTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot has not learned Windfury Totem yet, set grounding totem instead (AC .cpp fallback).
    // #382: Windfury Totem is NOT-LEARNABLE in 5.4.8 (removed 5.0.4), so the AC fallback is the only path.
    if (KnowsTotemSpell(botAI, "windfury totem"))
        triggers.push_back(new TriggerNode("set windfury totem", NextAction::array(0, new NextAction("set windfury totem", 60.0f), nullptr)));
    else if (KnowsTotemSpell(botAI, "grounding totem"))
        triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("grounding totem", 55.0f), nullptr)));
}

NatureResistanceTotemStrategy::NatureResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void NatureResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // #382: Nature Resistance Totem is NOT-LEARNABLE in 5.4.8 (removed 4.0.1, no successor): like Strength of Earth /
    // Stoneskin above, the strategy name stays registered (#431, AC key parity) but adds no totem edges (the action was only
    // rejected with no-spellid every tick; w76: 51 A: lines).
}

GroundingTotemStrategy::GroundingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void GroundingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("grounding totem", 55.0f), nullptr)));
}
