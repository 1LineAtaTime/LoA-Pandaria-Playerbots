#include "UseFoodStrategy.h"

#include "Config.h"
#include "Playerbots.h"

#include <sstream>

namespace
{
// #527: AC botAI->HasCheat(BotCheatMask::food) - configured half only (AiPlayerbot.BotCheats, AC
// PlayerbotAIConfig.cpp:545-563, default "food,taxi,raid"); same reader as NonCombatActions.cpp.
bool HasFoodCheat()
{
    static bool const value = []
    {
        std::stringstream ss(sConfigMgr->GetStringDefault("AiPlayerbot.BotCheats", "food,taxi,raid"));
        std::string token;
        while (std::getline(ss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (token == "food")
                return true;
        }
        return false;
    }();
    return value;
}
}  // namespace

// AC Ai/Base/Strategy/UseFoodStrategy.cpp:11-24: with the food cheat a bot refills from medium health /
// below high mana; without it, only when low.
void UseFoodStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    if (HasFoodCheat())
    {
        triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("food", 3.0f), nullptr)));
        triggers.push_back(new TriggerNode("high mana", NextAction::array(0, new NextAction("drink", 3.0f), nullptr)));
    }
    else
    {
        triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("food", 3.0f), nullptr)));
        triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("drink", 3.0f), nullptr)));
    }
}
