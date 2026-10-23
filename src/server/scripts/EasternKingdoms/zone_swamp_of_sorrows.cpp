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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "GameObject.h"
#include "ObjectAccessor.h"
#include "Player.h"

/*######
## npc_galen_goodward
######*/

enum Galen
{
    QUEST_GALENS_ESCAPE     = 1393,
    GO_GALENS_CAGE          = 37118,
    SAY_PERIODIC            = 0,
    SAY_QUEST_ACCEPTED      = 1,
    SAY_ATTACKED            = 2,
    SAY_QUEST_COMPLETE      = 3,
    EMOTE_WHISPER           = 4,
    EMOTE_DISAPPEAR         = 5
};

class npc_galen_goodward : public CreatureScript
{
public:
    npc_galen_goodward() : CreatureScript("npc_galen_goodward") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_GALENS_ESCAPE)
        {
            creature->SetFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
            creature->AI()->Talk(SAY_QUEST_ACCEPTED, player);
            if (npc_galen_goodwardAI* pEscortAI = CAST_AI(npc_galen_goodward::npc_galen_goodwardAI, creature->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), quest);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_galen_goodwardAI(creature);
    }

    struct npc_galen_goodwardAI : public npc_escortAI
    {
        npc_galen_goodwardAI(Creature* creature) : npc_escortAI(creature)
        {
            galensCageGUID.Clear();
            periodicSay = 6000;
        }

        void Reset() override
        {
            periodicSay = 6000;
        }

        void JustEngagedWith(Unit* who) override
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
                Talk(SAY_ATTACKED, who);
        }

        void WaypointStart(uint32 pointId) override
        {
            switch (pointId)
            {
                case 0:
                {
                    GameObject* cage = nullptr;
                    if (galensCageGUID)
                        cage = ObjectAccessor::GetGameObject(*me, galensCageGUID);
                    else
                        cage = me->FindNearestGameObject(GO_GALENS_CAGE, INTERACTION_DISTANCE);
                    if (cage)
                    {
                        cage->UseDoorOrButton();
                        galensCageGUID = cage->GetGUID();
                    }
                    break;
                }
                case 21:
                    Talk(EMOTE_DISAPPEAR);
                    break;
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 0:
                    if (GameObject* cage = ObjectAccessor::GetGameObject(*me, galensCageGUID))
                        cage->ResetDoorOrButton();
                    break;
                case 20:
                    if (Player* player = GetPlayerForEscort())
                    {
                        me->SetFacingToObject(player);
                        Talk(SAY_QUEST_COMPLETE, player);
                        Talk(EMOTE_WHISPER, player);
                        player->GroupEventHappens(QUEST_GALENS_ESCAPE, me);
                    }
                    SetRun(true);
                    break;
            }
        }

        void UpdateEscortAI(uint32 diff) override
        {
            if (periodicSay < diff)
            {
                if (!HasEscortState(STATE_ESCORT_ESCORTING))
                    Talk(SAY_PERIODIC);
                periodicSay = 15000;
            }
            else
                periodicSay -= diff;

            npc_escortAI::UpdateEscortAI(diff);
        }

    private:
        ObjectGuid galensCageGUID;
        uint32 periodicSay;
    };
};

void AddSC_swamp_of_sorrows()
{
    new npc_galen_goodward();
}
