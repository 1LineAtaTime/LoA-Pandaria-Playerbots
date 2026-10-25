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
SDName: Silverpine_Forest
SD%Complete: 100
SDComment: Quest support: 435
SDCategory: Silverpine Forest
EndScriptData */

/* ContentData
npc_deathstalker_erland
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "SpellScript.h"
#include "Vehicle.h"
#include "CombatAI.h"
#include "GameObjectAI.h"
#include "ObjectMgr.h"

/*######
## npc_deathstalker_erland
######*/

enum eErland
{
    SAY_QUESTACCEPT     = 0,
    SAY_START           = 1,
    SAY_AGGRO           = 2,
    SAY_PROGRESS        = 3,
    SAY_LAST            = 4,

    SAY_RANE            = 0,
    SAY_RANE_ANSWER     = 5,
    SAY_MOVE_QUINN      = 6,

    SAY_QUINN           = 7,
    SAY_QUINN_ANSWER    = 0,
    SAY_BYE             = 8,

    QUEST_ESCORTING     = 435,
    NPC_RANE            = 1950,
    NPC_QUINN           = 1951
};

class npc_deathstalker_erland : public CreatureScript
{
public:
    npc_deathstalker_erland() : CreatureScript("npc_deathstalker_erland") { }

    struct npc_deathstalker_erlandAI : public npc_escortAI
    {
        npc_deathstalker_erlandAI(Creature* creature) : npc_escortAI(creature) { }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    Talk(SAY_START, player);
                    break;
                case 10:
                    Talk(SAY_PROGRESS);
                    break;
                case 13:
                    Talk(SAY_LAST, player);
                    player->GroupEventHappens(QUEST_ESCORTING, me);
                    break;
                case 15:
                    if (Creature* rane = me->FindNearestCreature(NPC_RANE, 20.0f))
                        rane->AI()->Talk(SAY_RANE);
                    break;
                case 16:
                    Talk(SAY_RANE_ANSWER);
                    break;
                case 17:
                    Talk(SAY_MOVE_QUINN);
                    break;
                case 24:
                    Talk(SAY_QUINN);
                    break;
                case 25:
                    if (Creature* quinn = me->FindNearestCreature(NPC_QUINN, 20.0f))
                        quinn->AI()->Talk(SAY_QUINN_ANSWER);
                    break;
                case 26:
                    Talk(SAY_BYE);
                    break;
            }
        }

        void Reset() override { }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_AGGRO, who);
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_ESCORTING)
        {
            creature->AI()->Talk(SAY_QUESTACCEPT, player);

            if (npc_escortAI* pEscortAI = CAST_AI(npc_deathstalker_erland::npc_deathstalker_erlandAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_deathstalker_erlandAI(creature);
    }
};

enum FenwickThatros
{
    SAY_FENWICK_AGGRO          = 0,

    SPELL_DRINK_HEALING_POTION = 54633,
    SPELL_POTION_OF_STRENGTH   = 80471,
    SPELL_ELIXIR_OF_AGILITY    = 80472,
    SPELL_TONIC_OF_HEALTH      = 80473,

    EVENT_BUFF_STRENGTH        = 1,
    EVENT_BUFF_AGILITY         = 2,
    EVENT_BUFF_STAMINA         = 3,
    EVENT_HEALING              = 4
};

struct npc_fenwick_thatros : public ScriptedAI
{
    npc_fenwick_thatros(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        half_hp_reached_once = false;
        events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_FENWICK_AGGRO);
        events.ScheduleEvent(EVENT_BUFF_STRENGTH, 3000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override
    {
        if (!half_hp_reached_once && !HealthAbovePct(50))
        {
            half_hp_reached_once = true;
            me->CastSpell(me, SPELL_DRINK_HEALING_POTION);
            events.ScheduleEvent(EVENT_HEALING, 10000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BUFF_STRENGTH:
                    me->CastSpell(me, SPELL_POTION_OF_STRENGTH);
                    events.ScheduleEvent(EVENT_BUFF_AGILITY, 3000);
                    break;
                case EVENT_BUFF_AGILITY:
                    me->CastSpell(me, SPELL_ELIXIR_OF_AGILITY);
                    events.ScheduleEvent(EVENT_BUFF_STAMINA, 3000);
                    break;
                case EVENT_BUFF_STAMINA:
                    me->CastSpell(me, SPELL_TONIC_OF_HEALTH);
                    break;
                case EVENT_HEALING:
                    if (!HealthAbovePct(50))
                    {
                        me->CastSpell(me, SPELL_DRINK_HEALING_POTION);
                        events.ScheduleEvent(EVENT_HEALING, 10000);
                    }
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
private:
    EventMap events;
    bool half_hp_reached_once = false;
};

/*######
## Quest The Warchief Cometh (26965)
######*/

Position const OrgrimmarPortalPos[3] =
{
    { 1358.62f, 1054.72f, 53.1200f, 0.0f },
    { 1393.27f, 1021.20f, 53.2225f, 0.0f },
    { 1404.71f, 1063.73f, 60.5617f, 0.0f }
};

Position const HellscreamElitePos[16] =
{
    { 1387.90f, 1029.71f, 53.21853f, 2.827433f },
    { 1389.79f, 1024.51f, 53.20833f, 2.827433f },
    { 1388.05f, 1026.91f, 53.20833f, 2.827433f },
    { 1388.16f, 1020.88f, 53.25523f, 2.827433f },
    { 1385.79f, 1025.99f, 53.22593f, 2.827433f },
    { 1386.69f, 1023.26f, 53.24393f, 2.827433f },
    { 1384.33f, 1022.04f, 53.28123f, 2.827433f },
    { 1391.10f, 1027.73f, 53.20483f, 2.827433f },

    { 1359.10f, 1046.55f, 52.97053f, 5.253441f },
    { 1360.89f, 1051.81f, 53.19793f, 5.253441f },
    { 1360.75f, 1048.84f, 53.12893f, 5.253441f },
    { 1364.43f, 1053.62f, 53.29343f, 5.253441f },
    { 1363.08f, 1048.15f, 53.22223f, 5.253441f },
    { 1364.08f, 1050.84f, 53.29163f, 5.253441f },
    { 1366.69f, 1050.31f, 53.34203f, 5.253441f },
    { 1357.85f, 1050.12f, 52.99823f, 5.253441f }
};

Position const GarroshPos        = { 1402.45f, 1061.62f, 60.56173f, 3.926991f };
Position const GarroshJumpPos    = { 1378.65f, 1044.23f, 53.8389f, 5.51524f };
Position const CromushPos        = { 1404.71f, 1063.73f, 60.5617f, 2.827433f };
Position const AgathaPreRisePos  = { 1364.02f, 1028.54f, 66.99143f };
Position const AgathaRisePos     = { 1368.65f, 1032.19f, 63.3033f };
Position const AgathaPreResetPos = { 1364.02f, 1028.54f, 55.9914f };
Position const AgathaResetPos    = { 1364.02f, 1028.54f, 58.1319f };

enum QuestTheWarchiefCometh
{
    QUEST_THE_WARCHIEF_COMETH               = 26965,

    NPC_LADY_SYLVANAS_WINDRUNNER_COMETH     = 44365,
    NPC_AGATHA_COMETH                       = 44608,
    NPC_PORTAL_FROM_ORGRIMMAR               = 44630,
    NPC_GARROSH_HELLSCREAM                  = 44629,
    NPC_HIGH_WARLORD_CROMUSH_COMETH         = 44640,
    NPC_HELLSCREAM_ELITE_COMETH             = 44636,

    SPELL_RAISE_FORSAKEN_COMETH             = 83173,
    SPELL_AIR_REVENANT_ENTRANCE             = 55761,
    SPELL_SIMPLE_TELEPORT                   = 12980,
    SPELL_WELCOME_TO_SILVERPINE_CREDIT      = 83384,

    EVENT_START_SCENE_COMETH                = 1,
    EVENT_SUMMON_PORTAL_COMETH              = 2,
    EVENT_SUMMON_GARROSH_COMETH             = 3,
    EVENT_AGATHA_RAISE_FORSAKEN             = 4, // 4-8
    EVENT_SCENE_TALK_COMETH                 = 9, // 9-36

    TALK_SYLVANAS_COMETH_0                  = 0,
    TALK_SYLVANAS_COMETH_1                  = 1,
    TALK_SYLVANAS_COMETH_2                  = 2,
    TALK_SYLVANAS_COMETH_3                  = 3,
    TALK_SYLVANAS_COMETH_4                  = 4,
    TALK_SYLVANAS_COMETH_5                  = 5,
    TALK_SYLVANAS_COMETH_6                  = 6,
    TALK_SYLVANAS_COMETH_7                  = 7,
    TALK_SYLVANAS_COMETH_8                  = 8,
    TALK_SYLVANAS_COMETH_9                  = 9,
    TALK_SYLVANAS_COMETH_10                 = 10,
    TALK_GARROSH_COMETH_0                   = 0,
    TALK_GARROSH_COMETH_1                   = 1,
    TALK_GARROSH_COMETH_2                   = 2,
    TALK_GARROSH_COMETH_3                   = 3,
    TALK_GARROSH_COMETH_4                   = 4,
    TALK_GARROSH_COMETH_5                   = 5,
    TALK_GARROSH_COMETH_6                   = 6,
    TALK_GARROSH_COMETH_7                   = 7,
    TALK_GARROSH_COMETH_8                   = 8,
    TALK_CROMUSH_COMETH_0                   = 0,
    TALK_CROMUSH_COMETH_1                   = 1,

    PATH_GARROSH                            = 446290,
    PATH_CROMUSH                            = 446402,

    POINT_AGATHA_PRE_RISE                   = 1,
    POINT_AGATHA_RISE                       = 2,
    POINT_AGATHA_PRE_RESET                  = 3,
    POINT_AGATHA_RESET                      = 4,

    ANIMKIT_GENERAL_1                       = 609,
    ANIMKIT_SYLV_1                          = 595,
    ANIMKIT_SYLV_2                          = 606,
    ANIMKIT_GARROSH_1                       = 662,
    ANIMKIT_GARROSH_2                       = 595
};

// 44615 - Grand Executor Mortuus
// Accepting The Warchief Cometh plays the scene: Garrosh comes through portals from Orgrimmar, argues with Sylvanas while Agatha
// raises the fallen humans, and his Welcome to Silverpine Credit completes the quest for everyone nearby. Ported from TrinityCore.
struct npc_silverpine_grand_executor_mortuus : public ScriptedAI
{
    npc_silverpine_grand_executor_mortuus(Creature* creature) : ScriptedAI(creature), _summons(me), _eventInProgress(false) { }

    void OnQuestAccept(Player* /*player*/, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_THE_WARCHIEF_COMETH)
            StartScene();
    }

    // Someone who accepted while the scene was already past Garrosh's credit, or who left before it, gets it played again
    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_THE_WARCHIEF_COMETH) == QUEST_STATUS_INCOMPLETE)
            StartScene();
        return false;
    }

    void StartScene()
    {
        if (_eventInProgress)
            return;

        Creature* sylvanas = me->FindNearestCreature(NPC_LADY_SYLVANAS_WINDRUNNER_COMETH, 100.0f);
        Creature* agatha = me->FindNearestCreature(NPC_AGATHA_COMETH, 100.0f);
        if (!sylvanas || !agatha)
            return;

        _eventInProgress = true;
        _sylvanasGUID = sylvanas->GetGUID();
        _agathaGUID = agatha->GetGUID();
        _events.ScheduleEvent(EVENT_START_SCENE_COMETH, 250ms);
    }

    void Reset() override
    {
        _events.Reset();
        _garroshGUID.Clear();
        _cromushGUID.Clear();
        _sylvanasGUID.Clear();
        _agathaGUID.Clear();
        _summons.DespawnAll();
        _eventInProgress = false;
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_GARROSH_HELLSCREAM:
                summon->CastSpell(summon, SPELL_SIMPLE_TELEPORT, false);
                _garroshGUID = summon->GetGUID();
                break;
            case NPC_HELLSCREAM_ELITE_COMETH:
                summon->CastSpell(summon, SPELL_SIMPLE_TELEPORT, false);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Creature* sylvanas = ObjectAccessor::GetCreature(*me, _sylvanasGUID);
            Creature* garrosh = ObjectAccessor::GetCreature(*me, _garroshGUID);
            Creature* cromush = ObjectAccessor::GetCreature(*me, _cromushGUID);
            Creature* agatha = ObjectAccessor::GetCreature(*me, _agathaGUID);

            switch (eventId)
            {
                case EVENT_START_SCENE_COMETH:
                    _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH, 1s);
                    _events.ScheduleEvent(EVENT_SUMMON_PORTAL_COMETH, 4s);
                    _events.ScheduleEvent(EVENT_SUMMON_GARROSH_COMETH, 7s + 500ms);
                    break;
                case EVENT_SUMMON_PORTAL_COMETH:
                    SummonPortalsFromOrgrimmar();
                    break;
                case EVENT_SUMMON_GARROSH_COMETH:
                    SummonGarroshAndHisEliteGuards();
                    break;
                case EVENT_SCENE_TALK_COMETH:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_0);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 1, 4s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 1:
                    if (sylvanas)
                    {
                        sylvanas->SetFacingTo(0.808979f);
                        Say(sylvanas, TALK_SYLVANAS_COMETH_1);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 2, 3s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 2:
                    if (garrosh)
                    {
                        garrosh->GetMotionMaster()->MoveJump(GarroshJumpPos, 15.595897f, 15.595897f);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 3, 2s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 3:
                    if (sylvanas)
                    {
                        sylvanas->SetFacingTo(3.924652f);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 4, 1s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 4:
                    if (garrosh)
                    {
                        if (sylvanas)
                            garrosh->SetFacingToObject(sylvanas);
                        garrosh->PlayOneShotAnimKitId(ANIMKIT_GARROSH_1);
                        Say(garrosh, TALK_GARROSH_COMETH_0);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 5, 12s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 5:
                    if (garrosh)
                    {
                        garrosh->SetFacingTo(3.9444442f);
                        Say(garrosh, TALK_GARROSH_COMETH_1);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 6, 7s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 6:
                    if (sylvanas)
                    {
                        sylvanas->SetFacingTo(2.4260077f);
                        Say(sylvanas, TALK_SYLVANAS_COMETH_2);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 7, 5s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 7:
                    if (sylvanas)
                    {
                        sylvanas->SetFacingTo(3.7350047f);
                        sylvanas->PlayOneShotAnimKitId(ANIMKIT_SYLV_1);
                        Say(sylvanas, TALK_SYLVANAS_COMETH_3);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 8, 16s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 8:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_4);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 9, 4s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 9:
                    if (garrosh)
                    {
                        Say(garrosh, TALK_GARROSH_COMETH_2);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 10, 3s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 10:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_5);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 11, 6s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 11:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_6);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 12, 6s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 12:
                    if (sylvanas)
                    {
                        sylvanas->PlayOneShotAnimKitId(ANIMKIT_SYLV_2);
                        Say(sylvanas, TALK_SYLVANAS_COMETH_7);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 13, 9s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 13:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_8);
                        _events.ScheduleEvent(EVENT_AGATHA_RAISE_FORSAKEN, 3s);
                    }
                    break;
                case EVENT_AGATHA_RAISE_FORSAKEN:
                    if (agatha)
                    {
                        agatha->GetMotionMaster()->MovePoint(POINT_AGATHA_PRE_RISE, AgathaPreRisePos, false);
                        _events.ScheduleEvent(EVENT_AGATHA_RAISE_FORSAKEN + 1, 2s + 500ms);
                    }
                    break;
                case EVENT_AGATHA_RAISE_FORSAKEN + 1:
                    if (agatha)
                    {
                        agatha->SetWalk(true);
                        agatha->GetMotionMaster()->MovePoint(POINT_AGATHA_RISE, AgathaRisePos, false);
                        _events.ScheduleEvent(EVENT_AGATHA_RAISE_FORSAKEN + 2, 6s);
                    }
                    break;
                case EVENT_AGATHA_RAISE_FORSAKEN + 2:
                    if (agatha)
                    {
                        agatha->CastSpell(agatha, SPELL_RAISE_FORSAKEN_COMETH, false);
                        _events.ScheduleEvent(EVENT_AGATHA_RAISE_FORSAKEN + 3, 10s);
                    }
                    break;
                case EVENT_AGATHA_RAISE_FORSAKEN + 3:
                    if (agatha)
                    {
                        agatha->GetMotionMaster()->MovePoint(POINT_AGATHA_PRE_RESET, AgathaPreResetPos, false);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 14, 750ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 14:
                    if (cromush)
                    {
                        Say(cromush, TALK_CROMUSH_COMETH_0);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 15, 3s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 15:
                    if (garrosh)
                    {
                        garrosh->PlayOneShotAnimKitId(ANIMKIT_GENERAL_1);
                        Say(garrosh, TALK_GARROSH_COMETH_3);
                        if (agatha)
                            agatha->SetWalk(false);
                        _events.ScheduleEvent(EVENT_AGATHA_RAISE_FORSAKEN + 4, 12s);
                    }
                    break;
                case EVENT_AGATHA_RAISE_FORSAKEN + 4:
                    if (agatha)
                    {
                        agatha->GetMotionMaster()->MovePoint(POINT_AGATHA_RESET, AgathaResetPos, false);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 16, 1s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 16:
                    if (sylvanas)
                    {
                        sylvanas->PlayOneShotAnimKitId(ANIMKIT_GENERAL_1);
                        Say(sylvanas, TALK_SYLVANAS_COMETH_9);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 17, 10s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 17:
                    if (garrosh)
                    {
                        Say(garrosh, TALK_GARROSH_COMETH_4);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 18, 6s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 18:
                    if (garrosh)
                    {
                        Say(garrosh, TALK_GARROSH_COMETH_5);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 19, 6s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 19:
                    if (sylvanas)
                    {
                        Say(sylvanas, TALK_SYLVANAS_COMETH_10);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 20, 5s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 20:
                    if (garrosh)
                    {
                        garrosh->SetFacingTo(5.51524f);
                        Say(garrosh, TALK_GARROSH_COMETH_6);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 21, 4s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 21:
                    if (garrosh && cromush)
                    {
                        garrosh->SetFacingToObject(cromush);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 22, 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 22:
                    if (garrosh)
                    {
                        garrosh->PlayOneShotAnimKitId(ANIMKIT_GARROSH_2);
                        Say(garrosh, TALK_GARROSH_COMETH_7);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 23, 14s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 23:
                    if (cromush && garrosh)
                    {
                        Say(cromush, TALK_CROMUSH_COMETH_1);
                        cromush->SetFacingToObject(garrosh);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 24, 2s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 24:
                    if (garrosh)
                    {
                        garrosh->SetFacingTo(5.6199603f);
                        garrosh->PlayOneShotAnimKitId(ANIMKIT_GARROSH_2);
                        Say(garrosh, TALK_GARROSH_COMETH_8);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 25, 8s + 500ms);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 25:
                    if (garrosh)
                    {
                        garrosh->CastSpell(garrosh, SPELL_WELCOME_TO_SILVERPINE_CREDIT, true);
                        garrosh->GetMotionMaster()->MovePath(PATH_GARROSH, false);
                        _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 26, 9s);
                    }
                    break;
                case EVENT_SCENE_TALK_COMETH + 26:
                    for (ObjectGuid const& summonGUID : _summons)
                        if (Creature* summon = ObjectAccessor::GetCreature(*me, summonGUID))
                            summon->CastSpell(summon, SPELL_SIMPLE_TELEPORT, false);
                    _events.ScheduleEvent(EVENT_SCENE_TALK_COMETH + 27, 500ms);
                    break;
                case EVENT_SCENE_TALK_COMETH + 27:
                    if (agatha)
                        agatha->SetFacingTo(0.7155f);
                    _summons.DespawnAll();
                    Reset();
                    break;
                default:
                    break;
            }
        }

        // A step whose actor is gone (a GM despawned it) leaves the scene stuck; end it so the next quest accept starts it again
        if (_eventInProgress && _events.Empty())
        {
            _summons.DespawnAll();
            Reset();
        }
    }

private:
    void Say(Creature* speaker, uint8 group)
    {
        if (speaker->IsAIEnabled)
            speaker->AI()->Talk(group);
    }

    void SummonPortalsFromOrgrimmar()
    {
        for (Position const& pos : OrgrimmarPortalPos)
            if (Creature* portal = me->SummonCreature(NPC_PORTAL_FROM_ORGRIMMAR, pos, TEMPSUMMON_TIMED_DESPAWN, 300s))
                portal->CastSpell(portal, SPELL_AIR_REVENANT_ENTRANCE, false);
    }

    void SummonGarroshAndHisEliteGuards()
    {
        for (Position const& pos : HellscreamElitePos)
            me->SummonCreature(NPC_HELLSCREAM_ELITE_COMETH, pos, TEMPSUMMON_TIMED_DESPAWN, 300s);

        me->SummonCreature(NPC_GARROSH_HELLSCREAM, GarroshPos, TEMPSUMMON_TIMED_DESPAWN, 300s);

        if (Creature* cromush = me->SummonCreature(NPC_HIGH_WARLORD_CROMUSH_COMETH, CromushPos, TEMPSUMMON_TIMED_DESPAWN, 300s))
        {
            _cromushGUID = cromush->GetGUID();
            cromush->RemoveNpcFlag(UNIT_NPC_FLAG_QUESTGIVER);
            cromush->GetMotionMaster()->MovePath(PATH_CROMUSH, false);
        }
    }

    EventMap _events;
    ObjectGuid _garroshGUID;
    ObjectGuid _cromushGUID;
    ObjectGuid _sylvanasGUID;
    ObjectGuid _agathaGUID;
    SummonList _summons;
    bool _eventInProgress;
};

enum RaiseForsakenCometh
{
    ACTION_RISE_DURING_RAISE                = 1,
    ACTION_DESCEND_AFTER_RAISE              = 2,

    POINT_BEING_RISEN                       = 1,

    ANIMKIT_RESET                           = 0,
    ANIMKIT_FALLEN_HUMAN                    = 721
};

// 83173 - Raise Forsaken
class spell_silverpine_raise_forsaken_83173 : public AuraScript
{
    PrepareAuraScript(spell_silverpine_raise_forsaken_83173);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* fallenHuman = GetTarget()->ToCreature())
            if (fallenHuman->IsAIEnabled)
                fallenHuman->AI()->DoAction(ACTION_RISE_DURING_RAISE);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* fallenHuman = GetTarget()->ToCreature())
            if (fallenHuman->IsAIEnabled)
                fallenHuman->AI()->DoAction(ACTION_DESCEND_AFTER_RAISE);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_silverpine_raise_forsaken_83173::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_silverpine_raise_forsaken_83173::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

enum FallenHuman
{
    SPELL_FEIGNED                           = 80636,
    SPELL_FORSAKEN_TROOPER_MS_COMETH        = 83149,

    EVENT_ASCEND                            = 1,
    EVENT_TRANSFORM_INTO_FORSAKEN           = 2,
    EVENT_FACE_TOWARDS_SYLVANAS             = 3,
    EVENT_EMOTE_TO_SYLVANAS                 = 4
};

// 44592, 44593 - Fallen Human
struct npc_silverpine_fallen_human : public ScriptedAI
{
    npc_silverpine_fallen_human(Creature* creature) : ScriptedAI(creature), _transformDone(false) { }

    void Reset() override
    {
        _transformDone = false;
        _events.Reset();
        // Lies dead (the sniffed feign death flag) until raised into a trooper
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_RISE_DURING_RAISE:
                me->SetAIAnimKitId(ANIMKIT_FALLEN_HUMAN);
                _events.ScheduleEvent(EVENT_ASCEND, 1s);
                break;
            case ACTION_DESCEND_AFTER_RAISE:
                me->SetWalk(false);
                me->SetAIAnimKitId(ANIMKIT_RESET);
                me->GetMotionMaster()->MoveFall();
                _events.ScheduleEvent(EVENT_TRANSFORM_INTO_FORSAKEN, 1s);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ASCEND:
                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(POINT_BEING_RISEN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 3.5f, false);
                    break;
                case EVENT_TRANSFORM_INTO_FORSAKEN:
                    if (_transformDone)
                        return;
                    DoCastSelf(SPELL_FORSAKEN_TROOPER_MS_COMETH);
                    _transformDone = true;
                    _events.ScheduleEvent(EVENT_FACE_TOWARDS_SYLVANAS, 1s + 500ms);
                    break;
                case EVENT_FACE_TOWARDS_SYLVANAS:
                    me->SetFacingTo(0.706837f);
                    _events.ScheduleEvent(EVENT_EMOTE_TO_SYLVANAS, 2s + 500ms);
                    break;
                case EVENT_EMOTE_TO_SYLVANAS:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    me->DespawnOrUnsummon(80 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    bool _transformDone;
};

enum SpellForsakenTrooperMasterScriptCometh
{
    SPELL_FORSAKEN_TROOPER_MALE_01_HC       = 83150,
    SPELL_FORSAKEN_TROOPER_MALE_02_HC       = 83163,
    SPELL_FORSAKEN_TROOPER_MALE_03_HC       = 83164,
    SPELL_FORSAKEN_TROOPER_MALE_04_HC       = 83165,
    SPELL_FORSAKEN_TROOPER_FEMALE_01_HC     = 83152,
    SPELL_FORSAKEN_TROOPER_FEMALE_02_HC     = 83166,
    SPELL_FORSAKEN_TROOPER_FEMALE_03_HC     = 83167,
    SPELL_FORSAKEN_TROOPER_FEMALE_04_HC     = 83168,

    DISPLAY_MALE_01_HC                      = 33978,
    DISPLAY_MALE_02_HC                      = 33980,
    DISPLAY_MALE_03_HC                      = 33979,
    DISPLAY_MALE_04_HC                      = 33981,
    DISPLAY_FEMALE_01_HC                    = 33982,
    DISPLAY_FEMALE_02_HC                    = 33983,
    DISPLAY_FEMALE_03_HC                    = 33984,
    DISPLAY_FEMALE_04_HC                    = 33985
};

// 83149 - Forsaken Trooper Master Script (Forsaken High Command)
// Turns a raised Fallen Human into the Forsaken Trooper that matches its human model
class spell_silverpine_forsaken_trooper_masterscript_high_command : public SpellScript
{
    PrepareSpellScript(spell_silverpine_forsaken_trooper_masterscript_high_command);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_FORSAKEN_TROOPER_MALE_01_HC,
            SPELL_FORSAKEN_TROOPER_MALE_02_HC,
            SPELL_FORSAKEN_TROOPER_MALE_03_HC,
            SPELL_FORSAKEN_TROOPER_MALE_04_HC,
            SPELL_FORSAKEN_TROOPER_FEMALE_01_HC,
            SPELL_FORSAKEN_TROOPER_FEMALE_02_HC,
            SPELL_FORSAKEN_TROOPER_FEMALE_03_HC,
            SPELL_FORSAKEN_TROOPER_FEMALE_04_HC
        });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->RemoveAurasDueToSpell(SPELL_FEIGNED);
        caster->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);

        uint32 spellId = SPELL_FORSAKEN_TROOPER_MALE_01_HC;
        switch (caster->GetDisplayId())
        {
            case DISPLAY_MALE_02_HC:   spellId = SPELL_FORSAKEN_TROOPER_MALE_02_HC;   break;
            case DISPLAY_MALE_03_HC:   spellId = SPELL_FORSAKEN_TROOPER_MALE_03_HC;   break;
            case DISPLAY_MALE_04_HC:   spellId = SPELL_FORSAKEN_TROOPER_MALE_04_HC;   break;
            case DISPLAY_FEMALE_01_HC: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_01_HC; break;
            case DISPLAY_FEMALE_02_HC: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_02_HC; break;
            case DISPLAY_FEMALE_03_HC: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_03_HC; break;
            case DISPLAY_FEMALE_04_HC: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_04_HC; break;
            default: break;
        }

        caster->CastSpell(caster, spellId, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_forsaken_trooper_masterscript_high_command::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum BatHandlerMaggotbreath
{
    QUEST_ITERATING_UPON_SUCCESS                = 26998,

    SPELL_SUMMON_FORSAKEN_BAT                   = 83584,

    TALK_MAGGOTBREATH_ITERATING_UPON_SUCCESS    = 0,

    GOSSIP_MENU_MAGGOTBREATH                    = 11892,
    GOSSIP_OPTION_TAKE_A_BAT                    = 0
};

// 44825 - Bat Handler Maggotbreath
// The gossip option (shown while Iterating Upon Success is in progress) lends the player a bat; the flight master option stays default
struct npc_silverpine_bat_handler_maggotbreath : public ScriptedAI
{
    npc_silverpine_bat_handler_maggotbreath(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_MAGGOTBREATH || gossipListId != GOSSIP_OPTION_TAKE_A_BAT)
            return false;

        CloseGossipMenuFor(player);

        if (player->GetQuestStatus(QUEST_ITERATING_UPON_SUCCESS) != QUEST_STATUS_INCOMPLETE || player->GetVehicle())
            return true;

        Talk(TALK_MAGGOTBREATH_ITERATING_UPON_SUCCESS, player);
        player->CastSpell(player, SPELL_SUMMON_FORSAKEN_BAT, true);
        return true;
    }
};

enum ForsakenBat
{
    NPC_BAT_HANDLER_MAGGOTBREATH                = 44825,
    NPC_FORSAKEN_BAT                            = 44821,

    SPELL_BLIGHT_CONCOCTION                     = 83573,
    SPELL_GO_HOME                               = 83594,

    EVENT_CHECK_FINISH_ITERATING                = 1,
    EVENT_CIRCLE_THE_LAKE                       = 2,
    EVENT_LAND_AT_HIGH_COMMAND                  = 3,

    ACTION_GO_HOME                              = 1,

    TALK_BAT_ARRIVED_TO_ISLE                    = 0,
    TALK_BAT_GOING_HOME                         = 1,

    PATH_BAT_TO_LAKE                            = 448210,
    PATH_BAT_AROUND_LAKE                        = 448211,
    PATH_BAT_TO_HOME                            = 448212,

    WAYPOINT_LAST_POINT_TO_LAKE                 = 7,
    WAYPOINT_LAST_POINT_TO_HOME                 = 6
};

// TrinityCore's absolute speeds (17.79, 17.98 and 35.79 yards per second) as rates of the 7.0 base speed
float const BatSpeedToLake   = 2.542f;
float const BatSpeedAtLake   = 2.569f;
float const BatSpeedToHome   = 5.113f;

// 44821 - Forsaken Bat
// Flies its rider to Lordamere Lake, circles it while the Blight Concoction kills the Vile Fin murlocs, and brings the rider back to
// Forsaken High Command when the 50 murlocs are slain or the rider uses Go Home
struct npc_silverpine_forsaken_bat : public VehicleAI
{
    npc_silverpine_forsaken_bat(Creature* creature) : VehicleAI(creature), _currentPath(0), _goingHome(false)
    {
        // The rider's action bar is sent when it boards, so the first spell has to be there from the start
        SetInitialActionBar();
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply)
            return;

        Player* player = passenger->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_ITERATING_UPON_SUCCESS) != QUEST_STATUS_INCOMPLETE)
            return;

        player->KilledMonsterCredit(NPC_BAT_HANDLER_MAGGOTBREATH);

        StartPath(PATH_BAT_TO_LAKE, BatSpeedToLake, false);
        _events.ScheduleEvent(EVENT_CHECK_FINISH_ITERATING, 500ms);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        // The next path is started from UpdateAI: the waypoint generator resets the motion master after its last point
        if (_currentPath == PATH_BAT_TO_LAKE && pointId == WAYPOINT_LAST_POINT_TO_LAKE)
            _events.ScheduleEvent(EVENT_CIRCLE_THE_LAKE, 0ms);
        else if (_currentPath == PATH_BAT_TO_HOME && pointId == WAYPOINT_LAST_POINT_TO_HOME)
            _events.ScheduleEvent(EVENT_LAND_AT_HIGH_COMMAND, 0ms);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_GO_HOME || _goingHome)
            return;

        _goingHome = true;
        _events.CancelEvent(EVENT_CHECK_FINISH_ITERATING);
        _events.CancelEvent(EVENT_CIRCLE_THE_LAKE);

        me->GetMotionMaster()->Clear();
        StartPath(PATH_BAT_TO_HOME, BatSpeedToHome, false);

        SetFinishActionBar();

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            if (player->GetVehicleBase() == me)
                player->VehicleSpellInitialize();
            Talk(TALK_BAT_GOING_HOME, player);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_FINISH_ITERATING:
                {
                    // The bat's credit is given on boarding, so the quest is complete once the 50 murlocs are slain
                    Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
                    if (player && player->GetQuestStatus(QUEST_ITERATING_UPON_SUCCESS) == QUEST_STATUS_INCOMPLETE)
                        _events.ScheduleEvent(EVENT_CHECK_FINISH_ITERATING, 500ms);
                    else
                        DoAction(ACTION_GO_HOME);
                    break;
                }
                case EVENT_CIRCLE_THE_LAKE:
                    StartPath(PATH_BAT_AROUND_LAKE, BatSpeedAtLake, true);
                    SetCircularActionBar();

                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    {
                        Talk(TALK_BAT_ARRIVED_TO_ISLE, player);
                        if (player->GetVehicleBase() == me)
                            player->VehicleSpellInitialize();
                    }
                    break;
                case EVENT_LAND_AT_HIGH_COMMAND:
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        vehicle->RemoveAllPassengers();
                    me->DespawnOrUnsummon(0);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void StartPath(uint32 pathId, float speedRate, bool repeatable)
    {
        _currentPath = pathId;
        me->SetSpeed(MOVE_RUN, speedRate);
        me->SetSpeed(MOVE_FLIGHT, speedRate);
        me->GetMotionMaster()->MovePath(pathId, repeatable);
    }

    void SetInitialActionBar()
    {
        me->m_spells[0] = SPELL_BLIGHT_CONCOCTION;
        me->m_spells[1] = 0;
    }

    void SetCircularActionBar()
    {
        me->m_spells[0] = SPELL_BLIGHT_CONCOCTION;
        me->m_spells[1] = SPELL_GO_HOME;
    }

    void SetFinishActionBar()
    {
        me->m_spells[0] = 0;
        me->m_spells[1] = 0;
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    uint32 _currentPath;
    bool _goingHome;
};

// 83594 - Go Home
// Cast from the bat's action bar, so the caster is the bat itself, which has no vehicle for the TARGET_UNIT_VEHICLE effect to hit
// (TrinityCore's SpellHit never fires here); act before target selection instead, for the bat or for a rider casting it
class spell_silverpine_go_home : public SpellScript
{
    PrepareSpellScript(spell_silverpine_go_home);

    void HandleBeforeCast()
    {
        Unit* caster = GetCaster();
        Unit* bat = caster->GetVehicleBase() ? caster->GetVehicleBase() : caster;
        if (Creature* creature = bat->ToCreature())
            if (creature->GetEntry() == NPC_FORSAKEN_BAT && creature->IsAIEnabled)
                creature->AI()->DoAction(ACTION_GO_HOME);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_silverpine_go_home::HandleBeforeCast);
    }
};

enum WaitingToExsanguinate
{
    QUEST_WAITING_TO_EXSANGUINATE               = 27045,

    NPC_DEATHSTALKER_RANE_YORICK                = 44882,
    NPC_LORD_DARIUS_CROWLEY_EXSANGUINATE        = 44883,
    NPC_PACKLEADER_IVAR_BLOODFANG_EXSANGUINATE  = 44884,

    SPELL_SUMMON_DEATHSTALKER_YORICK            = 83751,
    SPELL_SUMMON_CROWLEY_EXSANGUINATE           = 83752,
    SPELL_SUMMON_BLOODFANG_EXSANGUINATE         = 83753,
    SPELL_ARMOIRE_CAMERA_ON_CROWLEY             = 83763,
    SPELL_ARMOIRE_CAMERA_ON_BLOODFANG           = 83764,
    SPELL_KILL_CREDIT_YORICK                    = 83786,
    SPELL_HIDE_IN_ARMOIRE                       = 83788,
    SPELL_STEALTH_YORICK                        = 34189,
    SPELL_PERMANENT_FEIGN_DEATH_YORICK          = 29266,
    SPELL_RIDE_VEHICLE_HARDCODED_YORICK         = 46598,

    GOSSIP_MENU_ABANDONED_OUTHOUSE              = 11897,
    GOSSIP_OPTION_YORICK_READY                  = 0,

    // Deathstalker Rane Yorick
    EVENT_YORICK_GREET                          = 1,
    EVENT_YORICK_WALK_UP                        = 2,
    EVENT_YORICK_WAIT_FOR_PLAYER                = 3,
    EVENT_YORICK_TALK_HIDE                      = 4,
    EVENT_YORICK_HIDE                           = 5,
    EVENT_YORICK_SKIP_PATH                      = 6,
    EVENT_YORICK_STEALTH                        = 7,
    EVENT_YORICK_FEIGN_DEATH                    = 8,
    EVENT_YORICK_ENABLE_GRAVITY                 = 9,

    // Armoire: the scene steps follow each other from 1 up to EVENT_SCENE_EXSANGUINATE_FINISH
    EVENT_SCENE_EXSANGUINATE_START              = 1,
    EVENT_SCENE_EXSANGUINATE_FINISH             = 39,

    ACTION_YORICK_DIE                           = 1,

    TALK_YORICK_SUMMONED                        = 0,
    TALK_YORICK_HIDE                            = 1,
    TALK_YORICK_DEATH                           = 2,
    TALK_CROWLEY_EXSANGUINATE_0                 = 0,
    TALK_CROWLEY_EXSANGUINATE_1                 = 1,
    TALK_CROWLEY_EXSANGUINATE_2                 = 2,
    TALK_CROWLEY_EXSANGUINATE_3                 = 3,
    TALK_BLOODFANG_EXSANGUINATE_0               = 0,
    TALK_BLOODFANG_EXSANGUINATE_1               = 1,
    TALK_BLOODFANG_EXSANGUINATE_2               = 2,
    TALK_BLOODFANG_EXSANGUINATE_3               = 3,
    TALK_BLOODFANG_EXSANGUINATE_4               = 4,
    TALK_BLOODFANG_EXSANGUINATE_5               = 5,
    TALK_BLOODFANG_EXSANGUINATE_6               = 6,
    TALK_BLOODFANG_EXSANGUINATE_7               = 7,
    TALK_BLOODFANG_EXSANGUINATE_8               = 8,
    TALK_BLOODFANG_EXSANGUINATE_9               = 9,
    TALK_BLOODFANG_EXSANGUINATE_10              = 10,

    PATH_YORICK_UP                              = 448820,
    PATH_YORICK_HIDE                            = 448821,
    PATH_CROWLEY_ENTER                          = 448830,
    PATH_CROWLEY_EXIT                           = 448831,
    PATH_BLOODFANG_ENTER                        = 448840,
    PATH_BLOODFANG_NEAR_YORICK                  = 448841,
    PATH_BLOODFANG_WITH_YORICK                  = 448842,
    PATH_BLOODFANG_EXIT                         = 448843,

    // Indexes of the last point of each path
    WAYPOINT_YORICK_CLOSE_TO_ARMOIRE            = 14,
    WAYPOINT_YORICK_HIDDEN_NEXT_TO_ARMOIRE      = 1,
    WAYPOINT_CROWLEY_EXIT_LAST                  = 1,
    WAYPOINT_BLOODFANG_EXIT_LAST                = 2
};

Position const YorickReadyPosition = { 1313.7f, 1211.99f, 58.5f, 4.564474f };

// 205143 - Abandoned Outhouse
// Ends Dangerous Intentions and starts Waiting to Exsanguinate: Deathstalker Rane Yorick comes out on accepting the quest, or later
// through the "Yorick, you in there?" option (shown while the quest is in progress and Yorick is not out)
struct go_silverpine_abandoned_outhouse : public GameObjectAI
{
    go_silverpine_abandoned_outhouse(GameObject* go) : GameObjectAI(go) { }

    bool OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_WAITING_TO_EXSANGUINATE)
            SummonYorick(player);
        return false;
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_ABANDONED_OUTHOUSE || gossipListId != GOSSIP_OPTION_YORICK_READY)
            return false;

        CloseGossipMenuFor(player);
        SummonYorick(player);
        return true;
    }

private:
    static void SummonYorick(Player* player)
    {
        if (player->GetQuestStatus(QUEST_WAITING_TO_EXSANGUINATE) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_SUMMON_DEATHSTALKER_YORICK))
            player->CastSpell(player, SPELL_SUMMON_DEATHSTALKER_YORICK, true);
    }
};

// 44882 - Deathstalker Rane Yorick
// The player's personal summon: leads the way up to the armoire in the house, waits for the player, then hides in the shadows next
// to it until Bloodfang finds him. Leaves when the summoner loses Summon Deathstalker Yorick.
struct npc_silverpine_deathstalker_rane_yorick : public ScriptedAI
{
    npc_silverpine_deathstalker_rane_yorick(Creature* creature) : ScriptedAI(creature), _currentPath(0), _playerArrived(false), _hidden(false) { }

    void Reset() override
    {
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 100);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();
        me->SetFacingToObject(summoner);
        _events.ScheduleEvent(EVENT_YORICK_GREET, 1s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (_currentPath == PATH_YORICK_UP && pointId == WAYPOINT_YORICK_CLOSE_TO_ARMOIRE)
        {
            _currentPath = 0;
            _events.ScheduleEvent(EVENT_YORICK_WAIT_FOR_PLAYER, 1s);
        }
        else if (_currentPath == PATH_YORICK_HIDE && pointId == WAYPOINT_YORICK_HIDDEN_NEXT_TO_ARMOIRE)
        {
            _currentPath = 0;
            me->SetFacingTo(4.6425757f);
            Hide();
        }
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_YORICK_DIE)
            return;

        _events.Reset();
        me->SetDisableGravity(true);
        _events.ScheduleEvent(EVENT_YORICK_FEIGN_DEATH, 10ms);
    }

    void UpdateAI(uint32 diff) override
    {
        Player* summoner = ObjectAccessor::GetPlayer(*me, _summonerGUID);
        if (!summoner || !summoner->HasAura(SPELL_SUMMON_DEATHSTALKER_YORICK))
        {
            me->DespawnOrUnsummon(0);
            return;
        }

        // The player got into the armoire before Yorick reached it: take the hiding spot at once
        if (!_hidden && summoner->HasAura(SPELL_HIDE_IN_ARMOIRE))
        {
            _hidden = true;
            _currentPath = 0;
            _events.Reset();
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
            _events.ScheduleEvent(EVENT_YORICK_SKIP_PATH, 250ms);
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_YORICK_GREET:
                    Talk(TALK_YORICK_SUMMONED, summoner);
                    _events.ScheduleEvent(EVENT_YORICK_WALK_UP, 1s + 500ms);
                    break;
                case EVENT_YORICK_WALK_UP:
                    _currentPath = PATH_YORICK_UP;
                    me->GetMotionMaster()->MovePath(PATH_YORICK_UP, false);
                    break;
                case EVENT_YORICK_WAIT_FOR_PLAYER:
                    if (!_playerArrived && summoner->GetDistance2d(me) <= 5.0f)
                    {
                        _playerArrived = true;
                        _events.ScheduleEvent(EVENT_YORICK_TALK_HIDE, 1s);
                    }
                    else
                        _events.ScheduleEvent(EVENT_YORICK_WAIT_FOR_PLAYER, 1s);
                    break;
                case EVENT_YORICK_TALK_HIDE:
                    Talk(TALK_YORICK_HIDE, summoner);
                    _events.ScheduleEvent(EVENT_YORICK_HIDE, 3s);
                    break;
                case EVENT_YORICK_HIDE:
                    _currentPath = PATH_YORICK_HIDE;
                    me->GetMotionMaster()->MovePath(PATH_YORICK_HIDE, false);
                    break;
                case EVENT_YORICK_SKIP_PATH:
                    me->NearTeleportTo(YorickReadyPosition.GetPositionX(), YorickReadyPosition.GetPositionY(), YorickReadyPosition.GetPositionZ(),
                        YorickReadyPosition.GetOrientation());
                    _events.ScheduleEvent(EVENT_YORICK_STEALTH, 250ms);
                    break;
                case EVENT_YORICK_STEALTH:
                    Hide();
                    break;
                case EVENT_YORICK_FEIGN_DEATH:
                    DoCast(me, SPELL_PERMANENT_FEIGN_DEATH_YORICK, true);
                    _events.ScheduleEvent(EVENT_YORICK_ENABLE_GRAVITY, 2s);
                    break;
                case EVENT_YORICK_ENABLE_GRAVITY:
                    me->SetDisableGravity(false);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void Hide()
    {
        _hidden = true;
        DoCast(me, SPELL_STEALTH_YORICK, true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
    uint32 _currentPath;
    bool _playerArrived;
    bool _hidden;
};

// The player's own summon of an entry. FindNearestCreature also finds group members' personal summons, which play the same scene
// at the same spot.
static ObjectGuid FindOwnSummon(Player* player, uint32 entry)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, 50.0f);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return creature->GetGUID();
    return ObjectGuid::Empty;
}

// 44893 - Armoire
// Summoned by clicking the armoire in the house (Summon Hiding Spot), with the player hidden inside. Plays the meeting the player
// eavesdrops on: Crowley asks Bloodfang for his pack's help, Bloodfang sniffs out and kills Yorick, and Crowley's Kill Credit Yorick
// completes the quest before the player is let out. Leaving the armoire ends the scene.
struct npc_silverpine_armoire : public VehicleAI
{
    npc_silverpine_armoire(Creature* creature) : VehicleAI(creature), _sceneStarted(false) { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        Player* player = passenger->ToPlayer();
        if (!player)
            return;

        if (apply)
        {
            if (_sceneStarted || player->GetQuestStatus(QUEST_WAITING_TO_EXSANGUINATE) != QUEST_STATUS_INCOMPLETE)
                return;

            _sceneStarted = true;
            _playerGUID = player->GetGUID();
            _events.ScheduleEvent(EVENT_SCENE_EXSANGUINATE_START, 400ms);
            return;
        }

        _events.Reset();

        if (Creature* crowley = ObjectAccessor::GetCreature(*me, _crowleyGUID))
            crowley->DespawnOrUnsummon(0);
        if (Creature* bloodfang = ObjectAccessor::GetCreature(*me, _bloodfangGUID))
            bloodfang->DespawnOrUnsummon(0);

        // Yorick leaves with his summoning aura
        player->RemoveAurasDueToSpell(SPELL_SUMMON_DEATHSTALKER_YORICK);
        player->RemoveAurasDueToSpell(SPELL_HIDE_IN_ARMOIRE);

        me->DespawnOrUnsummon(1000);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player || player->GetVehicleBase() != me)
            {
                _events.Reset();
                me->DespawnOrUnsummon(0);
                return;
            }

            if (eventId == EVENT_SCENE_EXSANGUINATE_START)
            {
                // The three spells of Summon Crowley/Bloodfang Master, cast directly
                player->CastSpell(player, SPELL_SUMMON_BLOODFANG_EXSANGUINATE, true);
                player->CastSpell(player, SPELL_SUMMON_CROWLEY_EXSANGUINATE, true);
                player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_CROWLEY, true);

                // They are personal summons (as is Yorick), so they are found from the player's side. Group members see each
                // other's, so only this player's own are taken.
                _crowleyGUID = FindOwnSummon(player, NPC_LORD_DARIUS_CROWLEY_EXSANGUINATE);
                _bloodfangGUID = FindOwnSummon(player, NPC_PACKLEADER_IVAR_BLOODFANG_EXSANGUINATE);
                _yorickGUID = FindOwnSummon(player, NPC_DEATHSTALKER_RANE_YORICK);
            }

            Creature* crowley = ObjectAccessor::GetCreature(*me, _crowleyGUID);
            Creature* bloodfang = ObjectAccessor::GetCreature(*me, _bloodfangGUID);
            Creature* yorick = ObjectAccessor::GetCreature(*me, _yorickGUID);
            // From step 37 on Bloodfang and then Crowley walk off and despawn at the end of their paths, so only the steps
            // before that need all three
            if (eventId < 37 && (!crowley || !bloodfang || !yorick || !crowley->IsAIEnabled || !bloodfang->IsAIEnabled || !yorick->IsAIEnabled))
            {
                // An actor is missing: let the player out, which ends the scene
                _events.Reset();
                player->ExitVehicle();
                return;
            }

            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_SCENE_EXSANGUINATE_START:
                    next = 100ms;
                    break;
                case 2:
                    crowley->AI()->DoAction(PATH_CROWLEY_ENTER);
                    bloodfang->AI()->DoAction(PATH_BLOODFANG_ENTER);
                    next = 7s;
                    break;
                case 3:
                    crowley->SetFacingToObject(bloodfang);
                    bloodfang->SetFacingToObject(crowley);
                    next = 2s;
                    break;
                case 4:
                    crowley->AI()->Talk(TALK_CROWLEY_EXSANGUINATE_0, player);
                    next = 3s + 900ms;
                    break;
                case 5:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_BLOODFANG, true);
                    next = 2s + 500ms;
                    break;
                case 6:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_0, player);
                    next = 3s + 100ms;
                    break;
                case 7:
                    bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    next = 4s + 800ms;
                    break;
                case 8:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_CROWLEY, true);
                    next = 2s + 500ms;
                    break;
                case 9:
                    crowley->AI()->Talk(TALK_CROWLEY_EXSANGUINATE_1, player);
                    next = 3s + 500ms;
                    break;
                case 10:
                    crowley->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                    next = 5s + 400ms;
                    break;
                case 11:
                    crowley->AI()->Talk(TALK_CROWLEY_EXSANGUINATE_2, player);
                    next = 3s + 100ms;
                    break;
                case 12:
                    crowley->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                    next = 3s + 550ms;
                    break;
                case 13:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_BLOODFANG, true);
                    next = 2s + 500ms;
                    break;
                case 14:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_1, player);
                    next = 3s + 800ms;
                    break;
                case 15:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_2, player);
                    next = 2s + 400ms;
                    break;
                case 16:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_3, player);
                    next = 3s + 900ms;
                    break;
                case 17:
                    bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                    next = 4s + 450ms;
                    break;
                case 18:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_CROWLEY, true);
                    next = 3s;
                    break;
                case 19:
                    crowley->AI()->Talk(TALK_CROWLEY_EXSANGUINATE_3, player);
                    next = 1s + 900ms;
                    break;
                case 20:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_BLOODFANG, true);
                    next = 2s + 500ms;
                    break;
                case 21:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_4, player);
                    next = 2s + 300ms;
                    break;
                case 22:
                    bloodfang->SetFacingTo(0.6457718f);
                    next = 1s + 300ms;
                    break;
                case 23:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_5, player);
                    next = 4s;
                    break;
                case 24:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_6, player);
                    next = 3s + 500ms;
                    break;
                case 25:
                    bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_YES);
                    next = 2s + 800ms;
                    break;
                case 26:
                    bloodfang->AI()->DoAction(PATH_BLOODFANG_NEAR_YORICK);
                    next = 3s;
                    break;
                case 27:
                    bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                    next = 200ms;
                    break;
                case 28:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_7, player);
                    next = 2s + 300ms;
                    break;
                case 29:
                    // Bloodfang grabs Yorick: TrinityCore's Reverse Cast Ride Vehicle (83781), which makes Yorick ride him
                    yorick->RemoveAurasDueToSpell(SPELL_STEALTH_YORICK);
                    yorick->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    yorick->CastSpell(bloodfang, SPELL_RIDE_VEHICLE_HARDCODED_YORICK, true);
                    next = 1s + 100ms;
                    break;
                case 30:
                    bloodfang->AI()->DoAction(PATH_BLOODFANG_WITH_YORICK);
                    next = 3s;
                    break;
                case 31:
                    bloodfang->SetFacingTo(3.054326f);
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_8, player);
                    next = 3s + 600ms;
                    break;
                case 32:
                    yorick->AI()->Talk(TALK_YORICK_DEATH, player);
                    next = 4s + 850ms;
                    break;
                case 33:
                    // TrinityCore's Eject Passenger 1 (80743)
                    yorick->ExitVehicle();
                    yorick->AI()->DoAction(ACTION_YORICK_DIE);
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_9, player);
                    next = 3s + 600ms;
                    break;
                case 34:
                    bloodfang->SetFacingTo(0.0f);
                    next = 1s + 100ms;
                    break;
                case 35:
                    bloodfang->AI()->Talk(TALK_BLOODFANG_EXSANGUINATE_10, player);
                    next = 4s + 100ms;
                    break;
                case 36:
                    bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                    next = 3s + 250ms;
                    break;
                case 37:
                    player->CastSpell(player, SPELL_ARMOIRE_CAMERA_ON_CROWLEY, true);
                    if (bloodfang && bloodfang->IsAIEnabled)
                        bloodfang->AI()->DoAction(PATH_BLOODFANG_EXIT);
                    next = 3s;
                    break;
                case 38:
                    if (crowley && crowley->IsAIEnabled)
                        crowley->AI()->DoAction(PATH_CROWLEY_EXIT);
                    next = 4s;
                    break;
                case EVENT_SCENE_EXSANGUINATE_FINISH:
                    // Crowley has gone by now; the armoire's summoner is the player, so its cast still lands on the player
                    me->CastSpell(player, SPELL_KILL_CREDIT_YORICK, true);
                    player->RemoveAurasDueToSpell(SPELL_HIDE_IN_ARMOIRE);
                    player->ExitVehicle();
                    return;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _yorickGUID;
    ObjectGuid _crowleyGUID;
    ObjectGuid _bloodfangGUID;
    bool _sceneStarted;
};

