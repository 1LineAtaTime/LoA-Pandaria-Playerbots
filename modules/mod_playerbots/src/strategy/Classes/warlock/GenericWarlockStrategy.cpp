/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarlockStrategy.h"

#include "Playerbots.h"

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["corruption"] = &corruption;
        creators["curse of enfeeblement"] = curse_of_enfeeblement;
        creators["shadow bolt"] = &shadow_bolt;
        creators["banish"] = &banish;
        creators["fear"] = &fear;
        creators["howl of terror"] = &howl_of_terror;
        creators["drain life"] = &drain_life;
        creators["life tap"] = &life_tap;
        creators["summon infernal"] = &summon_infernal;

        // low threat
        creators["soulshatter"] = &soulshatter;
        // low life cd
        creators["unending resolve"] = &unending_resolve;
        creators["fel flame"] = &fel_flame;
        creators["dark intent"] = &dark_intent;

        // low life cd
        creators["twilight ward"] = &twilight_ward;
        creators["health funnel"] = &health_funnel;

        // -- talents
        creators["dark regeneration"] = &dark_regeneration;
        creators["demonic breath"] = &demonic_breath;
        creators["mortal coil"] = &mortal_coil;
        creators["shadowfury"] = &shadowfury;
        creators["soul link"] = &soul_link;
        creators["sacrificial pact"] = &sacrificial_pact;
        creators["dark bargain"] = &dark_bargain;
        creators["blood horror"] = &blood_horror;
        creators["grimoire of supremacy"] = &grimoire_of_supremacy;
        creators["grimoire of sacrifice"] = &grimoire_of_sacrifice;
        creators["mannoroth's fury"] = &mannoroths_fury;
    }

private:
    ACTION_NODE_A(corruption, "corruption", "curse of enfeeblement");
    ACTION_NODE(curse_of_enfeeblement, "curse of enfeeblement");
    ACTION_NODE(shadow_bolt, "shadow bolt");
    ACTION_NODE_A(banish, "banish", "fear");
    ACTION_NODE_A(fear, "fear", "howl of terror");
    ACTION_NODE(howl_of_terror, "howl of terror");
    ACTION_NODE(drain_life, "drain life");
    ACTION_NODE(life_tap, "life tap");
    ACTION_NODE(summon_infernal, "summon infernal");

    ACTION_NODE(unending_resolve, "unending resolve");
    ACTION_NODE(fel_flame, "fel flame");
    ACTION_NODE(dark_intent, "dark intent");
    ACTION_NODE(twilight_ward, "twilight ward");
    ACTION_NODE(soulshatter, "soulshatter");
    ACTION_NODE(health_funnel, "health funnel");
    // -- talents
    ACTION_NODE(dark_regeneration, "dark regeneration");
    ACTION_NODE(demonic_breath, "demonic breath");
    ACTION_NODE(mortal_coil, "mortal coil");
    ACTION_NODE(shadowfury, "shadowfury");
    ACTION_NODE(soul_link, "soul link");
    ACTION_NODE(sacrificial_pact, "sacrificial pact");
    ACTION_NODE(dark_bargain, "dark bargain");
    ACTION_NODE(blood_horror, "blood horror");
    ACTION_NODE(grimoire_of_supremacy, "grimoire of supremacy");
    ACTION_NODE(grimoire_of_sacrifice, "grimoire of sacrifice");
    ACTION_NODE(mannoroths_fury, "mannoroth's fury");
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

NextAction** GenericWarlockStrategy::getDefaultActions() { return NextAction::array(0, nullptr); }

void GenericWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("curse of enfeeblement", NextAction::array(0, new NextAction("curse of enfeeblement", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(new TriggerNode("curse of the elements", NextAction::array(0, new NextAction("curse of the elements", ACTION_HIGH + 5), nullptr)));

    // #521 (AC GenericWarlockStrategy.cpp:50-56 "medium threat" -> "soulshatter" 55.0f). Soulshatter 29858,
    // L66, learnable (cls.py 2026-10-04); action already registered.
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("soulshatter", 55.0f), nullptr)));

    triggers.push_back(new TriggerNode("warlock pet medium health", NextAction::array(0, new NextAction("health funnel", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("warlock pet low health", NextAction::array(0, new NextAction("dark regeneration", 60.0f), new NextAction("health funnel", 60.0f), nullptr)));

    triggers.push_back(new TriggerNode("soul link", NextAction::array(0, new NextAction("soul link", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("life tap", ACTION_EMERGENCY + 5), nullptr)));
    // #521: "dark bargaing" was a typo for the registered "dark bargain" action (Dark Bargain, MoP talent).
    // "soul burn" is the existing action key; its spell is now "soulburn" 74434 (see WarlockActions.h).
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("dark bargain", ACTION_INTERRUPT), new NextAction("twilight ward", ACTION_INTERRUPT), new NextAction("soul burn", ACTION_INTERRUPT), new NextAction("drain life", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("sacrificial pact", ACTION_INTERRUPT), new NextAction("unending resolve", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("dark regeneration", ACTION_INTERRUPT), new NextAction("drain life", ACTION_INTERRUPT), nullptr)));

    // #864 (epic #854, class 9 prio group 10 = the top bucket): interrupts on
    // any casting enemy in range (lane 0 "enemy caster in range"). Pet Spell Lock / Optical Blast (NB)
    // and Mortal Coil are separate nodes so the pet cast does not consume the bot's own interrupt.
    triggers.push_back(new TriggerNode("enemy caster in range", NextAction::array(0, new NextAction("pet interrupt", ACTION_INTERRUPT + 2), nullptr)));
    triggers.push_back(new TriggerNode("enemy caster in range", NextAction::array(0, new NextAction("mortal coil on enemy caster", ACTION_INTERRUPT + 1), nullptr)));
    // #864 (affliction prio 7 / demonology prio 15 / destruction prio 15,
    // group 0): Summon Abyssal 112921 (Grimoire of Supremacy) on cooldown in combat. 21.0 = above the DoT edges
    // (<= 20.0); the action no-ops for a warlock without the talent (the trigger needs the spell known).
    triggers.push_back(new TriggerNode("summon abyssal ready", NextAction::array(0, new NextAction("summon abyssal", 21.0f), nullptr)));
    // #864 (Fear row, every spec, group 5): Fear on a player in melee range, battlegrounds/arenas only,
    // behind the "is cc" guard (WarlockPvpFearTrigger). Sits in the CC strategy's band (AC "fear" 33.0).
    triggers.push_back(new TriggerNode("fear pvp", NextAction::array(0, new NextAction("fear", 33.5f), nullptr)));

    // #381 (AC GenericWarlockStrategy.cpp:15-103 has NO "enemy is close" edge): the fork-only fear / blood horror /
    // demonic breath / mortal coil / shadowfury edge is removed. Fear returned OK every tick on the dummy (wave 10:
    // "A:fear - OK" x18) and the GCD it consumed left Agony / Unstable Affliction / Malefic Grasp / Shadow Bolt
    // NOT_READY (result=70) for the whole fight. Fear stays available through the "cc" strategy ("fear" trigger ->
    // "fear on cc") and shadowfury through the AoE strategy, as in AC.

    // Rotation fix lane K (#868, BEYOND AC): re-target the duel opponent after "drop target" (see WarlockDuelRetargetTrigger).
    triggers.push_back(new TriggerNode("warlock duel retarget",
                                       NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));
}

void WarlockBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #381/#521: "amplify curse" removed - no such spell in 5.4.8 (cls.py warlock NO-SUCH-NAME) and neither
    // AC nor this module registers the trigger/action, so the edge was dead.

    // #594 (AC GenericWarlockStrategy.cpp:104-112, "boost" = cooldown strategy): Dark Soul is the MoP-only warlock
    // cooldown (spec265/266/267 L84, cls.py 2026-10-05); the two other specs' souls are impossible for a spec that
    // has not learned them and fall through. Moved here from the spec default lists (was above every trigger).
    triggers.push_back(new TriggerNode("dark soul: misery", NextAction::array(0, new NextAction("dark soul: misery", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("dark soul: knowledge", NextAction::array(0, new NextAction("dark soul: knowledge", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("dark soul: instability", NextAction::array(0, new NextAction("dark soul: instability", 28.0f), nullptr)));
}

void WarlockCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("banish", NextAction::array(0, new NextAction("banish on cc", 32.0f), nullptr)));
    triggers.push_back(new TriggerNode("fear", NextAction::array(0, new NextAction("fear on cc", 33.0f), nullptr)));
}

void AoeWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #521 (AC GenericWarlockStrategy.cpp:79-101): "seed of corruption on attacker" sits just above
    // "seed of corruption" (AC 22.0 vs 21.5) and "rain of fire channel check" -> "cancel channel" just above
    // "rain of fire". Rain of Fire 5740 is a channel (6 sec) in 5.4.8 (Wowhead tooltip 5740, 2026-10-04); the
    // 104233 form is instant. "shadowflame" is not ported: NOT-LEARNABLE in 5.4.8 (cls.py).
    triggers.push_back(
        new TriggerNode("rain of fire channel check", NextAction::array(0, new NextAction("cancel channel", 30.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0,
            new NextAction("mannoroth's fury", 35.0f),
            new NextAction("immolation aura", 34.0f),
            new NextAction("seed of corruption on attacker", 33.5f),
            new NextAction("seed of corruption", 33.0f),
            new NextAction("shadowfury", 32.0f),
            new NextAction("summon abyssal", 31.0f), 
            new NextAction("rain of fire", 30.0f),
            nullptr)));
}

// -- #408 (AC parity, AC Strategy/TankWarlockStrategy.cpp)
// #521: "shadow ward" (no such spell name) and "searing pain" (NOT-LEARNABLE) do not exist in 5.4.8 (cls.py
// 2026-10-04); kept for AC key parity, both resolve to spell id 0 and no-op. 5.x has no warlock tank spec.
TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
}

NextAction** TankWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("shadow ward", 27.5f), new NextAction("searing pain", 27.0f), nullptr);
}

void TankWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);
}

