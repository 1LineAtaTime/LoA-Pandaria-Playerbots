/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #648: ported from AC mod-playerbots Ai/Base/Actions/SeeSpellAction.cpp (AC 7f12e89ee). Same flow and same branches;
 * the 5.4.8 adaptations are:
 *  - The packet. AC reads `castCount, spellId, castFlags` and then SpellCastTargets::Read. On 5.4.8 CMSG_CAST_SPELL is
 *    bit-packed and the spell id comes near the end, so ReadMopCastSpell() below walks the packet in exactly the order
 *    WorldSession::HandleCastSpellOpcode reads it (src/server/game/Handlers/SpellHandler.cpp:729-1023) and builds the
 *    SpellCastTargets with the same constructor the handler uses (:1068), which resolves a transport-relative destination.
 *    A malformed packet throws ByteBufferException, caught like AC's catch (AC :86-91).
 *  - FISHING_SPELLS: AC's set (7620...51294, WotLK skill ranks) plus 131474, the castable "Fishing" on 5.4.8 (the spell
 *    the fork's FishingAction.cpp:49 casts; 7620 is an unscripted DUMMY here).
 *  - AiPlayerbot.EnableFishingWithMaster (AC PlayerbotAIConfig.cpp:295, default true) is read with sConfigMgr here: the
 *    fork's PlayerbotAIConfig has no field for it (same pattern as PlayerbotAI.cpp:2328).
 *  - AC Unit::SendPlaySpellVisual(guid, kit) (SMSG_PLAY_SPELL_VISUAL: kit played on the bot, sent to the set around
 *    the master) -> bot->SendPlaySpellVisualKit(kit, 0, 0) (5.4.8 SMSG_PLAY_SPELL_VISUAL_KIT on the bot, sent to the set
 *    around the bot, which includes the master in range).
 *  - SummonCreature takes Milliseconds on this core; a failed summon is skipped instead of dereferenced.
 */

#include "SeeSpellAction.h"

#include "ByteBuffer.h"
#include "Config.h"
#include "Event.h"
#include "Formations.h"
#include "Log.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "PositionValue.h"
#include "RTSCValues.h"
#include "RtscAction.h"
#include "SpellDefines.h"
#include "WorldPosition.h"

#include <set>

std::set<uint32> const FISHING_SPELLS = {7620, 7731, 7732, 18248, 33095, 51294, 131474};

