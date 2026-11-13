/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MonkStrategies.h"

#include "Playerbots.h"

// Shared filler used by all three specs. The first action that is possible (on cooldown, resource,
// range) wins; the generic spell machinery resolves each name to a spell id and skips what the bot
// cannot cast, so an unlearned ability simply falls through to the next one.
NextAction** MonkStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("tiger palm", ACTION_DEFAULT + 0.2f),
                             new NextAction("jab", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT),
                             nullptr);
}

// Windwalker: rising sun kick and fists of fury are the priority damage, blackout kick the filler.
// #521 w13: Tiger Palm (1 Chi) is the Tiger Power upkeep (CastTigerPalmAction::isUseful: buff missing or < 4 s), and the
// 5.4.8 Windwalker priority puts that refresh right after Rising Sun Kick, ahead of Fists of Fury / Blackout Kick. At +0.2
// it was only reached after Blackout Kick (2 Chi) had spent the Chi each Jab built (the trail shows Jab -> Rising Sun Kick, 2 Chi), so every Tiger
// Palm failed NO_POWER (88) and Tiger Power was never applied (wave 13: 12 casts, 12 x result 88).
NextAction** WindwalkerMonkStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("rising sun kick", ACTION_DEFAULT + 0.6f),
                             new NextAction("tiger palm", ACTION_DEFAULT + 0.55f),
                             new NextAction("fists of fury", ACTION_DEFAULT + 0.5f),
                             new NextAction("blackout kick", ACTION_DEFAULT + 0.4f),
                             new NextAction("chi wave", ACTION_DEFAULT + 0.3f),
                             new NextAction("jab", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT),
                             nullptr);
}

// Brewmaster: keep keg smash on cooldown, guard for mitigation, breath of fire for threat.
NextAction** BrewmasterMonkStrategy::getDefaultActions()
{
    // #526 (5.4.8 Brewmaster, Wowhead tooltips 2026-10-05): Guard 115295 (2 Chi), Purifying Brew 119582 only under
    // Moderate/Heavy Stagger (CastPurifyingBrewAction::isUseful), Shuffle upkeep via Blackout Kick 100784
    // (CastShuffleAction), Keg Smash 121253 (builder, 2 Chi), Breath of Fire 115181, Expel Harm 115072,
    // Tiger Palm 100787 only for Tiger Power upkeep (CastTigerPalmAction::isUseful), Jab 100780 (builder) filler.
    // Chi cost is enforced by the spell check itself (checkcast NO_POWER falls through to the next edge).
    return NextAction::array(0, new NextAction("guard spell", ACTION_DEFAULT + 0.7f),
                             new NextAction("purifying brew", ACTION_DEFAULT + 0.6f),
                             new NextAction("shuffle", ACTION_DEFAULT + 0.55f),
                             new NextAction("keg smash", ACTION_DEFAULT + 0.5f),
                             new NextAction("breath of fire", ACTION_DEFAULT + 0.4f),
                             new NextAction("tiger palm", ACTION_DEFAULT + 0.3f),
                             new NextAction("expel harm", ACTION_DEFAULT + 0.2f),
                             new NextAction("jab", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT),
                             nullptr);
}

// Mistweaver: renewing mist rolling, then the direct heals. Fists of fury is left to Windwalker.
NextAction** MistweaverMonkStrategy::getDefaultActions()
{
    // rot-fix-F: MoP rotation row 27016 Chi Wave on the current target (group 2, above the Renewing Mist / Soothing Mist rows);
    // 8 s cooldown so it takes one GCD per cooldown. The old default list had no Chi Wave at all (115098: 0 casts).
    return NextAction::array(0, new NextAction("chi wave target", ACTION_DEFAULT + 0.6f),
                             new NextAction("renewing mist", ACTION_DEFAULT + 0.5f),
                             new NextAction("enveloping mist", ACTION_DEFAULT + 0.4f),
                             new NextAction("surging mist", ACTION_DEFAULT + 0.3f),
                             new NextAction("soothing mist", ACTION_DEFAULT + 0.2f),
                             new NextAction("uplift", ACTION_DEFAULT + 0.1f),
                             // #526: solo / nothing-to-heal fallback in Wise Serpent: Jab 100780 (8% mana,
                             // 5.4.8 Wowhead 2026-10-05) before the auto attack, AC healer shape ("shoot"/"smite"
                             // below the heals, AC HolyPriestStrategy.cpp:35,86).
                             new NextAction("jab", ACTION_DEFAULT + 0.05f),
                             new NextAction("melee", ACTION_DEFAULT), nullptr);
}

