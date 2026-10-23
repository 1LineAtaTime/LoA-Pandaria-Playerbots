#include "AffliWarlockStrategy.h"
#include "Playerbots.h"

class AffliWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AffliWarlockStrategyActionNodeFactory()
    {
        creators["dark soul: misery"] = &dark_soul;
        creators["malefic grasp"] = &malefic_grasp;
        creators["agony"] = &agony;
        creators["drain soul"] = &drain_soul;
        creators["haunt"] = &haunt;
        creators["unstable affliction"] = &unstable_affliction;
        creators["unstable affliction on attacker"] = &unstable_affliction_on_attacker;
        creators["seed of corruption"] = &seed_of_corruption;
    }

private:
    
    ACTION_NODE(dark_soul, "dark soul: misery");
    ACTION_NODE_A(malefic_grasp, "malefic grasp", "shadow bolt");
    ACTION_NODE(agony, "agony");
    ACTION_NODE_P(drain_soul, "drain soul", "corruption");
    ACTION_NODE(haunt, "haunt");
    ACTION_NODE(unstable_affliction, "unstable affliction");
    ACTION_NODE(unstable_affliction_on_attacker, "unstable affliction on attacker");
    ACTION_NODE(seed_of_corruption, "seed of corruption");
};

AffliWarlockStrategy::AffliWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new AffliWarlockStrategyActionNodeFactory());
}

NextAction** AffliWarlockStrategy::getDefaultActions()
{
    // #594/#381 (AC AfflictionWarlockStrategy.cpp:19-27): AC defaults corruption 5.5 / unstable affliction 5.4 /
    // haunt 5.3 / shadow bolt 5.2 / shoot 5.0, all in the 5.x band so the DoT triggers (below) and every other
    // trigger edge outrank them. Mapping to 5.4.8 (cls.py 2026-10-05): Corruption 172, Unstable Affliction 30108,
    // Haunt 48181 are affliction spells; AC's "shadow bolt" 5.2 slot is the affliction filler Malefic Grasp 103103
    // (spec265 L42; node falls back to shadow bolt 686). Wave 9 had "malefic grasp" at ACTION_HIGH and "dark soul:
    // misery"/"fel flame" here, which starved every DoT in short fights. Dark Soul: Misery 113860 (MoP-only
    // cooldown, no AC counterpart) moved to the "boost" strategy (GenericWarlockStrategy.cpp). "fel flame" dropped
    // (AC has no such slot). Agony 980 comes from the "curse of agony" strategy (AiFactory.cpp:432/442).
    return NextAction::array(0,
        new NextAction("corruption", 5.5f),
        new NextAction("unstable affliction", 5.4f),
        new NextAction("haunt", 5.3f),
        new NextAction("malefic grasp", 5.2f),
        new NextAction("shoot", 5.0f),
        nullptr);
}

void AffliWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // #521 (AC AfflictionWarlockStrategy.cpp:30-47): DoTs on other attackers, placed just above the main-target DoTs
    // (AC 19.5/19.0 vs 18.0/17.5). Corruption 172, Unstable Affliction 30108 learnable (cls.py 2026-10-04).
    // #594 (AC AfflictionWarlockStrategy.cpp:30-70): AC numbers restored - 19.5/19.0 on other attackers, 18.0/17.5
    // main target, haunt 16.5. The 20.6/20.4/20.0 values were only there to beat the ACTION_HIGH malefic grasp.
    // Agony 980 is cast by the "curse of agony" strategy (18.5 on attacker / 17.0), so the fork-only "agony" edge
    // at 20.0 is removed (it duplicated that strategy and outranked Corruption/UA).
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction on attacker", NextAction::array(0, new NextAction("unstable affliction on attacker", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction", NextAction::array(0, new NextAction("unstable affliction", 17.5f), nullptr)));
    triggers.push_back(new TriggerNode("haunt", NextAction::array(0, new NextAction("haunt", 16.5f), nullptr)));

    // #594 (AC AfflictionWarlockStrategy.cpp:73-81): "shadow trance" -> shadow bolt 16.0 NOT ported: Shadow Trance
    // (17941) is NOT-LEARNABLE / removed in 5.x (cls.py 2026-10-05), no free-cast proc exists to react to.
    // #864 (Shadow Bolt, same group-0 prio-1 bucket as Malefic Grasp; AC casts it as
    // the affliction filler): 5.25 = just above the Malefic Grasp default (5.2), fired by the random filler trigger.
    triggers.push_back(new TriggerNode("shadow bolt filler", NextAction::array(0, new NextAction("shadow bolt", 5.25f), nullptr)));

    // Execute: Drain Soul 1120 (spec265 L19) at AC 15.5.
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("drain soul", 15.5f), nullptr)));

    // #594 (AC AfflictionWarlockStrategy.cpp:84-101): Life Tap glyph buff 29.5, Life Tap as filler 5.1. The fork's
    // "high mana" -> life tap (ACTION_DEFAULT+0.1) fired when mana was HIGH and is replaced by AC's "life tap"
    // trigger (LifeTapTrigger: mana < 85% and health above low health).
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.5f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));

    // #594 (AC AfflictionWarlockStrategy.cpp:103-110)
    // lane K (#868): flee only outside a pvp instance / duel
    triggers.push_back(new TriggerNode("warlock flee close", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));

    // #594: the old "metamorphosis" edge is removed - Metamorphosis 103958 is a Demonology spell in 5.4.8 (cls.py:
    // spec266 only), an Affliction warlock cannot cast it.
}