namespace
{
// What SeeSpellAction needs from a 5.4.8 CMSG_CAST_SPELL.
struct MopCastSpellPacket
{
    uint8 castCount = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;
    uint32 targetMask = 0;
    ObjectGuid targetGuid;
    ObjectGuid itemTargetGuid;
    ObjectGuid srcTransportGuid;
    ObjectGuid destTransportGuid;
    Position srcPos;
    Position destPos;
    float elevation = 0.0f;
    float missileSpeed = 0.0f;
    std::string targetString;
};

// Reads the packet in the order of WorldSession::HandleCastSpellOpcode (SpellHandler.cpp:729-1023). The movement block
// and the research data are read and dropped. Throws ByteBufferException on a short packet.
void ReadMopCastSpell(WorldPacket& p, Player* caster, MopCastSpellPacket& out)
{
    p.rpos(0);
    p.ResetBitPos();

    uint8 mg[8] = {};   // movement guid bytes (dropped)
    uint8 mtg[8] = {};  // movement transport guid bytes (dropped)
    bool hasTransport = false, hasTransportTime2 = false, hasTransportTime3 = false, hasFallData = false,
         hasFallDirection = false, hasTimestamp = false, hasSplineElevation = false, hasPitch = false,
         hasOrientation = false, hasUnkMovementField = false;
    uint32 unkMovementLoopCounter = 0;
    uint32 targetStringLength = 0;

    p.ReadBit();  // fake bit
    bool const hasTargetString = !p.ReadBit();
    p.ReadBit();  // fake bit
    bool const hasCastCount = !p.ReadBit();
    bool const hasSrcLocation = p.ReadBit();
    bool const hasDestLocation = p.ReadBit();
    bool const hasSpellId = !p.ReadBit();
    uint8 const researchDataCount = uint8(p.ReadBits(2));
    bool const hasTargetMask = !p.ReadBit();
    bool const hasMissileSpeed = !p.ReadBit();

    for (uint8 i = 0; i < researchDataCount; ++i)
        p.ReadBits(2);

    bool const hasGlyphIndex = !p.ReadBit();
    bool const hasMovement = p.ReadBit();
    bool const hasElevation = !p.ReadBit();
    bool const hasCastFlags = !p.ReadBit();

    ObjectGuid& tg = out.targetGuid;
    tg[5] = p.ReadBit();
    tg[4] = p.ReadBit();
    tg[2] = p.ReadBit();
    tg[7] = p.ReadBit();
    tg[1] = p.ReadBit();
    tg[6] = p.ReadBit();
    tg[3] = p.ReadBit();
    tg[0] = p.ReadBit();

    ObjectGuid& dtg = out.destTransportGuid;
    if (hasDestLocation)
    {
        dtg[1] = p.ReadBit();
        dtg[3] = p.ReadBit();
        dtg[5] = p.ReadBit();
        dtg[0] = p.ReadBit();
        dtg[2] = p.ReadBit();
        dtg[6] = p.ReadBit();
        dtg[7] = p.ReadBit();
        dtg[4] = p.ReadBit();
    }

    if (hasMovement)
    {
        unkMovementLoopCounter = p.ReadBits(22);
        p.ReadBit();
        mg[4] = p.ReadBit();
        hasTransport = p.ReadBit();

        if (hasTransport)
        {
            hasTransportTime2 = p.ReadBit();
            mtg[7] = p.ReadBit();
            mtg[4] = p.ReadBit();
            mtg[1] = p.ReadBit();
            mtg[0] = p.ReadBit();
            mtg[6] = p.ReadBit();
            mtg[3] = p.ReadBit();
            mtg[5] = p.ReadBit();
            hasTransportTime3 = p.ReadBit();
            mtg[2] = p.ReadBit();
        }

        p.ReadBit();
        mg[7] = p.ReadBit();
        hasOrientation = !p.ReadBit();
        mg[6] = p.ReadBit();
        hasSplineElevation = !p.ReadBit();
        hasPitch = !p.ReadBit();
        mg[0] = p.ReadBit();
        p.ReadBit();
        bool const hasMovementFlags = !p.ReadBit();
        hasTimestamp = !p.ReadBit();
        hasUnkMovementField = !p.ReadBit();

        if (hasMovementFlags)
            p.ReadBits(30);

        mg[1] = p.ReadBit();
        mg[3] = p.ReadBit();
        mg[2] = p.ReadBit();
        mg[5] = p.ReadBit();
        hasFallData = p.ReadBit();

        if (hasFallData)
            hasFallDirection = p.ReadBit();

        bool const hasMovementFlags2 = !p.ReadBit();
        if (hasMovementFlags2)
            p.ReadBits(13);
    }

    ObjectGuid& itg = out.itemTargetGuid;
    itg[1] = p.ReadBit();
    itg[0] = p.ReadBit();
    itg[7] = p.ReadBit();
    itg[4] = p.ReadBit();
    itg[6] = p.ReadBit();
    itg[5] = p.ReadBit();
    itg[3] = p.ReadBit();
    itg[2] = p.ReadBit();

    ObjectGuid& stg = out.srcTransportGuid;
    if (hasSrcLocation)
    {
        stg[4] = p.ReadBit();
        stg[5] = p.ReadBit();
        stg[3] = p.ReadBit();
        stg[0] = p.ReadBit();
        stg[7] = p.ReadBit();
        stg[1] = p.ReadBit();
        stg[6] = p.ReadBit();
        stg[2] = p.ReadBit();
    }

    if (hasTargetMask)
        out.targetMask = p.ReadBits(20);

    if (hasCastFlags)
        out.castFlags = uint8(p.ReadBits(5));

    if (hasTargetString)
        targetStringLength = p.ReadBits(7);

    for (uint8 i = 0; i < researchDataCount; ++i)
    {
        p.read_skip<uint32>();  // count
        p.read_skip<uint32>();  // itemId
    }

    if (hasMovement)
    {
        p.read_skip<float>();  // x
        p.ReadByteSeq(mg[0]);

        if (hasTransport)
        {
            p.ReadByteSeq(mtg[2]);
            p.read_skip<int8>();  // seat
            p.ReadByteSeq(mtg[3]);
            p.ReadByteSeq(mtg[7]);
            p.read_skip<float>();  // transport x
            p.ReadByteSeq(mtg[5]);

            if (hasTransportTime3)
                p.read_skip<uint32>();

            p.read_skip<float>();  // transport z
            p.read_skip<float>();  // transport y
            p.ReadByteSeq(mtg[6]);
            p.ReadByteSeq(mtg[1]);
            p.read_skip<float>();  // transport o
            p.ReadByteSeq(mtg[4]);

            if (hasTransportTime2)
                p.read_skip<uint32>();

            p.ReadByteSeq(mtg[0]);
            p.read_skip<uint32>();  // transport time
        }

        p.ReadByteSeq(mg[5]);

        if (hasFallData)
        {
            p.read_skip<uint32>();  // fall time
            p.read_skip<float>();   // z speed

            if (hasFallDirection)
            {
                p.read_skip<float>();  // sin
                p.read_skip<float>();  // xy speed
                p.read_skip<float>();  // cos
            }
        }

        if (hasSplineElevation)
            p.read_skip<float>();

        p.ReadByteSeq(mg[6]);

        if (hasUnkMovementField)
            p.read_skip<uint32>();

        p.ReadByteSeq(mg[4]);

        if (hasOrientation)
            p.read_skip<float>();

        if (hasTimestamp)
            p.read_skip<uint32>();

        p.ReadByteSeq(mg[1]);

        if (hasPitch)
            p.read_skip<float>();

        p.ReadByteSeq(mg[3]);

        for (uint32 i = 0; i != unkMovementLoopCounter; i++)
            p.read_skip<uint32>();

        p.read_skip<float>();  // y
        p.ReadByteSeq(mg[7]);
        p.read_skip<float>();  // z
        p.ReadByteSeq(mg[2]);
    }

    p.ReadByteSeq(itg[4]);
    p.ReadByteSeq(itg[2]);
    p.ReadByteSeq(itg[1]);
    p.ReadByteSeq(itg[5]);
    p.ReadByteSeq(itg[7]);
    p.ReadByteSeq(itg[3]);
    p.ReadByteSeq(itg[6]);
    p.ReadByteSeq(itg[0]);

    if (hasDestLocation)
    {
        float x, y, z;
        p.ReadByteSeq(dtg[2]);
        p >> x;
        p.ReadByteSeq(dtg[4]);
        p.ReadByteSeq(dtg[1]);
        p.ReadByteSeq(dtg[0]);
        p.ReadByteSeq(dtg[3]);
        p >> y;
        p.ReadByteSeq(dtg[7]);
        p >> z;
        p.ReadByteSeq(dtg[5]);
        p.ReadByteSeq(dtg[6]);
        out.destPos.Relocate(x, y, z);
    }
    else
    {
        out.destTransportGuid = caster->GetTransGUID();
        if (out.destTransportGuid)
            out.destPos.Relocate(caster->GetTransOffsetX(), caster->GetTransOffsetY(), caster->GetTransOffsetZ(),
                                 caster->GetTransOffsetO());
        else
            out.destPos.Relocate(caster);
    }

    p.ReadByteSeq(tg[3]);
    p.ReadByteSeq(tg[4]);
    p.ReadByteSeq(tg[7]);
    p.ReadByteSeq(tg[6]);
    p.ReadByteSeq(tg[2]);
    p.ReadByteSeq(tg[0]);
    p.ReadByteSeq(tg[1]);
    p.ReadByteSeq(tg[5]);

    if (hasSrcLocation)
    {
        float x, y, z;
        p >> y;
        p.ReadByteSeq(stg[5]);
        p.ReadByteSeq(stg[1]);
        p.ReadByteSeq(stg[7]);
        p.ReadByteSeq(stg[6]);
        p >> x;
        p.ReadByteSeq(stg[3]);
        p.ReadByteSeq(stg[2]);
        p.ReadByteSeq(stg[0]);
        p.ReadByteSeq(stg[4]);
        p >> z;
        out.srcPos.Relocate(x, y, z);
    }
    else
    {
        out.srcTransportGuid = caster->GetTransGUID();
        if (out.srcTransportGuid)
            out.srcPos.Relocate(caster->GetTransOffsetX(), caster->GetTransOffsetY(), caster->GetTransOffsetZ(),
                                caster->GetTransOffsetO());
        else
            out.srcPos.Relocate(caster);
    }

    if (hasTargetString)
        out.targetString = p.ReadString(targetStringLength);

    if (hasMissileSpeed)
        p >> out.missileSpeed;

    if (hasElevation)
        p >> out.elevation;

    if (hasCastCount)
        p >> out.castCount;

    if (hasSpellId)
        p >> out.spellId;

    if (hasGlyphIndex)
        p.read_skip<uint32>();
}
}  // namespace

