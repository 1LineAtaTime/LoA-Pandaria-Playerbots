/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsPaladinStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        // #525: AC has no node for "seal of corruption" in DpsPaladinStrategy (its Offheal factory chains
        // corruption -> vengeance -> command -> righteousness, OffhealRetPaladinStrategy.cpp:34-62). On 5.4.8
        // Seal of Truth is learned at L24 (cls.py 2026-10-04), so below that fall back to Righteousness.
        // #730: AC's chain ends "seal of command" -> "seal of righteousness" (GenericPaladinStrategyActionNodeFactory.h:91-97).
        // On 5.4.8 Seal of Command (105361, SpellLevels 3, skill line 800) is the only seal below L24 and Righteousness is
        // L42, so a L3-23 bot had no seal, and Judgment (20271, CasterAuraState 5 = AURA_STATE_JUDGEMENT, set by a seal)
        // failed with result=24 CASTER_AURASTATE (t537 w16b Omascon L6: x1917). Truth -> Command -> Righteousness.
        creators["seal of truth"] = &seal_of_truth;
    }

private:
    ACTION_NODE_A(seal_of_truth, "seal of truth", "seal of command");
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

NextAction** DpsPaladinStrategy::getDefaultActions()
{
    // #525 (AC DpsPaladinStrategy.cpp:102-112): same defaults and priorities. "judgement of wisdom" ->
    // "judgement" (5.4.8 Judgment 20271; Judgement of Wisdom no longer exists). #735: the 5.x Holy Power
    // finisher "templar's verdict" (85256, 3 Holy Power) is no longer a default action - it fires from the
    // "holy power 3 available" / "divine purpose" triggers below, as AC fires its finishers from "combo points N
    // available" (AC CombatRogueStrategy.cpp:118-126).
    // #735 priorities (5.4.8 Ret: Crusader Strike 4.5 s cd and Judgment 6 s cd both grant 1 Holy Power, Hammer of
    // Wrath 6 s cd grants 1 with Sword of Light and is only usable <= 20% health or during Avenging Wrath, Templar's
    // Verdict spends 3 - Wowhead mop-classic tooltips 35395/20271/24275/85256, 2026-10-09): finisher 0.65 > Hammer of
    // Wrath 0.6 > Crusader Strike 0.55 > Judgment 0.5. Crusader Strike was 0.4, below Judgment/Hammer of Wrath/the
    // finisher, so it lost every open GCD (w20: 29 NOT_READY rejects, no cast in 23 s; 22 casts vs 53 Judgments over
    // one bot's run). AC's order (HoW .6 > Judgement .5 > CS .4, DpsPaladinStrategy.cpp:105-107) is the WotLK one;
    // the finisher above the generators is AC's own layout (Eviscerate HIGH over Sinister Strike NORMAL+3,
    // CombatRogueStrategy.cpp:95,126-127).
    // rotation lane 6 (#861), MoP rows 7000-7012: ... Execution Sentence (7005, prio 8, same as Hammer of Wrath) >
    // Crusader Strike > Judgment > Exorcism (7010) > Divine Storm > Holy Wrath (7011) > Consecration. Execution Sentence
    // is 0.62, above Hammer of Wrath (kept 1.0 apart from the finisher at 0.65); Exorcism 0.45 and Holy Wrath 0.15 are new.
    // rot-fix-F (wave 74, MoP L90): Judgment (7009, prio 6) and Exorcism (7010, prio 5) sit below Crusader Strike
    // (7007, prio 7) in the MoP list, but the bot only gets an open GCD every 3rd engine tick (the 2 ticks between are GCD
    // NOT_READY) and Hammer of Wrath (usable all fight under Avenging Wrath) / Templar's Verdict / Crusader Strike take
    // those slots in a fixed cycle (HoW, CS, HoW, TV, ...): Judgment 25 and Exorcism 25 NOT_READY rejects, 0 casts in 40 s,
    // so the 6 s / 15 s cooldown spells never ran. Both have long cooldowns, so ranking them just above Crusader Strike
    // costs it almost nothing and lets each fire once per cooldown. The MoP order is otherwise kept.
    return NextAction::array(0,
                             new NextAction("execution sentence", ACTION_DEFAULT + 0.62f),
                             new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                             new NextAction("judgement", ACTION_DEFAULT + 0.57f),
                             new NextAction("exorcism", ACTION_DEFAULT + 0.56f),
                             new NextAction("crusader strike", ACTION_DEFAULT + 0.55f),
                             new NextAction("divine storm", ACTION_DEFAULT + 0.3f),
                             new NextAction("holy wrath", ACTION_DEFAULT + 0.15f),
                             new NextAction("consecration", ACTION_DEFAULT + 0.1f),
                             new NextAction("melee", ACTION_DEFAULT),
                             nullptr);
}

