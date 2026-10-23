#include "GrindTargetValue.h"

#include "LootMgr.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "ServerFacade.h"

#include <cmath>
#include <unordered_map>

Unit* GrindTargetValue::Calculate()
{
    uint32 memberCount = 1;
    Group* group = bot->GetGroup();
    if (group)
        memberCount = group->GetMembersCount();

    Unit* target = nullptr;
    uint32 assistCount = 0;
    while (!target && assistCount < memberCount)
    {
        target = FindTargetForGrinding(assistCount++);
    }

    return target;
}

Unit* GrindTargetValue::FindTargetForGrinding(uint32 assistCount)
{
    uint32 memberCount = 1;
    Group* group = bot->GetGroup();
    Player* master = GetMaster();

    if (master && (master == bot || master->GetMapId() != bot->GetMapId() || master->IsBeingTeleported() || !GET_PLAYERBOT_AI(master)))
        master = nullptr;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        return unit;
    }

    GuidVector targets = *context->GetValue<GuidVector>("possible targets");

    // issue #62: "no grind target" was the dominant reason bots never fought, which starves quest
    // objectives. Record why candidates are rejected, for the watched bots, at most every 10 s.
    bool const debugBot =
        !sPlayerbotAIConfig->debugBotNames.empty() && sPlayerbotAIConfig->debugBotNames.count(bot->GetName());
    uint32 candidates = 0;
    char const* lastReject = "none";

    auto debugReport = [&]()
    {
        if (!debugBot)
            return;
        static thread_local std::map<ObjectGuid, uint32> lastLog;
        uint32 const now = getMSTime();
        uint32& last = lastLog[bot->GetGUID()];
        if (last && getMSTimeDiff(last, now) < 10000)
            return;
        last = now;
        TC_LOG_DEBUG("playerbots", "[GRIND] %s grind targets: possible=%zu candidates=%u lastReject=%s",
                     bot->GetName().c_str(), targets.size(), candidates, lastReject);
    };

    if (targets.empty())
    {
        debugReport();
        return nullptr;
    }

    float distance = 0;
    Unit* result = nullptr;
    std::unordered_map<uint32, bool> needForQuestMap;

    for (ObjectGuid const guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);

        if (!unit)
            continue;

        ++candidates;

        if (unit->isDead()) continue;
        // #843 (AC GrindTargetValue.cpp:64-65): a unit that is leaving the world is no grind target.
        if (!unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
            continue;

        auto& rep = bot->ToPlayer()->GetReputationMgr();
        if (unit->ToCreature() && !unit->ToCreature()->GetCreatureTemplate()->lootid &&
            bot->GetReactionTo(unit) >= REP_NEUTRAL)
        {
            lastReject = "no lootid + not hostile";
            continue;
        }

        if (!bot->IsHostileTo(unit) && unit->GetNpcFlags() != UNIT_NPC_FLAG_NONE)
        {
            lastReject = "friendly npc with npc flags";
            continue;
        }

        if (!bot->IsHonorOrXPTarget(unit))
        {
            lastReject = "gray (no honor/xp)";
            continue;
        }

        // AC parity (#62/owner rule): AC filters on INTERACTION_DISTANCE vertical offset (AC GrindTargetValue.cpp:77).
        // #843: AC's INTERACTION_DISTANCE is 5.5 yd (AC ObjectDefines.h:24), this core's is 5.0 (ObjectDefines.h:24), so
        // the same check rejected every mob 5.0-5.5 yd above or below the bot here (the last reject reason in 140 of the
        // 192 w54 [GRIND] lines). Use AC's value so the grind pick matches AC on the same slope.
        constexpr float AC_INTERACTION_DISTANCE = 5.5f;
        if (std::fabs(bot->GetPositionZ() - unit->GetPositionZ()) > AC_INTERACTION_DISTANCE)
        {
            lastReject = "z difference over 5.5 yd";
            continue;
        }

        if (!bot->InBattleground() && GetTargetingPlayerCount(unit) > assistCount)
        {
            lastReject = "already targeted by another player";
            continue;
        }

        // if (!bot->InBattleground() && master && master->GetDistance(unit) >= sPlayerbotAIConfig->grindDistance &&
        // !sRandomPlayerbotMgr->IsRandomBot(bot)) continue;

        // Bots in bot-groups no have a more limited range to look for grind target
        if (!bot->InBattleground() && master && botAI->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT)
            && sServerFacade->GetDistance2d(master, unit) > sPlayerbotAIConfig->lootDistance)
        {
            /*if (botAI->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                botAI->TellMaster(chat->FormatWorldobject(unit) + " ignored (far from master).");
            continue;*/
        }

        if (!bot->InBattleground() && (int)unit->GetLevel() - (int)bot->GetLevel() > 4 && !unit->GetGUID().IsPlayer())
        {
            lastReject = "more than 4 levels above the bot";
            continue;
        }

        if (Creature* creature = unit->ToCreature())
            if (CreatureTemplate const* CreatureTemplate = creature->GetCreatureTemplate())
                if (CreatureTemplate->rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
                {
                    lastReject = "elite and cannot fight elite";
                    continue;
                }

        if (!bot->IsWithinLOSInMap(unit))
        {
            lastReject = "no line of sight";
            continue;
        }

        // AC parity (GrindTargetValue.cpp:108-121): while actively travelling (not just wandering/idle),
        // only accept mobs within their aggro range (+10, capped at 30 yd) unless a quest needs them.
        bool inactiveGrindStatus =
            botAI->GetRpgInfo().GetStatus() != RPG_WANDER_RANDOM && botAI->GetRpgInfo().GetStatus() != RPG_IDLE;

        float aggroRange = 30.0f;
        if (unit->ToCreature())
            aggroRange = std::min(30.0f, unit->ToCreature()->GetAttackDistance(bot) + 10.0f);
        bool outOfAggro = unit->ToCreature() && bot->GetDistance(unit) > aggroRange;
        if (inactiveGrindStatus && outOfAggro)
        {
            if (needForQuestMap.find(unit->GetEntry()) == needForQuestMap.end())
                needForQuestMap[unit->GetEntry()] = needForQuest(unit);

            if (!needForQuestMap[unit->GetEntry()])
            {
                lastReject = "out of aggro range while travelling and not needed for quest";
                continue;
            }
        }

        if (group)
        {
            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player* member = ObjectAccessor::FindPlayer(itr->guid);
                if (!member || !member->IsAlive())
                    continue;

                float d = member->GetDistance(unit);
                if (!result || d < distance)
                {
                    distance = d;
                    result = unit;
                }
            }
        }
        else
        {
            float newdistance = bot->GetDistance(unit);
            if (!result || (newdistance < distance))
            {
                distance = newdistance;
                result = unit;
            }
        }
    }

    debugReport();
    return result;
}

