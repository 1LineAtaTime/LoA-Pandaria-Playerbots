#include "AcceptDuelAction.h"

#include "Event.h"
#include "Player.h"
#include "Playerbots.h"
#include "WorldPacket.h"

void AcceptDuelAction::SendDuelResponse(Player* bot, ObjectGuid const& guid, bool accepted)
{
    // Bit/byte order copied from WorldSession::HandleDuelResponseOpcode's read order
    // (DuelHandler.cpp: no leading byte, bits first, then accepted, then the byte sequence),
    // reversed for the write side (ReviveFromCorpseAction.cpp:82-99 is the established idiom in
    // this module for round-tripping a bit-packed guid + flag this way).
    WorldPacket p;
    p.WriteBit(guid[7]);
    p.WriteBit(guid[1]);
    p.WriteBit(guid[3]);
    p.WriteBit(guid[4]);
    p.WriteBit(guid[0]);
    p.WriteBit(guid[2]);
    p.WriteBit(guid[6]);
    p.WriteBit(accepted);
    p.WriteBit(guid[5]);
    p.FlushBits();
    p.WriteByteSeq(guid[6]);
    p.WriteByteSeq(guid[4]);
    p.WriteByteSeq(guid[5]);
    p.WriteByteSeq(guid[0]);
    p.WriteByteSeq(guid[1]);
    p.WriteByteSeq(guid[2]);
    p.WriteByteSeq(guid[7]);
    p.WriteByteSeq(guid[3]);
    bot->GetSession()->HandleDuelResponseOpcode(p);
}

bool AcceptDuelAction::Execute([[maybe_unused]] Event event)
{
    if (!bot->duel || !bot->duel->opponent)
        return false;

    ObjectGuid opponentGuid = bot->duel->opponent->GetGUID();

    // Do not auto duel with low health (AC AcceptDuelAction.cpp:20-23, 1:1): below 90 % the bot declines unless it has
    // a game-client master AND that master is the requester. #398: the old "!HasRealPlayerMaster()" term let any
    // mastered bot accept a third party's duel at any health. AC's playerGuid is SMSG_DUEL_REQUESTED's initiator guid;
    // here it is bot->duel->initiator (set before the packet is sent; the opponent when it is ever unset).
    ObjectGuid const requesterGuid = bot->duel->initiator ? bot->duel->initiator->GetGUID() : opponentGuid;
    if ((!botAI->HasGameClientMaster() ||
         (botAI->GetMaster() && botAI->GetMaster()->GetGUID() != requesterGuid)) &&
        AI_VALUE2(uint8, "health", "self target") < 90)
    {
        SendDuelResponse(bot, opponentGuid, false);
        return false;
    }

    SendDuelResponse(bot, opponentGuid, true);

    botAI->ResetStrategies();
    return true;
}
