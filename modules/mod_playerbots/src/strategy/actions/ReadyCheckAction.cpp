/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/ReadyCheckAction.cpp) into
 * the Legends of Azeroth Pandaria fork (#210 S10 G4
 * "Ready check" row). Deviations from AC:
 *  - 5.4.8 splits WotLK's single MSG_RAID_READY_CHECK(_CONFIRM/_FINISHED) opcode into
 *    CMSG/SMSG_RAID_READY_CHECK, CMSG/SMSG_RAID_READY_CHECK_CONFIRM and SMSG_RAID_READY_CHECK_COMPLETED
 *    (Handlers/GroupHandler.cpp:998-1158, verified). AC parses the broadcast's leading guid to test
 *    "am I the initiator"; this fork instead reads `Group::ReadyCheckInitiator()`
 *    (Group.h:310-311, set by HandleRaidReadyCheckOpcode via `group->ReadyCheck(playerGuid)`) - no
 *    packet parse needed at all for that check.
 *  - SendReadyConfirm builds 5.4.8's bit-packed CMSG_RAID_READY_CHECK_CONFIRM
 *    (HandleRaidReadyCheckConfirmOpcode, GroupHandler.cpp:1071-1158): `u8 partyIndex` then 9 bits
 *    (8 guid-byte-present flags + 1 status bit) then up to 8 conditional guid bytes. The guid itself
 *    is read into a local ObjectGuid the handler never uses ("currently unused", verified in its own
 *    source) validates against, so an all-zero ObjectGuid is used - every WriteByteSeq for it is then
 *    a no-op and the wire body is exactly 3 bytes (party index, the bit-packed byte, and the pad byte
 *    the 9th bit forces `FlushBits()` to emit). Built via the real WriteBit/WriteByteSeq calls (mirror
 *    of the handler's ReadBit/ReadByteSeq order) rather than hand-computed bytes, so it stays correct
 *    if this core's ByteBuffer bit-packing ever changes.
 *  - HunterChecker: AC's WotLK version checks PLAYER_AMMO_ID and Pet::GetHappinessState() (UNHAPPY).
 *    Neither exists on this core (ammo and pet happiness were both removed pre-Cataclysm) - grep
 *    confirmed no PLAYER_AMMO_ID/GetHappinessState/UNHAPPY anywhere in src/server/game. The MoP-only
 *    equivalent check is just "a hunter has a pet summoned".
 *  - #628: ForceRebuffAction/ReadyReplyAction (AC's "rebuff before you answer" deferral, opt-in via
 *    AiPlayerbot.ForceRebuffOnReadyCheck, default off; and the "rebuff" chat command) are ported 1:1
 *    (AC ReadyCheckAction.cpp:251-276). The "force rebuff" strategy that drives "ready reply" is
 *    AI/ForceRebuff.cpp (AC Bot/ForceRebuff.cpp); AiFactory adds it to every grouped/BG non-combat set
 *    exactly where AC does (AC AiFactory.cpp:586, :678).
 */

#include "ReadyCheckAction.h"

#include "Event.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "Pet.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <memory>
#include <mutex>
#include <vector>

static void SendReadyConfirm(Player* bot)
{
    // GroupHandler.cpp:1071-1158 (HandleRaidReadyCheckConfirmOpcode): the guid this reads is
    // "currently unused" by the handler (its own comment), so an empty one is fine - see the file
    // header for why this is built via WriteBit/WriteByteSeq instead of hand-picked bytes.
    ObjectGuid const guid = ObjectGuid::Empty;
    bool const status = true;

    WorldPacket packet(CMSG_RAID_READY_CHECK_CONFIRM);
    packet << uint8(0);  // party index

    packet.WriteBit(guid[2]);
    packet.WriteBit(guid[1]);
    packet.WriteBit(guid[0]);
    packet.WriteBit(guid[3]);
    packet.WriteBit(guid[6]);
    packet.WriteBit(status);
    packet.WriteBit(guid[7]);
    packet.WriteBit(guid[4]);
    packet.WriteBit(guid[5]);
    packet.FlushBits();

    packet.WriteByteSeq(guid[1]);
    packet.WriteByteSeq(guid[0]);
    packet.WriteByteSeq(guid[3]);
    packet.WriteByteSeq(guid[2]);
    packet.WriteByteSeq(guid[4]);
    packet.WriteByteSeq(guid[5]);
    packet.WriteByteSeq(guid[7]);
    packet.WriteByteSeq(guid[6]);

    bot->GetSession()->HandleRaidReadyCheckConfirmOpcode(packet);
}

