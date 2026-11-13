/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/* ScriptData
SDName: Arathi Highlands
SD%Complete: 100
SDComment: Quest support: 26050, 26628
SDCategory: Arathi Highlands
EndScriptData */

/* ContentData
npc_professor_phizzlethorpe
npc_shakes_obreen
npc_daggerspine_marauder
go_faldir_cove_cannon
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "Group.h"
#include "GameObjectAI.h"
#include "ObjectAccessor.h"

/*######
## npc_professor_phizzlethorpe
######*/

enum ProfessorPhizzlethorpe
{
    // Yells
    SAY_PROGRESS_1      = 0,
    SAY_PROGRESS_2      = 1,
    SAY_PROGRESS_3      = 2,
    EMOTE_PROGRESS_4    = 3,
    SAY_AGGRO           = 4,
    SAY_PROGRESS_5      = 5,
    SAY_PROGRESS_6      = 6,
    SAY_PROGRESS_7      = 7,
    EMOTE_PROGRESS_8    = 8,
    SAY_PROGRESS_9      = 9,

    EVENT_SAY_3         = 1,
    EVENT_SAY_6         = 2,
    EVENT_SAY_8         = 3,

    // Quests
    QUEST_SUNKEN_TREASURE   = 665,
    QUEST_GOGGLE_BOGGLE     = 26050,

    // Creatures
    NPC_VENGEFUL_SURGE      = 2776
};

class npc_professor_phizzlethorpe : public CreatureScript
{
    public:

        npc_professor_phizzlethorpe()
            : CreatureScript("npc_professor_phizzlethorpe")
        {
        }

        struct npc_professor_phizzlethorpeAI : public npc_escortAI
        {
            npc_professor_phizzlethorpeAI(Creature* creature) : npc_escortAI(creature), _playerCheckTimer(1000)
            {
                // The base despawns him silently when the players are gone; UpdateAI does it here and fails the quest
                SetDespawnAtFar(false);
            }

            // Runs at a respawn. An evade during the escort doesn't come here, so the escort's lines survive a fight.
            void Reset() override
            {
                events.Reset();
            }

            void WaypointReached(uint32 waypointId) override
            {
                Player* player = GetPlayerForEscort();
                if (!player)
                    return;

                switch (waypointId)
                {
                    case 6:
                        Talk(SAY_PROGRESS_2, player);
                        events.ScheduleEvent(EVENT_SAY_3, 3000);
                        break;
                    case 8:
                        Talk(EMOTE_PROGRESS_4);
                        me->SummonCreature(NPC_VENGEFUL_SURGE, -2065.505f, -2136.88f, 22.20362f, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0ms);
                        me->SummonCreature(NPC_VENGEFUL_SURGE, -2059.249f, -2134.88f, 21.51582f, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0ms);
                        break;
                    case 11:
                        Talk(SAY_PROGRESS_5, player);
                        events.ScheduleEvent(EVENT_SAY_6, 11000);
                        break;
                    case 17:
                        Talk(SAY_PROGRESS_7, player);
                        events.ScheduleEvent(EVENT_SAY_8, 6000);
                        break;
                }
            }

            void JustSummoned(Creature* summoned) override
            {
                summoned->AI()->AttackStart(me);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
            }

            // npc_escortAI::JustDied only fails the quest of a jumping escort
            void JustDied(Unit* /*killer*/) override
            {
                if (!HasEscortState(STATE_ESCORT_ESCORTING))
                    return;

                FailEscortQuest();
                events.Reset();
            }

            void UpdateAI(uint32 diff) override
            {
                if (HasEscortState(STATE_ESCORT_ESCORTING) && !HasEscortState(STATE_ESCORT_RETURNING) && !me->GetVictim())
                {
                    if (_playerCheckTimer <= diff)
                    {
                        _playerCheckTimer = 1000;

                        if (!IsEscortPlayerOrGroupNear())
                        {
                            FailEscortQuest();
                            events.Reset();
                            me->DespawnOrUnsummon();
                            return;
                        }
                    }
                    else
                        _playerCheckTimer -= diff;
                }

                // The escort AI below has to run without the player too: it fights.
                Player* player = HasEscortState(STATE_ESCORT_ESCORTING) ? GetPlayerForEscort() : nullptr;
                if (player)
                {
                    events.Update(diff);

                    while (uint32 event = events.ExecuteEvent())
                    {
                        switch (event)
                        {
                            case EVENT_SAY_3:
                                Talk(SAY_PROGRESS_3, player);
                                break;
                            case EVENT_SAY_6:
                                Talk(SAY_PROGRESS_6, player);
                                SetRun();
                                break;
                            case EVENT_SAY_8:
                                Talk(EMOTE_PROGRESS_8);
                                Talk(SAY_PROGRESS_9, player);
                                player->GroupEventHappens(QUEST_GOGGLE_BOGGLE, me);
                                break;
                        }
                    }
                }
                else
                    events.Reset();

                npc_escortAI::UpdateAI(diff);
            }