Creature* SeeSpellAction::CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* /*lastWp*/,
                                    bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    if (!important)
        delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(uint32(delay)));
    if (wpCreature && !important)
        wpCreature->SetObjectScale(0.2f);

    return wpCreature;
}

bool SeeSpellAction::Execute(Event event)
{
    // RTSC packet data (5.4.8 CMSG_CAST_SPELL, see ReadMopCastSpell)
    WorldPacket p(event.getPacket());

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    MopCastSpellPacket cast;
    try
    {
        ReadMopCastSpell(p, master, cast);
    }
    catch (ByteBufferException const&)
    {
        // ignore malformed packets instead of crashing (AC :86-91)
        TC_LOG_WARN("playerbots", "SeeSpellAction: Failed deserialization (malformed) CMSG_CAST_SPELL size=%u",
                    uint32(p.size()));
        return false;
    }

    uint32 const spellId = cast.spellId;

    // if (!botAI->HasStrategy("RTSC", botAI->GetState()))
    //     return false;

    if (FISHING_SPELLS.find(spellId) != FISHING_SPELLS.end())
    {
        static bool const enableFishingWithMaster = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableFishingWithMaster", true);
        if (AI_VALUE(bool, "can fish") && enableFishingWithMaster)
        {
            botAI->ChangeStrategy("+master fishing", BOT_STATE_NON_COMBAT);
            return true;
        }
        return false;
    }

    if (spellId != RTSC_MOVE_SPELL)
        return false;

    SpellCastTargets targets(master, cast.targetMask, cast.targetGuid, cast.itemTargetGuid, cast.srcTransportGuid,
                             cast.destTransportGuid, cast.srcPos, cast.destPos, cast.elevation, cast.missileSpeed,
                             cast.targetString);
    if (!targets.HasDst() || !targets.GetDstPos())
    {
        // do not use a missing destination; ignore malformed RTSC packets instead of crashing (AC :79-84)
        TC_LOG_WARN("playerbots", "SeeSpellAction: (malformed) RTSC payload does not contain full targets data");
        return false;
    }

    WorldPosition spellPosition(master->GetMapId(), *targets.GetDstPos());
    SET_AI_VALUE(WorldPosition, "see spell location", spellPosition);

    bool selected = AI_VALUE(bool, "RTSC selected");
    bool inRange = spellPosition.distance(bot) <= 10;
    std::string const nextAction = AI_VALUE(std::string, "RTSC next spell action");

    if (nextAction.empty())
    {
        if (!inRange && selected)
            bot->SendPlaySpellVisualKit(6372, 0, 0);
        else if (inRange && !selected)
            bot->SendPlaySpellVisualKit(5036, 0, 0);

        SET_AI_VALUE(bool, "RTSC selected", inRange);

        if (selected)
            return MoveToSpell(spellPosition);

        return inRange;
    }
    else if (nextAction == "move")
    {
        return MoveToSpell(spellPosition);
    }
    else if (nextAction.find("save ") != std::string::npos)
    {
        std::string locationName;
        if (nextAction.find("save selected ") != std::string::npos)
        {
            if (!selected)
                return false;

            locationName = nextAction.substr(14);
        }
        else
            locationName = nextAction.substr(5);

        SetFormationOffset(spellPosition);

        SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);

        Creature* wpCreature =
            bot->SummonCreature(15631, spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ(),
                                spellPosition.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(2000));
        if (wpCreature)
            wpCreature->SetObjectScale(0.5f);
        RESET_AI_VALUE(std::string, "RTSC next spell action");

        return true;
    }

    return false;
}

