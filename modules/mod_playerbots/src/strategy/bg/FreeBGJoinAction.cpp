/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * #611: AC FreeBGJoinAction (Ai/Base/Actions/BattleGroundJoinAction.h:38-45, .cpp:557-649), kept in its own file so the
 * BG lane's BattleGroundJoinAction.h/.cpp stay untouched. Registered as "free bg join" (AC ActionContext.h:196,403);
 * driven only by the dormant RpgSubActions path, exactly as in AC.
 */

#include "FreeBGJoinAction.h"

// AC .cpp:557 ("Not sure if this has ever worked, but it should be similar to BGJoinAction::shouldJoinBg"): AC's body
// (:558-649) is a line-for-line copy of AC BGJoinAction::shouldJoinBg (:225-336). The fork's BGJoinAction::shouldJoinBg
// is that same logic in its 5.4.8 form (rated-arena census via PlayerbotArena::Census, no skirmish arenas, captain test
// -> canLeadArenaParty), so the copy is replaced by a call to it rather than a second divergent copy.
bool FreeBGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    return BGJoinAction::shouldJoinBg(queueTypeId, bracketId);
}
