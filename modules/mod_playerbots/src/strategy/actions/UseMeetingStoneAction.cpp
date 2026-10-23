/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UseMeetingStoneAction.cpp (#177). Deviations:
 *  - (#177 follow-up, #322 landed) every tell goes through PlayerbotTextMgr::GetBotTextOrDefault() with
 *    AC's keys and default strings (AC UseMeetingStoneAction.cpp:37,67-80,98-104,115-125,150-155,
 *    167-175,184,200-232,266), so the same text variants are picked as on AC. A successful plain
 *    "summon" is silent on AC too (Teleport() tells nothing; "summon" is in AC SayAction.cpp noReplyMsgs):
 *    the only chat text is the error / "I live, again!" / GM-security branches.
 *  - SummonUsingGos()/SummonUsingNpcs() reuse this fork's existing "nearest game objects" and
 *    "nearest npcs" AI values (ValueContext.h, already registered and used by the loot/BG code)
 *    instead of AC's own inline Cell::VisitObjects()/Trinity::GameObjectListSearcher grid search -
 *    same result, one fewer hand-rolled grid query, and it reuses filtering (isSpawned/GetGOInfo)
 *    this fork's NearestGameObjects.cpp already had to get right (see its own file comment).
 *  - #194 (S8): botAI->PetFollow() is now ported as the free helper PlayerbotPetFollow() (PetsAction.h),
 *    so the "if (bot->GetPet()) PlayerbotPetFollow(bot);" line below is restored.
 *
 * UseMeetingStoneAction is ported for structural parity but is NOT registered anywhere yet: AC drives
 * it off an incoming meeting-stone-use WorldPacket via WorldPacketActionContext.h/
 * WorldPacketTriggerContext.h, and this batch may only touch ActionContext.h/TriggerContext.h/
 * ChatActionContext.h/ChatTriggerContext.h/StrategyContext.h (plus .cpp/new files) - see the #177
 * report's "riskiest spots" for the follow-up needed (also needs the 5.4.8 meeting-stone GameObject
 * entry id and opcode verified against the DB/core, neither done here).
 */

#include "UseMeetingStoneAction.h"
#include "LastMovementValue.h"

#include "Event.h"
#include "GameObject.h"
#include "Group.h"
#include "PetsAction.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "Spell.h"
#include "SpellMgr.h"

bool UseMeetingStoneAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);

    // #179: this was a plain (unpacked) `p >> guid` - AC's WotLK wire format. This core's
    // CMSG_GAMEOBJ_USE bit-packs the guid (Handlers/SpellHandler.cpp:HandleGameObjectUseOpcode,
    // verified); the previous parse silently misread every real click.
    ObjectGuid guid;
    guid[6] = p.ReadBit();
    guid[1] = p.ReadBit();
    guid[3] = p.ReadBit();
    guid[4] = p.ReadBit();
    guid[0] = p.ReadBit();
    guid[5] = p.ReadBit();
    guid[7] = p.ReadBit();
    guid[2] = p.ReadBit();

    p.ReadByteSeq(guid[0]);
    p.ReadByteSeq(guid[1]);
    p.ReadByteSeq(guid[6]);
    p.ReadByteSeq(guid[2]);
    p.ReadByteSeq(guid[3]);
    p.ReadByteSeq(guid[4]);
    p.ReadByteSeq(guid[5]);
    p.ReadByteSeq(guid[7]);

    if (master->GetTarget() && master->GetTarget() != bot->GetGUID())
        return false;

    if (!master->GetTarget() && master->GetGroup() != bot->GetGroup())
        return false;

    if (master->IsBeingTeleported())
        return false;

    if (bot->IsInCombat())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "meeting_stone_in_combat", "I am in combat", {}));
        return false;
    }

    Map* map = master->GetMap();
    if (!map)
        return false;

    GameObject* gameObject = map->GetGameObject(guid);
    if (!gameObject)
        return false;

    GameObjectTemplate const* goInfo = gameObject->GetGOInfo();
    if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_MEETINGSTONE)
        return false;

    return Teleport(master, bot, false);
}

bool SummonAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    // #194 (S8): AC UseMeetingStoneAction.cpp:66-67.
    if (bot->GetPet())
        PlayerbotPetFollow(bot);

    if (master->GetSession()->GetSecurity() >= SEC_PLAYER)
    {
        AI_VALUE(std::list<FleeInfo>&, "recently flee info").clear();
        return Teleport(master, bot, true);
    }

    if (SummonUsingGos(master, bot, true) || SummonUsingNpcs(master, bot, true))
    {
        botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "hello", "Hello!", {}));
        return true;
    }

    if (SummonUsingGos(bot, master, true) || SummonUsingNpcs(bot, master, true))
    {
        botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "meeting_stone_welcome", "Welcome!", {}));
        return true;
    }

    return false;
}