// Lord Darius Crowley and Packleader Ivar Bloodfang in the meeting: the armoire picks their paths, and they leave at the end of
// their exit path
struct npc_silverpine_exsanguinate_actor : public ScriptedAI
{
    npc_silverpine_exsanguinate_actor(Creature* creature, uint32 exitPath, uint32 exitLastPoint) : ScriptedAI(creature),
        _currentPath(0), _exitPath(exitPath), _exitLastPoint(exitLastPoint) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    // The action is the path to walk
    void DoAction(int32 pathId) override
    {
        _currentPath = uint32(pathId);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePath(_currentPath, false);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == WAYPOINT_MOTION_TYPE && _currentPath == _exitPath && pointId == _exitLastPoint)
            me->DespawnOrUnsummon(0);
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    uint32 _currentPath;
    uint32 _exitPath;
    uint32 _exitLastPoint;
};

// 44883 - Lord Darius Crowley
struct npc_silverpine_lord_darius_crowley_exsanguinate : public npc_silverpine_exsanguinate_actor
{
    npc_silverpine_lord_darius_crowley_exsanguinate(Creature* creature)
        : npc_silverpine_exsanguinate_actor(creature, PATH_CROWLEY_EXIT, WAYPOINT_CROWLEY_EXIT_LAST) { }
};

