/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/QuestValues.cpp for issue #257 slice G.
 *
 * 5.4.8 / this-fork adaptations vs AC (all forced by the core, none cosmetic):
 *
 *  - AC's ObjectMgr exposes the four whole-store quest relation maps by reference
 *    (GetCreatureQuestRelationMap(), GetCreatureQuestInvolvedRelationMap(), GetGOQuestRelationMap(),
 *    GetGOQuestInvolvedRelationMap(), AC QuestValues.cpp:19-29). 5.4.8 has no such accessors: only
 *    GetCreatureQuestRelationMapHACK()/GetGOQuestRelationMapHACK() return a whole
 *    std::multimap<uint32,uint32> (Globals/ObjectMgr.h:1207/1211), and the two *involved* stores have
 *    no whole-store getter at all - they are reachable per entry
 *    (GetCreatureQuestInvolvedRelations(entry) / GetGOQuestInvolvedRelations(entry),
 *    ObjectMgr.h:1213/1209) or per quest in reverse (questId -> entry)
 *    (GetCreatureQuestInvolvedRelationReverseBounds(questId) /
 *    GetGOQuestInvolvedRelationReverseBounds(questId), ObjectMgr.h:1214/1210).
 *    EntryQuestRelationMapValue::Calculate() is therefore rewritten to drive the two involved stores
 *    from sObjectMgr->GetQuestTemplates() via the reverse bounds, which yields exactly the same
 *    (entry, questId) pairs AC iterated. The per-entry QuestRelationResult iterator also hides the
 *    first/second pair and yields the mapped value (the quest id) directly
 *    (ObjectMgr.h:650-693).
 *    AC's fourth loop tags GO *involved* relations with QuestRelationFlag::questGiver rather than
 *    questTaker (AC QuestValues.cpp:28-29). That looks like an upstream slip, but it is load-bearing
 *    for nothing here and is preserved verbatim so the map stays 1:1 with AC.
 *
 *  - AC's QUEST_OBJECTIVES_COUNT loop reads the WotLK-only parallel arrays Quest::RequiredNpcOrGo[] /
 *    RequiredItemId[] (AC QuestValues.cpp:39-52, and the same in ActiveQuestObjectivesValue below).
 *    On 5.4.8 the same data is Quest::GetObjectives() (QuestDef.h:376) returning
 *    QuestObjective{ID, Type, ObjectID, Amount, Description} (QuestDef.h:280-291); the objective index
 *    within that vector takes the place of the old array index, so `1 << objective` relation flags are
 *    unchanged, and the type states creature/gameobject explicitly instead of AC's
 *    "negative entry means gameobject" sign test. Progress moved from
 *    QuestStatusData::ItemCount[]/CreatureOrGOCount[] to Player::GetQuestObjectiveCounter(objectiveId)
 *    (Player.h:3186). Same rewrite slices B/E applied.
 *  - AC's `GAI_VALUE2(std::vector<int32>, "item drop list", itemId)` loot objective (AC
 *    QuestValues.cpp:50-51) has no counterpart on this fork - there is no "item drop list" value
 *    anywhere in the module. The loot half of EntryQuestRelationMapValue is therefore not ported, with
 *    the site marked in place.
 *  - AC's SpawnEntries() reads CreatureData::id2/id3 (AC QuestValues.cpp:67-70); 5.4.8's CreatureData has
 *    no alternate-entry fields at all, so only the base entry is walked.
 *  - AC's `sObjectMgr->GetAllGOData()` loop (AC QuestValues.cpp:116-117) has no counterpart: 5.4.8
 *    exposes only GetGOData(guid) by spawn guid (Globals/ObjectMgr.h:1575) and keeps
 *    _gameObjectDataStore private (ObjectMgr.h:1952), with no public whole-store accessor. The gameobject
 *    half of QuestGuidpMapValue::Calculate() is therefore not ported, with the site marked in place -
 *    it needs an ObjectMgr::GetAllGOData() accessor, which is a core file this slice does not own.
 *  - AC's QuestRelationBounds / GetCreatureQuestRelationBounds / GetCreatureQuestInvolvedRelationBounds /
 *    GetGOQuestRelationBounds / GetGOQuestInvolvedRelationBounds (AC QuestValues.cpp:356-368) become
 *    5.4.8's per-entry QuestRelationResult objects (ObjectMgr.h:1208-1213), iterated with
 *    `for (uint32 questId : sObjectMgr->Get...Relations(entry))`.
 *  - AC's DIALOG_STATUS_* enum does not exist on 5.4.8 (it has the bitflag QuestGiverStatus,
 *    QuestDef.h:111-127). The file-local `DialogStatus` enum below reproduces AC's ordinals verbatim so
 *    getDialogStatus()'s "keep the highest status" comparison and the three Can*QuestValue classes' "=="
 *    comparisons keep exactly AC's meaning. See QuestValues.h:105-113 for why the fork's enum cannot be
 *    used instead.
 *  - Quest::IsAutoComplete() does not exist on 5.4.8; the equivalent is Quest::IsAutoAccept()
 *    (QuestDef.h:395) - the same SPECIAL_FLAGS_AUTO_ACCEPT bit AC's IsAutoComplete() tests. Four sites.
 *  - Player::GetQuestSlotQuestId(uint8) takes a uint16 slot on 5.4.8 (Player.h:1761), so AC's uint8 slot
 *    counter in FreeQuestLogSlotValue becomes uint16.
 *  - GuidPosition::IsCreatureOrGOAccessible() (AC TravelMgr.h:425 / AC TravelMgr.cpp:1037-1057) is not
 *    ported to this fork's lean GuidPosition (Travel/WorldPosition.h:360-385, which has GetCreature()/
 *    GetGameObject()/isDead() but no accessibility predicate). Its exact body is reproduced as the
 *    file-local IsCreatureOrGOAccessible() helper below.
 *  - MAX_QUEST_LOG_SIZE is 25 on 5.4.8 (QuestDef.h:35), the same as AC.
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault(...) is not ported on this fork (#322); no string
 *    in this file goes through it.
 *  - No IsSelfBot()/IsAltBot() and no BroadcastHelper call is involved on this path, so slices C/D's
 *    IsRealPlayer()/HasRealPlayerMaster() divergences do not apply.
 */

#include "QuestValues.h"

#include "DBCStores.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "SharedValueContext.h"
#include "WorldPosition.h"
#include "World.h"
#include <array>
#include <string>

namespace
{
// AC's DIALOG_STATUS_* enum, AC QuestDef.h:112-122. 5.4.8 has no such enum (its QuestGiverStatus is a
// bitflag with a different scale and different names, QuestDef.h:111-127), and these ordinals are what
// getDialogStatus() ranks with ">" and what the Can*QuestValue classes compare with "==". Reproduced
// verbatim so the value behaves exactly as it does in AC. See QuestValues.h:105-113.
enum DialogStatus
{
    DIALOG_STATUS_NONE = 0,
    DIALOG_STATUS_UNAVAILABLE = 1,
    DIALOG_STATUS_LOW_LEVEL_AVAILABLE = 2,
    DIALOG_STATUS_LOW_LEVEL_REWARD_REP = 3,
    DIALOG_STATUS_LOW_LEVEL_AVAILABLE_REP = 4,
    DIALOG_STATUS_INCOMPLETE = 5,
    DIALOG_STATUS_REWARD_REP = 6,
    DIALOG_STATUS_AVAILABLE_REP = 7,
    DIALOG_STATUS_AVAILABLE = 8,
    DIALOG_STATUS_REWARD2 = 9,  // no yellow dot on minimap
    DIALOG_STATUS_REWARD = 10   // yellow dot on minimap
};

// AC's GuidPosition::IsCreatureOrGOAccessible(), body copied from AC TravelMgr.cpp:1037-1057 - the fork's
// lean GuidPosition (Travel/WorldPosition.h:360-385) does not carry it. See the file header.
bool IsCreatureOrGOAccessible(GuidPosition& guidp)
{
    Map* map = guidp.getMap();
    // 5.4.8: the fork's WorldPosition exposes getX()/getY() (Travel/WorldPosition.h:129-130), not AC's
    // GetPositionX()/GetPositionY().
    if (!map || !map->IsGridLoaded(guidp.getX(), guidp.getY()))
        return false;

    if (guidp.IsCreature())
    {
        Creature* creature = guidp.GetCreature();
        if (creature && creature->IsInWorld() && creature->IsAlive())
            return true;
    }
    else if (guidp.IsGameObject())
    {
        GameObject* go = guidp.GetGameObject();
        if (go && go->IsInWorld())
            return true;
    }

    return false;
}

// AC's `bot->GetFactionReactionTo(bot->GetFactionTemplateEntry(), sFactionTemplateStore.LookupEntry(faction)) <
// REP_FRIENDLY` (AC QuestValues.cpp:189-191 and :245-246). 5.4.8 changed that helper into a *static*
// `Unit::GetFactionReactionTo(FactionTemplateEntry const* self, Unit const* target)`
// (Entities/Unit/Unit.h:1354), whose second argument is a loaded Unit - and both sites here only have a
// CreatureTemplate, never a loaded object. So the "common faction based check" tail of the core function
// (Entities/Unit/Unit.cpp:8665-8675) is reproduced below over the two faction template entries, which is
// the same primitive chain (IsHostileTo / IsFriendlyTo, both ways) and reaches the same >= REP_FRIENDLY
// decision the core would. The reputation / war / contested-guard branches of the core function are
// deliberately not reproduced: they need a real target Unit and a ReputationMgr owner, neither of which
// exists at either AC site either (AC's template-only call could not reach them).
bool IsFriendlyReactionTo(FactionTemplateEntry const* self, FactionTemplateEntry const* other)
{
    // Unit.cpp:8634-8639 - no template entry means REP_NEUTRAL, which is below REP_FRIENDLY.
    if (!self || !other)
        return false;

    if (self->IsHostileTo(*other))
        return false;

    if (self->IsFriendlyTo(*other))
        return true;

    if (other->IsFriendlyTo(*self))
        return true;

    // Either neutral by default or FACTION_TEMPLATE_FLAG_HOSTILE_BY_DEFAULT - both below REP_FRIENDLY
    // (Unit.cpp:8672-8675).
    return false;
}

// Every entry a spawn point can hold. 5.4.8's CreatureData has no alternate-entry fields, so AC's
// { creData.id, creData.id2, creData.id3 } (AC QuestValues.cpp:67-70) collapses to the base entry.
std::array<uint32, 1> SpawnEntries(CreatureData const& creData)
{
    return {creData.id};
}
}  // namespace

// What kind of a relation does this entry have with this quest.
entryQuestRelationMap EntryQuestRelationMapValue::Calculate()
{
    entryQuestRelationMap rMap;

    for (auto relation : *sObjectMgr->GetCreatureQuestRelationMapHACK())
        rMap[relation.first][relation.second] |= (int)QuestRelationFlag::questGiver;

    // 5.4.8: AC's *sObjectMgr->GetCreatureQuestInvolvedRelationMap() (AC QuestValues.cpp:22-23) has no
    // whole-store accessor; the reverse bounds (questId -> entry) are driven from the quest list instead,
    // yielding the same (entry, questId) pairs. See the file header.
    // Quest objectives
    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();

    for (auto& questItr : questMap)
    {
        uint32 questId = questItr.first;

        for (auto const& itr :
             sObjectMgr->GetCreatureQuestInvolvedRelationReverseBounds(questId))
            rMap[itr.first][questId] |= (int)QuestRelationFlag::questTaker;
    }

    for (auto relation : *sObjectMgr->GetGOQuestRelationMapHACK())
        rMap[-(int32)relation.first][relation.second] |= (int)QuestRelationFlag::questGiver;

    // 5.4.8: same reverse-bounds rewrite as the creature *involved* store above.
    for (auto& questItr : questMap)
    {
        uint32 questId = questItr.first;

        for (auto const& itr : sObjectMgr->GetGOQuestInvolvedRelationReverseBounds(questId))
            rMap[-(int32)itr.first][questId] |= (int)QuestRelationFlag::questGiver;
    }

    for (auto& questItr : questMap)
    {
        uint32 questId = questItr.first;
        Quest* quest = questItr.second;

        QuestObjectives const& objectives = quest->GetObjectives();

        for (uint32 objective = 0; objective < objectives.size(); ++objective)
        {
            QuestObjective const& objectiveData = objectives[objective];

            uint32 relationFlag = 1 << objective;

            // Kill objective
            // 5.4.8: AC's `if (quest->RequiredNpcOrGo[objective]) rMap[...][questId] |= relationFlag;`
            // (AC QuestValues.cpp:44-45) - the objective states whether the entry is a creature or a
            // gameobject instead of relying on the sign of the entry. See the file header.
            if (objectiveData.ObjectID &&
                (objectiveData.Type == QUEST_OBJECTIVE_MONSTER ||
                 objectiveData.Type == QUEST_OBJECTIVE_GAMEOBJECT))
            {
                int32 entry = objectiveData.Type == QUEST_OBJECTIVE_GAMEOBJECT
                                  ? -(int32)objectiveData.ObjectID
                                  : (int32)objectiveData.ObjectID;
                rMap[entry][questId] |= relationFlag;
            }

            // Loot objective
            // 5.4.8: AC's `for (auto& entry : GAI_VALUE2(std::vector<int32>, "item drop list",
            // quest->RequiredItemId[objective])) rMap[entry][questId] |= relationFlag;`
            // (AC QuestValues.cpp:48-52) is not ported: this fork has no "item drop list" value anywhere
            // in the module. See the file header.
        }
    }

    return rMap;
}

// Get all the objective entries for a specific quest.
void FindQuestObjectData::GetObjectiveEntries()
{
    relationMap = GAI_VALUE(entryQuestRelationMap, "entry quest relation");
}

// Data worker. Checks for a specific creature what quest they are needed for and puts them in the proper
// place in the quest map.
void FindQuestObjectData::operator()(CreatureData const& creData)
{
    // The GuidPosition keeps the spawn's base entry, which can disagree with the alternate it is filed
    // under. It resolves by spawnId, and consumers read the map key.
    // 5.4.8: CreatureData has no id2/id3, so SpawnEntries() yields the base entry only.
    for (uint32 entry : SpawnEntries(creData))
    {
        if (!entry)
            continue;

        auto relations = relationMap.find(int32(entry));
        if (relations == relationMap.end())
            continue;

        for (auto& relation : relations->second)
        {
            uint32 questId = relation.first;
            uint32 flag = relation.second;
            data[questId][flag][entry].push_back(GuidPosition(creData));
        }
    }
}

// GameObject data worker. Checks for a specific gameObject what quest they are needed for and puts them in
// the proper place in the quest map.
void FindQuestObjectData::operator()(GameObjectData const& goData)
{
    int32 entry = goData.id * -1;

    for (auto& relation : relationMap[entry])
    {
        uint32 questId = relation.first;
        uint32 flag = relation.second;
        data[questId][flag][entry].push_back(GuidPosition(goData));
    }
}

// Goes past all creatures and gameobjects and creatures the full quest guid map.
questGuidpMap QuestGuidpMapValue::Calculate()
{
    FindQuestObjectData worker;
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
        worker(itr.second);

    // #257 (AC QuestValues.cpp:116-117): the gameobject half. 5.4.8 keeps _gameObjectDataStore private, so this
    // needs the one-line accessor ObjectMgr::GetAllGOData() (Globals/ObjectMgr.h, next to GetGOData()):
    //     GameObjectDataContainer const& GetAllGOData() const { return _gameObjectDataStore; }
    for (auto const& itr : sObjectMgr->GetAllGOData())
        worker(itr.second);

    return worker.GetResult();
}

// Selects all questgivers for a specific level (range).
questGiverMap QuestGiversValue::Calculate()
{
    uint32 level = 0;
    std::string const q = getQualifier();
    bool hasQualifier = !q.empty();

    if (hasQualifier)
        level = std::stoi(q);

    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    questGiverMap guidps;

    for (auto& qPair : questMap)
    {
        for (auto& entry : qPair.second[(int)QuestRelationFlag::questGiver])
        {
            for (auto& guidp : entry.second)
            {
                uint32 questId = qPair.first;

                if (hasQualifier)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

                    // 5.4.8: GetMinLevel()/GetQuestLevel() return int32 (QuestDef.h:340-341), so the
                    // comparison casts are made explicit; AC's own casts are kept where AC had them.
                    if (quest && (level < quest->GetMinLevel() || (int)level > quest->GetQuestLevel() + 10))
                        continue;
                }

                guidps[questId].push_back(guidp);
            }
        }
    }

    return guidps;
}

