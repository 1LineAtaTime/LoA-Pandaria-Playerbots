#include "EmoteStrategy.h"

#include "Config.h"
#include "Playerbots.h"

namespace
{
    // AC keys (AC PlayerbotAIConfig.cpp:297-298,625; conf/playerbots.conf.dist:2574-2580). Not present
    // on this fork's PlayerbotAIConfig - read locally, the way LootAction.cpp reads
    // AiPlayerbot.FreeMethodLoot, instead of adding them to the widely included PlayerbotAIConfig.h.
    bool RandomBotEmote()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotEmote", false);
        return v;
    }

    // #322: AC PlayerbotAIConfig.cpp:297 (randomBotTalk, AC default false). Default follows this fork's
    // playerbots.conf.dist (AiPlayerbot.RandomBotTalk = 1) per the config-default rule.
    bool RandomBotTalk()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotTalk", true);
        return v;
    }

    bool EnableGreet()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableGreet", true);
        return v;
    }
}

void EmoteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #194 (S8, AC parity EmoteStrategy.cpp:10-20): idle talk/emote chatter and the "receive a real
    // player's emote" reactions share the same AC gate.
    if (RandomBotEmote())
    {
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("talk", 1.0f), nullptr)));
        triggers.push_back(new TriggerNode("seldom", NextAction::array(0, new NextAction("emote", 1.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("receive text emote", NextAction::array(0, new NextAction("emote", 10.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("receive emote", NextAction::array(0, new NextAction("emote", 10.0f), nullptr)));
    }

    // #322, AC parity (EmoteStrategy.cpp:22-29): random-bot world talk. "suggest what to do" /
    // "suggest dungeon" / "suggest trade" are SuggestWhatToDoAction.h; they post through BroadcastHelper.
    if (RandomBotTalk())
    {
        triggers.push_back(new TriggerNode("often", NextAction::array(0,
            new NextAction("suggest what to do", 10.0f),
            new NextAction("suggest dungeon", 3.0f),
            new NextAction("suggest trade", 3.0f), nullptr)));
    }

    // AC parity (EmoteStrategy.cpp:30-32).
    if (EnableGreet())
        triggers.push_back(
            new TriggerNode("new player nearby", NextAction::array(0, new NextAction("greet", 1.0f), nullptr)));

    // AC parity (EmoteStrategy.cpp:34, issue #609): unconditional, mounted idle bots send mount-special-anim.
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("rpg mount anim", 1.0f), nullptr)));
}