// 44884 - Packleader Ivar Bloodfang
struct npc_silverpine_packleader_ivar_bloodfang_exsanguinate : public npc_silverpine_exsanguinate_actor
{
    npc_silverpine_packleader_ivar_bloodfang_exsanguinate(Creature* creature)
        : npc_silverpine_exsanguinate_actor(creature, PATH_BLOODFANG_EXIT, WAYPOINT_BLOODFANG_EXIT_LAST) { }
};

// 83788 - Hide in Armoire
// Its second effect moves the player to phase 265, where nothing of the scene would be seen here (there is no phasing in Silverpine
// Forest, and Yorick and the armoire are summoned before the aura), so it is kept from applying
class spell_silverpine_hide_in_armoire : public AuraScript
{
    PrepareAuraScript(spell_silverpine_hide_in_armoire);

    void PreventPhase(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_silverpine_hide_in_armoire::PreventPhase, EFFECT_1, SPELL_AURA_PHASE, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_silverpine_hide_in_armoire::PreventPhase, EFFECT_1, SPELL_AURA_PHASE, AURA_EFFECT_HANDLE_REAL);
    }
};

enum ForsakenRearGuard
{
    QUEST_STEEL_THUNDER                         = 27069,
    QUEST_ITS_ONLY_POISONOUS_IF_YOU_INGEST_IT   = 27088,
    QUEST_LOST_IN_THE_DARKNESS                  = 27093,
    QUEST_DEEPER_INTO_DARKNESS                  = 27094,

