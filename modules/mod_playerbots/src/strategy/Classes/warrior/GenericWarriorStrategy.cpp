#include "GenericWarriorStrategy.h"

#include "Playerbots.h"

class GenericWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarriorStrategyActionNodeFactory()
    {
        creators["heroic strike"] = &GenericWarriorStrategyActionNodeFactory::heroic_strike;
        creators["heroic leap"] = &GenericWarriorStrategyActionNodeFactory::heroic_leap;
        creators["charge"] = &GenericWarriorStrategyActionNodeFactory::charge;

        creators["whirlwind"] = &GenericWarriorStrategyActionNodeFactory::whirlwind;
        creators["victory rush"] = &GenericWarriorStrategyActionNodeFactory::victory_rush;
        creators["execute"] = &GenericWarriorStrategyActionNodeFactory::execute;
        creators["sunder armor"] = &GenericWarriorStrategyActionNodeFactory::sunder_armor;
        creators["thunder clap"] = &GenericWarriorStrategyActionNodeFactory::thunder_clap;
        creators["bloodbath"] = &GenericWarriorStrategyActionNodeFactory::bloodbath;

        creators["berserker rage"] = &GenericWarriorStrategyActionNodeFactory::berserker_rage;
        creators["rallying cry"] = &GenericWarriorStrategyActionNodeFactory::rallying_cry;
        creators["shield wall"] = &GenericWarriorStrategyActionNodeFactory::shield_wall;
        creators["recklessness"] = &GenericWarriorStrategyActionNodeFactory::recklessness;
        creators["colossus smash"] = &GenericWarriorStrategyActionNodeFactory::colossus_smash;
        creators["die by the sword"] = &GenericWarriorStrategyActionNodeFactory::die_by_the_sword;
        creators["pummel"] = &GenericWarriorStrategyActionNodeFactory::pummel;
    }

