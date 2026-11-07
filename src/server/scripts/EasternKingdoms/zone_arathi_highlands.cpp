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
SDComment: Quest support: 665
SDCategory: Arathi Highlands
EndScriptData */

/* ContentData
npc_professor_phizzlethorpe
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "Group.h"

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

void AddSC_arathi_highlands()
{
    new npc_professor_phizzlethorpe();
}