    NPC_APOTHECARY_WORMCRUD                     = 44912,
    NPC_DRUNKEN_ORC_SEA_DOG                     = 44913,
    NPC_ORC_SEA_PUP                             = 44914,
    NPC_ORC_CRATE                               = 44915,
    NPC_ADMIRAL_HATCHET                         = 44916,
    NPC_WARLORD_TOROK                           = 44917,
    NPC_RESCUED_ORC_SEA_DOG                     = 44942,
    NPC_SALTY_GORGAR                            = 45497,

    SPELL_ORC_SEA_PUP                           = 83839,
    SPELL_SUMMON_ORC_CRATE                      = 83835,

    GOSSIP_MENU_ADMIRAL_HATCHET                 = 11901,
    GOSSIP_OPTION_MISPLACED_SEA_PUP             = 0,

    ACTION_HATCHET_START_CONVERSATION           = 1,
    ACTION_WORMCRUD_START_CONVERSATION          = 2,
    ACTION_SEA_PUP_DELIVER_CRATES               = 3,

    // "Salty" Rocka
    EVENT_ROCKA_FIRST_LINE                      = 1,
    EVENT_ROCKA_GORGAR_LINE                     = 2,
    EVENT_ROCKA_SECOND_LINE                     = 3,
    EVENT_ROCKA_CONVERSATION_COOLDOWN           = 4,

    // Apothecary Wormcrud
    EVENT_WORMCRUD_SEA_DOG_LINE                 = 1,
    EVENT_WORMCRUD_LINE                         = 2,
    EVENT_WORMCRUD_CONVERSATION_COOLDOWN        = 3,

    // Admiral Hatchet
    EVENT_HATCHET_LINE_0                        = 1,
    EVENT_TOROK_LINE_0                          = 2,
    EVENT_HATCHET_LINE_1                        = 3,
    EVENT_TOROK_LINE_1                          = 4,
    EVENT_HATCHET_CONVERSATION_COOLDOWN         = 5,

    // Orc Sea Pup
    EVENT_SEA_PUP_GREET                         = 1,
    EVENT_SEA_PUP_RELOAD_CRATES                 = 2,
    EVENT_SEA_PUP_REMOVE_PROTECTION             = 3,
    EVENT_SEA_PUP_COMPLAIN                      = 4,
    EVENT_SEA_PUP_DROP_CRATES                   = 5,

    TALK_HATCHET_0                              = 0,
    TALK_HATCHET_1                              = 1,
    TALK_TOROK_0                                = 0,
    TALK_TOROK_1                                = 1,
    TALK_WORMCRUD_0                             = 0,
    TALK_SEA_PUP_SUMMONED                       = 0,    // 1 to 5: a crate is loaded on seat 0 to 4
    TALK_SEA_PUP_DELIVER_CRATES                 = 6,
    TALK_SEA_PUP_WORN_OFF                       = 7,

    ANIMKIT_TOROK                               = 594
};

// The creatures a spell summoned for the player (their owner) around them
static std::list<Creature*> GetOwnedCreatures(Unit* owner, uint32 entry, float range)
{
    std::list<Creature*> creatures;
    owner->GetCreatureListWithEntryInGrid(creatures, entry, range);
    creatures.remove_if([owner](Creature* creature) { return creature->GetOwnerGUID() != owner->GetGUID(); });
    return creatures;
}

// 45498 - "Salty" Rocka
// When a player comes by, picks one of five drunken arguments with "Salty" Gorgar across the camp
struct npc_silverpine_salty_rocka : public ScriptedAI
{
    npc_silverpine_salty_rocka(Creature* creature) : ScriptedAI(creature), _conversation(0), _onCooldown(false) { }

    void Reset() override
    {
        _events.Reset();
        _onCooldown = false;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_onCooldown || who->GetTypeId() != TYPEID_PLAYER || who->GetDistance2d(me) >= 20.0f)
            return;

        _onCooldown = true;
        _conversation = urand(0, 4);
        _events.ScheduleEvent(EVENT_ROCKA_FIRST_LINE, 1s);
        _events.ScheduleEvent(EVENT_ROCKA_CONVERSATION_COOLDOWN, 180s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ROCKA_FIRST_LINE:
                    Talk(_conversation * 2);
                    _events.ScheduleEvent(EVENT_ROCKA_GORGAR_LINE, 20s);
                    break;
                case EVENT_ROCKA_GORGAR_LINE:
                    if (Creature* gorgar = me->FindNearestCreature(NPC_SALTY_GORGAR, 60.0f))
                        if (gorgar->IsAIEnabled)
                            gorgar->AI()->Talk(_conversation);
                    _events.ScheduleEvent(EVENT_ROCKA_SECOND_LINE, 20s);
                    break;
                case EVENT_ROCKA_SECOND_LINE:
                    Talk(_conversation * 2 + 1);
                    break;
                case EVENT_ROCKA_CONVERSATION_COOLDOWN:
                    _onCooldown = false;
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    uint32 _conversation;
    bool _onCooldown;
};

// 6222 - Forsaken Rear Guard
// Arriving at the camp starts Admiral Hatchet's talk with Warlord Torok until the spiders are dealt with, then Apothecary
// Wormcrud's with the drunken sea dogs until the ettin is
class at_silverpine_forsaken_rear_guard : public AreaTriggerScript
{
public:
    at_silverpine_forsaken_rear_guard() : AreaTriggerScript("at_silverpine_forsaken_rear_guard") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (!player->IsAlive())
            return true;

        if (!IsDone(player, QUEST_LOST_IN_THE_DARKNESS) && !IsDone(player, QUEST_DEEPER_INTO_DARKNESS))
        {
            if (Creature* hatchet = player->FindNearestCreature(NPC_ADMIRAL_HATCHET, 50.0f))
                if (hatchet->IsAIEnabled)
                    hatchet->AI()->DoAction(ACTION_HATCHET_START_CONVERSATION);
        }
        else if (!IsDone(player, QUEST_ITS_ONLY_POISONOUS_IF_YOU_INGEST_IT))
        {
            if (Creature* wormcrud = player->FindNearestCreature(NPC_APOTHECARY_WORMCRUD, 50.0f))
                if (wormcrud->IsAIEnabled)
                    wormcrud->AI()->DoAction(ACTION_WORMCRUD_START_CONVERSATION);
        }

        return true;
    }

private:
    static bool IsDone(Player* player, uint32 questId)
    {
        QuestStatus status = player->GetQuestStatus(questId);
        return status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_REWARDED;
    }
};

// 44912 - Apothecary Wormcrud
// Three of the drunken sea dogs next to him wonder about his huge chicken
struct npc_silverpine_apothecary_wormcrud : public ScriptedAI
{
    npc_silverpine_apothecary_wormcrud(Creature* creature) : ScriptedAI(creature), _seaDogLine(0), _onCooldown(false) { }

