/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * D4 (death-revive-parity.md section 5, F4): AC XpGainAction.cpp:16 resets "death count" to 0 on every
 * XP gain, plus (#322) AC XpGainAction.cpp:39 BroadcastHelper::BroadcastKill for non-dungeon kills. The
 * victim guid is decoded from the 5.4.8 bit-packed SMSG_LOG_XPGAIN (Player::SendLogXPGain, Player.cpp:2873).
 * The (dead, commented-out) randomBotXPRate scaling is not ported.
 */

#ifndef _PLAYERBOT_XPGAINACTION_H
#define _PLAYERBOT_XPGAINACTION_H

#include "Action.h"

class PlayerbotAI;

class XpGainAction : public Action
{
public:
    XpGainAction(PlayerbotAI* botAI) : Action(botAI, "xp gain") {}

    bool Execute(Event event) override;
};

#endif
