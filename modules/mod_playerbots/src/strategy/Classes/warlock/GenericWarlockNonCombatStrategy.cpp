/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarlockNonCombatStrategy.h"

#include "Playerbots.h"

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockNonCombatStrategyActionNodeFactory()
    {
        creators["dark intent"] = &dark_intent;
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
        creators["summon succubus"] = &summon_succubus;
        creators["summon felhunter"] = &summon_felhunter;
        creators["summon doomguard"] = &summon_doomguard;
        creators["summon infernal"] = &summon_infernal;

        creators["summon wrathguard"] = &summon_wrathguard;
        creators["summon fel imp"] = &summon_fel_imp;
        creators["summon voidlord"] = &summon_voidlord;
        creators["summon shivarra"] = &summon_shivarra;
        creators["summon observer"] = &summon_observer;
        creators["summon abyssal"] = &summon_abyssal;
        creators["summon terrorguard"] = &summon_terrorguard;
    }

private:
    ACTION_NODE(dark_intent, "dark intent");

    ACTION_NODE_A(summon_fel_imp, "summon fel imp", "summon imp");
    ACTION_NODE_A(summon_voidlord, "summon voidlord", "summon voidwalker");
    ACTION_NODE_A(summon_shivarra, "summon shivarra", "summon succubus");
    ACTION_NODE_A(summon_observer, "summon observer", "summon felhunter");
    ACTION_NODE_A(summon_abyssal, "summon abyssal", "summon infernal");
    ACTION_NODE_A(summon_terrorguard, "summon terrorguard", "summon doomguard");

    ACTION_NODE_A(summon_voidwalker, "summon voidwalker", "summon fel imp");
    ACTION_NODE_A(summon_felguard, "summon felguard", "summon voidlord");
    ACTION_NODE_A(summon_succubus, "summon succubus", "summon voidlord");
    ACTION_NODE_A(summon_felhunter, "summon felhunter", "summon shivarra");
    ACTION_NODE_A(summon_wrathguard, "summon wrathguard", "summon felguard");
    ACTION_NODE(summon_doomguard, "summon doomguard");
    ACTION_NODE(summon_infernal, "summon infernal");
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, GenericWarlockNonCombatStrategy.cpp:83): apply the configured stance
    // to a freshly summoned demon.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
    // Rotation fix lane K (#868, BEYOND AC): re-target the duel opponent after "drop target" (see WarlockDuelRetargetTrigger).
    triggers.push_back(new TriggerNode("warlock duel retarget",
                                       NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));

    // #864 (class 9 group 0 prio 0, out of combat, "NOT has aura Dark Intent 109773
    // (cast by me)"): see DarkIntentTrigger (WarlockTriggers.cpp) - own-aura test by spell id.
    triggers.push_back(new TriggerNode("dark intent", NextAction::array(0, new NextAction("dark intent", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no healthstone", NextAction::array(0, new NextAction("create healthstone", 15.0f), nullptr)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:84,86-88,90,93): armor / underwater breath / soul link /
    // life tap. Priorities keep AC's relative order on this module's lower scale (healthstone 15 above).
    // Fel Armor 104938 replaces Demon Armor/Demon Skin (cls.py: demon armor/skin NOT-LEARNABLE, 2026-10-04).
    triggers.push_back(new TriggerNode("soul link", NextAction::array(0, new NextAction("soul link", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("demon armor", NextAction::array(0, new NextAction("fel armor", 16.0f), nullptr)));
    triggers.push_back(new TriggerNode("unending breath", NextAction::array(0, new NextAction("unending breath", 12.0f), nullptr)));
    triggers.push_back(new TriggerNode("unending breath on party", NextAction::array(0, new NextAction("unending breath on party", 11.0f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 23.0f), nullptr)));
    // Not ported (5.4.8 divergences): "no pet" -> "fel domination" (spell gone: cls.py NO-SUCH-NAME);
    // "no soul shard"/"too many soul shards" -> create/destroy soul shard (shards are a resource, not items, since 4.0);
    // 5.4.8: AC's "no soulstone" -> "create soulstone" (GenericWarlockNonCombatStrategy.cpp:92) is
    // N/A - see the WarlockAiObjectContext.cpp comment on CastCreateSoulstoneAction/"create
    // soulstone" for why (the spell was removed in Cataclysm, no successor).
}

void WarlockPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // TODO Warlock pets
}

// #864 ("cast Summon <demon> ... out of combat, NOT has
// pet"): the pet is summoned with the BASE spell (Summon Imp 688 / Felhunter 691 / Felguard 30146). With Grimoire of
// Supremacy the client sends the base id and the server swaps it (BotOverrideSpell, PlayerbotAI.cpp, #597), so the
// Fel Imp / Observer / Wrathguard keys ("summon fel imp" etc.: NOT-LEARNABLE, spell id 0) only burnt a failed
// first try and logged the cast as "summon observer" instead of "summon felhunter" (wave 74). Priority 51 = just
// above "dps assist" (50, DpsAssistStrategy.cpp:8): at 11 the assist action won every tick once a target was in view
// and the bot walked into the fight petless (wave 74 demonology / destruction: T:no pet, PUSH 11.0, never run).
static float const WARLOCK_SUMMON_PET_PRIORITY = 51.0f;

SummonImpStrategy::SummonImpStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonImpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon imp", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:98-160): re-summon when the wrong demon is out
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon imp", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
}

SummonFelguardStrategy::SummonFelguardStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelguardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon felguard", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:98-160): re-summon when the wrong demon is out
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon felguard", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
}

SummonFelhunterStrategy::SummonFelhunterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelhunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon felhunter", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:98-160): re-summon when the wrong demon is out
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon felhunter", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
}

// #163 S8 (AC parity, GenericWarlockNonCombatStrategy.cpp:162-197): each strategy is an opt-in
// (toggled via ChangeStrategyAction, like "boost"/"save mana") naming who receives a soulstone.
void SoulstoneSelfStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone self", 24.0f), nullptr)));
}

void SoulstoneMasterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone master", 24.0f), nullptr)));
}

void SoulstoneTankStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone tank", 24.0f), nullptr)));
}

void SoulstoneHealerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone healer", 24.0f), nullptr)));
}

// -- #408 (AC parity, AC Strategy/GenericWarlockNonCombatStrategy.cpp:112-130). Uses this module's
// "no pet" trigger; #521 added AC's "wrong pet" edge below.
void SummonVoidwalkerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon voidwalker", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:110-132)
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon voidwalker", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
}

void SummonSuccubusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon succubus", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
    // #521 (AC GenericWarlockNonCombatStrategy.cpp:110-132)
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon succubus", WARLOCK_SUMMON_PET_PRIORITY), NULL)));
}

// #408 (AC parity, AC Strategy/GenericWarlockNonCombatStrategy.cpp:206-222), minus the
// "create spellstone"/"create firestone" half (no such actions on 5.4.8).
void UseSpellstoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("spellstone", NextAction::array(0, new NextAction("spellstone", 24.0f), nullptr)));
}

void UseFirestoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("firestone", NextAction::array(0, new NextAction("firestone", 24.0f), nullptr)));
}