    void Reset() override
    {
        _events.Reset();
        _onCooldown = false;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_WORMCRUD_START_CONVERSATION || _onCooldown)
            return;

        std::list<Creature*> seaDogs;
        me->GetCreatureListWithEntryInGrid(seaDogs, NPC_DRUNKEN_ORC_SEA_DOG, 10.0f);
        if (seaDogs.size() < _seaDogGUIDs.size())
            return;

        seaDogs.sort(Trinity::ObjectDistanceOrderPred(me));
        auto seaDog = seaDogs.begin();
        for (ObjectGuid& guid : _seaDogGUIDs)
            guid = (*seaDog++)->GetGUID();

        _onCooldown = true;
        _seaDogLine = 0;
        _events.ScheduleEvent(EVENT_WORMCRUD_SEA_DOG_LINE, 15s);
        _events.ScheduleEvent(EVENT_WORMCRUD_CONVERSATION_COOLDOWN, 215s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_WORMCRUD_SEA_DOG_LINE:
                    if (Creature* seaDog = ObjectAccessor::GetCreature(*me, _seaDogGUIDs[_seaDogLine]))
                        if (seaDog->IsAIEnabled)
                            seaDog->AI()->Talk(_seaDogLine);

                    if (++_seaDogLine < _seaDogGUIDs.size())
                        _events.ScheduleEvent(EVENT_WORMCRUD_SEA_DOG_LINE, 12s);
                    else
                        _events.ScheduleEvent(EVENT_WORMCRUD_LINE, 12s);
                    break;
                case EVENT_WORMCRUD_LINE:
                    Talk(TALK_WORMCRUD_0);
                    break;
                case EVENT_WORMCRUD_CONVERSATION_COOLDOWN:
                    _onCooldown = false;
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    std::array<ObjectGuid, 3> _seaDogGUIDs;
    uint32 _seaDogLine;
    bool _onCooldown;
};

// 44916 - Admiral Hatchet
// Lends an Orc Sea Pup to carry the Sea Dog Crates for Steel Thunder (on accepting it, or again through the gossip option while
// it is in progress), takes the crates from the pup when it is turned in, and sends the rescued sea dogs off after Lost in the
// Darkness. Talks with Warlord Torok when players arrive at the camp.
struct npc_silverpine_admiral_hatchet : public ScriptedAI
{
    npc_silverpine_admiral_hatchet(Creature* creature) : ScriptedAI(creature), _onCooldown(false) { }

    void Reset() override
    {
        _events.Reset();
        _onCooldown = false;
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_STEEL_THUNDER)
            SummonSeaPup(player);
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() == QUEST_STEEL_THUNDER)
        {
            for (Creature* seaPup : GetOwnedCreatures(player, NPC_ORC_SEA_PUP, 100.0f))
                if (seaPup->IsAIEnabled)
                    seaPup->AI()->DoAction(ACTION_SEA_PUP_DELIVER_CRATES);
        }
        else if (quest->GetQuestId() == QUEST_LOST_IN_THE_DARKNESS)
        {
            for (Creature* seaDog : GetOwnedCreatures(player, NPC_RESCUED_ORC_SEA_DOG, 100.0f))
                seaDog->DespawnOrUnsummon(0);
        }
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_ADMIRAL_HATCHET || gossipListId != GOSSIP_OPTION_MISPLACED_SEA_PUP)
            return false;

        CloseGossipMenuFor(player);
        SummonSeaPup(player);
        return true;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_HATCHET_START_CONVERSATION || _onCooldown)
            return;

        _onCooldown = true;
        _events.ScheduleEvent(EVENT_HATCHET_LINE_0, 1s);
        _events.ScheduleEvent(EVENT_HATCHET_CONVERSATION_COOLDOWN, 230s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_HATCHET_LINE_0:
                    Talk(TALK_HATCHET_0);
                    _events.ScheduleEvent(EVENT_TOROK_LINE_0, 8s);
                    break;
                case EVENT_TOROK_LINE_0:
                    if (Creature* torok = me->FindNearestCreature(NPC_WARLORD_TOROK, 30.0f))
                        if (torok->IsAIEnabled)
                            torok->AI()->Talk(TALK_TOROK_0);
                    _events.ScheduleEvent(EVENT_HATCHET_LINE_1, 6s);
                    break;
                case EVENT_HATCHET_LINE_1:
                    Talk(TALK_HATCHET_1);
                    _events.ScheduleEvent(EVENT_TOROK_LINE_1, 6s);
                    break;
                case EVENT_TOROK_LINE_1:
                    if (Creature* torok = me->FindNearestCreature(NPC_WARLORD_TOROK, 30.0f))
                    {
                        torok->PlayOneShotAnimKitId(ANIMKIT_TOROK);
                        if (torok->IsAIEnabled)
                            torok->AI()->Talk(TALK_TOROK_1);
                    }
                    break;
                case EVENT_HATCHET_CONVERSATION_COOLDOWN:
                    _onCooldown = false;
                    break;
                default:
                    break;
            }
        }
    }

private:
    // A lost pup (it stays behind when the player leaves the map, while its Orc Sea Pup aura lasts) is replaced
    static void SummonSeaPup(Player* player)
    {
        if (player->GetQuestStatus(QUEST_STEEL_THUNDER) != QUEST_STATUS_INCOMPLETE)
            return;

        for (Creature* seaPup : GetOwnedCreatures(player, NPC_ORC_SEA_PUP, 500.0f))
            seaPup->DespawnOrUnsummon(0);

        player->RemoveAurasDueToSpell(SPELL_ORC_SEA_PUP);
        player->CastSpell(player, SPELL_ORC_SEA_PUP, true);
    }

    EventMap _events;
    bool _onCooldown;
};

// 44914 - Orc Sea Pup
// Follows the player and carries the Sea Dog Crates they pick up (each one an Orc Crate on one of its five seats), complaining
// about the load. A replacement pup is given the crates already collected. Drops them at Admiral Hatchet's feet when the quest
// is turned in.
struct npc_silverpine_orc_sea_pup : public ScriptedAI
{
    npc_silverpine_orc_sea_pup(Creature* creature) : ScriptedAI(creature), _isJustSummoned(true), _delivering(false) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _playerGUID = player->GetGUID();
        me->GetMotionMaster()->MoveFollow(player, 2.0f, float(M_PI / 2.0f));

        if (player->GetQuestStatus(QUEST_STEEL_THUNDER) != QUEST_STATUS_INCOMPLETE)
            return;

        _events.ScheduleEvent(EVENT_SEA_PUP_GREET, 1s);
        _events.ScheduleEvent(EVENT_SEA_PUP_RELOAD_CRATES, 500ms);
        _events.ScheduleEvent(EVENT_SEA_PUP_REMOVE_PROTECTION, 1s + 500ms);
        _events.ScheduleEvent(EVENT_SEA_PUP_COMPLAIN, 5s);
    }

    void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
    {
        Creature* crate = passenger->ToCreature();
        if (!crate || crate->GetEntry() != NPC_ORC_CRATE)
            return;

        if (!apply)
        {
            crate->DespawnOrUnsummon(3000);
            return;
        }

        if (!_isJustSummoned)
            if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                Talk(TALK_SEA_PUP_SUMMONED + 1 + seatId, player);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_SEA_PUP_DELIVER_CRATES || _delivering)
            return;

        _delivering = true;
        _events.Reset();

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            Talk(TALK_SEA_PUP_DELIVER_CRATES, player);
            player->RemoveAurasDueToSpell(SPELL_ORC_SEA_PUP);
        }

        _events.ScheduleEvent(EVENT_SEA_PUP_DROP_CRATES, 1s + 250ms);
        me->DespawnOrUnsummon(3000);
    }

    void UpdateAI(uint32 diff) override
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!_delivering && (!player || !player->IsInWorld() || player->GetQuestStatus(QUEST_STEEL_THUNDER) == QUEST_STATUS_NONE))
        {
            if (player)
                player->RemoveAurasDueToSpell(SPELL_ORC_SEA_PUP);
            me->DespawnOrUnsummon(0);
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SEA_PUP_GREET:
                    Talk(TALK_SEA_PUP_SUMMONED, player);
                    break;
                case EVENT_SEA_PUP_RELOAD_CRATES:
                    for (uint32 i = 0; i < GetCollectedCrates(player); ++i)
                        DoCast(me, SPELL_SUMMON_ORC_CRATE, true);
                    break;
                case EVENT_SEA_PUP_REMOVE_PROTECTION:
                    _isJustSummoned = false;
                    break;
                case EVENT_SEA_PUP_COMPLAIN:
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        if (vehicle->IsVehicleInUse())
                            Talk(TALK_SEA_PUP_WORN_OFF, player);
                    _events.ScheduleEvent(EVENT_SEA_PUP_COMPLAIN, 30s, 75s);
                    break;
                case EVENT_SEA_PUP_DROP_CRATES:
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        vehicle->RemoveAllPassengers();
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    break;
                default:
                    break;
            }
        }
    }

private:
    // GetReqKillOrCastCurrentCount only counts for quests with item objectives here
    static uint32 GetCollectedCrates(Player* player)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_STEEL_THUNDER);
        if (!quest)
            return 0;

        for (QuestObjective const& objective : quest->GetObjectives())
            if (objective.Type == QUEST_OBJECTIVE_MONSTER && objective.ObjectID == NPC_ORC_CRATE)
                return std::min<uint32>(player->GetQuestObjectiveCounter(objective.ID), uint32(objective.Amount));

        return 0;
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    bool _isJustSummoned;
    bool _delivering;
};

// 44915 - Orc Crate
// Summoned by an Orc Sea Pup, it climbs onto the pup's next free seat
struct npc_silverpine_orc_crate : public ScriptedAI
{
    npc_silverpine_orc_crate(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (summoner->GetEntry() != NPC_ORC_SEA_PUP)
            return;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->EnterVehicle(summoner);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 83838 - Pick Up Orc Crate
// Forced on the player by a Sea Dog Crate: the player's own pup takes an Orc Crate on board and the crate is counted
class spell_silverpine_pick_up_orc_crate : public SpellScript
{
    PrepareSpellScript(spell_silverpine_pick_up_orc_crate);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        targets.remove_if([caster](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            return !unit || unit->GetEntry() != NPC_ORC_SEA_PUP || unit->GetOwnerGUID() != caster->GetGUID();
        });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* seaPup = GetHitUnit();
        if (!player || !seaPup || player->GetQuestStatus(QUEST_STEEL_THUNDER) != QUEST_STATUS_INCOMPLETE)
            return;

        seaPup->CastSpell(seaPup, SPELL_SUMMON_ORC_CRATE, true);
        player->KilledMonsterCredit(NPC_ORC_CRATE);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_silverpine_pick_up_orc_crate::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_pick_up_orc_crate::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum ItsOnlyPoisonous
{
    NPC_FOREST_ETTIN                            = 44367,
    NPC_MUTANT_BUSH_CHICKEN                     = 44935,

    SPELL_BONK                                  = 80146,
    SPELL_LOG_SMASH                             = 88421,
    SPELL_RELEASE_BUSH_CHICKEN                  = 83902,
    SPELL_BUSH_EXPLOSION                        = 83903,
    SPELL_RIDE_VEHICLE_HARDCODED_CHICKEN        = 46598,

    // Forest Ettin
    EVENT_ETTIN_BONK                            = 1,
    EVENT_ETTIN_LOG_SMASH                       = 2,
    EVENT_ETTIN_GRAB_CHICKEN                    = 3,
    EVENT_ETTIN_WALK_ON                         = 4,
    EVENT_ETTIN_CHICKEN_TO_MOUTH                = 5,
    EVENT_ETTIN_CHICKEN_BLASTS                  = 6,
    EVENT_ETTIN_FORGET_CHICKEN                  = 7,

    // Mutant Bush Chicken
    EVENT_CHICKEN_MOVE_TO_ETTIN                 = 1,

    ACTION_ETTIN_NOTICE_CHICKEN                 = 1,

    POINT_CHICKEN_NEAR_ETTIN                    = 1,

    SEAT_ETTIN_ARM                              = 0,
    SEAT_ETTIN_MOUTH                            = 1,

    SOUND_CHICKEN_MOUNT_WOUND                   = 15936
};

// 44367 - Forest Ettin
// Hauls its log around; a Mutant Bush Chicken released at it is picked up and eaten, and blows up in its mouth, which leaves the
// ettin badly weakened (Bush Chicken Explosion!) and angry at whoever released it
struct npc_silverpine_forest_ettin : public ScriptedAI
{
    npc_silverpine_forest_ettin(Creature* creature) : ScriptedAI(creature), _grabbingChicken(false) { }

    void Reset() override
    {
        _events.Reset();
        ForgetChicken();
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_RELEASE_BUSH_CHICKEN && caster->GetTypeId() == TYPEID_PLAYER)
            _playerGUID = caster->GetGUID();
    }

    // The released chicken runs up to the ettin, which waits for it (when not fighting)
    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id != NPC_MUTANT_BUSH_CHICKEN || _grabbingChicken)
            return;

        _chickenGUID = guid;

        if (!me->IsInCombat())
        {
            me->StopMoving();
            me->GetMotionMaster()->MoveIdle();
        }

        _events.RescheduleEvent(EVENT_ETTIN_FORGET_CHICKEN, 30s);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_ETTIN_NOTICE_CHICKEN || _grabbingChicken)
            return;

        if (Creature* chicken = ObjectAccessor::GetCreature(*me, _chickenGUID))
        {
            me->SetFacingToObject(chicken);
            _events.ScheduleEvent(EVENT_ETTIN_GRAB_CHICKEN, 1s + 250ms);
        }
    }

    void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
    {
        if (!apply || !_grabbingChicken || passenger->GetGUID() != _chickenGUID)
            return;

        if (seatId == SEAT_ETTIN_ARM)
            _events.ScheduleEvent(EVENT_ETTIN_WALK_ON, 2s);
        else if (seatId == SEAT_ETTIN_MOUTH)
            _events.ScheduleEvent(EVENT_ETTIN_CHICKEN_BLASTS, 2s);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_ETTIN_BONK, 2s);
        _events.ScheduleEvent(EVENT_ETTIN_LOG_SMASH, 8s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ETTIN_BONK:
                    if (me->GetVictim())
                        DoCastVictim(SPELL_BONK);
                    _events.ScheduleEvent(EVENT_ETTIN_BONK, 9s, 11s);
                    break;
                case EVENT_ETTIN_LOG_SMASH:
                    if (me->GetVictim())
                        DoCastVictim(SPELL_LOG_SMASH);
                    _events.ScheduleEvent(EVENT_ETTIN_LOG_SMASH, 15s, 16s);
                    break;
                case EVENT_ETTIN_GRAB_CHICKEN:
                    // TrinityCore's Reverse Cast Ride Vehicle (83904): the chicken is made to ride the ettin's arm
                    if (Creature* chicken = ObjectAccessor::GetCreature(*me, _chickenGUID))
                    {
                        _grabbingChicken = true;
                        chicken->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED_CHICKEN, true);
                    }
                    break;
                case EVENT_ETTIN_WALK_ON:
                    if (!me->IsInCombat())
                        me->GetMotionMaster()->InitDefault();
                    _events.ScheduleEvent(EVENT_ETTIN_CHICKEN_TO_MOUTH, 2s);
                    break;
                case EVENT_ETTIN_CHICKEN_TO_MOUTH:
                    // Ettin's Mouth (83907) moves the chicken to the second seat
                    if (Creature* chicken = ObjectAccessor::GetCreature(*me, _chickenGUID))
                    {
                        chicken->ChangeSeat(SEAT_ETTIN_MOUTH);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                            if (player->GetQuestStatus(QUEST_ITS_ONLY_POISONOUS_IF_YOU_INGEST_IT) == QUEST_STATUS_INCOMPLETE)
                                chicken->PlayDirectSound(SOUND_CHICKEN_MOUNT_WOUND, player);
                    }
                    break;
                case EVENT_ETTIN_CHICKEN_BLASTS:
                    if (Creature* chicken = ObjectAccessor::GetCreature(*me, _chickenGUID))
                    {
                        chicken->CastSpell(chicken, SPELL_BUSH_EXPLOSION, true);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                            if (player->IsAlive() && !me->IsInCombat())
                                AttackStart(player);
                    }
                    break;
                case EVENT_ETTIN_FORGET_CHICKEN:
                    ForgetChicken();
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
    void ForgetChicken()
    {
        bool waiting = !_chickenGUID.IsEmpty() && !_grabbingChicken;

        _events.CancelEvent(EVENT_ETTIN_GRAB_CHICKEN);
        _events.CancelEvent(EVENT_ETTIN_WALK_ON);
        _events.CancelEvent(EVENT_ETTIN_CHICKEN_TO_MOUTH);
        _events.CancelEvent(EVENT_ETTIN_CHICKEN_BLASTS);
        _events.CancelEvent(EVENT_ETTIN_FORGET_CHICKEN);

        _grabbingChicken = false;
        _chickenGUID.Clear();
        _playerGUID.Clear();

        // It stopped for a chicken that never came
        if (waiting && !me->IsInCombat())
            me->GetMotionMaster()->InitDefault();
    }

    EventMap _events;
    ObjectGuid _chickenGUID;
    ObjectGuid _playerGUID;
    bool _grabbingChicken;
};

