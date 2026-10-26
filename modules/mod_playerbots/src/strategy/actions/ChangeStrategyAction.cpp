#include "ChangeStrategyAction.h"

#include "Event.h"
#include "Helper.h"
#include "LastMovementValue.h"
#include "PlayerbotRepository.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool ChangeCombatStrategyAction::Execute(Event event)
{
    std::string const text = event.getParam();
    botAI->ChangeStrategy(text.empty() ? getName() : text, BOT_STATE_COMBAT);
    if (event.GetSource() == "co")
    {
        std::vector<std::string> splitted = split(text, ',');
        for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
        {
            const char* name = i->c_str();
            switch (name[0])
            {
            case '+':
            case '-':
            case '~':
                PlayerbotRepository::instance().Save(botAI);
                break;
            case '!':
                botAI->SelectiveResetStrategies(BOT_STATE_COMBAT);
                PlayerbotRepository::instance().Save(botAI);
                break;
            case '?':
                break;
            }
        }
    }

    return true;
}

bool ChangeNonCombatStrategyAction::Execute(Event event)
{
    std::string const text = event.getParam();

    uint32 account = bot->GetSession()->GetAccountId();
    if (sPlayerbotAIConfig->IsInRandomAccountList(account) && botAI->GetMaster() &&
        botAI->GetMaster()->GetSession()->GetSecurity() < SEC_GAMEMASTER)
    {
        if (text.find("loot") != std::string::npos || text.find("gather") != std::string::npos)
        {
            botAI->TellError("You can change any strategy except loot and gather");
            return false;
        }
    }

    botAI->ChangeStrategy(text, BOT_STATE_NON_COMBAT);
    if (event.GetSource() == "nc")
    {
        std::vector<std::string> splitted = split(text, ',');
        for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
        {
            const char* name = i->c_str();
            switch (name[0])
            {
            case '+':
            case '-':
            case '~':
                PlayerbotRepository::instance().Save(botAI);
                break;
            case '!':
                botAI->SelectiveResetStrategies(BOT_STATE_NON_COMBAT);
                PlayerbotRepository::instance().Save(botAI);
                break;
            case '?':
                break;
            }
        }
    }

    return true;
}

bool ChangeDeadStrategyAction::Execute(Event event)
{
    std::string const text = event.getParam();
    botAI->ChangeStrategy(text, BOT_STATE_DEAD);
    return true;
}

// Ported from AC mod-playerbots CombatActions.cpp (#149); ammo check dropped, see header comment.
bool SwitchToMeleeAction::isUseful()
{
    if (bot->GetClass() == CLASS_HUNTER)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
        return botAI->HasStrategy("ranged", BOT_STATE_COMBAT) &&
               ((bot->IsInCombat() && target &&
                 (target->GetVictim() == bot && (!bot->GetGroup() || lastFlee) &&
                  sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), 8.0f))) ||
                (!bot->IsInCombat()));
    }

    return botAI->HasStrategy("ranged", BOT_STATE_COMBAT);
}

bool SwitchToRangedAction::isUseful()
{
    if (bot->GetClass() == CLASS_HUNTER)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        return botAI->HasStrategy("close", BOT_STATE_COMBAT) &&
               ((bot->IsInCombat() && target &&
                 ((target->GetVictim() != bot || target->GetTarget() != bot->GetGUID()) ||
                  sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), 8.0f))) ||
                (!bot->IsInCombat()));
    }

    return botAI->HasStrategy("close", BOT_STATE_COMBAT);
}