std::vector<GuidPosition> ActiveQuestGiversValue::Calculate()
{
    questGiverMap qGivers = GAI_VALUE2(questGiverMap, "quest givers", bot->GetLevel());

    std::vector<GuidPosition> retQuestGivers;

    for (auto& qGiver : qGivers)
    {
        uint32 questId = qGiver.first;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            continue;
        }

        if (!bot->CanTakeQuest(quest, false))
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);

        if (status != QUEST_STATUS_NONE)
            continue;

        for (auto& guidp : qGiver.second)
        {
            CreatureTemplate const* creatureTemplate = guidp.GetCreatureTemplate();

            if (creatureTemplate)
            {
                if (!IsFriendlyReactionTo(
                        bot->GetFactionTemplateEntry(),
                        sFactionTemplateStore.LookupEntry(creatureTemplate->faction)))
                    continue;
            }

            // 5.4.8: file-local IsCreatureOrGOAccessible() - see the file header.
            if (!IsCreatureOrGOAccessible(guidp))
                continue;

            retQuestGivers.push_back(guidp);
        }
    }

    return retQuestGivers;
}

std::vector<GuidPosition> ActiveQuestTakersValue::Calculate()
{
    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    std::vector<GuidPosition> retQuestTakers;

    QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

    for (auto& questStatus : questStatusMap)
    {
        uint32 questId = questStatus.first;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

        if (!quest)
        {
            continue;
        }

        QuestStatus status = questStatus.second.Status;
        // 5.4.8: AC's quest->IsAutoComplete() -> quest->IsAutoAccept() (QuestDef.h:395), the same
        // SPECIAL_FLAGS_AUTO_ACCEPT bit. See the file header.
        if ((status != QUEST_STATUS_COMPLETE || bot->GetQuestRewardStatus(questId)) &&
            (!quest->IsAutoAccept() || !bot->CanTakeQuest(quest, false)))
            continue;

        auto q = questMap.find(questId);

        if (q == questMap.end())
            continue;

        auto qt = q->second.find((int)QuestRelationFlag::questTaker);

        if (qt == q->second.end())
            continue;

        for (auto& entry : qt->second)
        {
            if (entry.first > 0)
            {
                if (CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(entry.first))
                {
                    if (!IsFriendlyReactionTo(bot->GetFactionTemplateEntry(),
                                              sFactionTemplateStore.LookupEntry(info->faction)))
                        continue;
                }
            }

            for (auto& guidp : entry.second)
            {
                // 5.4.8: file-local IsCreatureOrGOAccessible() - see the file header.
                if (!IsCreatureOrGOAccessible(guidp))
                    continue;

                retQuestTakers.push_back(guidp);
            }
        }
    }

    return retQuestTakers;
}