static std::string const formatPercent(std::string const name, uint8 value, float percent)
{
    std::ostringstream out;

    std::string color;
    if (percent > 75)
        color = "|cff00ff00";
    else if (percent > 50)
        color = "|cffffff00";
    else
        color = "|cffff0000";

    out << "|cffffffff[" << name << "]" << color << "x" << (int)value;
    return out.str();
}

namespace
{
class ReadyChecker
{
public:
    virtual ~ReadyChecker() = default;
    virtual bool Check(PlayerbotAI* botAI, AiObjectContext* context) = 0;
    virtual std::string const getName() = 0;
    virtual bool PrintAlways() { return true; }

    static std::vector<std::unique_ptr<ReadyChecker>> checkers;
    static std::once_flag initFlag;
};

std::vector<std::unique_ptr<ReadyChecker>> ReadyChecker::checkers;
std::once_flag ReadyChecker::initFlag;

class HealthChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig->almostFullHealth;
    }

    std::string const getName() override { return "HP"; }
};

class ManaChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") ||
               AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->mediumHealth;
    }

    std::string const getName() override { return "MP"; }
};

class DistanceChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* botAI, AiObjectContext* /*context*/) override
    {
        Player* bot = botAI->GetBot();
        if (Player* master = botAI->GetMaster())
        {
            if (bot->GetDistance(master) > sPlayerbotAIConfig->sightDistance)
                return false;
        }

        return true;
    }

    bool PrintAlways() override { return false; }
    std::string const getName() override { return "Far away"; }
};

// AC parity note: WotLK's HunterChecker also gates on PLAYER_AMMO_ID and
// Pet::GetHappinessState()==UNHAPPY. Neither ammo nor pet happiness exists on this core (removed
// pre-Cataclysm - see file header), so this is just "a hunter has a pet".
class HunterChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* botAI, AiObjectContext* /*context*/) override
    {
        Player* bot = botAI->GetBot();
        if (bot->GetClass() == CLASS_HUNTER && !bot->GetPet())
        {
            botAI->TellError("No pet!");
            return false;
        }

        return true;
    }

    bool PrintAlways() override { return false; }
    std::string const getName() override { return "Far away"; }
};

class ItemCountChecker : public ReadyChecker
{
public:
    ItemCountChecker(std::string const item, std::string const name) : item(item), name(name) {}

    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return AI_VALUE2(uint32, "item count", item) > 0;
    }

    std::string const getName() override { return name; }

private:
    std::string const item;
    std::string const name;
};

class ManaPotionChecker : public ItemCountChecker
{
public:
    ManaPotionChecker(std::string const item, std::string const name) : ItemCountChecker(item, name) {}

    bool Check(PlayerbotAI* botAI, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") || ItemCountChecker::Check(botAI, context);
    }
};
}  // namespace

