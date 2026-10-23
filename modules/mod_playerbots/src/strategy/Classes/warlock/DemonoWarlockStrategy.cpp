#include "DemonoWarlockStrategy.h"
#include "Playerbots.h"

class DemonoWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DemonoWarlockStrategyActionNodeFactory()
    {
        creators["dark soul: knowledge"] = &dark_soul;
        creators["hellfire"] = &hellfire;
        creators["hand of guldan"] = &hand_of_guldan;
        creators["soul fire"] = &soul_fire;

        creators["metamorphosis"] = &metamorphosis;
        creators["chaos wave"] = &metamorphosis_chaos_wave;
        creators["doom"] = &metamorphosis_doom;
        creators["immolation aura"] = &metamorphosis_immolation;
        creators["touch of chaos"] = &metamorphosis_touch_chaos;
        creators["void ray"] = &metamorphosis_void_ray;
    }

private:
    ACTION_NODE(metamorphosis, "metamorphosis");
    ACTION_NODE_A(metamorphosis_chaos_wave, "chaos wave", "hand of guldan");
    ACTION_NODE_A(metamorphosis_doom, "doom", "corruption");
    ACTION_NODE_A(metamorphosis_immolation, "immolation aura", "hellfire");
    ACTION_NODE_A(metamorphosis_touch_chaos, "touch of chaos", "shadow bolt");
    ACTION_NODE_A(metamorphosis_void_ray, "void ray", "fel flame");

    ACTION_NODE(dark_soul, "dark soul: knowledge");
    ACTION_NODE(hellfire, "hellfire");
    ACTION_NODE(hand_of_guldan, "hand of guldan");
    ACTION_NODE(soul_fire, "soul fire");
};

DemonoWarlockStrategy::DemonoWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DemonoWarlockStrategyActionNodeFactory());
}

NextAction** DemonoWarlockStrategy::getDefaultActions()
{
    // #594/#381 (AC DemonologyWarlockStrategy.cpp:19-26): AC defaults corruption 5.5 / immolate 5.4 / shadow bolt 5.3 /
    // incinerate 5.2 / shoot 5.0. 5.4.8 mapping (cls.py 2026-10-05): Corruption 172 (all specs) kept at 5.5;
    // "immolate" 5.4 and "incinerate" 5.2 dropped - Immolate 348 and Incinerate 29722 are Destruction-only
    // (spec267), a Demonology warlock cannot cast them; the 5.4 slot takes Hand of Gul'dan 105174 (spec266 L19,
    // 2 charges / 15 s, Wowhead tooltip 2026-10-05) and the 5.2 slot Soul Fire 6353 (spec266 L13, 4 s nuke), the
    // demo spells in those AC slots; Shadow Bolt 686 5.3 stays. Touch of Chaos 103964 / Chaos Wave 124916 are the
    // Metamorphosis overrides of Shadow Bolt / Hand of Gul'dan (NOT-LEARNABLE in cls.py - cast only while the
    // override path is applied); they stay as LOW alternatives just under the spell they replace instead of the
    // old ACTION_HIGH defaults. Void Ray (Fel Flame override) dropped. "dark soul: knowledge" moved to "boost".
    return NextAction::array(0,
        new NextAction("corruption", 5.5f),
        new NextAction("hand of guldan", 5.4f),
        new NextAction("chaos wave", 5.35f),
        new NextAction("shadow bolt", 5.3f),
        new NextAction("touch of chaos", 5.25f),
        new NextAction("soul fire", 5.2f),
        new NextAction("shoot", 5.0f),
        nullptr);
}

void DemonoWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // #521 (AC DemonologyWarlockStrategy.cpp "immolate on attacker" 19.0f) NOT ported: Immolate 348 is
    // Destruction-only in 5.4.8 (cls.py: 348/spec267 only), a Demonology warlock cannot cast it. Likewise
    // "demonic empowerment" (no such spell in 5.x) and "corruption on attacker" (Doom is the demo DoT).
    // #594 (AC DemonologyWarlockStrategy.cpp:35-44): metamorphosis 28.5 (fork trigger also needs 800 demonic fury).
    triggers.push_back(new TriggerNode("metamorphosis", NextAction::array(0, new NextAction("metamorphosis", 28.5f), nullptr)));

    // #594 (AC DemonologyWarlockStrategy.cpp:47-63): DoTs at AC 19.5 (other attackers) / 18.0 (main target). The
    // main-target edge keeps the fork's "doom" action (Meta override of Corruption; its node falls back to
    // "corruption" 172 when not metamorphosed) at AC's 18.0 instead of 20.0.
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("doom", 18.0f), nullptr)));

    // #594 (AC DemonologyWarlockStrategy.cpp:66-82): "decimation" -> soul fire 17.0 NOT ported: Decimation 108869 is
    // a passive in 5.4.8 that grants Molten Core on Soul Fire/Shadow Bolt vs targets <25% (Wowhead tooltip
    // 2026-10-05); there is no self aura with a timer to react to (DecimationTrigger is dead). Molten Core 122351
    // (spec266 L69, halves Soul Fire cast time/cost) takes AC's "molten core" edge (AC incinerate 16.5 ->
    // Soul Fire here, it is Soul Fire that Molten Core buffs).
    triggers.push_back(new TriggerNode("molten core", NextAction::array(0, new NextAction("soul fire", 16.5f), nullptr)));

    // #594 (AC DemonologyWarlockStrategy.cpp:85-101): Life Tap glyph buff 29.5 / Life Tap filler 5.1; flee 39.0.
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.5f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));
    // lane K (#868): flee only outside a pvp instance / duel
    triggers.push_back(new TriggerNode("warlock meta flee close", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

void MetaMeleeAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("immolation aura active", NextAction::array(0,
            new NextAction("reach melee", 25.5f),
            new NextAction("demon charge", 25.0f),
            nullptr)));
}

