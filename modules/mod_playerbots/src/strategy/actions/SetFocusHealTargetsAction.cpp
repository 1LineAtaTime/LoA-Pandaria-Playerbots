#include "SetFocusHealTargetsAction.h"

#include <algorithm>
#include <sstream>

#include "Event.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

namespace
{
    std::string Lowercase(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    Player* FindGroupPlayerByName(Player* bot, std::string const& lowerName)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && Lowercase(member->GetName()) == lowerName)
                return member;
        }

        return nullptr;
    }
}

bool SetFocusHealTargetsAction::Execute(Event event)
{
    if (!PlayerBotSpec::IsHeal(bot))
    {
        botAI->TellError("I'm not a healer");
        return false;
    }

    std::string const param = Lowercase(event.getParam());
    if (param.empty())
    {
        botAI->TellError("Please provide one or more player names (+name to add, -name to remove, clear, or ?)");
        return false;
    }

    GuidVector focusHealTargets = AI_VALUE(GuidVector, "focus heal targets");

    if (param.find('?') != std::string::npos)
    {
        if (focusHealTargets.empty())
        {
            botAI->TellMaster("I don't have any focus heal targets");
            return true;
        }

        std::stringstream names;
        for (GuidVector::const_iterator it = focusHealTargets.begin(); it != focusHealTargets.end(); ++it)
        {
            Unit* target = botAI->GetUnit(*it);
            if (target)
            {
                if (it != focusHealTargets.begin())
                    names << ", ";
                names << target->GetName();
            }
        }
        botAI->TellMaster("My focus heal targets are " + names.str());
        return true;
    }

    if (param == "none" || param == "unset" || param == "clear")
    {
        focusHealTargets.clear();
        SET_AI_VALUE(GuidVector, "focus heal targets", focusHealTargets);
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
        botAI->TellMaster("Removed focus heal targets");
        return true;
    }

    std::vector<std::string> targetNames;
    if (param.find(',') != std::string::npos)
    {
        std::string name;
        std::stringstream ss(param);
        while (std::getline(ss, name, ','))
            targetNames.push_back(name);
    }
    else
        targetNames.push_back(param);

    if (!bot->GetGroup())
    {
        botAI->TellError("I'm not in a group");
        return false;
    }

    for (std::string const& token : targetNames)
    {
        bool const add = token.find('+') != std::string::npos;
        bool const remove = token.find('-') != std::string::npos;
        if (!add && !remove)
        {
            botAI->TellError("Please specify a + for add or - to remove a target");
            continue;
        }

        std::string playerName = token;
        playerName.erase(std::remove(playerName.begin(), playerName.end(), '+'), playerName.end());
        playerName.erase(std::remove(playerName.begin(), playerName.end(), '-'), playerName.end());

        Player* target = FindGroupPlayerByName(bot, playerName);
        if (!target)
        {
            botAI->TellError("I'm not in a group with " + playerName);
            continue;
        }

        ObjectGuid const targetGuid = target->GetGUID();
        if (add)
        {
            if (std::find(focusHealTargets.begin(), focusHealTargets.end(), targetGuid) == focusHealTargets.end())
                focusHealTargets.push_back(targetGuid);
            botAI->TellMaster("Added " + playerName + " to focus heal targets");
        }
        else
        {
            focusHealTargets.erase(std::remove(focusHealTargets.begin(), focusHealTargets.end(), targetGuid),
                                    focusHealTargets.end());
            botAI->TellMaster("Removed " + playerName + " from focus heal targets");
        }
    }

    SET_AI_VALUE(GuidVector, "focus heal targets", focusHealTargets);

    if (focusHealTargets.empty())
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
    else
        botAI->ChangeStrategy("+focus heal targets", BOT_STATE_COMBAT);

    return true;
}
