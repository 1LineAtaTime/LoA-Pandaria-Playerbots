/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DeadStrategy.h"

#include "Playerbots.h"

void DeadStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("auto release", relevance), nullptr)));
    // Ported from AC mod-playerbots DeadStrategy.cpp:16-17. Inside a battleground the corpse run
    // is disabled (FindCorpseAction returns false there), so this is the only trigger that gets a
    // dead battleground bot back: "auto release" -> AutoReleaseSpiritAction, whose battleground
    // branch teleports to / gossips the friendly spirit healer. This line was commented out,
    // which is why bots stayed ghosts for the whole match (issue #99).
    triggers.push_back(new TriggerNode("bg active", NextAction::array(0, new NextAction("auto release", relevance), nullptr)));
    triggers.push_back(new TriggerNode("dead", NextAction::array(0, new NextAction("find corpse", relevance), nullptr)));
    triggers.push_back(new TriggerNode("corpse near", NextAction::array(0, new NextAction("revive from corpse", relevance - 1.0f), nullptr)));

    // Ported from AC mod-playerbots DeadStrategy.cpp:28-29. SelfResurrectTrigger is added below in
    // TriggerContext.h because GenericTriggers.h (where upstream defines it) is outside this
    // slice's allowed files. NOTE: the "self resurrect" action is not registered in this fork
    // (no SelfResurrectAction and no ActionContext entry), so this line is inert until it is.
    triggers.push_back(new TriggerNode("can self resurrect", NextAction::array(0, new NextAction("self resurrect", relevance + 2.0f), nullptr)));

    // Ported from AC mod-playerbots DeadStrategy.cpp:24-25. IsFallingFarTrigger already exists in
    // this fork's GenericTriggers.h, it was just never registered in TriggerContext. NOTE: the
    // "repop" action (RepopAction) is not registered in this fork either, so this line is inert
    // until RepopAction is wired into an ActionContext.
    triggers.push_back(new TriggerNode("falling far", NextAction::array(0, new NextAction("repop", relevance + 1.f), nullptr)));

    // Ported from AC mod-playerbots DeadStrategy.cpp:22-23 (#149). "resurrect request" is now wired in
    // WorldPacketTriggerContext.h / PlayerbotAI.cpp (SMSG_RESURRECT_REQUEST), and "accept resurrect" is
    // registered in WorldPacketActionContext.h (AcceptResurrectAction, ReleaseSpiritAction.h).
    triggers.push_back(new TriggerNode("resurrect request", NextAction::array(0, new NextAction("accept resurrect", relevance), nullptr)));

    // Not ported: "location stuck" -> "repop" (upstream DeadStrategy.cpp:26-27). Upstream's
    // DeadStrategy references it, but no "location stuck" trigger is defined anywhere in the
    // reference module, so there is nothing to port.
    //triggers.push_back(new TriggerNode("location stuck", NextAction::array(0, new NextAction("repop", relevance + 1), nullptr)));
}

DeadStrategy::DeadStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}