bool SeeSpellAction::SelectSpell(WorldPosition& spellPosition)
{
    if (spellPosition.distance(bot) <= 5 || AI_VALUE(bool, "RTSC selected"))
    {
        SET_AI_VALUE(bool, "RTSC selected", true);
        bot->SendPlaySpellVisualKit(5036, 0, 0);
    }

    return true;
}

bool SeeSpellAction::MoveToSpell(WorldPosition& spellPosition, bool inFormation)
{
    if (inFormation)
        SetFormationOffset(spellPosition);

    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionInfo stayPosition = posMap["stay"];

        stayPosition.Set(spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ(), spellPosition.GetMapId());
        posMap["stay"] = stayPosition;
    }

    if (bot->IsWithinLOS(spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ()))
        return MoveNear(spellPosition.GetMapId(), spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ(), 0);

    return MoveTo(spellPosition.GetMapId(), spellPosition.GetPositionX(), spellPosition.GetPositionY(), spellPosition.GetPositionZ(), false,
                  false);
}

void SeeSpellAction::SetFormationOffset(WorldPosition& spellPosition)
{
    Player* master = botAI->GetMaster();
    if (!master)
        return;

    Formation* formation = AI_VALUE(Formation*, "formation");

    WorldLocation formationLocation = formation->GetLocation();

    if (formationLocation.GetPositionX() != 0 || formationLocation.GetPositionY() != 0)
    {
        spellPosition -= WorldPosition(master);
        spellPosition += formationLocation;
    }
}