// 44935 - Mutant Bush Chicken
// Released from its cage at a Forest Ettin, it runs up to the ettin to be eaten. Nothing attacks it on the way.
struct npc_silverpine_mutant_bush_chicken : public ScriptedAI
{
    npc_silverpine_mutant_bush_chicken(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();

        if (Player* player = summoner->ToPlayer())
            if (player->GetQuestStatus(QUEST_ITS_ONLY_POISONOUS_IF_YOU_INGEST_IT) == QUEST_STATUS_INCOMPLETE)
                me->PlayDirectSound(SOUND_CHICKEN_MOUNT_WOUND, player);
    }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id != NPC_FOREST_ETTIN || !_ettinGUID.IsEmpty())
            return;

        _ettinGUID = guid;

        if (Creature* ettin = ObjectAccessor::GetCreature(*me, _ettinGUID))
            me->SetFacingToObject(ettin);

        _events.ScheduleEvent(EVENT_CHICKEN_MOVE_TO_ETTIN, 1s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE || pointId != POINT_CHICKEN_NEAR_ETTIN)
            return;

        Creature* ettin = ObjectAccessor::GetCreature(*me, _ettinGUID);
        if (!ettin)
            return;

        // The ettin may have walked on, or still be fighting
        if (me->GetDistance(ettin) > 5.0f)
            _events.ScheduleEvent(EVENT_CHICKEN_MOVE_TO_ETTIN, 500ms);
        else if (ettin->IsAIEnabled)
            ettin->AI()->DoAction(ACTION_ETTIN_NOTICE_CHICKEN);
    }

    void UpdateAI(uint32 diff) override
    {
        Unit* summoner = ObjectAccessor::GetUnit(*me, _summonerGUID);
        if (!summoner || !summoner->IsInWorld())
        {
            me->DespawnOrUnsummon(0);
            return;
        }

        _events.Update(diff);

        if (_events.ExecuteEvent() == EVENT_CHICKEN_MOVE_TO_ETTIN && !me->GetVehicle())
            if (Creature* ettin = ObjectAccessor::GetCreature(*me, _ettinGUID))
                me->GetMotionMaster()->MovePoint(POINT_CHICKEN_NEAR_ETTIN, ettin->GetPositionX(), ettin->GetPositionY(), ettin->GetPositionZ(), false);
    }

    bool IsBoundToEttin() const { return !_ettinGUID.IsEmpty(); }

private:
    EventMap _events;
    ObjectGuid _summonerGUID;
    ObjectGuid _ettinGUID;
};

// 83902 - Release Diseased Mutant Bush Chicken
// The chicken just released by the player is sent at the targeted Forest Ettin
class spell_silverpine_release_diseased_mutant_bush_chicken : public SpellScript
{
    PrepareSpellScript(spell_silverpine_release_diseased_mutant_bush_chicken);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Creature* ettin = GetHitCreature();
        if (!ettin || ettin->GetEntry() != NPC_FOREST_ETTIN || !ettin->IsAIEnabled)
            return;

        for (Creature* chicken : GetOwnedCreatures(caster, NPC_MUTANT_BUSH_CHICKEN, 10.0f))
        {
            auto chickenAI = CAST_AI(npc_silverpine_mutant_bush_chicken, chicken->AI());
            if (!chickenAI || chickenAI->IsBoundToEttin())
                continue;

            ettin->AI()->SetGUID(chicken->GetGUID(), NPC_MUTANT_BUSH_CHICKEN);
            chicken->AI()->SetGUID(ettin->GetGUID(), NPC_FOREST_ETTIN);
            break;
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_release_diseased_mutant_bush_chicken::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

enum LostInTheDarkness
{
    NPC_BLOODFANG_SCAVENGER                     = 44547,
    NPC_RABID_WORG                              = 1766,
    NPC_GIANT_RABID_BEAR                        = 1797,

    SPELL_FREE_WEBBED_VICTIM_RANDOM             = 83919,
    SPELL_FREE_WEBBED_VICTIM                    = 83927,
    SPELL_SINISTER_STRIKE                       = 60195,

    EVENT_SEA_DOG_FREED_TALK                    = 1,
    EVENT_SEA_DOG_SINISTER_STRIKE               = 2,

    TALK_SEA_DOG_FREED                          = 0
};

// 44941 - Webbed Victim
// Cutting a victim free during Lost in the Darkness frees an Orc Sea Dog half of the time; otherwise there is a foe in the web, or
// nothing. The spells are cast just before the killing blow, since they cannot target the dead web.
struct npc_silverpine_webbed_victim : public ScriptedAI
{
    npc_silverpine_webbed_victim(Creature* creature) : ScriptedAI(creature), _freed(false) { }

    void Reset() override
    {
        _freed = false;
        me->SetReactState(REACT_PASSIVE);
    }

    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (_freed || damage < me->GetHealth() || !attacker)
            return;

        _freed = true;

        Player* player = attacker->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (player && player->GetQuestStatus(QUEST_LOST_IN_THE_DARKNESS) == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(me, roll_chance_i(50) ? SPELL_FREE_WEBBED_VICTIM : SPELL_FREE_WEBBED_VICTIM_RANDOM, true);
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    bool _freed;
};

// 83919 - Free Webbed Victim
// A Bloodfang Scavenger, a Rabid Worg or a Giant Rabid Bear comes out of the web and attacks, or nothing does
class spell_silverpine_free_webbed_victim_random : public SpellScript
{
    PrepareSpellScript(spell_silverpine_free_webbed_victim_random);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* web = GetHitUnit();
        if (!web)
            return;

        static uint32 const foes[] = { NPC_BLOODFANG_SCAVENGER, NPC_RABID_WORG, NPC_GIANT_RABID_BEAR, 0 };
        uint32 entry = foes[urand(0, 3)];
        if (!entry)
            return;

        if (Creature* foe = caster->SummonCreature(entry, *web, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30s))
            if (foe->IsAIEnabled)
                foe->AI()->AttackStart(caster);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_free_webbed_victim_random::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 44942 - Orc Sea Dog
// Freed from a web: counts for Lost in the Darkness, follows the player and joins their fights. Leaves when the player dies or
// is gone, or when Admiral Hatchet takes the quest.
struct npc_silverpine_orc_sea_dog : public ScriptedAI
{
    npc_silverpine_orc_sea_dog(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 100);
        me->SetReactState(REACT_DEFENSIVE);
        _events.CancelEvent(EVENT_SEA_DOG_SINISTER_STRIKE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _playerGUID = player->GetGUID();
        player->KilledMonsterCredit(NPC_RESCUED_ORC_SEA_DOG);

        me->GetMotionMaster()->MoveFollow(player, 5.0f, frand(1.57f, 4.71f));
        _events.ScheduleEvent(EVENT_SEA_DOG_FREED_TALK, 1s + 500ms);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_SEA_DOG_SINISTER_STRIKE, 2s, 4s);
    }

    void UpdateAI(uint32 diff) override
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || !player->IsAlive())
        {
            me->DespawnOrUnsummon(0);
            return;
        }

        // Help the player
        if (!me->IsInCombat())
            if (Unit* victim = player->GetVictim())
                if (me->IsValidAttackTarget(victim))
                    AttackStart(victim);

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SEA_DOG_FREED_TALK:
                    Talk(TALK_SEA_DOG_FREED, player);
                    break;
                case EVENT_SEA_DOG_SINISTER_STRIKE:
                    if (me->GetVictim())
                        DoCastVictim(SPELL_SINISTER_STRIKE);
                    _events.ScheduleEvent(EVENT_SEA_DOG_SINISTER_STRIKE, 6s, 8s);
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
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## Fenris Isle: Rise, Forsaken (27097) and No Escape (27099)
######*/

enum FenrisIsle
{
    QUEST_RISE_FORSAKEN                         = 27097,
    QUEST_NO_ESCAPE                             = 27099,

    NPC_AGATHA_FENRIS                           = 44951,
    NPC_FENRIS_KEEP_STALKER                     = 45032,

    SPELL_AGATHA_BROADCAST                      = 83978,
    SPELL_BOND_OF_THE_VALKYR                    = 83979,
    SPELL_SUMMON_AGATHA_FENRIS                  = 83982,
    SPELL_RAISE_FORSAKEN_FENRIS                 = 83993,
    SPELL_DOOMHOWL                              = 84012,
    SPELL_UNHOLY_DARKNESS                       = 84013,
    SPELL_UNHOLY_SMITE                          = 84014,
    SPELL_RIDE_VEHICLE_HARDCODED_AGATHA         = 46598,

    TALK_AGATHA_BROADCAST                       = 0,
    TALK_AGATHA_RISE_FORSAKEN                   = 1,
    TALK_AGATHA_PRE_EVENT                       = 2,
    TALK_AGATHA_POST_EVENT1                     = 3,
    TALK_AGATHA_POST_EVENT2                     = 4
};

// Agatha is the player's guardian (Summon Agatha), so she is found through her summoner
static Creature* FindAgathaOf(Player* player, float range)
{
    std::list<Creature*> agathas;
    player->GetCreatureListWithEntryInGrid(agathas, NPC_AGATHA_FENRIS, range);
    for (Creature* agatha : agathas)
        if (agatha->IsAlive() && agatha->IsSummon() && agatha->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return agatha;
    return nullptr;
}

// 83979 - Bond of the Val'kyr
// Keeps Agatha with the player from Rise, Forsaken until No Escape is turned in
class spell_silverpine_bond_of_the_valkyr : public AuraScript
{
    PrepareAuraScript(spell_silverpine_bond_of_the_valkyr);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_AGATHA_FENRIS, SPELL_AGATHA_BROADCAST });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget()->IsInWorld())
            GetTarget()->CastSpell(GetTarget(), SPELL_SUMMON_AGATHA_FENRIS, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_SUMMON_AGATHA_FENRIS);
    }

    // The bond ticks every minute with Agatha Broadcast; check for Agatha every few seconds and broadcast on every 12th tick
    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    // Agatha is not saved with the player and is left behind by a flight or a teleport: bring her back after a login, her death,
    // the player's death or when the player is out of her reach
    void OnPeriodic(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        if (aurEff->GetTickNumber() % 12 == 0)
            player->CastSpell(player, SPELL_AGATHA_BROADCAST, true, nullptr, aurEff);

        if (!player->IsAlive() || player->IsInFlight() || player->GetVehicle())
            return;

        if (!FindAgathaOf(player, 50.0f))
            player->CastSpell(player, SPELL_SUMMON_AGATHA_FENRIS, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_silverpine_bond_of_the_valkyr::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_silverpine_bond_of_the_valkyr::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_silverpine_bond_of_the_valkyr::CalcPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_silverpine_bond_of_the_valkyr::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 44365 - Lady Sylvanas Windrunner
// Gives the player Agatha (Bond of the Val'kyr) on accepting Rise, Forsaken and dismisses her when No Escape is turned in. Her lines
// in The Warchief Cometh are played by Grand Executor Mortuus through this AI.
struct npc_silverpine_sylvanas_windrunner_high_command : public ScriptedAI
{
    npc_silverpine_sylvanas_windrunner_high_command(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_RISE_FORSAKEN && !player->HasAura(SPELL_BOND_OF_THE_VALKYR))
            player->CastSpell(player, SPELL_BOND_OF_THE_VALKYR, true);
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        // Despawn All Summons, done directly: without the bond Agatha leaves
        if (quest->GetQuestId() == QUEST_NO_ESCAPE)
            player->RemoveAurasDueToSpell(SPELL_BOND_OF_THE_VALKYR);
    }
};

Position const AgathaBackFrontyardPos = { 982.57f, 671.04f, 77.298f };

enum AgathaFenrisIsle
{
    SPELL_SUMMON_FENRIS_CAMERA                  = 84068,

    EVENT_AGATHA_CHECK_PLAYER_HEALTH            = 1,
    EVENT_UNHOLY_SMITE                          = 2,
    EVENT_DOOMHOWL                              = 3,
    EVENT_AGATHA_SUMMON_CAMERA                  = 4,
    EVENT_AGATHA_RUN_TALK                       = 5,
    EVENT_AGATHA_FLEE                           = 6,
    EVENT_AGATHA_ARRIVED                        = 7,
    EVENT_AGATHA_CHECK_CAMERA                   = 8,

    ACTION_AGATHA_NO_ESCAPE_START               = 1,
    ACTION_AGATHA_RUN_TALK                      = 2,
    ACTION_AGATHA_FLEE                          = 3,
    ACTION_AGATHA_RESET                         = 4,

    DATA_AGATHA_SCENE_STARTED                   = 1,

    PATH_AGATHA_TO_FORSAKEN                     = 449510,

    // Indexes in the path
    WAYPOINT_SPEED_UP                           = 13,
    WAYPOINT_ARRIVED_TO_FORSAKEN                = 18,

    POINT_AGATHA_BACK_FRONTYARD                 = 1
};

float const AgathaFleeSpeedRate     = 7.7937083f / 7.0f;
float const AgathaFastFleeSpeedRate = 15.880999f / 7.0f;
float const AgathaNormalSpeedRate   = 1.14286f;
float const AgathaChaseDistance     = 1.5f;

// 44951 - Agatha
// The player's guardian while Bond of the Val'kyr lasts: follows and helps the player, heals them, raises the Hillsbrad Refugees they
// kill (Rise, Forsaken), and in Fenris Keep leads the player into the meeting of the human leaders, then flies them back to
// Forsaken High Command (No Escape)
struct npc_silverpine_agatha_fenris_isle : public ScriptedAI
{
    npc_silverpine_agatha_fenris_isle(Creature* creature) : ScriptedAI(creature), _sceneStarted(false), _fleeing(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();
        me->SetReactState(REACT_ASSIST);
        FollowSummoner();
        _events.ScheduleEvent(EVENT_AGATHA_CHECK_PLAYER_HEALTH, 1s);
    }

    void Reset() override
    {
        _events.CancelEvent(EVENT_UNHOLY_SMITE);
        _events.CancelEvent(EVENT_DOOMHOWL);

        if (!_sceneStarted)
            FollowSummoner();
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_AGATHA_BROADCAST || _sceneStarted)
            return;

        if (Player* player = GetSummoner())
            Talk(TALK_AGATHA_BROADCAST, player);
    }

    uint32 GetData(uint32 type) const override
    {
        return type == DATA_AGATHA_SCENE_STARTED && _sceneStarted ? 1 : 0;
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_AGATHA_NO_ESCAPE_START:
                StartNoEscape();
                break;
            case ACTION_AGATHA_RUN_TALK:
                _events.ScheduleEvent(EVENT_AGATHA_RUN_TALK, 1s);
                break;
            case ACTION_AGATHA_FLEE:
                _events.ScheduleEvent(EVENT_AGATHA_FLEE, 1s);
                break;
            case ACTION_AGATHA_RESET:
                if (!_fleeing)
                    EndScene();
                break;
            default:
                break;
        }
    }