std::vector<GuidPosition> ActiveQuestObjectivesValue::Calculate()
{
    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    std::vector<GuidPosition> retQuestObjectives;

    QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

    for (auto& questStatus : questStatusMap)
    {
        uint32 questId = questStatus.first;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            continue;
        }

        QuestStatusData statusData = questStatus.second;
        if (statusData.Status != QUEST_STATUS_INCOMPLETE)
            continue;

        // 5.4.8: AC's QUEST_OBJECTIVES_COUNT loop over RequiredItemCount[]/RequiredNpcOrGoCount[] against
        // statusData.ItemCount[]/CreatureOrGOCount[] (AC QuestValues.cpp:286-304) is rewritten over
        // Quest::GetObjectives() and Player::GetQuestObjectiveCounter(objective.ID): on 5.4.8 one
        // objective entry carries both the required amount and whether it is an item or a creature/GO
        // objective, so AC's two sequential guards collapse into one. The objective's index in that vector
        // is what `1 << objective` keys on, unchanged. See the file header.
        QuestObjectives const& objectives = quest->GetObjectives();

        for (uint32 objective = 0; objective < objectives.size(); ++objective)
        {
            QuestObjective const& objectiveData = objectives[objective];

            if (objectiveData.Amount > 0)
            {
                uint32 reqCount = uint32(objectiveData.Amount);
                uint32 hasCount = bot->GetQuestObjectiveCounter(objectiveData.ID);

                if (!reqCount || hasCount >= reqCount)
                    continue;
            }

            auto q = questMap.find(questId);

            if (q == questMap.end())
                continue;

            auto qt = q->second.find((int)QuestRelationFlag(1 << objective));

            if (qt == q->second.end())
                continue;

            for (auto& entry : qt->second)
            {
                for (auto& guidp : entry.second)
                {
                    // 5.4.8: file-local IsCreatureOrGOAccessible() - see the file header.
                    if (!IsCreatureOrGOAccessible(guidp))
                        continue;

                    retQuestObjectives.push_back(guidp);
                }
            }
        }
    }

    return retQuestObjectives;
}

