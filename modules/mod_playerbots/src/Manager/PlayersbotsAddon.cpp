#include "PlayersbotsAddon.h"
#include "CompanionAccess.h"
#include "Chat.h"
#include "PlayerbotMgr.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Group.h"
#include "Random.h"
#include <sstream>

bool HandlePlayersbotsAddon(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
    if (!CanUseCompanions(player))
    {
        handler->PSendSysMessage("Companions require level %u and account rank %u.",
            uint32(sConfigMgr->GetIntDefault("AiPlayerbot.Companion.MinLevel", 60)),
            uint32(sConfigMgr->GetIntDefault("AiPlayerbot.Companion.MinSecurity", 0)));
        return true;
    }
    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
    if (!mgr) return false;
    std::istringstream in(args ? args : "");
    std::string section, command, extra;
    in >> section >> command;
    if (section == "group")
    {
        unsigned role = 0, count = 1;
        if (!(in >> role) || role < 1 || role > 3) return false;
        if (command == "addRoleBotsToGroup") { if (!(in >> count)) return false; }
        else if (command != "add" && command != "addPvPBotsToGroup") return false;
        if ((in >> extra) || count == 0 || count > 39) return false;
        Group* group = player->GetGroup();
        if (group && (!group->IsLeader(player->GetGUID()) || group->isLFGGroup() || group->isBGGroup()))
        {
            handler->SendSysMessage("Only the leader of a normal party or raid can add companions.");
            return true;
        }
        // The native login path creates a party and promotes it to a raid when needed.
        static char const* heals[] = {"addclass priest 0", "addclass paladin 0", "addclass shaman 2", "addclass druid 2", "addclass monk 1"};
        static char const* tanks[] = {"addclass warrior 2", "addclass paladin 1", "addclass dk 0", "addclass druid 3", "addclass monk 0"};
        static char const* damage[] = {"addclass mage 1", "addclass rogue 1", "addclass hunter 1", "addclass warlock 1", "addclass warrior 0"};
        for (unsigned i = 0; i < count; ++i)
        {
            std::string text((role == 1 ? heals : role == 3 ? tanks : damage)[urand(0, 4)]);
            for (auto const& message : mgr->HandlePlayerbotCommand(&text[0], player))
                handler->SendSysMessage(message.c_str());
        }
        return true;
    }
    if (section == "control" || section == "summon")
    {
        std::string action;
        if (section == "summon") action = "summon";
        else if (command == "attack") action = "attack";
        else if (command == "follow")
        {
            unsigned value; if (!(in >> value) || value > 1) return false;
            action = value ? "follow" : "stay";
        }
        else if (command == "aggressive")
        {
            unsigned value; if (!(in >> value) || value > 1) return false;
            action = value ? "co -passive" : "co +passive";
        }
        else return false;
        if (in >> extra) return false;
        for (Player* bot : mgr->GetPlayerBotsSnapshot())
            if (PlayerbotAI* ai = GET_PLAYERBOT_AI(bot))
                if (ai->GetMaster() == player)
                    ai->HandleCommand(CHAT_MSG_WHISPER, action, player);
        return true;
    }
    if (section == "altchar" && (command == "login" || command == "logout" || command == "list"))
    {
        std::string name; in >> name;
        if (in >> extra) return false;
        std::string text = command + " " + name;
        for (auto const& message : mgr->HandlePlayerbotCommand(&text[0], player))
            handler->SendSysMessage(message.c_str());
        return true;
    }
    handler->SendSysMessage("Usage: .bot group add ROLE; .bot group addRoleBotsToGroup ROLE COUNT; .bot control attack|follow 0/1|aggressive 0/1; .bot summon");
    return true;
}