bool ReadyCheckAction::Execute(Event event)
{
    // Fired two ways, same as AC: from the SMSG_RAID_READY_CHECK WorldPacketTrigger (a real packet is
    // attached - skip if this bot is the one who started the check) or from the "ready" chat command
    // (ChatCommandTrigger fires with no packet), which always reports+confirms regardless of whether
    // a check is even in progress (AC ReadyCheckAction.cpp:160-182: the initiator test and the
    // forceRebuff deferral both live inside AC's `if (!p.empty())`).
    if (!event.getPacket().empty())
    {
        // AC parses the broadcast packet's leading guid to test this; this core carries the same
        // information on the group itself (Group::ReadyCheckInitiator(), Group.h:310-311 - see file
        // header for why no packet parse is needed here).
        Group* group = bot->GetGroup();
        if (group && group->ReadyCheckInitiator() == bot->GetGUID())
            return false;

        // Defer the ready reply until buffs are topped off.
        if (sPlayerbotAIConfig->forceRebuffOnReadyCheck && !bot->IsInCombat() &&
            botAI->HasStrategy("force rebuff", BOT_STATE_NON_COMBAT))
        {
            ReportReadinessToMaster();
            botAI->forceRebuff.Begin(true);
            return true;
        }
    }

    return ReadyCheck();
}

void ReadyCheckAction::ReportReadinessToMaster()
{
    std::call_once(
        ReadyChecker::initFlag,
        []()
        {
            ReadyChecker::checkers.reserve(7);

            ReadyChecker::checkers.emplace_back(std::make_unique<HealthChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<DistanceChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<HunterChecker>());

            ReadyChecker::checkers.emplace_back(std::make_unique<ItemCountChecker>("food", "Food"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaPotionChecker>("drink", "Water"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ItemCountChecker>("healing potion", "Hpot"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaPotionChecker>("mana potion", "Mpot"));
        });

    // AC keeps this loop (and its `result`) without ever reporting `result` in the whisper below -
    // ported as-is (AC ReadyCheckAction.cpp:198-217): the whisper text is fixed-format percentages,
    // not the pass/fail from the checkers.
    for (auto const& checkerPtr : ReadyChecker::checkers)
    {
        if (checkerPtr)
            checkerPtr->Check(botAI, context);
    }

    std::ostringstream out;

    uint32 hp = AI_VALUE2(uint32, "item count", "healing potion");
    out << formatPercent("Hp", hp, 100.0f * hp / 5);

    out << ", ";
    uint32 food = AI_VALUE2(uint32, "item count", "food");
    out << formatPercent("Food", food, 100.0f * food / 20);

    if (AI_VALUE2(bool, "has mana", "self target"))
    {
        out << ", ";
        uint32 mp = AI_VALUE2(uint32, "item count", "mana potion");
        out << formatPercent("Mp", mp, 100.0f * mp / 5);

        out << ", ";
        uint32 water = AI_VALUE2(uint32, "item count", "water");
        out << formatPercent("Water", water, 100.0f * water / 20);
    }

    botAI->TellMaster(out);
}

bool ReadyCheckAction::ReadyCheck()
{
    ReportReadinessToMaster();

    SendReadyConfirm(bot);

    botAI->forceRebuff.End();

    // AC parity: "ready check" is never actually added as an ongoing strategy on either fork (no
    // AiFactory wiring - see LfgStrategy-style dormant registration notes), so this is a harmless
    // no-op kept for symmetry with AC's own source.
    botAI->ChangeStrategy("-ready check", BOT_STATE_NON_COMBAT);

    return true;
}

bool FinishReadyCheckAction::Execute(Event /*event*/) { return ReadyCheck(); }

// Manual "rebuff" command: opens a rebuff window with no ready check to answer.
bool ForceRebuffAction::Execute(Event /*event*/)
{
    if (bot->IsInCombat() || !botAI->HasStrategy("force rebuff", BOT_STATE_NON_COMBAT))
        return false;

    botAI->forceRebuff.Begin(false);
    return true;
}

bool ReadyReplyAction::isUseful()
{
    ForceRebuffState const& rebuff = botAI->forceRebuff;
    return rebuff.IsPending() && !rebuff.IsOnGlobalCooldown() &&
           !bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) && !bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) &&
           !rebuff.IsBuffPendingThisCycle();
}

bool ReadyReplyAction::Execute(Event /*event*/)
{
    if (botAI->forceRebuff.ShouldReplyToReadyCheck())
        SendReadyConfirm(bot);

    botAI->forceRebuff.End();
    return true;
}
