/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/PetitionSignAction.cpp for #258. 5.4.8 differences:
 *  - SMSG_PETITION_SHOW_SIGNATURES is bit-packed (writer: Handlers/PetitionsHandler.cpp:794-855, the offer path,
 *    sent with playerGuid = the offering player): ReadOfferPacket() below reads it field for field to get the
 *    petition guid and the inviter guid.
 *  - CMSG_PETITION_SIGN (PetitionsHandler.cpp:539-563) = skipped uint8 + masked petition guid; CMSG_PETITION_DECLINE
 *    (:649-672) = masked petition guid. Both are built as those handlers read them.
 *  - Charter type ids: guild = GUILD_CHARTER_TYPE (4, Server/WorldSession.h:155), arena = 2/3/5. AC's "type != 9"
 *    guild test becomes "type != GUILD_CHARTER_TYPE". This core keeps arena membership in the rated-PvP slots, not a
 *    per-slot Player::GetArenaTeamId(), so AC's "already in such team" arena check reduces to
 *    GetArenaTeamIdInvited().
 *  - AC's CharacterDatabase.Query(fmt) -> PQuery(%u); AC's LOG_INFO -> TC_LOG_INFO.
 */

#include "PetitionSignAction.h"

#include "DatabaseEnv.h"
#include "Event.h"
#include "ObjectAccessor.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
// Mirrors the writer in PetitionsHandler.cpp:794-855.
void ReadOfferPacket(WorldPacket& p, ObjectGuid& petitionGuid, ObjectGuid& ownerGuid)
{
    p.rpos(0);

    ObjectGuid& pe = petitionGuid;
    ObjectGuid& pl = ownerGuid;

    pl[1] = p.ReadBit();
    pe[3] = p.ReadBit();
    pl[3] = p.ReadBit();
    pe[4] = p.ReadBit();
    pe[0] = p.ReadBit();
    pl[7] = p.ReadBit();
    pl[5] = p.ReadBit();
    pe[1] = p.ReadBit();
    pe[5] = p.ReadBit();
    pe[7] = p.ReadBit();
    pl[0] = p.ReadBit();
    pl[6] = p.ReadBit();
    pe[6] = p.ReadBit();
    pl[2] = p.ReadBit();
    pl[4] = p.ReadBit();
    uint32 const count = p.ReadBits(21);

    std::vector<ObjectGuid> signers(count);
    for (uint32 i = 0; i < count; ++i)
    {
        signers[i][2] = p.ReadBit();
        signers[i][0] = p.ReadBit();
        signers[i][4] = p.ReadBit();
        signers[i][7] = p.ReadBit();
        signers[i][5] = p.ReadBit();
        signers[i][1] = p.ReadBit();
        signers[i][6] = p.ReadBit();
        signers[i][3] = p.ReadBit();
    }

    pe[2] = p.ReadBit();

    for (uint32 i = 0; i < count; ++i)
    {
        p.ReadByteSeq(signers[i][6]);
        p.ReadByteSeq(signers[i][0]);
        p.ReadByteSeq(signers[i][1]);
        p.ReadByteSeq(signers[i][3]);
        p.ReadByteSeq(signers[i][2]);
        p.ReadByteSeq(signers[i][5]);
        p.ReadByteSeq(signers[i][7]);
        p.ReadByteSeq(signers[i][4]);
        p.read<uint32>();  // choice
    }

    p.ReadByteSeq(pe[6]);
    p.ReadByteSeq(pe[5]);
    p.ReadByteSeq(pe[4]);
    p.ReadByteSeq(pl[4]);
    p.ReadByteSeq(pe[1]);
    p.read<uint32>();  // guildID (= petition low guid)
    p.ReadByteSeq(pe[2]);
    p.ReadByteSeq(pe[3]);
    p.ReadByteSeq(pe[7]);
    p.ReadByteSeq(pl[5]);
    p.ReadByteSeq(pl[6]);
    p.ReadByteSeq(pl[3]);
    p.ReadByteSeq(pl[7]);
    p.ReadByteSeq(pl[1]);
    p.ReadByteSeq(pl[0]);
    p.ReadByteSeq(pe[0]);
    p.ReadByteSeq(pl[2]);
}
}