// rotation lane 6 (#861): MoP Ret cooldown/defensive group 5 (rows 7016-7024), shared by Dps and Offheal.
static void AddRetCooldownTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("holy avenger", NextAction::array(0, new NextAction("holy avenger", ACTION_HIGH + 2.7f), nullptr)));              // 7016
    triggers.push_back(new TriggerNode("guardian of ancient kings", NextAction::array(0, new NextAction("guardian of ancient kings", ACTION_HIGH + 2.6f), nullptr)));  // 7021
    triggers.push_back(new TriggerNode("paladin health 50", NextAction::array(0, new NextAction("divine protection", ACTION_HIGH + 6), nullptr)));      // 7017
    triggers.push_back(new TriggerNode("paladin health 25", NextAction::array(0, new NextAction("flash of light", ACTION_CRITICAL_HEAL + 1), nullptr))); // 7024 (grouped self heal)
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    // #525 (AC DpsPaladinStrategy.cpp:118-125): art of war -> exorcism (Exorcism 879 exists in 5.4.8,
    // Ret L46; "The Art of War" 87138 is Ret L50 - cls.py 2026-10-04)
    triggers.push_back(new TriggerNode("art of war", NextAction::array(0, new NextAction("exorcism", ACTION_DEFAULT + 0.2f), nullptr)));
    // #525 (AC DpsPaladinStrategy.cpp:126-133): seal -> "seal of corruption" -> Seal of Truth (31801)
    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of truth", ACTION_HIGH), nullptr)));
    // #525 (AC DpsPaladinStrategy.cpp:134-141): low mana -> seal of wisdom NOT ported. Seal of Wisdom has no
    // 5.4.8 spell, and its successor Seal of Insight (20165) restores mana only for Holy ("[Holy: restore 6%
    // of base mana]", Wowhead 20165 tooltip 2026-10-04), so a Ret swap would gain nothing.
    // #525 (AC DpsPaladinStrategy.cpp:143-150)
    triggers.push_back(new TriggerNode("avenging wrath", NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 2), nullptr)));
    // #525 (AC DpsPaladinStrategy.cpp:151-159)
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                                         new NextAction("divine storm", ACTION_HIGH + 4),
                                                                         new NextAction("consecration", ACTION_HIGH + 3),
                                                                         // 7008: Hammer of the Righteous with 3+ enemies
                                                                         new NextAction("hammer of the righteous", ACTION_HIGH + 2),
                                                                         nullptr)));
    AddRetCooldownTriggers(triggers);
    // #525: Inquisition (84963) upkeep, 5.x Ret Holy Power buff with no AC equivalent (Wowhead 2026-10-04:
    // "+30% Holy damage ... consumes up to 3 Holy Power"). #735: buff missing AND >= 1 Holy Power, a TwoTriggers
    // as AC's "combo points not full and high energy" (AC TriggerContext.h:125,375).
    triggers.push_back(new TriggerNode("inquisition and holy power 1 available", NextAction::array(0, new NextAction("inquisition", ACTION_HIGH + 1), nullptr)));
    // #735: the Holy Power finisher (AC CombatRogueStrategy.cpp:118-126 "combo points 5 available" -> eviscerate).
    triggers.push_back(new TriggerNode("holy power 3 available", NextAction::array(0, new NextAction("templar's verdict", ACTION_DEFAULT + 0.65f), nullptr)));
    triggers.push_back(new TriggerNode("divine purpose", NextAction::array(0, new NextAction("templar's verdict", ACTION_DEFAULT + 0.65f), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), NULL)));
}

