#pragma once

#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include "CompanionPolicy.h"

// Enforced on the server, independently of client addons and command aliases.
inline bool CanUseCompanions(Player* player)
{
    if (!player || !player->GetSession())
        return false;
    return CompanionPolicy::Allowed(player->GetLevel(), player->GetSession()->GetSecurity(),
        player->GetSession()->IsBot(), sConfigMgr->GetIntDefault("AiPlayerbot.Companion.MinLevel", 60),
        sConfigMgr->GetIntDefault("AiPlayerbot.Companion.MinSecurity", 0),
        sConfigMgr->GetBoolDefault("AiPlayerbot.Companion.GMBypass", true));
}
