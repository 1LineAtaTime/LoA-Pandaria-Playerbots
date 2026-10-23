#include "DestruWarlockStrategy.h"
#include "Playerbots.h"

class DestruWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DestruWarlockStrategyActionNodeFactory()
    {
        creators["dark soul: instability"] = &dark_soul;
        creators["chaos bolt"] = &chaos_bolt;
        creators["conflagrate"] = &conflagrate;
        creators["fire and brimstone"] = &fire_and_brimstone;

        creators["flames of xoroth"] = &flames_of_xoroth;
        creators["immolate"] = &immolate;
        creators["incinerate"] = &incinerate;
        creators["shadowburn"] = &shadowburn;
    }

private:
    ACTION_NODE(dark_soul, "dark soul: instability");
    ACTION_NODE(chaos_bolt, "chaos bolt");
    ACTION_NODE(conflagrate, "conflagrate");
    ACTION_NODE(fire_and_brimstone, "fire and brimstone");
    ACTION_NODE(flames_of_xoroth, "flames of xoroth");
    ACTION_NODE_C(immolate, "immolate", "conflagrate");
    ACTION_NODE(incinerate, "incinerate");
    ACTION_NODE(shadowburn, "shadowburn");
};

DestruWarlockStrategy::DestruWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DestruWarlockStrategyActionNodeFactory());
}

NextAction** DestruWarlockStrategy::getDefaultActions()
{
    // #594/#381 (AC DestructionWarlockStrategy.cpp:18-28): AC defaults immolate 5.9 / conflagrate 5.8 / chaos bolt 5.7 /
    // incinerate 5.6 / corruption 5.3 / shadow bolt 5.2 / shoot 5.0. All are destruction-castable in 5.4.8 (cls.py
    // 2026-10-05): Immolate 348, Conflagrate 17962, Chaos Bolt 116858 (spender, CastChaosBoltAction::isUseful needs
    // a Burning Ember), Incinerate 29722 (spec267), Corruption 172 and Shadow Bolt 686 (skill). As in AC, the last
    // two never run once Incinerate is known. "dark soul: instability" 113858 (MoP-only cooldown) moved to "boost".
    return NextAction::array(0,
        new NextAction("immolate", 5.9f),
        new NextAction("conflagrate", 5.8f),
        new NextAction("chaos bolt", 5.7f),
        new NextAction("incinerate", 5.6f),
        new NextAction("corruption", 5.3f),
        new NextAction("shadow bolt", 5.2f),
        new NextAction("shoot", 5.0f),
        nullptr);
}

void DestruWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // #594 (AC DestructionWarlockStrategy.cpp:37-62): "immolate" DoT 20.0. AC's "conflagrate" 19.5 / "chaos bolt"
    // 19.0 edges use triggers AC never registers (dormant in AC, only the 5.8/5.7 defaults run), so they are not
    // ported; the fork-only "backdraft" -> chaos bolt (Backdraft 117896, spec267 L69) moves from 20.0 to AC's chaos
    // bolt slot 19.0 so it no longer outranks the Immolate DoT.
    triggers.push_back(new TriggerNode("immolate", NextAction::array(0, new NextAction("immolate", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("backdraft", NextAction::array(0, new NextAction("chaos bolt", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("fire and brimstone", NextAction::array(0, new NextAction("fire and brimstone", 20.0f), nullptr)));

    // #864 (Destruction group 0 prio 9): Havoc 80240 on a non-primary enemy. 19.8 =
    // just under the Immolate DoT (20.0) and above Chaos Bolt (19.0) and the 5.x defaults.
    triggers.push_back(new TriggerNode("havoc", NextAction::array(0, new NextAction("havoc", 19.8f), nullptr)));

    // #594 (AC DestructionWarlockStrategy.cpp:65-85): AC's corruption edges (5.5 / 5.4). Dormant behind Incinerate
    // as in AC (Corruption is only reached when Incinerate is unavailable).
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 5.5f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 5.4f), nullptr)));

    // #594 (AC DestructionWarlockStrategy.cpp:88-97): Shadowburn 17877 (spec267 L47) execute at AC 18.0.
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("shadowburn", 18.0f), nullptr)));

    // #594 (AC DestructionWarlockStrategy.cpp:100-122): Life Tap glyph buff 29.5 / filler 5.1; flee 39.0.
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.5f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));
    // lane K (#868): flee only outside a pvp instance / duel
    triggers.push_back(new TriggerNode("warlock flee close", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

