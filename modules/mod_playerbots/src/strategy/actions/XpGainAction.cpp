/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/XpGainAction.cpp:14-42 (#322).
 *
 * 5.4.8 adaptation: SMSG_LOG_XPGAIN is bit-packed. Layout read here is the exact mirror of
 * Player::SendLogXPGain() (src/server/game/Entities/Player/Player.cpp:2873-2911):
 *   bits : noVictim, guid[1], guid[2], guid[7], guid[4], guid[3], unk, guid[0], guid[5], guid[6], hasGroupBonus
 *   bytes: guid[4], guid[2], uint8 raf, float groupRate, guid[7], guid[1], guid[3], guid[6],
 *          uint32 total, (uint32 given if victim), guid[0], guid[5]
 * Only the victim guid is needed (AC reads only the guid too); the rest is not consumed.
 */

#include "XpGainAction.h"

#include "BroadcastHelper.h"
#include "Creature.h"
#include "Event.h"
#include "Map.h"
#include "Playerbots.h"

bool XpGainAction::Execute(Event event)
{
    context->GetValue<uint32>("death count")->Set(0);

    WorldPacket p(event.getPacket());
    p.rpos(0);

    bool const noVictim = p.ReadBit();
    ObjectGuid guid;
    guid[1] = p.ReadBit();
    guid[2] = p.ReadBit();
    guid[7] = p.ReadBit();
    guid[4] = p.ReadBit();
    guid[3] = p.ReadBit();
    p.ReadBit();  // unknown
    guid[0] = p.ReadBit();
    guid[5] = p.ReadBit();
    guid[6] = p.ReadBit();
    p.ReadBit();  // has group bonus

    if (noVictim)
        return true;

    p.ReadByteSeq(guid[4]);
    p.ReadByteSeq(guid[2]);
    p.read_skip<uint8>();  // recruit-a-friend flag
    p.read_skip<float>();  // group rate
    p.ReadByteSeq(guid[7]);
    p.ReadByteSeq(guid[1]);
    p.ReadByteSeq(guid[3]);
    p.ReadByteSeq(guid[6]);
    p.read_skip<uint32>();  // total xp
    p.read_skip<uint32>();  // xp without bonus (victim present)
    p.ReadByteSeq(guid[0]);
    p.ReadByteSeq(guid[5]);

    Creature* creature = botAI->GetCreature(guid);
    if (creature && !creature->GetMap()->IsDungeon())
        BroadcastHelper::BroadcastKill(botAI, bot, creature);  // AC XpGainAction.cpp:39

    return true;
}
