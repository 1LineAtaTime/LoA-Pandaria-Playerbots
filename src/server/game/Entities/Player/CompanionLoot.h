#pragma once
#include "Config.h"
#include "Group.h"
#include "Player.h"
#include "WorldSession.h"

inline bool IsGoldOnlyCompanion(Player const* player)
{
    if (!player || !player->GetSession() || !player->GetSession()->IsBot() ||
        !sConfigMgr->GetBoolDefault("AiPlayerbot.Companion.GoldOnlyLoot", true))
        return false;
    Group const* group = player->GetGroup();
    if (!group) return false;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
        if (Player* member = ref->GetSource())
            if (member->GetSession() && !member->GetSession()->IsBot()) return true;
    return false;
}