    // Her combat reach (0.6) is far smaller than her model, so a melee chase put her inside her target: keep to its back
    void AttackStart(Unit* who) override
    {
        if (who && me->Attack(who, true))
            me->GetMotionMaster()->MoveChase(who, AgathaChaseDistance, float(M_PI));
    }

    void OwnerAttacked(Unit* target) override
    {
        HelpOwner(target);
    }

    void OwnerAttackedBy(Unit* attacker) override
    {
        HelpOwner(attacker);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_UNHOLY_SMITE, 500ms);
        _events.ScheduleEvent(EVENT_DOOMHOWL, 2s, 3s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE || !_fleeing)
            return;

        if (pointId == WAYPOINT_SPEED_UP)
            SetFlightSpeedRate(AgathaFastFleeSpeedRate);
        else if (pointId == WAYPOINT_ARRIVED_TO_FORSAKEN)
            _events.ScheduleEvent(EVENT_AGATHA_ARRIVED, 100ms);
    }

    void UpdateAI(uint32 diff) override
    {
        Player* player = GetSummoner();
        if (!player || !player->IsInWorld() || !player->IsAlive() || !player->HasAura(SPELL_SUMMON_AGATHA_FENRIS))
        {
            if (player && player->GetVehicleBase() == me)
                player->ExitVehicle();
            me->DespawnOrUnsummon(0);
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_AGATHA_CHECK_PLAYER_HEALTH:
                    if (!_sceneStarted && !me->HasUnitState(UNIT_STATE_CASTING) && player->GetHealthPct() < 75.0f)
                        me->CastSpell(player, SPELL_UNHOLY_DARKNESS, false);
                    _events.ScheduleEvent(EVENT_AGATHA_CHECK_PLAYER_HEALTH, 1s);
                    break;
                case EVENT_DOOMHOWL:
                    DoCastVictim(SPELL_DOOMHOWL);
                    _events.ScheduleEvent(EVENT_DOOMHOWL, 14s, 18s);
                    break;
                case EVENT_UNHOLY_SMITE:
                    DoCastVictim(SPELL_UNHOLY_SMITE);
                    _events.ScheduleEvent(EVENT_UNHOLY_SMITE, 4s, 6s);
                    break;
                case EVENT_AGATHA_SUMMON_CAMERA:
                    // Force Cast Fenris Camera, done directly: the player sits in the camera, which plays the meeting
                    player->CastSpell(player, SPELL_SUMMON_FENRIS_CAMERA, true);
                    _events.ScheduleEvent(EVENT_AGATHA_CHECK_CAMERA, 3s);
                    break;
                case EVENT_AGATHA_CHECK_CAMERA:
                    // The player did not get into the camera: wait for the next try
                    if (!player->GetVehicle())
                        EndScene();
                    break;
                case EVENT_AGATHA_RUN_TALK:
                    me->SetFacingTo(3.159046f);
                    Talk(TALK_AGATHA_POST_EVENT1, player);
                    break;
                case EVENT_AGATHA_FLEE:
                    // Reverse Cast Ride Vehicle, done directly
                    _fleeing = true;
                    player->ExitVehicle();
                    player->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED_AGATHA, true);
                    Talk(TALK_AGATHA_POST_EVENT2, player);
                    player->KilledMonsterCredit(NPC_AGATHA_FENRIS);
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    SetFlightSpeedRate(AgathaFleeSpeedRate);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePath(PATH_AGATHA_TO_FORSAKEN, false);
                    break;
                case EVENT_AGATHA_ARRIVED:
                    if (player->GetVehicleBase() == me)
                        player->ExitVehicle();
                    _fleeing = false;
                    SetFlightSpeedRate(AgathaNormalSpeedRate);
                    EndScene();
                    break;
                default:
                    break;
            }
        }

        // She is an ally summon, not one of the player's controlled units, so OwnerAttacked/OwnerAttackedBy never reach her:
        // join the player's fight from here
        if (!me->GetVictim())
        {
            if (Unit* target = player->GetVictim())
                HelpOwner(target);
            else if (!player->getAttackers().empty())
                HelpOwner(*player->getAttackers().begin());
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    Player* GetSummoner() const
    {
        return ObjectAccessor::GetPlayer(*me, _summonerGUID);
    }

    void FollowSummoner()
    {
        if (Player* player = GetSummoner())
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(player, 3.0f, float(M_PI / 2.0f));
        }
    }

    void HelpOwner(Unit* target)
    {
        if (!_sceneStarted && target && !me->GetVictim() && me->IsValidAttackTarget(target))
            AttackStart(target);
    }

    void SetFlightSpeedRate(float rate)
    {
        me->SetSpeed(MOVE_RUN, rate);
        me->SetSpeed(MOVE_FLIGHT, rate);
    }

    void StartNoEscape()
    {
        if (_sceneStarted)
            return;

        _sceneStarted = true;

        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(POINT_AGATHA_BACK_FRONTYARD, AgathaBackFrontyardPos, false);

        if (Player* player = GetSummoner())
            Talk(TALK_AGATHA_PRE_EVENT, player);

        // The player has just been moved to the back of the frontyard
        _events.ScheduleEvent(EVENT_AGATHA_SUMMON_CAMERA, 1s);
    }

    void EndScene()
    {
        _sceneStarted = false;
        _events.CancelEvent(EVENT_AGATHA_SUMMON_CAMERA);
        _events.CancelEvent(EVENT_AGATHA_CHECK_CAMERA);
        _events.CancelEvent(EVENT_AGATHA_RUN_TALK);
        _events.CancelEvent(EVENT_AGATHA_FLEE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_ASSIST);
        FollowSummoner();
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
    bool _sceneStarted;
    bool _fleeing;
};

enum SpellForsakenTrooperMasterScriptFenrisIsle
{
    SPELL_FORSAKEN_TROOPER_MALE_01_F            = 83998,
    SPELL_FORSAKEN_TROOPER_MALE_02_F            = 83999,
    SPELL_FORSAKEN_TROOPER_MALE_03_F            = 84000,
    SPELL_FORSAKEN_TROOPER_MALE_04_F            = 84001,
    SPELL_FORSAKEN_TROOPER_FEMALE_01_F          = 84002,
    SPELL_FORSAKEN_TROOPER_FEMALE_02_F          = 84003,
    SPELL_FORSAKEN_TROOPER_FEMALE_03_F          = 84004,
    SPELL_FORSAKEN_TROOPER_FEMALE_04_F          = 84005,

    DISPLAY_MALE_01_F                           = 33978,
    DISPLAY_MALE_02_F                           = 33979,
    DISPLAY_MALE_03_F                           = 33980,
    DISPLAY_MALE_04_F                           = 33981,
    DISPLAY_FEMALE_01_F                         = 33982,
    DISPLAY_FEMALE_02_F                         = 33983,
    DISPLAY_FEMALE_03_F                         = 33984,
    DISPLAY_FEMALE_04_F                         = 33985,

    DISPLAY_MALE_01_D_F                         = 33986,
    DISPLAY_MALE_02_D_F                         = 33987,
    DISPLAY_MALE_03_D_F                         = 33988,
    DISPLAY_MALE_04_D_F                         = 33989,
    DISPLAY_FEMALE_01_D_F                       = 33991,
    DISPLAY_FEMALE_02_D_F                       = 33992,
    DISPLAY_FEMALE_03_D_F                       = 33993,
    DISPLAY_FEMALE_04_D_F                       = 33994
};

// 83997 - Forsaken Trooper Master Script (Fenris Isle)
// Raises a Hillsbrad Refugee as the Forsaken Trooper that matches its human model
class spell_silverpine_forsaken_trooper_masterscript_fenris_isle : public SpellScript
{
    PrepareSpellScript(spell_silverpine_forsaken_trooper_masterscript_fenris_isle);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_FORSAKEN_TROOPER_MALE_01_F,
            SPELL_FORSAKEN_TROOPER_MALE_02_F,
            SPELL_FORSAKEN_TROOPER_MALE_03_F,
            SPELL_FORSAKEN_TROOPER_MALE_04_F,
            SPELL_FORSAKEN_TROOPER_FEMALE_01_F,
            SPELL_FORSAKEN_TROOPER_FEMALE_02_F,
            SPELL_FORSAKEN_TROOPER_FEMALE_03_F,
            SPELL_FORSAKEN_TROOPER_FEMALE_04_F
        });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        uint32 spellId = caster->GetGender() == GENDER_MALE ? SPELL_FORSAKEN_TROOPER_MALE_01_F : SPELL_FORSAKEN_TROOPER_FEMALE_01_F;
        switch (caster->GetDisplayId())
        {
            case DISPLAY_MALE_01_F:   spellId = SPELL_FORSAKEN_TROOPER_MALE_01_F;   break;
            case DISPLAY_MALE_02_F:   spellId = SPELL_FORSAKEN_TROOPER_MALE_02_F;   break;
            case DISPLAY_MALE_03_F:   spellId = SPELL_FORSAKEN_TROOPER_MALE_03_F;   break;
            case DISPLAY_MALE_04_F:   spellId = SPELL_FORSAKEN_TROOPER_MALE_04_F;   break;
            case DISPLAY_FEMALE_01_F: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_01_F; break;
            case DISPLAY_FEMALE_02_F: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_02_F; break;
            case DISPLAY_FEMALE_03_F: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_03_F; break;
            case DISPLAY_FEMALE_04_F: spellId = SPELL_FORSAKEN_TROOPER_FEMALE_04_F; break;
            default: break;
        }

        caster->CastSpell(caster, spellId, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_forsaken_trooper_masterscript_fenris_isle::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum HillsbradRefugee
{
    SPELL_TROOPER_MASTERSCRIPT_FENRIS           = 83997,
    SPELL_MIGHT_OF_LORDAERON                    = 87104,

    EVENT_MIGHT_OF_LORDAERON                    = 1
};

// 44954, 44966 - Hillsbrad Refugee
// Killed by a player on Rise, Forsaken, it is raised by the player's Agatha (Notify Agatha, done directly), whose Raise Forsaken
// gives the player the Forsaken raised credit
struct npc_silverpine_hillsbrad_refugee : public ScriptedAI
{
    npc_silverpine_hillsbrad_refugee(Creature* creature) : ScriptedAI(creature), _raised(false) { }

    void Reset() override
    {
        _events.Reset();
        _raised = false;
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_RAISE_FORSAKEN_FENRIS || _raised)
            return;

        _raised = true;
        DoCast(me, SPELL_TROOPER_MASTERSCRIPT_FENRIS, true);
        me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
    }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_RISE_FORSAKEN) != QUEST_STATUS_INCOMPLETE)
            return;

        Creature* agatha = FindAgathaOf(player, 50.0f);
        if (!agatha || !agatha->IsAIEnabled || agatha->AI()->GetData(DATA_AGATHA_SCENE_STARTED))
            return;

        agatha->CastSpell(me, SPELL_RAISE_FORSAKEN_FENRIS, true);

        if (roll_chance_i(50))
            agatha->AI()->Talk(TALK_AGATHA_RISE_FORSAKEN, player);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_MIGHT_OF_LORDAERON, 3s, 8s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MIGHT_OF_LORDAERON:
                    DoCast(me, SPELL_MIGHT_OF_LORDAERON);
                    _events.ScheduleEvent(EVENT_MIGHT_OF_LORDAERON, 15s, 18s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    bool _raised;
};

enum ForsakenTrooperFenrisIsle
{
    EVENT_REFUGEE_JUST_RISEN                    = 1,

    TALK_REFUGEE_JUST_RISEN                     = 0
};

// 44958, 44959, 44960, 44961 - Forsaken Trooper (Male)
// 44962, 44963, 44964, 44965 - Forsaken Trooper (Female)
// A raised Hillsbrad Refugee takes its model, speaks and leaves
struct npc_silverpine_forsaken_trooper_fenris_isle : public ScriptedAI
{
    npc_silverpine_forsaken_trooper_fenris_isle(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Creature* refugee = summoner->ToCreature();
        if (!refugee)
            return;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_PASSIVE);

        uint32 displayId = me->GetGender() == GENDER_MALE ? DISPLAY_MALE_01_D_F : DISPLAY_FEMALE_01_D_F;
        switch (refugee->GetDisplayId())
        {
            case DISPLAY_MALE_01_F:   displayId = DISPLAY_MALE_01_D_F;   break;
            case DISPLAY_MALE_02_F:   displayId = DISPLAY_MALE_02_D_F;   break;
            case DISPLAY_MALE_03_F:   displayId = DISPLAY_MALE_03_D_F;   break;
            case DISPLAY_MALE_04_F:   displayId = DISPLAY_MALE_04_D_F;   break;
            case DISPLAY_FEMALE_01_F: displayId = DISPLAY_FEMALE_01_D_F; break;
            case DISPLAY_FEMALE_02_F: displayId = DISPLAY_FEMALE_02_D_F; break;
            case DISPLAY_FEMALE_03_F: displayId = DISPLAY_FEMALE_03_D_F; break;
            case DISPLAY_FEMALE_04_F: displayId = DISPLAY_FEMALE_04_D_F; break;
            default: break;
        }

        me->SetDisplayId(displayId);

        _events.ScheduleEvent(EVENT_REFUGEE_JUST_RISEN, 1s);

        me->DespawnOrUnsummon(4 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_REFUGEE_JUST_RISEN)
                Talk(TALK_REFUGEE_JUST_RISEN);
        }
    }

private:
    EventMap _events;
};

enum WorgenSentry
{
    SPELL_BATTLE_ROAR                           = 6507,
    SPELL_UNDYING_FRENZY                        = 80515,

    EVENT_UNDYING_FRENZY                        = 1,

    TALK_SENTRY_WARNING                         = 0
};