// #383 (AC Strategy/OffhealRetPaladinStrategy.cpp:75-221): Retribution paladin that also heals.
OffhealRetPaladinStrategy::OffhealRetPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

NextAction** OffhealRetPaladinStrategy::getDefaultActions()
{
    // AC OffhealRetPaladinStrategy.cpp:80-89; "judgement of wisdom" -> "judgement" (5.4.8 Judgment 20271).
    // No consecration (AC omits it here too). #735 "templar's verdict" fires from triggers: see
    // DpsPaladinStrategy::getDefaultActions and InitTriggers below.
    return NextAction::array(0,
                             new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                             new NextAction("judgement", ACTION_DEFAULT + 0.5f),
                             new NextAction("execution sentence", ACTION_DEFAULT + 0.62f),  // rotation lane 6 (#861), row 7005
                             new NextAction("crusader strike", ACTION_DEFAULT + 0.55f),
                             new NextAction("divine storm", ACTION_DEFAULT + 0.3f),
                             new NextAction("melee", ACTION_DEFAULT),
                             nullptr);
}

void OffhealRetPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    // Damage Triggers
    // AC :96-103 seal -> seal of corruption -> Seal of Truth
    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of truth", ACTION_HIGH), nullptr)));
    // AC :104-112 low mana -> seal of wisdom (no 5.4.8 spell; Insight restores mana for Holy only, so
    // omitted - see DpsPaladinStrategy) + divine plea (54428 is Holy-learnable only in 5.4.8, cls.py
    // 2026-10-04; kept 1:1, the cast is skipped when the spell is unknown)
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("divine plea", ACTION_HIGH + 4), nullptr)));
    // AC :113-120
    triggers.push_back(new TriggerNode("art of war", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 1), nullptr)));
    // #735: the Holy Power finisher, as in DpsPaladinStrategy::InitTriggers
    triggers.push_back(new TriggerNode("holy power 3 available", NextAction::array(0, new NextAction("templar's verdict", ACTION_DEFAULT + 0.65f), nullptr)));
    triggers.push_back(new TriggerNode("divine purpose", NextAction::array(0, new NextAction("templar's verdict", ACTION_DEFAULT + 0.65f), nullptr)));
    // AC :121-128
    triggers.push_back(new TriggerNode("avenging wrath", NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 2), nullptr)));
    // AC :129-137
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                                         new NextAction("divine storm", ACTION_HIGH + 4),
                                                                         new NextAction("consecration", ACTION_HIGH + 3),
                                                                         nullptr)));
    AddRetCooldownTriggers(triggers);  // rotation lane 6 (#861)
    // AC :138-145
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));
    // AC :146-153 retribution aura NOT ported: Paladin auras other than Devotion Aura were removed in
    // patch 5.0 (retribution aura 8990/13008/79976 NOT-LEARNABLE, cls.py 2026-10-04).
    // AC :154-161 "blessing of might" trigger NOT ported: AC never registers a trigger of that name
    // (AC PaladinAiObjectContext.cpp registers only "blessing"), so the edge is dead on AC too.
    // AC :162-169
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("holy light", ACTION_CRITICAL_HEAL + 2), nullptr)));

    // Healing Triggers (AC :171-220)
    triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0,
                                                                                           new NextAction("holy shock on party", ACTION_CRITICAL_HEAL + 6),
                                                                                           new NextAction("holy light on party", ACTION_CRITICAL_HEAL + 4),
                                                                                           nullptr)));
    triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 3), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("beacon of light on main tank", NextAction::array(0, new NextAction("beacon of light on main tank", ACTION_CRITICAL_HEAL + 7), nullptr)));
}