bool GrindTargetValue::needForQuest(Unit* target)
{
    // AC parity port: AC's needForQuest walked RequiredNpcOrGo[]/CreatureOrGOCount[] (WotLK quest
    // layout). MoP's Quest class replaced that with QuestObjective + per-slot storage indices, so
    // this is a QUEST_OBJECTIVE_MONSTER port of the same check, not a literal copy.
    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (auto& quest : questMap)
    {
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(quest.first);
        if (!questTemplate)
            continue;

        uint32 questId = questTemplate->GetQuestId();
        if (!questId)
            continue;

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
            continue;

        if (questTemplate->GetQuestLevel() > bot->GetLevel() + 5)
            continue;

        uint16 logSlot = bot->FindQuestSlot(questId);
        if (logSlot >= MAX_QUEST_LOG_SIZE)
            continue;

        for (QuestObjective const& objective : questTemplate->GetObjectives())
        {
            if (objective.Type != QUEST_OBJECTIVE_MONSTER)
                continue;

            if (uint32(objective.ObjectID) != target->GetEntry())
                continue;

            if (bot->GetQuestSlotCounter(logSlot, objective.StorageIndex) < objective.Amount)
                return true;
        }
    }

    if (CreatureTemplate const* data = sObjectMgr->GetCreatureTemplate(target->GetEntry()))
    {
        if (uint32 lootId = data->lootid)
        {
            if (LootTemplates_Creature.HaveQuestLootForPlayer(lootId, bot))
                return true;
        }
    }

    return false;
}

uint32 GrindTargetValue::GetTargetingPlayerCount(Unit* unit)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint32 count = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive() || member == bot)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
        if ((botAI && *botAI->GetAiObjectContext()->GetValue<Unit*>("current target") == unit) ||
            (!botAI && member->GetTarget() == unit->GetGUID()))
            ++count;
    }

    return count;
}