uint8 FreeQuestLogSlotValue::Calculate()
{
    uint8 numQuest = 0;
    // 5.4.8: uint8 slot -> uint16, Player::GetQuestSlotQuestId takes uint16 (Player.h:1761).
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);

        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        numQuest++;
    }

    return MAX_QUEST_LOG_SIZE - numQuest;
}

uint32 DialogStatusValue::getDialogStatus(Player* bot, int32 questgiver, uint32 questId)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    // QuestRelations (quest-giver) and InvolvedRelations (quest-finisher), for a creature when
    // `questgiver` is positive and for a gameobject (negated entry) when it is not - AC
    // QuestValues.cpp:359-368.
    // 5.4.8: AC's four *Bounds getters become the per-entry QuestRelationResult accessors
    // (ObjectMgr.h:1208-1213); their iterator yields the mapped value, the quest id, directly.
    QuestRelationResult rbounds;   // QuestRelations (quest-giver)
    QuestRelationResult irbounds;  // InvolvedRelations (quest-finisher)

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelations(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelations(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelations(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelations(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (uint32 relatedQuestId : irbounds)
    {
        if (questId && relatedQuestId != questId)
            continue;

        Quest const* pQuest = sObjectMgr->GetQuestTemplate(relatedQuestId);
        if (!pQuest)
        {
            continue;
        }

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        QuestStatus status = bot->GetQuestStatus(relatedQuestId);

        // 5.4.8: AC's pQuest->IsAutoComplete() -> pQuest->IsAutoAccept() (QuestDef.h:395).
        if ((status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(relatedQuestId)) ||
            (pQuest->IsAutoAccept() && bot->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoAccept() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (uint32 relatedQuestId : rbounds)
    {
        if (questId && relatedQuestId != questId)
            continue;

        Quest const* pQuest = sObjectMgr->GetQuestTemplate(relatedQuestId);
        if (!pQuest)
        {
            continue;
        }

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        QuestStatus status = bot->GetQuestStatus(relatedQuestId);

        // For all other cases the mark is handled either at some place else, or with involved-relations
        // already
        if (status == QUEST_STATUS_NONE)
        {
            if (bot->CanSeeStartQuest(pQuest))
            {
                if (bot->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoAccept() ||
                        (pQuest->IsRepeatable() && bot->getQuestStatusMap()[relatedQuestId].Status ==
                                                        QUEST_STATUS_REWARDED))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 || bot->GetLevel() <= bot->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

uint32 DialogStatusValue::Calculate() { return getDialogStatus(bot, std::stoi(getQualifier())); }

uint32 DialogStatusQuestValue::Calculate()
{
    return getDialogStatus(bot, getMultiQualifier(getQualifier(), 0), getMultiQualifier(getQualifier(), 1));
}

bool CanAcceptQuestValue::Calculate() { return AI_VALUE2(uint32, "dialog status", getQualifier()) == DIALOG_STATUS_AVAILABLE; }

bool CanAcceptQuestLowLevelValue::Calculate()
{
    uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
    return dialogStatus == DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
}

bool CanTurnInQuestValue::Calculate()
{
    uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
    return dialogStatus == DIALOG_STATUS_REWARD2 || dialogStatus == DIALOG_STATUS_REWARD ||
           dialogStatus == DIALOG_STATUS_REWARD_REP;
}