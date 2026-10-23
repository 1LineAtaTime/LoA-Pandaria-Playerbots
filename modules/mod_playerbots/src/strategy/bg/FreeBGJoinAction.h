/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * #611: AC FreeBGJoinAction (Ai/Base/Actions/BattleGroundJoinAction.h:38-45, .cpp:557-649), kept in its own file so the
 * BG lane's BattleGroundJoinAction.h/.cpp stay untouched. Registered as "free bg join" (AC ActionContext.h:196,403);
 * driven only by the dormant RpgSubActions path, exactly as in AC.
 */

#ifndef _PLAYERBOT_FREEBGJOINACTION_H
#define _PLAYERBOT_FREEBGJOINACTION_H

#include "BattleGroundJoinAction.h"

class PlayerbotAI;

class FreeBGJoinAction : public BGJoinAction
{
public:
    FreeBGJoinAction(PlayerbotAI* botAI, std::string const name = "free bg join") : BGJoinAction(botAI, name) {}

    bool shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId) override;
};

#endif