            EventMap events;

        private:
            // As npc_escortAI::IsPlayerOrGroupInRange, which is private
            bool IsEscortPlayerOrGroupNear()
            {
                Player* player = GetPlayerForEscort();
                if (!player)
                    return false;

                if (Group* group = player->GetGroup())
                {
                    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                        if (Player* member = itr->GetSource())
                            if (me->IsWithinDistInMap(member, GetMaxPlayerDistance()))
                                return true;

                    return false;
                }

                return me->IsWithinDistInMap(player, GetMaxPlayerDistance());
            }

            // The starter and their group, and whoever holds the quest nearby: the group of a starter who logged out can't be found any more.
            // A logged out starter can't be sent a fail: their quest stays incomplete, to abandon and retake.
            void FailEscortQuest()
            {
                if (Player* player = GetPlayerForEscort())
                {
                    if (Group* group = player->GetGroup())
                    {
                        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                            if (Player* member = itr->GetSource())
                                if (member->IsInMap(me))
                                    FailIfOnQuest(member);
                    }
                    else
                        FailIfOnQuest(player);
                }

                std::list<Player*> nearby;
                me->GetPlayerListInGrid(nearby, GetMaxPlayerDistance());
                for (Player* player : nearby)
                    FailIfOnQuest(player);
            }

            static void FailIfOnQuest(Player* player)
            {
                if (player->GetQuestStatus(QUEST_GOGGLE_BOGGLE) == QUEST_STATUS_INCOMPLETE)
                    player->FailQuest(QUEST_GOGGLE_BOGGLE);
            }

            uint32 _playerCheckTimer;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_professor_phizzlethorpeAI(creature);
        }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_GOGGLE_BOGGLE)
            {
                creature->AI()->Talk(SAY_PROGRESS_1, player);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_professor_phizzlethorpeAI, (creature->AI())))
                    pEscortAI->Start(false, false, player->GetGUID(), quest);

                creature->SetFaction(42);
            }
            return true;
        }
};

/*######
## Death From Below (26628)
##
## Sourced: the waves, positions, delays and lines are the action list 261000 of both TDBs and AzerothCore (there keyed to the old quest 667), the
## cannon is Cannon Ball 4170 (DBC: instant kill, 20 yd around a database position), which both TDBs cast from the cannon's use and limit to the
## marauders, its position is AzerothCore's, and failing on Shakes' death is theirs too. The Wowhead comments of 2010-2014 describe the same play.
## Ours: the player fires the cannon, not the object (the object's cast has faction 14, which can't target faction 74); the event ends when all
## marauders are gone after the sourced delay, at most 30 seconds later, where the sourced list gives the quest flag back; failing a player who dies
## or goes beyond 100 yd (the comments' "radius-based fail", the radius is the sourced credit range); stopping when nobody is left.
######*/

enum DeathFromBelow
{
    QUEST_DEATH_FROM_BELOW      = 26628,

    NPC_SHAKES_OBREEN           = 2610,
    NPC_DAGGERSPINE_MARAUDER    = 2775,

    SPELL_CANNON_BALL           = 4170,

    SAY_SHAKES_INCOMING         = 0,
    SAY_SHAKES_HOLD             = 1,
    SAY_MARAUDER_APPROACH       = 0,

    EVENT_DFB_INCOMING          = 1,
    EVENT_DFB_WAVE_1            = 2,
    EVENT_DFB_WAVE_2            = 3,
    EVENT_DFB_HOLD              = 4,
    EVENT_DFB_WAVE_3            = 5,
    EVENT_DFB_FINISH            = 6,
    EVENT_DFB_CHECK_PLAYERS     = 7,