private:
    ACTION_NODE(heroic_strike, "heroic strike");
    // #742 (AC pattern: AC ArmsWarriorStrategy.cpp:22-29 charge A: "reach melee", FuryWarriorStrategy.cpp:31-38
    // intercept A: "reach melee") - every AC gap-closer chain ends in "reach melee". Heroic Leap is Cata+, so it takes
    // that end of the chain: charge -> heroic leap -> reach melee. Its old "melee" alternative (relevance 40.006) beat
    // "reach melee" (ACTION_HIGH + 1): with charge and leap refused (mounted: SPELL_FAILED_NOT_MOUNTED 67; cooldown;
    // range) the bot ran AttackAction::Attack, which returns true although Unit::Attack refuses a mounted player and
    // which StopMoving()s the ride - a mounted warrior 18-25 yd from its target never closed in and never reached
    // the dismount distance (w61: 561 of 894 mounted in-combat "melee" picks; Rinarisca 318 ticks, 100 -> 6 hp).
    ACTION_NODE_A(heroic_leap, "heroic leap", "reach melee");
    ACTION_NODE_A(charge, "charge", "heroic leap");

    ACTION_NODE(whirlwind, "whirlwind");
    ACTION_NODE(victory_rush, "victory rush");
    ACTION_NODE(execute, "execute");
    ACTION_NODE(sunder_armor, "sunder armor");
    ACTION_NODE(thunder_clap, "thunder clap");
    ACTION_NODE(bloodbath, "bloodbath");
    
    ACTION_NODE(berserker_rage, "berserker rage");
    ACTION_NODE_A(rallying_cry, "rallying cry", "die by the sword");
    ACTION_NODE_A(shield_wall, "shield wall", "last stand");
    ACTION_NODE(recklessness, "recklessness");
    ACTION_NODE(colossus_smash, "colossus smash");
    ACTION_NODE_A(die_by_the_sword, "die by the sword", "enraged regeneration");
    ACTION_NODE(pummel, "pummel");
};

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarriorStrategyActionNodeFactory());
}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("battle stance", NextAction::array(0, new NextAction("battle stance", ACTION_DEFAULT), nullptr)));
    triggers.push_back(new TriggerNode("defensive stance", NextAction::array(0, new NextAction("defensive stance", ACTION_DEFAULT), nullptr)));
    triggers.push_back(new TriggerNode("berserker stance", NextAction::array(0, new NextAction("berserker stance", ACTION_DEFAULT), nullptr)));

    triggers.push_back(new TriggerNode("light rage available", NextAction::array(0, new NextAction("berserker rage", ACTION_MOVE + 10), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("charge", ACTION_MOVE + 10), nullptr)));
    // Lane 11 (#866, prio 8/0, below Charge prio 9/0): the "charge" node only falls back
    // to Heroic Leap when Charge is IMPOSSIBLE; at 8-8.5 yd Charge is USELESS (min range), which ended the chain and left
    // Heroic Leap never cast (w74: 0 casts, all three specs). Own edge just under Charge.
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("heroic leap", ACTION_MOVE + 9), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("execute", ACTION_HIGH + 20), nullptr)));
    triggers.push_back(new TriggerNode("victory rush", NextAction::array(0, new NextAction("victory rush", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("colossus smash", NextAction::array(0, new NextAction("colossus smash", ACTION_HIGH + 4), nullptr)));
    //triggers.push_back(new TriggerNode("sunder armor", NextAction::array(0, new NextAction("sunder armor", ACTION_HIGH + 4), nullptr)));
    //triggers.push_back(new TriggerNode("thunder clap", NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 2), nullptr)));

    //triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("", ACTION_INTERRUPT), nullptr)));
    // Lane 11 (#866, MoP thresholds): the AC "low health" -> shield wall and "critical health" -> rallying cry edges
    // (config-driven percentages) are replaced by the exact MoP ones: Shield Wall 871 at 20 %,
    // Rallying Cry 97462 at 20 %, Demoralizing Banner 114203 at 20 %,
    // Die by the Sword 118038 at 40 % (Arms/Fury only: not learned by Protection, so it is a no-op there).
    triggers.push_back(new TriggerNode("warrior health 20", NextAction::array(0,
        new NextAction("shield wall", ACTION_INTERRUPT),
        new NextAction("rallying cry", ACTION_INTERRUPT - 1),
        new NextAction("demoralizing banner", ACTION_INTERRUPT - 2), nullptr)));
    triggers.push_back(new TriggerNode("warrior health 40", NextAction::array(0, new NextAction("die by the sword", ACTION_INTERRUPT), nullptr)));

    //triggers.push_back(new TriggerNode("pummel on enemy healer", NextAction::array(0, new NextAction("pummel on enemy healer", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("pummel", NextAction::array(0, new NextAction("pummel", ACTION_INTERRUPT), nullptr)));
    // Lane 11 (#866): interrupts on any casting enemy (shared "enemy caster in range", lane 0; BG/arena only):
    // Pummel 6552 (prio 10/1), Disrupting Shout 102060 (10/1), Intimidating Shout 5246 (10/1).
    triggers.push_back(new TriggerNode("enemy caster in range", NextAction::array(0,
        new NextAction("pummel on enemy caster", ACTION_INTERRUPT),
        new NextAction("disrupting shout on enemy caster", ACTION_INTERRUPT - 1),
        new NextAction("intimidating shout on enemy caster", ACTION_INTERRUPT - 2), nullptr)));
    // Lane 11 (#866): Avatar 107574 and Skull Banner 114207, prio group 5, all specs.
    triggers.push_back(new TriggerNode("avatar", NextAction::array(0, new NextAction("avatar", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("skull banner can cast", NextAction::array(0, new NextAction("skull banner", ACTION_HIGH), nullptr)));
    // Lane 11 (#866): Berserker Rage while stunned/feared/confused.
    triggers.push_back(new TriggerNode("bot is cc", NextAction::array(0, new NextAction("berserker rage", ACTION_EMERGENCY + 1), nullptr)));

    // #522 (AC GenericWarriorStrategy.cpp:37-38): break fear / sleep / sap with Berserker Rage (18499, L54;
    // 5.4.8 tooltip "removes and grants immunity to Fear, Sap and Incapacitate", Wowhead 2026-10-04).
    triggers.push_back(new TriggerNode("fear sleep sap", NextAction::array(0, new NextAction("berserker rage", ACTION_EMERGENCY + 1), nullptr)));
}

class WarriorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarriorAoeStrategyActionNodeFactory()
    {
        // -- talents
        creators["cleave"] = &WarriorAoeStrategyActionNodeFactory::cleave;
        creators["bladestorm"] = &WarriorAoeStrategyActionNodeFactory::bladestorm;
        creators["dragon roar"] = &WarriorAoeStrategyActionNodeFactory::dragon_roar;  // #522: key/action renamed from "dragon_roar"
        creators["shockwave"] = &WarriorAoeStrategyActionNodeFactory::shockwave;
    }

private:
    // #742 (AC parity, AC GenericWarriorStrategy.cpp:41-51): AC's WarriorAoeStrategyActionNodeFactory is empty -
    // cleave, bladestorm and shockwave have no alternative there (dragon roar is MoP-only: same pattern). The "melee"
    // alternative queued AttackAction above "reach melee" whenever the AoE was refused (shockwave/bladestorm/dragon
    // roar at 25.003 > 21), which froze a mounted warrior in place like the heroic leap chain above.
    ACTION_NODE(cleave, "cleave");
    ACTION_NODE(bladestorm, "bladestorm");
    ACTION_NODE(dragon_roar, "dragon roar");
    ACTION_NODE(shockwave, "shockwave");
};

WarriorAoeStrategy::WarriorAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new WarriorAoeStrategyActionNodeFactory());
}

void WarriorAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
        new NextAction("mocking banner", ACTION_DEFAULT + 5.0f),
        new NextAction("demoralizing banner", ACTION_DEFAULT + 5.0f),
        new NextAction("skull banner", ACTION_DEFAULT + 5.0f),
        new NextAction("bloodbath", 25.0f),
        new NextAction("bladestorm", 25.0f),
        new NextAction("dragon roar", 25.0f),
        new NextAction("shockwave", 25.0f),
        new NextAction("demoralizing shout", 21.0f),
        new NextAction("sweeping strikes", ACTION_HIGH + 7),
        new NextAction("thunder clap", ACTION_HIGH + 5),
        new NextAction("cleave", ACTION_HIGH),
        nullptr)));

    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0,
        new NextAction("mocking banner", ACTION_DEFAULT + 2.0f),
        new NextAction("demoralizing banner", ACTION_DEFAULT + 2.0f),
        new NextAction("skull banner", ACTION_DEFAULT + 2.0f),
        new NextAction("bloodbath", 25.0f),
        new NextAction("bladestorm", 25.0f),
        new NextAction("dragon roar", 25.0f),
        new NextAction("shockwave", 25.0f),
        // #522 (AC GenericWarriorStrategy.cpp:65): Demoralizing Shout 1160 (L56, prot spec) at AC's priority.
        new NextAction("demoralizing shout without life time check", ACTION_HIGH + 1),
        new NextAction("sweeping strikes", ACTION_HIGH + 7),
        new NextAction("thunder clap", ACTION_HIGH + 5),
        new NextAction("cleave", ACTION_HIGH),
        nullptr)));

    // #522 (AC GenericWarriorStrategy.cpp:67-69): stun a snared enemy with Shockwave (46968, talent).
    triggers.push_back(new TriggerNode("shockwave on snare target", NextAction::array(0,
        new NextAction("shockwave on snare target", ACTION_HIGH + 5), nullptr)));
}