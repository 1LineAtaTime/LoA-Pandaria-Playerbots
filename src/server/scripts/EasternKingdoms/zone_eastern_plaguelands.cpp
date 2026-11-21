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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742. Special vendor Augustus the Touched
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
npc_tirion_fordring
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Group.h"
#include "WorldSession.h"

/*######
## npc_tirion_fordring
######*/

#define GOSSIP_HELLO    "I am ready to hear your tale, Tirion."
#define GOSSIP_SELECT1  "Thank you, Tirion.  What of your identity?"
#define GOSSIP_SELECT2  "That is terrible."
#define GOSSIP_SELECT3  "I will, Tirion."

class npc_tirion_fordring : public CreatureScript
{
public:
    npc_tirion_fordring() : CreatureScript("npc_tirion_fordring") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(4493, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(4494, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(4495, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->CLOSE_GOSSIP_MENU();
                player->AreaExploredOrEventHappens(5742);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && player->GetStandState() == UNIT_STAND_STATE_SIT)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

enum
{
    TALK_DIED                  = 0,
    TALK_CREDIT_1              = 1,
    TALK_CREDIT_2              = 2,
    TALK_NONE                  = 0xFF,               // no line: the player is not on the quest

    EVENT_THROW_DYNAMITE       = 1,
    SPELL_THROW_DYNAMITE       = 86088,

    QUEST_BEAT_IT_OUT_OF_THEM  = 27522,

    OBJECTIVE_ENGINEERS        = 265859,
    OBJECTIVE_GIDVINS_LOCATION = 265860,
    OBJECTIVE_GIDVINS_CAPTOR   = 265861,

    NPC_CREDIT_1               = 45845,
    NPC_CREDIT_2               = 45846,
};

// Scourge Siege Engineer - 17878
struct npc_scourge_siege_engineer : public ScriptedAI
{
    npc_scourge_siege_engineer(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_THROW_DYNAMITE, 10000);
    }

    // What this death tells one player on Beat it Out of Them: Gidwin's location once he has beaten three engineers, his
    // captor once he has beaten six, else nothing (a fail line). His engineer count was already raised by this kill:
    // Unit::Kill rewards the kill credit before it calls JustDied.
    static uint8 GetOutcome(Player* player)
    {
        if (player->GetQuestStatus(QUEST_BEAT_IT_OUT_OF_THEM) != QUEST_STATUS_INCOMPLETE)
            return TALK_NONE;

        uint32 beaten = player->GetQuestObjectiveCounter(OBJECTIVE_ENGINEERS);
        if (beaten >= 3 && !player->GetQuestObjectiveCounter(OBJECTIVE_GIDVINS_LOCATION))
            return TALK_CREDIT_1;
        if (beaten >= 6 && !player->GetQuestObjectiveCounter(OBJECTIVE_GIDVINS_CAPTOR))
            return TALK_CREDIT_2;
        return TALK_DIED;
    }

    // Every player who got this kill's credit gets his own "Discover" credit: the same players Unit::Kill and KillRewarder
    // credit (the tapper, else the killer's owner, so a pet's or guardian's kill counts for its owner; with a group, every
    // member within reward distance who is alive or has not released). The engineer speaks one line: the tapper's credit
    // line, else the first credit line someone else gets, else a fail line.
    void JustDied(Unit* killer) override
    {
        if (!me->IsDamageEnoughForLootingAndReward())
            return;

        Player* tapper = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (Player* looter = me->GetLootRecipient())
            if (looter->IsInMap(me))
                tapper = looter;
        if (!tapper)
            return;

        std::vector<Player*> credited;
        if (Group* group = tapper->GetGroup())
        {
            for (Player* member : *group)
                if (member && member->IsInMap(me) && member->IsAtGroupRewardDistance(me) && (member->IsAlive() || !member->GetCorpse()))
                    credited.push_back(member);
        }
        else
            credited.push_back(tapper);

        uint8 line = TALK_NONE;
        uint8 lineRank = 0;                                 // 3 the tapper's credit, 2 someone else's credit, 1 a fail line
        Player* lineTarget = nullptr;
        for (Player* player : credited)
        {
            uint8 outcome = GetOutcome(player);
            if (outcome == TALK_NONE)
                continue;

            if (outcome == TALK_CREDIT_1)
                player->KilledMonsterCredit(NPC_CREDIT_1);
            else if (outcome == TALK_CREDIT_2)
                player->KilledMonsterCredit(NPC_CREDIT_2);

            uint8 rank = outcome == TALK_DIED ? 1 : (player == tapper ? 3 : 2);
            if (rank > lineRank)
            {
                line = outcome;
                lineRank = rank;
                lineTarget = player;
            }
        }

        if (lineTarget)
            Talk(line, lineTarget);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_THROW_DYNAMITE:
                    DoCastVictim(SPELL_THROW_DYNAMITE);
                    events.ScheduleEvent(EVENT_THROW_DYNAMITE, urand(5000, 7000));
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

void AddSC_eastern_plaguelands()
{
    new npc_tirion_fordring();
    new creature_script<npc_scourge_siege_engineer>("npc_scourge_siege_engineer");
}
