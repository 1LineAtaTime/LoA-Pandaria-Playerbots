/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Trigger/LfgTriggers.cpp) into the
 * Legends of Azeroth Pandaria fork (#210 S10).
 *
 * UnknownDungeonTrigger::IsActive - AC tests `IsRealPlayer(botAI->GetMaster())`, a free function that
 * asks "is this a live human client" (true for a real player, false for any bot). This fork's own
 * `PlayerbotAI::IsRealPlayer()` means the opposite ("I am a self-bot"), so the established mapping
 * (see PassLeadershipToMasterAction.cpp's class comment) is: AC's `IsRealPlayer(master)` becomes
 * "master has no PlayerbotAI" here.
 */

#include "LfgTriggers.h"

#include "Map.h"
#include "Playerbots.h"

bool LfgProposalActiveTrigger::IsActive() { return AI_VALUE(uint32, "lfg proposal") != 0; }

bool UnknownDungeonTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master || !master->IsInWorld())
        return false;

    bool const masterIsHuman = !GET_PLAYERBOT_AI(master);
    return masterIsHuman && master->GetMap()->IsDungeon() && bot->GetMapId() == master->GetMapId();
}
