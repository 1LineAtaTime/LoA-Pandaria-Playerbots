/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ChatShortcutActions.cpp. #173 ported "follow"/"stay"/
 * "move from group"; #177 completes the set (flee/runaway/grind/tank attack/max dps) and restores the
 * in-combat formation-move block in FollowChatShortcutAction::Execute() that #173 had deferred.
 *
 * Deviations from AC:
 *  - (#177 follow-up, #322 landed) every reply goes through PlayerbotTextMgr::GetBotTextOrDefault()
 *    with AC's keys and default strings (AC ChatShortcutActions.cpp:89,113,131,147,168,173,191,207,228),
 *    so the same ai_playerbot_texts variants are picked as on AC.
 *  - The in-combat formation-move block below follows this fork's own FollowAction::Execute()
 *    idiom (FollowActions.cpp) rather than AC's verbatim text: WorldLocation::GetMapId() is compared
 *    to -1 (this fork has no MAPID_INVALID constant).
 *  - AC's Naxx/BWL chat shortcuts (raid-strategy toggles for content this fork does not have) are not
 *    ported.
 *  - #194 (S8): botAI->PetFollow() is now ported as the free helper PlayerbotPetFollow() (PetsAction.h)
 *    instead of a PlayerbotAI member - see PetsAction.cpp's file comment.
 */

#include "ChatShortcutActions.h"

#include "Event.h"
#include "Formations.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "PlayerbotTextMgr.h"
#include "PositionValue.h"

void PositionsResetAction::ResetReturnPosition()
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Reset();
    posMap["return"] = pos;
}

void PositionsResetAction::SetReturnPosition(float x, float y, float z)
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Set(x, y, z, botAI->GetBot()->GetMapId());
    posMap["return"] = pos;
}

void PositionsResetAction::ResetStayPosition()
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["stay"];
    pos.Reset();
    posMap["stay"] = pos;
}

void PositionsResetAction::SetStayPosition(float x, float y, float z)
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["stay"];
    pos.Set(x, y, z, botAI->GetBot()->GetMapId());
    posMap["stay"] = pos;
}

bool FollowChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->ChangeStrategy("+follow,-passive,-grind,-move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-stay,-follow,-passive,-grind,-move from group", BOT_STATE_COMBAT);
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Reset();

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Reset();
    posMap["return"] = pos;

    pos = posMap["stay"];
    pos.Reset();
    posMap["stay"] = pos;

    // AC ChatShortcutActions.cpp:66-93: snap straight into formation instead of waiting for the
    // persistent "follow" strategy's own next tick when the bot is already in combat.
    if (bot->IsInCombat())
    {
        Formation* formation = AI_VALUE(Formation*, "formation");
        std::string const target = formation->GetTargetName();
        bool moved = false;
        if (!target.empty())
            moved = Follow(AI_VALUE(Unit*, target));
        else
        {
            WorldLocation loc = formation->GetLocation();
            if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
                return false;

            MovementPriority priority = botAI->GetState() == BOT_STATE_COMBAT ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL;
            moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), false, false, false,
                        true, priority);
        }

        // #194 (S8): AC ChatShortcutActions.cpp:85 - PetFollow ported as a free helper (PetsAction.h).
        if (bot->GetPet())
            PlayerbotPetFollow(bot);

        if (moved)
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "following", "Following", {}));
            return true;
        }
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "following", "Following", {}));
    return true;
}

bool StayChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+stay,-passive,-move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+stay,-follow,-passive,-move from group", BOT_STATE_COMBAT);

    SetReturnPosition(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    SetStayPosition(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "staying", "Staying", {}));
    return true;
}

bool MoveFromGroupChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    // No need to remove stay/follow - "move from group" takes priority over both (see their
    // isUseful() methods).
    botAI->ChangeStrategy("+move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+move from group", BOT_STATE_COMBAT);

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "move_from_group", "Moving away from group", {}));
    return true;
}


bool FleeChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+follow,-stay,+passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+follow,-stay,+passive", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    if (bot->GetMapId() != master->GetMapId() || bot->GetDistance(master) > sPlayerbotAIConfig->sightDistance)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "fleeing_far", "I will not flee with you - too far away", {}));
        return true;
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "fleeing", "Fleeing", {}));
    return true;
}

bool GoawayChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+runaway,-stay", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+runaway,-stay", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "running_away", "Running away", {}));
    return true;
}

bool GrindChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+grind,-passive,-stay", BOT_STATE_NON_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "grinding", "Grinding", {}));
    return true;
}

bool TankAttackChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!PlayerBotSpec::IsTank(bot))
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("-passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-passive", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "attacking", "Attacking", {}));
    return true;
}

bool MaxDpsChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!botAI->ContainsStrategy(STRATEGY_TYPE_DPS))
        return false;

    botAI->Reset();

    botAI->ChangeStrategy("-threat,-conserve mana,-cast time,+dps debuff,+boost", BOT_STATE_COMBAT);
    botAI->TellMaster("Max DPS!");

    return true;
}
