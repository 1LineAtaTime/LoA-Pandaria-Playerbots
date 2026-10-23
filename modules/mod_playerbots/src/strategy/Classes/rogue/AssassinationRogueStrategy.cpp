
#include "AssassinationRogueStrategy.h"

#include "Playerbots.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
        // #604/#386 (AC AssassinationRogueStrategy.cpp:16-20): AC's mutilate -> backstab -> sinister strike chain and the
        // rupture -> eviscerate node. Subtlety (no Mutilate, learns Backstab 53 spec 261 L40, cls.py) runs this strategy too.
        creators["backstab"] = &backstab;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* mutilate(PlayerbotAI* ai)
    {
        return new ActionNode("mutilate",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("backstab"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* backstab(PlayerbotAI* ai)
    {
        return new ActionNode("backstab",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("sinister strike"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* rupture(PlayerbotAI* ai)
    {
        return new ActionNode("rupture",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("eviscerate"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* envenom(PlayerbotAI* ai)
    {
        return new ActionNode("envenom",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("eviscerate"), NULL),
                              /*C*/ NULL);
    }
};

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

NextAction** AssassinationRogueStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_DEFAULT), NULL);
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("high energy available",
                                       NextAction::array(0, new NextAction("garrote", ACTION_HIGH + 7),
                                                         new NextAction("ambush", ACTION_HIGH + 6), nullptr)));

    triggers.push_back(new TriggerNode("high energy available",
                                       NextAction::array(0, new NextAction("mutilate", ACTION_NORMAL + 3), NULL)));

    // #386 (AC AssassinationRogueStrategy.cpp:69-80): AC's "hunger for blood" edge is removed -- Hunger for Blood
    // is not learnable by a rogue in 5.4.8 (cls.py rogue: NOT-LEARNABLE; removed before MoP).

    triggers.push_back(new TriggerNode("slice and dice",
                                       NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 5), NULL)));

    // #386 (AC AssassinationRogueStrategy.cpp:117-135): AC fires Envenom at 4 combo points (priority HIGH+5) and
    // on a target that will die soon (HIGH+4). AC's "cold blood" sibling is not learnable in 5.4.8 (cls.py
    // rogue: NOT-LEARNABLE) and is left out. Envenom = 32645, Assassination spec 259, L20 (cls.py).
    triggers.push_back(new TriggerNode("combo points 4 available",
                                       NextAction::array(0, new NextAction("envenom", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode("target with combo points almost dead",
                                       NextAction::array(0, new NextAction("envenom", ACTION_HIGH + 4), NULL)));

    triggers.push_back(
        new TriggerNode("expose armor", NextAction::array(0, new NextAction("expose armor", ACTION_HIGH + 3), NULL)));

    triggers.push_back(
        new TriggerNode("medium threat", NextAction::array(0, new NextAction("vanish", ACTION_HIGH), NULL)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("evasion", ACTION_HIGH + 9),
                                                        new NextAction("feint", ACTION_HIGH + 8), nullptr)));

    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("cloak of shadows", ACTION_HIGH + 7), nullptr)));

    triggers.push_back(
        new TriggerNode("kick", NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(
        new TriggerNode("kick on enemy healer",
                        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fan of knives", ACTION_NORMAL + 5), NULL)));

    // Rotation port (epic #854 / #859), MoP Assassination and Subtlety rows. AC's kit above is kept.
    // This strategy runs Assassination and Subtlety; a spell the spec does not know fails isPossible and is skipped.
    // (prio 10/2, 10/1): Kick any casting enemy; BG/arena only (action gate).
    triggers.push_back(new TriggerNode(
        "enemy caster in range", NextAction::array(0, new NextAction("kick on enemy caster", ACTION_INTERRUPT), NULL)));
    // Marked for Death (0/20), Shadowstep (0/10), Vendetta (0/10).
    triggers.push_back(new TriggerNode(
        "marked for death", NextAction::array(0, new NextAction("marked for death", ACTION_NORMAL + 7), NULL)));
    triggers.push_back(new TriggerNode(
        "shadowstep", NextAction::array(0, new NextAction("shadowstep", ACTION_NORMAL + 6), NULL)));
    triggers.push_back(new TriggerNode(
        "vendetta", NextAction::array(0, new NextAction("vendetta", ACTION_NORMAL + 5), NULL)));
    // Dispatch (0/4): above Mutilate (ACTION_NORMAL + 3); the game limits it to targets under 35% hp.
    triggers.push_back(new TriggerNode(
        "high energy available", NextAction::array(0, new NextAction("dispatch", ACTION_NORMAL + 4), NULL)));
    // Hemorrhage (Subtlety, 0/2): refresh the debuff.
    triggers.push_back(new TriggerNode(
        "hemorrhage", NextAction::array(0, new NextAction("hemorrhage", ACTION_NORMAL + 4), NULL)));
    // Group 5: Smoke Bomb (<= 20% hp; BG/arena gate), Recuperate (<= 30% hp, 5 CP),
    // Shadow Blades, Preparation.
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("smoke bomb", ACTION_HIGH + 6), NULL)));
    triggers.push_back(new TriggerNode(
        "recuperate", NextAction::array(0, new NextAction("recuperate", ACTION_HIGH + 4), NULL)));
    triggers.push_back(new TriggerNode(
        "shadow blades", NextAction::array(0, new NextAction("shadow blades", ACTION_HIGH + 2), NULL)));
    triggers.push_back(new TriggerNode(
        "preparation", NextAction::array(0, new NextAction("preparation", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low tank threat",
        NextAction::array(0, new NextAction("tricks of the trade on main tank", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("stealth", ACTION_HIGH + 3), new NextAction("sprint", ACTION_HIGH + 2),
                          new NextAction("reach melee", ACTION_HIGH + 1), NULL)));
}
