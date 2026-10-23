/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Strategy/RpgStrategy.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 *
 * 5.4.8 adaptation: AC builds TriggerNodes with brace-initialised std::vector<NextAction>; this fork uses the old
 * NextAction::array(0, new NextAction(...), nullptr) API (strategy/Action.h:27-31, same idiom as
 * strategy/RPG/NewRpgStrategy.cpp). Names and relevances are byte-identical to AC. InitTriggers lists exactly the
 * nodes AC lists (no "rpg duel", "rpg sell", "rpg train", "rpg taxi", "rpg queue bg", "rpg spell", "rpg craft",
 * "rpg trade useful", "rpg mount anim" nodes).
 */

#include "RpgStrategy.h"
#include "Playerbots.h"
#include "RpgSubActions.h"

float RpgActionMultiplier::GetValue(Action* action)
{
    if (action == nullptr)
        return 1.0f;

    std::string const nextAction = AI_VALUE(std::string, "next rpg action");
    std::string const name = action->getName();

    if (!nextAction.empty() && dynamic_cast<RpgEnabled*>(action) && name != nextAction)
        return 0.0f;

    return 1.0f;
}

RpgStrategy::RpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

NextAction** RpgStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("rpg", 1.0f), nullptr);
}

void RpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no rpg target",
            NextAction::array(0, new NextAction("choose rpg target", 5.0f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            NextAction::array(0, new NextAction("move random", 1.10f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from rpg target",
            NextAction::array(0, new NextAction("move to rpg target", 5.0f), nullptr)
        )
    );

    // Sub actions
    triggers.push_back(
        new TriggerNode(
            "rpg",
            NextAction::array(0, new NextAction("rpg stay", 1.101f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            NextAction::array(0, new NextAction("rpg work", 1.101f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            NextAction::array(0, new NextAction("rpg emote", 1.101f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "has rpg target",
            NextAction::array(0, new NextAction("rpg cancel", 1.101f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg discover",
            NextAction::array(0, new NextAction("rpg discover", 1.210f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg start quest",
            NextAction::array(0, new NextAction("rpg start quest", 1.180f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg end quest",
            NextAction::array(0, new NextAction("rpg end quest", 1.190f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy",
            NextAction::array(0, new NextAction("rpg buy", 1.130f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg repair",
            NextAction::array(0, new NextAction("rpg repair", 1.195f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg heal",
            NextAction::array(0, new NextAction("rpg heal", 1.125f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg home bind",
            NextAction::array(0, new NextAction("rpg home bind", 1.160f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy petition",
            NextAction::array(0, new NextAction("rpg buy petition", 1.140f), nullptr)
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg use",
            NextAction::array(0, new NextAction("rpg use", 1.102f), nullptr)
        )
    );
}

void RpgStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(botAI));
}