    CANNON_ANIM                 = 1,
    CANNON_COOLDOWN             = 3000,     // the cannon's autoClose: it can't be used again sooner on the client either
    MARAUDER_CHECK_SHAKES       = 3000,
    DFB_MAX_FINISH_WAITS        = 30        // Seconds after the sourced credit time
};

float const DFB_RANGE = 100.0f;             // The range of the sourced quest credit
float const MARAUDER_ATTACK_SHAKES_RANGE = 30.0f;

Position const MarauderSpawns[4] =
{
    { -2158.64f, -1967.59f, 15.3479f, 5.52555f },
    { -2161.89f, -1968.63f, 15.6413f, 5.46271f },
    { -2158.25f, -1965.68f, 15.0634f, 5.60016f },
    { -2158.36f, -1971.42f, 15.5962f, 4.96721f }
};

Position const MarauderTarget = { -2086.07f, -2028.86f, 3.22088f, 2.67035f };

struct npc_shakes_obreen : public ScriptedAI
{
    npc_shakes_obreen(Creature* creature) : ScriptedAI(creature), _running(false), _finishWaits(0) { }

    // Reset also runs at an evade: the event goes on through it

    void JustAppeared() override
    {
        ScriptedAI::JustAppeared();

        if (!_running)
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_DEATH_FROM_BELOW)
            return;

        _players.insert(player->GetGUID());

        if (_running)
            return;