// -- #408 (AC parity, AC Strategy/GenericWarlockStrategy.cpp:142-240)
// #521: curse of doom / tongues / weakness - Curse of Doom is gone in 5.x (Doom 603 is Metamorphosis-only and
// is already cast by the demono "doom" action); Curse of Tongues and Curse of Weakness were replaced by Curse
// of Enfeeblement 109466 in 5.0.4 (Wowpedia Curse_of_Enfeeblement, 2026-10-04), so those two actions/triggers
// now cast/track "curse of enfeeblement". "curse of agony" casts the 5.x Agony 980 (see WarlockActions.h).
void WarlockCurseOfAgonyStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 17.0f), nullptr)));
}

void WarlockCurseOfTheElementsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of the elements", NextAction::array(0, new NextAction("curse of the elements", 29.0f), nullptr)));
}

void WarlockCurseOfDoomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of doom", NextAction::array(0, new NextAction("curse of doom", 29.0f), nullptr)));
}

void WarlockCurseOfExhaustionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of exhaustion", NextAction::array(0, new NextAction("curse of exhaustion", 29.0f), nullptr)));
}

void WarlockCurseOfTonguesStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of tongues", NextAction::array(0, new NextAction("curse of tongues", 29.0f), nullptr)));
}

void WarlockCurseOfWeaknessStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of weakness", NextAction::array(0, new NextAction("curse of weakness", 29.0f), nullptr)));
}