// 44987 - Worgen Sentry
struct npc_silverpine_worgen_sentry : public ScriptedAI
{
    npc_silverpine_worgen_sentry(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(TALK_SENTRY_WARNING);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        DoCast(me, SPELL_BATTLE_ROAR);
        _events.ScheduleEvent(EVENT_UNDYING_FRENZY, 3s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_UNDYING_FRENZY:
                    DoCastVictim(SPELL_UNDYING_FRENZY);
                    _events.ScheduleEvent(EVENT_UNDYING_FRENZY, 10s, 12s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

Position const NoEscapeStartPos = { 981.782f, 670.953f, 74.898f, 3.1887f };

// 6230 - No Escape
// Entering Fenris Keep with Agatha on No Escape starts the meeting: the player is moved to the back of the frontyard and Agatha
// leads on. The Fenris Keep Stalker's Force Cast Fenris Camera is done by Agatha, for each player.
class at_silverpine_no_escape : public AreaTriggerScript
{
public:
    at_silverpine_no_escape() : AreaTriggerScript("at_silverpine_no_escape") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (!player->IsAlive() || player->GetVehicle() || player->GetQuestStatus(QUEST_NO_ESCAPE) != QUEST_STATUS_INCOMPLETE)
            return true;

        Creature* agatha = FindAgathaOf(player, 10.0f);
        if (!agatha || !agatha->IsAIEnabled || agatha->AI()->GetData(DATA_AGATHA_SCENE_STARTED))
            return true;

        player->NearTeleportTo(NoEscapeStartPos.GetPositionX(), NoEscapeStartPos.GetPositionY(), NoEscapeStartPos.GetPositionZ(),
            NoEscapeStartPos.GetOrientation(), true);
        agatha->AI()->DoAction(ACTION_AGATHA_NO_ESCAPE_START);
        return true;
    }
};

Position const CameraFrontyardPos = { 980.7f, 689.14f, 76.9f };

enum FenrisKeepCamera
{
    NPC_CROWLEY_FENRIS                          = 44989,
    NPC_BLOODFANG_FENRIS                        = 44990,
    NPC_PHIN_ODELIC                             = 44993,
    NPC_BARTOLO_GINSETTI                        = 44994,
    NPC_LOREMASTER_DIBBS                        = 44995,
    NPC_MAGISTRATE_HENRY_MALEB                  = 44996,
    NPC_CARETAKER_SMITHERS                      = 44997,
    NPC_SOPHIA_ZWOSKI                           = 45002,

    SPELL_SUMMON_BLOODFANG_FENRIS               = 84054,
    SPELL_SUMMON_CROWLEY_FENRIS                 = 84055,
    SPELL_SUMMON_PHIN_ODELIC                    = 84056,
    SPELL_SUMMON_BARTOLO_GINSETTI               = 84057,
    SPELL_SUMMON_LOREMASTER_DIBBS               = 84058,
    SPELL_SUMMON_MAGISTRATE_HENRY               = 84059,
    SPELL_SUMMON_CARETAKER_SMITHERS             = 84060,
    SPELL_SUMMON_SOPHIA                         = 84061,
    SPELL_BLOOD_ODELIC                          = 84094,
    SPELL_BLOOD_BARTOLO                         = 84095,
    SPELL_BLOOD_DIBBS                           = 84096,
    SPELL_BLOOD_HENRY                           = 84097,
    SPELL_BLOOD_SMITHERS                        = 84098,
    SPELL_BLOOD_ZWOSKI                          = 84099,

    SOUND_HOWLING                               = 17671,

    // The scene steps follow each other from 1 up to EVENT_SCENE_FENRIS_FINISH
    EVENT_SCENE_FENRIS_START                    = 1,
    EVENT_SCENE_FENRIS_CHANGE_SEAT              = 2,
    EVENT_SCENE_FENRIS_MORPH                    = 14,
    EVENT_SCENE_FENRIS_AGATHA_TALK              = 16,
    EVENT_SCENE_FENRIS_FINISH                   = 18,

    TALK_CROWLEY_NO_ESCAPE_0                    = 0,
    TALK_CROWLEY_NO_ESCAPE_1                    = 1,
    TALK_CROWLEY_NO_ESCAPE_2                    = 2,
    TALK_CROWLEY_NO_ESCAPE_3                    = 3,
    TALK_CROWLEY_NO_ESCAPE_4                    = 4,
    TALK_CROWLEY_NO_ESCAPE_5                    = 5,
    TALK_HENRY_NO_ESCAPE_0                      = 0,
    TALK_HENRY_NO_ESCAPE_1                      = 1,

    POINT_CAMERA_FRONTYARD                      = 1,

    SEAT_FENRIS_CAMERA                          = 0,
    SEAT_FENRIS_CAMERA_FORCE                    = 1,

    FENRIS_ACTOR_COUNT                          = 8
};

uint32 const FenrisActorSummonSpells[FENRIS_ACTOR_COUNT] =
{
    SPELL_SUMMON_CROWLEY_FENRIS, SPELL_SUMMON_BLOODFANG_FENRIS, SPELL_SUMMON_PHIN_ODELIC, SPELL_SUMMON_BARTOLO_GINSETTI,
    SPELL_SUMMON_LOREMASTER_DIBBS, SPELL_SUMMON_MAGISTRATE_HENRY, SPELL_SUMMON_CARETAKER_SMITHERS, SPELL_SUMMON_SOPHIA
};

uint32 const FenrisActorEntries[FENRIS_ACTOR_COUNT] =
{
    NPC_CROWLEY_FENRIS, NPC_BLOODFANG_FENRIS, NPC_PHIN_ODELIC, NPC_BARTOLO_GINSETTI,
    NPC_LOREMASTER_DIBBS, NPC_MAGISTRATE_HENRY_MALEB, NPC_CARETAKER_SMITHERS, NPC_SOPHIA_ZWOSKI
};

// Blood of the Worgen of the six humans, in the order above (none for Crowley and Bloodfang)
uint32 const FenrisActorBloodSpells[FENRIS_ACTOR_COUNT] =
{
    0, 0, SPELL_BLOOD_ODELIC, SPELL_BLOOD_BARTOLO, SPELL_BLOOD_DIBBS, SPELL_BLOOD_HENRY, SPELL_BLOOD_SMITHERS, SPELL_BLOOD_ZWOSKI
};

enum FenrisActorIndex
{
    ACTOR_CROWLEY                               = 0,
    ACTOR_HENRY                                 = 5,
    ACTOR_SOPHIA                                = 7
};

// 45003 - Fenris Keep Camera
// Summoned with the player in it (Summon Fenris Camera). Summons the human leaders, Crowley and Bloodfang (Summon Fenris Actors, done
// directly), moves to the frontyard and plays the meeting: Crowley offers the humans his blood, they choose to become worgen, and
// Agatha flies the player out, which completes No Escape. TrinityCore passes the steps along with General Trigger, Conversation
// Trigger and Armoire Camera spells; the camera runs them itself here, as all of the actors are the player's personal summons.
struct npc_silverpine_fenris_keep_camera : public VehicleAI
{
    npc_silverpine_fenris_keep_camera(Creature* creature) : VehicleAI(creature), _sceneStarted(false), _agathaTookOver(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _playerGUID = summoner->GetGUID();
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_PASSIVE);
    }

    void PassengerBoarded(Unit* passenger, int8 seatId, bool apply) override
    {
        if (!apply || _sceneStarted || seatId != SEAT_FENRIS_CAMERA || passenger->GetGUID() != _playerGUID)
            return;

        _sceneStarted = true;
        _events.ScheduleEvent(EVENT_SCENE_FENRIS_START, 10ms);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_CAMERA_FRONTYARD)
            _events.ScheduleEvent(EVENT_SCENE_FENRIS_CHANGE_SEAT, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        if (!_sceneStarted)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || (!_agathaTookOver && player->GetVehicleBase() != me))
        {
            // The player left before the end: Agatha waits for the next try
            if (player)
                if (Creature* agatha = FindAgathaOf(player, 100.0f))
                    if (agatha->IsAIEnabled)
                        agatha->AI()->DoAction(ACTION_AGATHA_RESET);
            EndScene();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_SCENE_FENRIS_START)
            {
                // The eight spells of Summon Fenris Actors, cast directly. The actors are the player's personal summons, so they
                // are found from the player's side (only the player's own: group members play the same scene at the same spot).
                for (uint8 i = 0; i < FENRIS_ACTOR_COUNT; ++i)
                {
                    player->CastSpell(player, FenrisActorSummonSpells[i], true);
                    _actorGUIDs[i] = FindOwnSummon(player, FenrisActorEntries[i]);
                }

                me->GetMotionMaster()->MovePoint(POINT_CAMERA_FRONTYARD, CameraFrontyardPos, false);
                continue;
            }

            Creature* actors[FENRIS_ACTOR_COUNT];
            bool actorMissing = false;
            for (uint8 i = 0; i < FENRIS_ACTOR_COUNT; ++i)
            {
                actors[i] = ObjectAccessor::GetCreature(*me, _actorGUIDs[i]);
                if (!actors[i] || !actors[i]->IsAIEnabled)
                    actorMissing = true;
            }

            Creature* agatha = FindAgathaOf(player, 100.0f);
            if (actorMissing || !agatha || !agatha->IsAIEnabled)
            {
                // An actor is missing: let the player out, which ends the scene
                _events.Reset();
                if (!_agathaTookOver)
                    player->ExitVehicle();
                return;
            }

            Creature* crowley = actors[ACTOR_CROWLEY];
            Creature* henry = actors[ACTOR_HENRY];

            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_SCENE_FENRIS_CHANGE_SEAT:
                    // Seat 02
                    player->ChangeSeat(SEAT_FENRIS_CAMERA_FORCE);
                    next = 2s;
                    break;
                case 3:
                    next = 2s + 500ms;
                    break;
                case 4:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_0, player);
                    next = 4s + 700ms;
                    break;
                case 5:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_1, player);
                    next = 4s + 700ms;
                    break;
                case 6:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_2, player);
                    next = 4s + 700ms;
                    break;
                case 7:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_3, player);
                    next = 4s + 700ms;
                    break;
                case 8:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_4, player);
                    next = 6s + 100ms;
                    break;
                case 9:
                    crowley->AI()->Talk(TALK_CROWLEY_NO_ESCAPE_5, player);
                    next = 9s + 500ms;
                    break;
                case 10:
                    next = 1s + 300ms;
                    break;
                case 11:
                    henry->AI()->Talk(TALK_HENRY_NO_ESCAPE_0, player);
                    next = 10s + 800ms;
                    break;
                case 12:
                    henry->AI()->Talk(TALK_HENRY_NO_ESCAPE_1, player);
                    next = 2s;
                    break;
                case 13:
                    // The humans drink Crowley's blood
                    next = 2s + 500ms;
                    break;
                case EVENT_SCENE_FENRIS_MORPH:
                    for (uint8 i = 0; i < FENRIS_ACTOR_COUNT; ++i)
                        if (FenrisActorBloodSpells[i])
                            actors[i]->CastSpell(actors[i], FenrisActorBloodSpells[i], false);
                    actors[ACTOR_SOPHIA]->PlayDirectSound(SOUND_HOWLING, player);
                    next = 1s;
                    break;
                case 15:
                    for (uint8 i = 0; i < FENRIS_ACTOR_COUNT; ++i)
                        if (FenrisActorBloodSpells[i])
                            actors[i]->SetEmoteState(EMOTE_STATE_READY_UNARMED);
                    next = 2s;
                    break;
                case EVENT_SCENE_FENRIS_AGATHA_TALK:
                    agatha->AI()->DoAction(ACTION_AGATHA_RUN_TALK);
                    next = 2s + 500ms;
                    break;
                case 17:
                    // Agatha takes the player away a second later
                    _agathaTookOver = true;
                    agatha->AI()->DoAction(ACTION_AGATHA_FLEE);
                    me->SetFacingTo(0.08278348f);
                    next = 4s;
                    break;
                case EVENT_SCENE_FENRIS_FINISH:
                    EndScene();
                    return;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    // Despawn All Summons, done directly
    void EndScene()
    {
        _events.Reset();
        for (uint8 i = 0; i < FENRIS_ACTOR_COUNT; ++i)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, _actorGUIDs[i]))
                actor->DespawnOrUnsummon(0);
        me->DespawnOrUnsummon(0);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _actorGUIDs[FENRIS_ACTOR_COUNT];
    bool _sceneStarted;
    bool _agathaTookOver;
};

// 44989 - Lord Darius Crowley, 44990 - Packleader Ivar Bloodfang, 44993 - Phin Odelic, 44994 - Bartolo Ginsetti,
// 44995 - Loremaster Dibbs, 44996 - Magistrate Henry Maleb, 44997 - Caretaker Smithers, 45002 - Sophia Zwoski
// The actors of the Fenris Keep meeting, played by the camera
struct npc_silverpine_fenris_keep_actor : public ScriptedAI
{
    npc_silverpine_fenris_keep_actor(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_PASSIVE);

        // The humans face the keep's hall, where Crowley and Bloodfang stand
        if (me->GetEntry() != NPC_CROWLEY_FENRIS && me->GetEntry() != NPC_BLOODFANG_FENRIS)
            if (Creature* fenrisStalker = me->FindNearestCreature(NPC_FENRIS_KEEP_STALKER, 50.0f))
                me->SetFacingToObject(fenrisStalker);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

/*######
## AddSC
######*/

void AddSC_silverpine_forest()
{
    new npc_deathstalker_erland();
    new creature_script<npc_fenwick_thatros>("npc_fenwick_thatros");
    new creature_script<npc_silverpine_grand_executor_mortuus>("npc_silverpine_grand_executor_mortuus");
    new aura_script<spell_silverpine_raise_forsaken_83173>("spell_silverpine_raise_forsaken_83173");
    new creature_script<npc_silverpine_fallen_human>("npc_silverpine_fallen_human");
    new spell_script<spell_silverpine_forsaken_trooper_masterscript_high_command>("spell_silverpine_forsaken_trooper_masterscript_high_command");
    new creature_script<npc_silverpine_bat_handler_maggotbreath>("npc_silverpine_bat_handler_maggotbreath");
    new creature_script<npc_silverpine_forsaken_bat>("npc_silverpine_forsaken_bat");
    new spell_script<spell_silverpine_go_home>("spell_silverpine_go_home");
    RegisterGameObjectAI(go_silverpine_abandoned_outhouse);
    new creature_script<npc_silverpine_deathstalker_rane_yorick>("npc_silverpine_deathstalker_rane_yorick");
    new creature_script<npc_silverpine_armoire>("npc_silverpine_armoire");
    new creature_script<npc_silverpine_lord_darius_crowley_exsanguinate>("npc_silverpine_lord_darius_crowley_exsanguinate");
    new creature_script<npc_silverpine_packleader_ivar_bloodfang_exsanguinate>("npc_silverpine_packleader_ivar_bloodfang_exsanguinate");
    new aura_script<spell_silverpine_hide_in_armoire>("spell_silverpine_hide_in_armoire");
    new creature_script<npc_silverpine_salty_rocka>("npc_silverpine_salty_rocka");
    new at_silverpine_forsaken_rear_guard();
    new creature_script<npc_silverpine_apothecary_wormcrud>("npc_silverpine_apothecary_wormcrud");
    new creature_script<npc_silverpine_admiral_hatchet>("npc_silverpine_admiral_hatchet");
    new creature_script<npc_silverpine_orc_sea_pup>("npc_silverpine_orc_sea_pup");
    new creature_script<npc_silverpine_orc_crate>("npc_silverpine_orc_crate");
    new spell_script<spell_silverpine_pick_up_orc_crate>("spell_silverpine_pick_up_orc_crate");
    new creature_script<npc_silverpine_forest_ettin>("npc_silverpine_forest_ettin");
    new creature_script<npc_silverpine_mutant_bush_chicken>("npc_silverpine_mutant_bush_chicken");
    new spell_script<spell_silverpine_release_diseased_mutant_bush_chicken>("spell_silverpine_release_diseased_mutant_bush_chicken");
    new creature_script<npc_silverpine_webbed_victim>("npc_silverpine_webbed_victim");
    new spell_script<spell_silverpine_free_webbed_victim_random>("spell_silverpine_free_webbed_victim_random");
    new creature_script<npc_silverpine_orc_sea_dog>("npc_silverpine_orc_sea_dog");
    new aura_script<spell_silverpine_bond_of_the_valkyr>("spell_silverpine_bond_of_the_valkyr");
    new creature_script<npc_silverpine_sylvanas_windrunner_high_command>("npc_silverpine_sylvanas_windrunner_high_command");
    new creature_script<npc_silverpine_agatha_fenris_isle>("npc_silverpine_agatha_fenris_isle");
    new spell_script<spell_silverpine_forsaken_trooper_masterscript_fenris_isle>("spell_silverpine_forsaken_trooper_masterscript_fenris_isle");
    new creature_script<npc_silverpine_hillsbrad_refugee>("npc_silverpine_hillsbrad_refugee");
    new creature_script<npc_silverpine_forsaken_trooper_fenris_isle>("npc_silverpine_forsaken_trooper_fenris_isle");
    new creature_script<npc_silverpine_worgen_sentry>("npc_silverpine_worgen_sentry");
    new at_silverpine_no_escape();
    new creature_script<npc_silverpine_fenris_keep_camera>("npc_silverpine_fenris_keep_camera");
    new creature_script<npc_silverpine_fenris_keep_actor>("npc_silverpine_fenris_keep_actor");
}
