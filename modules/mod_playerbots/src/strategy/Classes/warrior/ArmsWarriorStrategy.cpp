/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ArmsWarriorStrategy.h"

#include "Playerbots.h"

class ArmsWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArmsWarriorStrategyActionNodeFactory()
    {
        creators["mortal strike"] = &mortal_strike;
        creators["slam"] = &slam;

        creators["overpower"] = &overpower;
        creators["sweeping strikes"] = &sweeping_strikes;
        creators["demoralizing banner"] = &demoralizing_banner;
        // #522 (AC ArmsWarriorStrategy.cpp:56,82-90): AC chain piercing howl -> mocking blow -> hamstring;
        // Mocking Blow is not learnable in 5.4.8 (cls.py 2026-10-04), so piercing howl falls back to hamstring.
        creators["piercing howl"] = &piercing_howl;
    }

private:
    ACTION_NODE(mortal_strike, "mortal strike");
    ACTION_NODE(slam, "slam");
    ACTION_NODE(overpower, "overpower");
    ACTION_NODE(sweeping_strikes, "sweeping strikes");
    ACTION_NODE(demoralizing_banner, "demoralizing banner");
    ACTION_NODE_A(piercing_howl, "piercing howl", "hamstring");
};

ArmsWarriorStrategy::ArmsWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new ArmsWarriorStrategyActionNodeFactory());
}

NextAction** ArmsWarriorStrategy::getDefaultActions()
{
    // #522 (AC ArmsWarriorStrategy.cpp:78-85): AC's defaults and numbers. Mortal Strike 12294 (Arms, L10, 6 s cd,
    // "Requires Melee Weapon", Wowhead mop-classic tooltip 2026-10-05) was missing here and sat behind the 60-rage
    // "high rage available" gate, so wave 10 saw it twice in 40 s. Sunder Armor 7386 is learnable (cls.py, L16).
    // The fork-only Recklessness / Bloodbath / Execute defaults moved: Recklessness to the AC boost-trigger edge
    // below, Bloodbath stays in WarriorAoeStrategy (25.0), Execute is the generic "target critical health" edge.
    return NextAction::array(0,
        new NextAction("bladestorm", ACTION_DEFAULT + 0.2f),
        // Lane 11 (#866): "Dragon Roar" (class row, 0/7), talent, target within melee.
        new NextAction("dragon roar", ACTION_DEFAULT + 0.15f),
        new NextAction("mortal strike", ACTION_DEFAULT + 0.1f),
        new NextAction("sunder armor", ACTION_DEFAULT + 0.05f),
        new NextAction("melee", ACTION_DEFAULT),
        nullptr);
}

void ArmsWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("battle shout", NextAction::array(0, new NextAction("battle shout", ACTION_HIGH + 9), nullptr)));
    // #522 (AC ArmsWarriorStrategy.cpp:137-144): "mortal strike" trigger -> Mortal Strike at HIGH+3 (was gated on
    // "high rage available", 60 rage, which Arms rarely has; MS generates rage in 5.4.8).
    triggers.push_back(new TriggerNode("mortal strike", NextAction::array(0, new NextAction("mortal strike", ACTION_HIGH + 3), nullptr)));
    // #522 (AC ArmsWarriorStrategy.cpp:101-108): hold Battle Stance (see CastBattleStanceAction::isUseful for Arms).
    triggers.push_back(new TriggerNode("battle stance", NextAction::array(0, new NextAction("battle stance", ACTION_HIGH + 10), nullptr)));
    // MoP extension (AC FuryWarriorStrategy.cpp:179-186 boost-trigger edge "recklessness" -> HIGH): Recklessness
    // 1719 (L62) replaces the fork-only Arms default at ACTION_DEFAULT + 0.7.
    triggers.push_back(new TriggerNode("recklessness", NextAction::array(0, new NextAction("recklessness", ACTION_HIGH), nullptr)));
    // Lane 11 (#866): Bloodbath 12292 (prio 5/2, "always").
    triggers.push_back(new TriggerNode("bloodbath buff", NextAction::array(0, new NextAction("bloodbath", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("overpower", NextAction::array(0, new NextAction("overpower", ACTION_HIGH + 4), nullptr)));
    triggers.push_back(new TriggerNode("taste for blood", NextAction::array(0, new NextAction("overpower", ACTION_HIGH + 4), nullptr)));

    // #522 (AC ArmsWarriorStrategy.cpp:239-246): rage dump. Heroic Strike 78 (30 rage) / Slam 1464 (25 rage, arms
    // spec L18) both exist on 5.4.8 (Wowhead tooltips 2026-10-04). "high rage available" = 60 rage.
    triggers.push_back(new TriggerNode("high rage available", NextAction::array(0,
        new NextAction("heroic strike", ACTION_HIGH),
        new NextAction("slam", ACTION_HIGH + 1),
        nullptr)));
    // #522 (AC :203-210): snare the target's escape with Piercing Howl (12323 talent), falling back to Hamstring (1715).
    triggers.push_back(new TriggerNode("hamstring", NextAction::array(0, new NextAction("piercing howl", ACTION_HIGH), nullptr)));
    // #522 (AC :266-273): Intimidating Shout 5246 (L52).
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("intimidating shout", ACTION_EMERGENCY), nullptr)));
    // #522 (AC :275-282): Enraged Regeneration 55694 (talent; heals 10% + 10% over 5s, Wowhead 2026-10-04).
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("enraged regeneration", ACTION_EMERGENCY), nullptr)));
    // #522 (AC :293-300): break an immunity (Divine Shield / Ice Block / BoP) with Shattering Throw 64382 (L74).
    triggers.push_back(new TriggerNode("shattering throw trigger", NextAction::array(0, new NextAction("shattering throw", ACTION_INTERRUPT + 1), nullptr)));

    // #522 DIVERGENCES (AC edge not ported):
    //  - "sudden death -> execute" (AC :194-201): on 5.4.8 Sudden Death (52437/29725) only resets Colossus Smash's
    //    cooldown (Wowhead tooltip 2026-10-04) and no longer enables Execute; the existing "colossus smash" trigger
    //    (GenericWarriorStrategy) already fires Colossus Smash when it is off cooldown.
    //  - "rend", "rend on attacker", "bloodrage", "death wish", "almost full health -> retaliation": not learnable
    //    in 5.4.8 (cls.py NOT-LEARNABLE / NO-SUCH-NAME 2026-10-04).
}
