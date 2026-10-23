/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RevealGatheringItemAction.cpp (issue #377).
 *
 * 5.4.8 / this-fork adaptations vs AC (no shared header was touched):
 *  - AnyGameObjectInObjectRangeCheck (AC Ai/Base/Value/NearestGameObjects.h:16-32) is private to
 *    strategy/value/NearestGameObjects.cpp here, so an identical predicate is a file-local struct.
 *  - Acore:: grid searchers -> Trinity::; Cell::VisitObjects -> Cell::VisitAllObjects (the idiom
 *    NearestGameObjects.cpp uses in this fork).
 *  - ServerFacade::GetDistance2d / IsDistanceLessOrEqualThan -> WorldObject::GetExactDist2d / <=.
 *  - ChatHelper::FormatGameobject (AC Bot/Cmd/ChatHelper.cpp:379-385) and PlayerbotAI::Ping (AC
 *    Bot/PlayerbotAI.cpp:5594-5609) do not exist here and ChatHelper.h / PlayerbotAI.h must not be
 *    edited, so both are file-local helpers below.
 *  - Ping packet: AC sends MSG_MINIMAP_PING (guid, x, y). 5.4.8 sends SMSG_MINIMAP_PING, built exactly
 *    as WorldSession::HandleMinimapPingOpcode (src/server/game/Handlers/GroupHandler.cpp:679-720)
 *    builds it. That handler names the first wire float "y" and the second "x"; the same order is used
 *    here, so if the client's wire order is really the reverse the ping icon would be transposed
 *    (cosmetic only).
 *  - bot->Say(text, LANG_UNIVERSAL) matches Player::Say(std::string const&, Language, WorldObject const*).
 *  - sLockStore/LockEntry/LOCK_KEY_SKILL/SkillByLockType/SKILL_MINING/SKILL_HERBALISM/SKILL_FISHING/
 *    GAMEOBJECT_TYPE_FISHINGNODE all exist unchanged in this core.
 */

#include "RevealGatheringItemAction.h"

#include "GridObject.h"  // must precede the grid notifiers (see NearestGameObjects.cpp)

#include "CellImpl.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
// AC Ai/Base/Value/NearestGameObjects.h:16-32 AnyGameObjectInObjectRangeCheck.
struct RevealGameObjectInRangeCheck
{
    RevealGameObjectInRangeCheck(WorldObject const* source, float range) : _source(source), _range(range) {}

    bool operator()(GameObject* go) const
    {
        return go && go->isSpawned() && go->GetGOInfo() && _source->IsWithinDist(go, _range, false);
    }

    WorldObject const* _source;
    float _range;
};

// AC botAI->HasSkill(SkillType): no such PlayerbotAI method on this fork; same file-local stand-in as
// LootAction.cpp / LootObjectStack.cpp BotHasSkill().
bool RevealBotHasSkill(Player* bot, uint32 skillId)
{
    return skillId != SKILL_NONE && bot->HasSkill(skillId) && bot->GetSkillValue(skillId) > 0;
}

// AC Bot/Cmd/ChatHelper.cpp:379-385 ChatHelper::FormatGameobject.
std::string RevealFormatGameobject(GameObject* go)
{
    std::ostringstream out;
    out << "|cFFFFFF00|Hfound:" << go->GetGUID().GetRawValue() << ":" << go->GetEntry() << ":"
        << "|h[" << go->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()) << "]|h|r";
    return out.str();
}

// AC Bot/PlayerbotAI.cpp:5594-5609 PlayerbotAI::Ping. Packet layout: HandleMinimapPingOpcode.
void RevealPing(Player* bot, float x, float y)
{
    ObjectGuid guid = bot->GetGUID();

    WorldPacket data(SMSG_MINIMAP_PING, 1 + 8 + 4 + 4);
    data << float(y);
    data << float(x);

    data.WriteBit(guid[0]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[4]);

    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[4]);

    if (bot->GetGroup())
        bot->GetGroup()->BroadcastPacket(&data, true, -1, bot->GetGUID());
    else
        bot->GetSession()->SendPacket(&data);
}
}  // namespace

// AC RevealGatheringItemAction.cpp:17-80.
bool RevealGatheringItemAction::Execute(Event /*event*/)
{
    if (!bot->GetGroup())
        return false;

    std::list<GameObject*> targets;
    RevealGameObjectInRangeCheck u_check(bot, sPlayerbotAIConfig->grindDistance);
    Trinity::GameObjectListSearcher<RevealGameObjectInRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, sPlayerbotAIConfig->reactDistance);

    std::vector<GameObject*> result;
    for (GameObject* go : targets)
    {
        if (!go || !go->isSpawned() || bot->GetExactDist2d(go) <= sPlayerbotAIConfig->lootDistance)
            continue;

        if (LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->GetLockId()))
        {
            for (uint8 i = 0; i < 8; ++i)
            {
                if (lockInfo->Type[i] == LOCK_KEY_SKILL)
                {
                    uint32 skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                    uint32 reqSkillValue = std::max(2u, lockInfo->Skill[i]);
                    if ((skillId == SKILL_MINING || skillId == SKILL_HERBALISM) &&
                        RevealBotHasSkill(bot, skillId) && uint32(bot->GetSkillValue(skillId)) >= reqSkillValue)
                    {
                        result.push_back(go);
                        break;
                    }
                }
            }
        }

        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE && RevealBotHasSkill(bot, SKILL_FISHING))
            result.push_back(go);
    }

    if (result.empty())
        return false;

    GameObject* go = result[urand(0, result.size() - 1)];
    if (!go)
        return false;

    std::ostringstream msg;
    msg << "I see a " << RevealFormatGameobject(go) << ". ";

    switch (go->GetGoType())
    {
        case GAMEOBJECT_TYPE_CHEST:
            msg << "Let's look at it.";
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            msg << "Let's fish a bit.";
            break;
        default:
            msg << "Should we go nearer?";
    }

    // everything is fine, do it
    RevealPing(bot, go->GetPositionX(), go->GetPositionY());
    bot->Say(msg.str(), LANG_UNIVERSAL);
    return true;
}