bool SummonAction::SummonUsingGos(Player* summoner, Player* player, bool preserveAuras)
{
    GuidVector nearGos = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
    for (ObjectGuid const& guid : nearGos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->GetDistance(summoner) <= sPlayerbotAIConfig->sightDistance &&
            go->GetGoType() == GAMEOBJECT_TYPE_MEETINGSTONE)
            return Teleport(summoner, player, preserveAuras);
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        summoner == bot ? "meeting_stone_none_nearby" : "meeting_stone_none_near_you",
        summoner == bot ? "There is no meeting stone nearby" : "There is no meeting stone near you",
        {}));
    return false;
}

bool SummonAction::SummonUsingNpcs(Player* summoner, Player* player, bool preserveAuras)
{
    if (!sPlayerbotAIConfig->summonAtInnkeepersEnabled)
        return false;

    GuidVector nearNpcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (ObjectGuid const& guid : nearNpcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetDistance(summoner) > sPlayerbotAIConfig->sightDistance ||
            !unit->HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER))
            continue;

        if (!player->HasItemCount(6948, 1, false))
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                player == bot ? "meeting_stone_no_hearthstone_self" : "meeting_stone_no_hearthstone_you",
                player == bot ? "I have no hearthstone" : "You have no hearthstone",
                {}));
            return false;
        }

        if (player->HasSpellCooldown(8690))
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                player == bot ? "meeting_stone_hearthstone_not_ready_self" : "meeting_stone_hearthstone_not_ready_you",
                player == bot ? "My hearthstone is not ready" : "Your hearthstone is not ready",
                {}));
            return false;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(8690);
        if (!spellInfo)
            return false;

        Spell spell(player, spellInfo, TRIGGERED_NONE);
        spell.SendSpellCooldown();

        return Teleport(summoner, player, preserveAuras);
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        summoner == bot ? "meeting_stone_no_innkeepers_nearby" : "meeting_stone_no_innkeepers_near_you",
        summoner == bot ? "There are no innkeepers nearby" : "There are no innkeepers near you",
        {}));
    return false;
}

bool SummonAction::Teleport(Player* summoner, Player* player, bool preserveAuras)
{
    if (!summoner || summoner == player)
        return false;

    if (player->GetVehicle())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "meeting_stone_cannot_summon_vehicle", "You cannot summon me while I'm on a vehicle", {}));
        return false;
    }

    if (!summoner->IsBeingTeleported() && !player->IsBeingTeleported())
    {
        float followAngle = GetFollowAngle();
        for (float angle = followAngle - static_cast<float>(M_PI); angle <= followAngle + static_cast<float>(M_PI); angle += static_cast<float>(M_PI) / 4)
        {
            uint32 mapId = summoner->GetMapId();
            float x = summoner->GetPositionX() + cos(angle) * sPlayerbotAIConfig->followDistance;
            float y = summoner->GetPositionY() + sin(angle) * sPlayerbotAIConfig->followDistance;
            float z = summoner->GetPositionZ();

            if (summoner->IsWithinLOS(x, y, z))
            {
                if (sPlayerbotAIConfig->botRepairWhenSummon)
                    bot->DurabilityRepairAll(false, 1.0f, false);

                if (summoner->IsInCombat() && !sPlayerbotAIConfig->allowSummonInCombat)
                {
                    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "meeting_stone_cannot_summon_master_in_combat", "You cannot summon me while you're in combat", {}));
                    return false;
                }

                if (!summoner->IsAlive() && !sPlayerbotAIConfig->allowSummonWhenMasterIsDead)
                {
                    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "meeting_stone_cannot_summon_master_dead", "You cannot summon me while you're dead", {}));
                    return false;
                }

                if (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) &&
                    !sPlayerbotAIConfig->allowSummonWhenBotIsDead)
                {
                    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "meeting_stone_cannot_summon_bot_dead", "You cannot summon me while I'm dead, you need to release my spirit first", {}));
                    return false;
                }

                bool revive =
                    sPlayerbotAIConfig->reviveBotWhenSummoned == 2 ||
                    (sPlayerbotAIConfig->reviveBotWhenSummoned == 1 && !summoner->IsInCombat() && summoner->IsAlive());

                if (bot->isDead() && revive)
                {
                    bot->ResurrectPlayer(1.0f, false);
                    bot->SpawnCorpseBones();
                    botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "meeting_stone_revived", "I live, again!", {}));
                    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Reset();
                }

                player->GetMotionMaster()->Clear();
                AI_VALUE(LastMovement&, "last movement").clear();

                if (!preserveAuras)
                    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP); // 5.4.8: CHANGE_MAP covers "leaving map/getting teleported"
                player->TeleportTo(mapId, x, y, z, 0);
                if (player->GetPet())
                    player->GetPet()->NearTeleportTo(x, y, z, player->GetOrientation());
                if (player->GetGuardianPet())
                    player->GetGuardianPet()->NearTeleportTo(x, y, z, player->GetOrientation());
                if (botAI->HasStrategy("stay", botAI->GetState()))
                {
                    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
                    PositionInfo stayPosition = posMap["stay"];

                    stayPosition.Set(x, y, z, mapId);
                    posMap["stay"] = stayPosition;
                }

                return true;
            }
        }
    }

    if (summoner != player)
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "meeting_stone_not_enough_space", "Not enough place to summon", {}));
    return false;
}