// #526 (wave 11): Spinning Crane Kick was a default at ACTION_DEFAULT + 0.3 whenever "aoe" / "melee aoe" was on, so a bot on a
// single dummy channelled it every GCD (why=channeling x34, 40 Energy each, Wowhead mop-classic tooltip 101546 2026-10-05:
// "Generates 1 Chi, if it hits at least 3 targets"), never banked Chi and cast no Tiger Palm / Blackout Kick. AC aoe edges are
// "medium aoe" trigger edges (>= 3 attackers within 8 yd, MediumAoeTrigger) at HIGH-band priorities, e.g. AC DpsPaladinStrategy.cpp
// :150-159 "medium aoe" -> divine storm ACTION_HIGH + 4 / consecration ACTION_HIGH + 3. Same shape here.
void MonkAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        // #862 MoP rotation row 1002 (group 3): Rushing Jade Wind when >= 3 enemies within 10 yd.
        "medium aoe", NextAction::array(0, new NextAction("rushing jade wind", ACTION_HIGH + 5),
                                        new NextAction("spinning crane kick", ACTION_HIGH + 4),
                                        new NextAction("breath of fire", ACTION_HIGH + 3), nullptr)));
}

// #526 (AC Ai/Class/Paladin/Strategy/GenericPaladinStrategy.cpp PaladinHealerDpsStrategy / AC GenericPriestStrategy.cpp
// PriestHealerDpsStrategy "healer should attack" -> damage spells at ACTION_DEFAULT + 0.x). AiFactory adds "healer dps" for every
// healer spec but the monk context never registered it (silent no-op), and Mistweaver's melee Jab is out of range from a healer's
// position (jab USELESS x24 in wave 11). Chi builders/spenders (Blackout Kick 100784 2 Chi, Tiger Palm 100787 upkeep, Jab 100780
// generates 1 Chi; Wowhead mop-classic tooltips 2026-10-05), then "reach melee" when none is in range.
void MonkHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "healer should attack", NextAction::array(0, new NextAction("blackout kick", ACTION_DEFAULT + 0.4f),
                                                  new NextAction("tiger palm", ACTION_DEFAULT + 0.3f),
                                                  new NextAction("jab", ACTION_DEFAULT + 0.2f),
                                                  new NextAction("reach melee", ACTION_DEFAULT + 0.1f), nullptr)));
}

// #140: MonkStrategy is now a CombatStrategy, so every spec (Mistweaver too) gets the base combat
// triggers from CombatStrategy::InitTriggers; the inline copies added for #129 are removed. #129's
// "enemy out of melee" -> "reach melee" stays in the two melee specs, as DpsPaladinStrategy/DpsRogueStrategy
// push it after calling their generic strategy's InitTriggers.
void MonkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // #526 (AC Ai/Class/Rogue/Strategy/AssassinationRogueStrategy.cpp:172-183 "kick" /
    // "kick on enemy healer"; MoP DpsRogueStrategy.cpp:121-125): Spear Hand Strike 116705 (skill L32).
    triggers.push_back(new TriggerNode(
        "spear hand strike", NextAction::array(0, new NextAction("spear hand strike", ACTION_INTERRUPT + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "spear hand strike on enemy healer",
        NextAction::array(0, new NextAction("spear hand strike on enemy healer", ACTION_INTERRUPT + 1), nullptr)));

    // #526 (AC Ai/Class/Dk/Strategy/GenericDKStrategy.cpp:68 "low health" -> "icebound fortitude"
    // ACTION_HIGH + 5): Fortifying Brew 115203 (skill L24, -20% damage taken).
    triggers.push_back(new TriggerNode(
        "low health", NextAction::array(0, new NextAction("fortifying brew", ACTION_HIGH + 5), nullptr)));

    // #862 (epic #854), MoP monk rotations (all three specs share the class rows).
    // MoP rotation row 18/19 (class, group 10): Spear Hand Strike on ANY enemy caster, not only the target /
    // the healer. Trigger + value are lane 0 (#855).
    triggers.push_back(new TriggerNode(
        "enemy caster in range",
        NextAction::array(0, new NextAction("spear hand strike on enemy caster", ACTION_INTERRUPT), nullptr)));
    // MoP rotation row 1003 (class, group 3): Invoke Xuen, the White Tiger (talent).
    triggers.push_back(new TriggerNode(
        "invoke xuen", NextAction::array(0, new NextAction("invoke xuen", ACTION_HIGH), nullptr)));
    // MoP rotation row 1001 (class, group 2): Chi Brew at Chi <= 2 (Windwalker also at Energy <= 30%, 26915).
    triggers.push_back(new TriggerNode(
        "chi brew", NextAction::array(0, new NextAction("chi brew", ACTION_HIGH + 1), nullptr)));
    // MoP rotation row 1000 (class, group 2): Zen Sphere on injured friendlies <= 95% ("almost full health" is the
    // closest AC party trigger; same edge Renewing Mist uses for Mistweaver below).
    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("zen sphere on party", ACTION_LIGHT_HEAL + 4), nullptr)));
    // MoP rotation row 26816 / 26917 / 27007 (BM/WW/MW, groups 5 and 1): Expel Harm at health <= 90%.
    triggers.push_back(new TriggerNode(
        "almost full health", NextAction::array(0, new NextAction("expel harm", ACTION_NORMAL), nullptr)));
}

void WindwalkerMonkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MonkStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));

    // #526 (AC Ai/Class/Warrior/Strategy/ArmsWarriorStrategy.cpp:100-105 "battle stance" ACTION_HIGH + 10 /
    // FuryWarriorStrategy.cpp:92-94 ACTION_HIGH + 9): the spec's own stance, re-applied whenever it is missing.
    // Stance of the Fierce Tiger 103985 (skill L1); every monk ability but Touch of Death needs a stance.
    triggers.push_back(new TriggerNode(
        "stance of the fierce tiger",
        NextAction::array(0, new NextAction("stance of the fierce tiger", ACTION_HIGH + 9), nullptr)));

    // #862 MoP Windwalker group 5/0 rows. Touch of Karma 122470 (26900, group 0 prio 20, "always"), Elusive Brew
    // at >= 7 stacks (26916), Energizing Brew (26914, gated on the existing "low energy" trigger).
    triggers.push_back(new TriggerNode(
        "touch of karma", NextAction::array(0, new NextAction("touch of karma", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "elusive brew", NextAction::array(0, new NextAction("elusive brew", ACTION_HIGH + 3), nullptr)));
    // rot-fix-F: was the generic "low energy" (< 30) edge, never reached in a Windwalker fight (Energy stays >= 60);
    // "energizing brew" is a class-local trigger (MonkActions.h EnergizingBrewTrigger: Energy <= 75% and castable).
    triggers.push_back(new TriggerNode(
        "energizing brew", NextAction::array(0, new NextAction("energizing brew", ACTION_HIGH + 2), nullptr)));
    // MoP rotation row 1010 Roll (gap closer), PvP instances only (the action's isUseful gates on "in pvp instance").
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("roll gap", ACTION_HIGH + 2), nullptr)));
}

void BrewmasterMonkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MonkStrategy::InitTriggers(triggers);

    // #526 (AC TankWarriorStrategy.cpp:154-156 "defensive stance" ACTION_HIGH + 9): Stance of the Sturdy Ox 115069
    // (spec 268 L10: -25% damage taken, Stagger).
    triggers.push_back(new TriggerNode(
        "stance of the sturdy ox",
        NextAction::array(0, new NextAction("stance of the sturdy ox", ACTION_HIGH + 9), nullptr)));

    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));

    // #526 (AC Ai/Class/Warrior/Strategy/TankWarriorStrategy.cpp:219-229, MoP TankWarriorStrategy.cpp:73-76):
    // "lose aggro" -> taunt. Provoke 115546 (skill L14; 40 yd, 8 s cd).
    triggers.push_back(new TriggerNode(
        "lose aggro", NextAction::array(0, new NextAction("provoke", ACTION_INTERRUPT + 1), nullptr)));
    // #862 MoP Brewmaster group 5 rows: Dampen Harm (26815, health <= 40%), Elusive Brew at >= 7 stacks (26809),
    // Summon Black Ox Statue (26812), Roll as the PvP gap closer (1010). Fortifying Brew (26813, <= 40%), Guard (26814),
    // Purifying Brew (26810/26811/26817) and Expel Harm (26816) were already wired.
    triggers.push_back(new TriggerNode(
        "low health", NextAction::array(0, new NextAction("dampen harm", ACTION_HIGH + 4), nullptr)));
    triggers.push_back(new TriggerNode(
        "elusive brew", NextAction::array(0, new NextAction("elusive brew", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "black ox statue", NextAction::array(0, new NextAction("summon black ox statue", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("roll gap", ACTION_HIGH + 2), nullptr)));

    // #835 (AC TankWarriorStrategy.cpp:225-232, :36-44): AC has no direct "taunt on snare target" -> taunt edge; its
    // only snare-target edge is a ranged pull (Heroic Throw, ACTION_INTERRUPT) with Taunt as that node's fallback.
    // The fork-only direct "provoke on snare target" -> Provoke edge (ACTION_INTERRUPT + 1) is removed, like the warrior
    // one: since the #2855 port enemy players are attackers, so "snare target" can be a player and the bot taunted
    // players in BGs. A monk has no Heroic Throw equivalent at L14 (no ranged pull/snare), so nothing replaces it; the
    // trigger and action stay registered (MonkAiObjectContext.cpp:81,197).
}

// #163 S8 / #194 (MoP-only group heals; AC has no monk, so this is designed in AC's healer pattern -
// see HolyPriestStrategy.cpp/HealDruidStrategy.cpp for the "medium group heal setting"/"party member
// critical health" shape this mirrors). Only the group-heal actions this issue asks for are wired;
// Mistweaver's single-target kit (soothing/surging mist, resource management beyond Chi/mana) has no
// triggers at all yet - a pre-existing gap outside this pass's scope.
void MistweaverMonkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #140: CombatStrategy base triggers + the shared monk edges (Mistweaver previously had none).
    MonkStrategy::InitTriggers(triggers);

    // #526 (same AC warrior stance edge, ACTION_HIGH + 9): Stance of the Wise Serpent 115070 (spec 270 L10;
    // Mistweaver's heals and Jab/Tiger Palm/Blackout Kick all require a stance, wave 10 result=97).
    triggers.push_back(new TriggerNode(
        "stance of the wise serpent",
        NextAction::array(0, new NextAction("stance of the wise serpent", ACTION_HIGH + 9), nullptr)));

    // AC-pattern ladder per research (s8-pet-control-and-class-spells.md B.7), scoped to the group
    // heals #194 asks for (Revival/Life Cocoon/on-party variants) plus the closest AC analogues
    // ("medium mana" -> mana tea, "party member to heal out of spell range" -> reach). #194 completes
    // this with Thunder Focus Tea (empowers the next Surging Mist/Uplift) and the Chi-gated
    // "uplift on party" (needs the "chi" value, StatsValues.h/.cpp); Uplift's own self-cast (already
    // registered) still heals every Renewing Mist target by spell design.
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10), nullptr)));

    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        NextAction::array(0, new NextAction("revival", ACTION_CRITICAL_HEAL + 7),
                          new NextAction("thunder focus tea", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("uplift", ACTION_CRITICAL_HEAL + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "group heal setting",
        NextAction::array(0, new NextAction("renewing mist on party", ACTION_MEDIUM_HEAL + 8),
                          new NextAction("uplift on party", ACTION_MEDIUM_HEAL + 7), nullptr)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("life cocoon on party", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("life cocoon on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("surging mist on party", ACTION_CRITICAL_HEAL + 3),
                          new NextAction("enveloping mist on party", ACTION_CRITICAL_HEAL + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("renewing mist on party", ACTION_MEDIUM_HEAL + 4),
                          new NextAction("enveloping mist on party", ACTION_MEDIUM_HEAL + 3),
                          new NextAction("surging mist on party", ACTION_MEDIUM_HEAL + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("renewing mist on party", ACTION_LIGHT_HEAL + 9),
                          new NextAction("surging mist on party", ACTION_LIGHT_HEAL + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("renewing mist on party", ACTION_LIGHT_HEAL + 3), nullptr)));

    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("mana tea", ACTION_HIGH), nullptr)));

    // #862 MoP Mistweaver rows: Summon Jade Serpent Statue (27006, group 2), Chi Wave heal on injured friendlies
    // (27017, target <= 80%; "party member medium health" is the AC trigger nearest that threshold). Mana Tea (27013)
    // now needs >= 2 stacks (CastManaTeaAction::isUseful); Revival <= 25% (27015) / Life Cocoon (27014) were wired.
    triggers.push_back(new TriggerNode(
        "jade serpent statue",
        NextAction::array(0, new NextAction("summon jade serpent statue", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("chi wave on party", ACTION_LIGHT_HEAL + 7), nullptr)));
}

void GenericMonkNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #526: keep the spec stance out of combat too (AC Druid CatDruidStrategy.cpp:124-126 keeps "cat form" up
    // outside combat; AC has no stance nc node for warriors/DKs). Spec picks the stance; unspecced monks get Tiger.
    {
        char const* stance = "stance of the fierce tiger";
        Specializations spec = botAI->GetBot()->GetSpecialization();
        if (spec == Specializations::SPEC_MONK_MISTWEAVER)
            stance = "stance of the wise serpent";
        else if (spec == Specializations::SPEC_MONK_BREWMASTER)
            stance = "stance of the sturdy ox";
        triggers.push_back(new TriggerNode(stance, NextAction::array(0, new NextAction(stance, 22.0f), nullptr)));
    }

    // #526 (AC Ai/Class/Mage/Strategy/GenericMageNonCombatStrategy.cpp:38,56 "arcane intellect" 21.0f /
    // "arcane intellect on party" 20.0f; MoP GenericMageNonCombatStrategy.cpp:55,78): party buffs out of
    // combat. Legacy of the Emperor 115921 (skill L22, all specs) and Legacy of the White Tiger 116781
    // (Windwalker spec L81; unlearned for the other specs, so the action is simply not possible).
    triggers.push_back(new TriggerNode(
        "legacy of the emperor", NextAction::array(0, new NextAction("legacy of the emperor", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "legacy of the emperor on party",
        NextAction::array(0, new NextAction("legacy of the emperor on party", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "legacy of the white tiger",
        NextAction::array(0, new NextAction("legacy of the white tiger", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "legacy of the white tiger on party",
        NextAction::array(0, new NextAction("legacy of the white tiger on party", 18.0f), nullptr)));
}

// #388 (AC Ai/Class/Paladin/Strategy/GenericPaladinStrategy.cpp:38-55 PaladinCureStrategy; MoP
// GenericPaladinStrategy.cpp:51-66): self + party cure triggers. Detox 115450 removes Poison and Disease
// (Wowhead tooltip 2026-10-04); Magic only with Mistweaver's Internal Medicine, so the magic edges are
// added for Mistweaver only (otherwise the edge would fire on every magic debuff and fail to cast).
void MonkCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "detox cure disease", NextAction::array(0, new NextAction("detox", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "detox party member cure disease",
        NextAction::array(0, new NextAction("detox disease on party", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "detox cure poison", NextAction::array(0, new NextAction("detox", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "detox party member cure poison",
        NextAction::array(0, new NextAction("detox poison on party", ACTION_DISPEL + 1), nullptr)));

    if (botAI->GetBot()->GetSpecialization() == Specializations::SPEC_MONK_MISTWEAVER)
    {
        triggers.push_back(new TriggerNode(
            "detox cure magic", NextAction::array(0, new NextAction("detox", ACTION_DISPEL + 2), nullptr)));
        triggers.push_back(new TriggerNode(
            "detox party member cure magic",
            NextAction::array(0, new NextAction("detox magic on party", ACTION_DISPEL + 1), nullptr)));
    }
}

// #526 (AC Ai/Class/Rogue/Strategy/CombatRogueStrategy.cpp:373-375 "adrenaline rush" boost node):
// Tigereye Brew 116740 (Windwalker L56) and Touch of Death 115080 (L22) per the Wowhead tooltips.
void MonkBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "tigereye brew", NextAction::array(0, new NextAction("tigereye brew", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "touch of death", NextAction::array(0, new NextAction("touch of death", ACTION_HIGH + 3), nullptr)));
}

// #526 (AC Ai/Class/Rogue/Strategy/CombatRogueStrategy.cpp:394-397 "sap"): Paralysis 115078 (skill L44).
void MonkCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "paralysis", NextAction::array(0, new NextAction("paralysis", ACTION_INTERRUPT), nullptr)));
}

// #526 (AC Ai/Class/Paladin/Strategy/PaladinPullStrategy.cpp:11-27): the tank pulls with its ranged
// taunt. Provoke 115546 (skill L14, 40 yd). Same shape as MoP WarriorPullStrategy.cpp:14-27: the action
// must also be registered because PullAction dispatches the name through DoSpecificAction.
std::string MonkPullStrategy::GetPullActionName() const
{
    Unit* target = GetTarget();
    if (!target ||
        (!botAI->HasStrategy("tank", BOT_STATE_COMBAT) && !botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT)))
        return PullStrategy::GetPullActionName();

    if (botAI->CanCastSpell("provoke", target) && botAI->GetAiObjectContext()->GetAction("provoke"))
        return "provoke";

    return PullStrategy::GetPullActionName();
}
