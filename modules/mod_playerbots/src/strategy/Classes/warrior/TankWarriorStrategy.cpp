/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TankWarriorStrategy.h"

#include "Playerbots.h"

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["commanding shout"] = &commanding_shout;
        creators["demoralizing shout"] = &demoralizing_shout;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["revenge"] = &revenge;
        creators["shield barrier"] = &shield_barrier;
        creators["shield block"] = &shield_block;
        creators["shield slam"] = &shield_slam;
        creators["taunt"] = &taunt;
        creators["enraged regeneration"] = &enraged_regeneration;
        creators["vigilance"] = &vigilance;
        creators["storm bolt"] = &storm_bolt;
        creators["mocking banner"] = &mocking_banner;
        // #522 (AC TankWarriorStrategy.cpp:36-44): chain heroic throw on snare target -> taunt on snare target.
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
    }

private:
    ACTION_NODE(commanding_shout, "commanding shout");
    ACTION_NODE(demoralizing_shout, "demoralizing shout");
    ACTION_NODE_A(devastate, "devastate", "sunder armor");
    // #522 (AC TankWarriorStrategy.cpp:46-54): last stand -> intimidating shout.
    ACTION_NODE_A(last_stand, "last stand", "intimidating shout");
    ACTION_NODE(revenge, "revenge");
    ACTION_NODE(shield_barrier, "shield barrier");
    ACTION_NODE(shield_block, "shield block");
    ACTION_NODE(shield_slam, "shield slam");
    ACTION_NODE(taunt, "taunt");
    ACTION_NODE(enraged_regeneration, "enraged regeneration");
    ACTION_NODE(vigilance, "vigilance");
    ACTION_NODE(storm_bolt, "storm bolt");
    ACTION_NODE(mocking_banner, "mocking banner");
    ACTION_NODE_A(heroic_throw_on_snare_target, "heroic throw on snare target", "taunt on snare target");
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

