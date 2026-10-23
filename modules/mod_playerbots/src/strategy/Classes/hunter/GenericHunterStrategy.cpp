/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericHunterStrategy.h"

#include "Playerbots.h"

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        // #521 (AC GenericHunterStrategy.cpp:96-123): AC's rapid fire -> readiness and mongoose bite -> raptor
        // strike -> melee fallback chains are dropped: Readiness, Raptor Strike, Wing Clip and Mongoose Bite
        // are not learnable by a 5.4.8 hunter (cls.py hunter 2026-10-04: NOT-LEARNABLE / NO-SUCH-NAME).
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["feign death"] = &feign_death;
    }

private:
    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the cheetah"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* feign_death([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("feign death",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // #521 (AC GenericHunterStrategy.cpp:162-164): enemy within melee -> explosive trap (AC raw 37.0). AC's
    // mongoose bite / wing clip alternatives are dropped (not learnable in 5.4.8, cls.py hunter 2026-10-04);
    // Explosive Trap 13813 is learnable (L38, cls.py; Wowhead mop-classic tooltip 2026-10-04: fire trap that
    // explodes when an enemy approaches). AC's explosive trap -> immolation trap fallback is dropped too
    // (Immolation Trap is not learnable per the 5.4.8 DBC, see the trap weave note in HunterTriggers.h).
    triggers.push_back(new TriggerNode("enemy within melee",
                                       NextAction::array(0, new NextAction("explosive trap", 37.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("hunter medium threat", NextAction::array(0, new NextAction("feign death", 35.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet medium health",
                                       NextAction::array(0, new NextAction("mend pet", 22.0f), nullptr)));
    // 5.4.8: ammo removed in Cataclysm, no MoP equivalent (#149).
    // triggers.push_back(new TriggerNode("no ammo", NextAction::array(0, new NextAction("switch to melee", ACTION_HIGH
    // + 1), new NextAction("say::no ammo", ACTION_HIGH), nullptr)));
    // #521: AC's "aspect of the viper" edge (AC GenericHunterStrategy.cpp:146) is removed: Aspect of the Viper
    // does not exist in the 5.4.8 DBC (cls.py hunter 2026-10-04: NO-SUCH-NAME) and MoP hunters use Focus.
    // #521 (AC GenericHunterStrategy.cpp:166-167): disengage first, flee as the fallback (AC 35 / 34). The
    // fork's flee keeps its existing ACTION_MOVE + 9; disengage sits one step above so AC's order is kept.
    // #380 (AC GenericHunterStrategy.cpp:166-167): AC's literal numbers 35 / 34 (were ACTION_MOVE+10/+9). Disengage 781.
    // Rotation fix lane K (#868): "hunter enemy too close" = this edge outside a pvp instance / started duel (HunterTriggers.h).
    triggers.push_back(new TriggerNode("hunter enemy too close",
                                       NextAction::array(0, new NextAction("disengage", 35.0f),
                                                         new NextAction("flee", 34.0f), nullptr)));
    // #521 (AC GenericHunterStrategy.cpp:145): rapid fire (Rapid Fire 3045, L54, cls.py) in every hunter combat engine.
    triggers.push_back(
        new TriggerNode("rapid fire", NextAction::array(0, new NextAction("rapid fire", 29.0f), nullptr)));
    // #521 (AC GenericHunterStrategy.cpp:155): pet low health -> mend pet (Mend Pet 136, L16, cls.py); the
    // medium-health edge above already exists.
    // #380 (AC GenericHunterStrategy.cpp:144,149,152): hunter's mark 29.5, has aggro / snare target -> concussive
    // shot 20.0. These used to live only in the fork's `dps` strategy (31.0 / 20 / 20), which AiFactory no longer adds
    // to hunters, so they are restored here for bm/mm/surv. Hunter's Mark 1130 (L14), Concussive Shot 5116 (L8), cls.py.
    triggers.push_back(
        new TriggerNode("hunter's mark", NextAction::array(0, new NextAction("hunter's mark", 29.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("has aggro", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("concussive shot on snare target",
                                       NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet low health",
                                       NextAction::array(0, new NextAction("mend pet", 21.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low tank threat",
                        NextAction::array(0, new NextAction("misdirection on main tank", 27.0f), NULL)));
    // #380 (AC GenericHunterStrategy.cpp:148): Deterrence 19263 (L78) at AC's 35.0 (was ACTION_HIGH+5 = 25).
    triggers.push_back(
        new TriggerNode("hunter low health", NextAction::array(0, new NextAction("deterrence", 35.0f), nullptr)));
    
    // #856 (epic #854) MoP group 10 interrupts: Counter Shot 147362 on the target just above AC's
    // Silencing Shot 40.0 (ACTION_INTERRUPT), and on any casting enemy in range (25, PvP maps only, lane 0 "enemy caster in range").
    triggers.push_back(new TriggerNode("counter shot",
                                       NextAction::array(0, new NextAction("counter shot", ACTION_INTERRUPT + 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("enemy caster in range",
                                       NextAction::array(0, new NextAction("counter shot on enemy caster", ACTION_INTERRUPT), nullptr)));
    // #856 (prio group 5): Deterrence at health <= 30 %, above AC's "low health" edge (35.0).
    triggers.push_back(new TriggerNode("deterrence health",
                                       NextAction::array(0, new NextAction("deterrence", 36.0f), nullptr)));
    // #856 (prio group 4): Disengage when an enemy player in melee range attacks me (BG/arena only).
    triggers.push_back(new TriggerNode("disengage pvp",
                                       NextAction::array(0, new NextAction("disengage", 36.5f), nullptr)));
    triggers.push_back(new TriggerNode("tranquilizing shot enrage",
                                       NextAction::array(0, new NextAction("tranquilizing shot", 61.0f), NULL)));
    triggers.push_back(new TriggerNode("tranquilizing shot magic",
                                       NextAction::array(0, new NextAction("tranquilizing shot", 61.0f), NULL)));
    // Rotation fix lane K (#868, BEYOND AC): re-target the duel opponent after "drop target" (see HunterDuelRetargetTrigger).
    triggers.push_back(new TriggerNode("hunter duel retarget",
                                       NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));
}

NextAction** HunterBoostStrategy::getDefaultActions()
{
    // Rotation fix lane K (#868): AC's "boost" default Bestial Wrath (AC GenericHunterStrategy.cpp:67) is BM-only on 5.4.8
    // (19574 is spec253, forkonly_verdicts.txt) and BeastMasteryHunterStrategy keeps its own default at 19.0; on SV/MM it was a
    // "[CAST] spell=0 why=no-spellid" every tick (342x on w76 Merick). The "rapid fire" boost edge below is unchanged.
    return nullptr;
}

void HunterBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("rapid fire", NextAction::array(0, new NextAction("rapid fire", 16.0f), nullptr)));
}

void HunterCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "scare beast", NextAction::array(0, new NextAction("scare beast on cc", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "freezing trap", NextAction::array(0, new NextAction("freezing trap on cc", ACTION_HIGH + 3), nullptr)));
}
