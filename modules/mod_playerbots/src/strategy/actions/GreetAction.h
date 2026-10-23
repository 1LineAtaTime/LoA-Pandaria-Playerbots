#ifndef _PLAYERBOT_GREETACTION_H
#define _PLAYERBOT_GREETACTION_H

#include "Action.h"

class PlayerbotAI;

// AC parity (Ai/Base/Actions/GreetAction.{h,cpp}, issue #194 S8). Waves/greets a newly-seen nearby
// player. Wired onto "new player nearby" in EmoteStrategy.cpp, gated behind AiPlayerbot.EnableGreet
// (AC default true), the same as AC's own EmoteStrategy wiring.
class GreetAction : public Action
{
public:
    GreetAction(PlayerbotAI* botAI) : Action(botAI, "greet") {}

    bool Execute(Event event) override;
};

#endif