NextAction** TankWarriorStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("shield slam", ACTION_DEFAULT + 0.5f),
        new NextAction("storm bolt", ACTION_DEFAULT + 0.4f),
        // Lane 11 (#866): "Dragon Roar" (class row, 0/7), talent, target within melee.
        new NextAction("dragon roar", ACTION_DEFAULT + 0.35f),
        new NextAction("devastate", ACTION_DEFAULT + 0.3f),
        new NextAction("revenge", ACTION_DEFAULT + 0.2f),
        new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void TankWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    // #522 (AC TankWarriorStrategy.cpp:153-158): Defensive Stance at ACTION_HIGH + 9. The generic strategy only registers it at
    // ACTION_DEFAULT, below the tank defaults (shield slam ACTION_DEFAULT + 0.5), so it was starved while any default succeeded.
    // Shield Slam generates 20 Rage in Defensive Stance (Wowhead mop-classic tooltip 23922, 2026-10-05).
    triggers.push_back(new TriggerNode("defensive stance", NextAction::array(0, new NextAction("defensive stance", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("commanding shout", NextAction::array(0, new NextAction("commanding shout", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(new TriggerNode("demoralizing shout", NextAction::array(0, new NextAction("demoralizing shout", ACTION_HIGH + 8), nullptr)));

    triggers.push_back(new TriggerNode("vigilance", NextAction::array(0, new NextAction("vigilance", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("shield block", NextAction::array(0, new NextAction("shield block", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("revenge", NextAction::array(0, new NextAction("revenge", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("lose aggro", NextAction::array(0, new NextAction("taunt", ACTION_INTERRUPT + 1), nullptr)));
    // #835 (AC TankWarriorStrategy.cpp:225-232, :36-44): AC has no direct "taunt on snare target" -> taunt edge. Its
    // only "taunt on snare target" trigger casts Heroic Throw (below, ACTION_INTERRUPT), and Taunt is that node's
    // alternative. The fork-only direct edge (ACTION_INTERRUPT + 1, from #163 S8) taunted the snare target, which since
    // the #2855 port can be an enemy player in a BG (w64 EotS: Mericar Taunt 355 on player 0:44, lastact
    // "taunt on snare target"). Removed.
    triggers.push_back(new TriggerNode("shield barrier", NextAction::array(0, new NextAction("shield barrier", ACTION_HIGH), nullptr)));
    // #790 (AC TankWarriorStrategy.cpp:184-192): "medium rage available" -> Shield Slam ACTION_HIGH + 2, Devastate
    // ACTION_HIGH + 1. The port only had both in the defaults (ACTION_DEFAULT + 0.5 / + 0.3), under Revenge (HIGH + 2),
    // Thunder Clap (HIGH + 1, aoe HIGH + 5), Shield Block (HIGH + 1) and Heroic Strike / Cleave (HIGH), so a busy tank
    // rarely reached Shield Slam (wave 35 SNT Pavalak, Viara: 37 Shield Slams in a 481 s pull, 944 default pushes).
    // 5.4.8 Shield Slam costs no rage (it generates 20 in Defensive Stance; 6 sec cooldown) and Devastate costs none
    // (Wowhead mop-classic tooltips 23922 / 20243, 2026-10-12), so the defaults stay as the low-rage path and this
    // edge restores AC's priority from 40 rage up.
    triggers.push_back(new TriggerNode("medium rage available", NextAction::array(0,
        new NextAction("shield slam", ACTION_HIGH + 2),
        new NextAction("devastate", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium rage available",NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 1), nullptr)));
    // Lane 11 (#866, Thunder Clap 6343, prio 0/7): single target, target within 7 yd and
    // NOT carrying our Weakened Blows (115798). Between Shield Slam / Revenge (HIGH + 2) and Devastate (HIGH + 1): in
    // the MoP list Devastate (0/8, "always") sits above it, but it is the debuff-gated refresh that keeps the 10 % damage
    // reduction up, and w74 saw 25 pushes at HIGH + 1 / 0 casts (NOT_READY behind Devastate every tick).
    triggers.push_back(new TriggerNode("thunder clap", NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 1.5f), nullptr)));
    // Lane 11 (#866): Battle Shout 6673 is also in the Protection list (the tank only
    // had Commanding Shout); BuffTrigger fires only when the aura is missing.
    triggers.push_back(new TriggerNode("battle shout", NextAction::array(0, new NextAction("battle shout", ACTION_HIGH + 7), nullptr)));
    // #163 S8 (AC parity, TankWarriorStrategy.cpp:250-256): AC's "challenging shout" is N/A (removed
    // in 5.0.4); mapped to the MoP-only Mocking Banner instead - see the WarriorActions.h comment.
    triggers.push_back(new TriggerNode("high aoe", NextAction::array(0, new NextAction("mocking banner", ACTION_HIGH + 3), nullptr)));

    // #522 (AC TankWarriorStrategy.cpp:134-142): Heroic Throw 57755 (L22, 30 yd) on an out-of-melee enemy, ahead of
    // charge (generic strategy, ACTION_MOVE + 10).
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 11), nullptr)));
    // #522 (AC :209-216): Disarm 676 (L28).
    triggers.push_back(new TriggerNode("disarm", NextAction::array(0, new NextAction("disarm", ACTION_HIGH + 1), nullptr)));
    // #522 (AC :225-232): pull a snared enemy with Heroic Throw.
    triggers.push_back(new TriggerNode("taunt on snare target", NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_INTERRUPT), nullptr)));
    // #522 (AC :241-249): Last Stand 12975 (prot spec L38, +30% health 20s) and Enraged Regeneration 55694 (talent).
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0,
        new NextAction("last stand", ACTION_EMERGENCY + 3),
        new NextAction("enraged regeneration", ACTION_EMERGENCY + 2),
        nullptr)));
    // #522 (AC :282-289): Spell Reflection 23920 (L66, reflects the next spell for 5s).
    triggers.push_back(new TriggerNode("spell reflection", NextAction::array(0, new NextAction("spell reflection", ACTION_INTERRUPT + 1), nullptr)));
    // #522 (AC :322-329): Intervene 3411 (L72, 25 yd, intercepts the next attack on the target).
    triggers.push_back(new TriggerNode("protect party member", NextAction::array(0, new NextAction("intervene", ACTION_EMERGENCY), nullptr)));
    // #522 (AC TankWarriorStrategy.cpp:299-306): "sword and board" -> Shield Slam at ACTION_INTERRUPT. 5.4.8 Sword
    // and Board (46953 passive, proc buff 50227): "Devastate has a 50% chance of resetting the cooldown of your
    // Shield Slam" (Wowhead mop-classic tooltip 50227, 2026-10-05), so the proc buff means Shield Slam is ready.
    triggers.push_back(new TriggerNode("sword and board", NextAction::array(0, new NextAction("shield slam", ACTION_INTERRUPT), nullptr)));
    // #522 (AC :330-337): Heroic Strike 78 rage dump at 60+ rage.
    triggers.push_back(new TriggerNode("high rage available", NextAction::array(0, new NextAction("heroic strike", ACTION_HIGH), nullptr)));

    // Lane 11 (#866, MoP thresholds): Last Stand 12975 at 20 %, Demoralizing Shout 1160 at
    // 50 %.
    triggers.push_back(new TriggerNode("warrior health 20", NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("warrior health 50", NextAction::array(0, new NextAction("demoralizing shout", ACTION_HIGH + 8), nullptr)));

    // #522 DIVERGENCES (AC edge not ported; not learnable in 5.4.8, cls.py 2026-10-04): "bloodrage", "concussion blow",
    // "shield bash" (+ on enemy healer), "rend" (+ on attacker), "high aoe -> challenging shout" (already mapped to
    // mocking banner above, #163 S8).
}