bool PetitionSignAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    ObjectGuid petitionGuid;
    ObjectGuid inviter;
    bool isArena = false;
    ReadOfferPacket(p, petitionGuid, inviter);

    QueryResult result = CharacterDatabase.PQuery("SELECT `type` FROM `petition` WHERE `petitionguid` = %u",
                                                  petitionGuid.GetCounter());
    if (!result)
    {
        return false;
    }

    Field* fields = result->Fetch();
    uint32 type = fields[0].GetUInt32();

    bool accept = true;

    if (type != GUILD_CHARTER_TYPE)
    {
        isArena = true;
        if (bot->GetArenaTeamIdInvited())
        {
            botAI->TellError("Sorry, I am invited to such team already");
            accept = false;
        }
    }
    else
    {
        if (bot->GetGuildId())
        {
            botAI->TellError("Sorry, I am in a guild already");
            accept = false;
        }

        if (bot->GetGuildIdInvited())
        {
            botAI->TellError("Sorry, I am invited to a guild already");
            accept = false;
        }
    }

    Player* _inviter = ObjectAccessor::FindPlayer(inviter);
    if (!_inviter)
        return false;

    if (_inviter == bot)
        return false;

    if (!accept || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, _inviter, true))
    {
        WorldPacket data(CMSG_PETITION_DECLINE);
        data.WriteBit(petitionGuid[5]);
        data.WriteBit(petitionGuid[6]);
        data.WriteBit(petitionGuid[4]);
        data.WriteBit(petitionGuid[3]);
        data.WriteBit(petitionGuid[1]);
        data.WriteBit(petitionGuid[7]);
        data.WriteBit(petitionGuid[0]);
        data.WriteBit(petitionGuid[2]);
        data.FlushBits();
        data.WriteByteSeq(petitionGuid[6]);
        data.WriteByteSeq(petitionGuid[2]);
        data.WriteByteSeq(petitionGuid[1]);
        data.WriteByteSeq(petitionGuid[5]);
        data.WriteByteSeq(petitionGuid[0]);
        data.WriteByteSeq(petitionGuid[7]);
        data.WriteByteSeq(petitionGuid[4]);
        data.WriteByteSeq(petitionGuid[3]);
        data.rpos(0);
        bot->GetSession()->HandlePetitionDeclineOpcode(data);
        TC_LOG_INFO("playerbots", "Bot %s <%s> declines %s invite", bot->GetGUID().ToString().c_str(),
                    bot->GetName().c_str(), isArena ? "Arena" : "Guild");
        return false;
    }

    WorldPacket data(CMSG_PETITION_SIGN, 20);
    data << uint8(0);
    data.WriteBit(petitionGuid[4]);
    data.WriteBit(petitionGuid[2]);
    data.WriteBit(petitionGuid[0]);
    data.WriteBit(petitionGuid[1]);
    data.WriteBit(petitionGuid[5]);
    data.WriteBit(petitionGuid[3]);
    data.WriteBit(petitionGuid[6]);
    data.WriteBit(petitionGuid[7]);
    data.FlushBits();
    data.WriteByteSeq(petitionGuid[6]);
    data.WriteByteSeq(petitionGuid[1]);
    data.WriteByteSeq(petitionGuid[7]);
    data.WriteByteSeq(petitionGuid[2]);
    data.WriteByteSeq(petitionGuid[5]);
    data.WriteByteSeq(petitionGuid[3]);
    data.WriteByteSeq(petitionGuid[0]);
    data.WriteByteSeq(petitionGuid[4]);
    data.rpos(0);
    bot->GetSession()->HandlePetitionSignOpcode(data);
    bot->Say("Thanks for the invite!", LANG_UNIVERSAL);
    TC_LOG_INFO("playerbots", "Bot %s <%s> accepts %s invite", bot->GetGUID().ToString().c_str(),
                bot->GetName().c_str(), isArena ? "Arena" : "Guild");
    return true;
}
