#ifndef _PLAYERBOT_EMOTESTRATEGY_H
#define _PLAYERBOT_EMOTESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/EmoteStrategy.{h,cpp}), completed for issue #194 (S8). AC gates
// "often"->"talk", "seldom"->"emote" and the two "receive a real player's emote" triggers behind
// AiPlayerbot.RandomBotEmote (default false), and "new player nearby"->"greet" behind
// AiPlayerbot.EnableGreet (default true); both are read locally via sConfigMgr in EmoteStrategy.cpp,
// the way LootAction.cpp reads AiPlayerbot.FreeMethodLoot, instead of being added to
// PlayerbotAIConfig.h. TalkAction/GreetAction now exist (TalkAction.h/.cpp, GreetAction.h/.cpp).
// The unconditional "often"->"rpg mount anim" line (AC EmoteStrategy.cpp:34) is ported in #609.
class EmoteStrategy : public Strategy
{
public:
    EmoteStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "emote"; }
};

#endif