        _running = true;
        _finishWaits = 0;
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        _events.Reset();
        _events.ScheduleEvent(EVENT_DFB_INCOMING, 1000);
        _events.ScheduleEvent(EVENT_DFB_WAVE_1, 10000);
        _events.ScheduleEvent(EVENT_DFB_WAVE_2, 30000);
        _events.ScheduleEvent(EVENT_DFB_HOLD, 33000);
        _events.ScheduleEvent(EVENT_DFB_WAVE_3, 48000);
        _events.ScheduleEvent(EVENT_DFB_FINISH, 78000);
        _events.ScheduleEvent(EVENT_DFB_CHECK_PLAYERS, 1000);
    }

    void JustSummoned(Creature* summon) override
    {
        _marauders.push_back(summon->GetGUID());
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!_running)
            return;

        for (ObjectGuid const& guid : _players)
            if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                if (player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) == QUEST_STATUS_INCOMPLETE)
                    player->FailQuest(QUEST_DEATH_FROM_BELOW);

        std::list<Player*> nearby;
        me->GetPlayerListInGrid(nearby, DFB_RANGE);
        for (Player* player : nearby)
            if (player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(QUEST_DEATH_FROM_BELOW);

        StopEvent();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DFB_INCOMING:
                    Talk(SAY_SHAKES_INCOMING);
                    break;
                case EVENT_DFB_WAVE_1:
                    SummonWave(4);
                    break;
                case EVENT_DFB_WAVE_2:
                    SummonWave(3);
                    break;
                case EVENT_DFB_HOLD:
                    Talk(SAY_SHAKES_HOLD);
                    break;
                case EVENT_DFB_WAVE_3:
                    SummonWave(2);
                    break;
                case EVENT_DFB_FINISH:
                    // A marauder's 60 second timer only runs out of combat, so the wait has its own limit
                    if (AnyMarauderAlive() && _finishWaits < DFB_MAX_FINISH_WAITS)
                    {
                        ++_finishWaits;
                        _events.ScheduleEvent(EVENT_DFB_FINISH, 1000);
                    }
                    else
                        FinishEvent();
                    break;
                case EVENT_DFB_CHECK_PLAYERS:
                    if (CheckPlayers())
                        _events.ScheduleEvent(EVENT_DFB_CHECK_PLAYERS, 1000);
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    void SummonWave(uint8 count)
    {
        for (uint8 i = 0; i < count; ++i)
            me->SummonCreature(NPC_DAGGERSPINE_MARAUDER, MarauderSpawns[i], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000ms);
    }

    bool AnyMarauderAlive() const
    {
        for (ObjectGuid const& guid : _marauders)
            if (Creature* marauder = ObjectAccessor::GetCreature(*me, guid))
                if (marauder->IsAlive())
                    return true;

        return false;
    }

    // Abandoned and logged out players just drop out; those who died or left fail. Returns false once the event has stopped for lack of players.
    // A group member who took the quest through the party prompt has no accept hook here: anyone in range with the quest joins this way.
    bool CheckPlayers()
    {
        std::list<Player*> nearby;
        me->GetPlayerListInGrid(nearby, DFB_RANGE);
        for (Player* player : nearby)
            if (player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) == QUEST_STATUS_INCOMPLETE)
                _players.insert(player->GetGUID());

        for (GuidSet::iterator itr = _players.begin(); itr != _players.end();)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, *itr);
            if (!player || player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) != QUEST_STATUS_INCOMPLETE)
            {
                itr = _players.erase(itr);
                continue;
            }

            if (!player->IsAlive() || !me->IsWithinDistInMap(player, DFB_RANGE))
            {
                player->FailQuest(QUEST_DEATH_FROM_BELOW);
                itr = _players.erase(itr);
                continue;
            }

            ++itr;
        }

        if (!_players.empty())
            return true;

        StopEvent();
        return false;
    }

    // As the sourced credit: everyone in range with the quest
    void FinishEvent()
    {
        std::list<Player*> nearby;
        me->GetPlayerListInGrid(nearby, DFB_RANGE);
        for (Player* player : nearby)
            if (player->IsAlive() && player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(QUEST_DEATH_FROM_BELOW);

        StopEvent();
    }

    void StopEvent()
    {
        _events.Reset();
        _running = false;
        _players.clear();

        for (ObjectGuid const& guid : _marauders)
            if (Creature* marauder = ObjectAccessor::GetCreature(*me, guid))
                marauder->DespawnOrUnsummon();
        _marauders.clear();

        if (me->IsAlive())
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    EventMap _events;
    GuidSet _players;
    GuidVector _marauders;
    bool _running;
    uint8 _finishWaits;
};

struct npc_daggerspine_marauder : public ScriptedAI
{
    npc_daggerspine_marauder(Creature* creature) : ScriptedAI(creature), _checkTimer(MARAUDER_CHECK_SHAKES) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _shakes = summoner->GetGUID();

        if (roll_chance_i(30))
            Talk(SAY_MARAUDER_APPROACH);

        // A creature won't fight beyond the threat radius of its home, and Shakes is farther than that from the spawn. An evade also leads back to him.
        me->SetHomePosition(MarauderTarget);
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(0, MarauderTarget);
    }

    // Players as any aggressive mob; of the NPCs only Shakes, and that below
    void MoveInLineOfSight(Unit* who) override
    {
        if (who->IsControlledByPlayer())
            ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim())
        {
            DoMeleeAttackIfReady();
            return;
        }

        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }

        _checkTimer = MARAUDER_CHECK_SHAKES;

        if (me->IsInEvadeMode())
            return;

        if (Creature* shakes = ObjectAccessor::GetCreature(*me, _shakes))
            if (shakes->IsAlive() && me->IsWithinDistInMap(shakes, MARAUDER_ATTACK_SHAKES_RANGE))
                AttackStart(shakes);
    }

private:
    ObjectGuid _shakes;
    uint32 _checkTimer;
};

// The sparkle and the click need the quest in the log: flag 4 and data1 of the template
struct go_faldir_cove_cannon : public GameObjectAI
{
    go_faldir_cove_cannon(GameObject* go) : GameObjectAI(go), _cooldown(0) { }

    bool OnGossipHello(Player* player, bool isUse) override
    {
        // The client's report of the same click comes with isUse false
        if (!isUse || _cooldown || player->GetQuestStatus(QUEST_DEATH_FROM_BELOW) != QUEST_STATUS_INCOMPLETE)
            return false;

        _cooldown = CANNON_COOLDOWN;
        me->SendCustomAnim(CANNON_ANIM);
        player->CastSpell(player, SPELL_CANNON_BALL, true);
        return false;
    }

    void UpdateAI(uint32 diff) override
    {
        _cooldown = _cooldown > diff ? _cooldown - diff : 0;
    }

private:
    uint32 _cooldown;
};

void AddSC_arathi_highlands()
{
    new npc_professor_phizzlethorpe();
    RegisterCreatureAI(npc_shakes_obreen);
    RegisterCreatureAI(npc_daggerspine_marauder);
    RegisterGameObjectAI(go_faldir_cove_cannon);
}
