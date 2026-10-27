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

enum SkitterwebMatriarch
{
    SPELL_SKITTERWEB                            = 83827,
    SPELL_SUMMON_SPIDERLINGS                    = 87084,
    SPELL_VENOM_SPLASH                          = 79607,

    EVENT_MATRIARCH_DROP                        = 1,
    EVENT_MATRIARCH_LAND                        = 2,
    EVENT_MATRIARCH_FIGHT                       = 3,
    EVENT_MATRIARCH_BACK_ON_CEILING             = 4,
    EVENT_MATRIARCH_SUMMON_SPIDERLINGS          = 5,
    EVENT_MATRIARCH_VENOM_SPLASH                = 6,

    ANIMKIT_MATRIARCH_INTERACT                  = 1,
    ANIMKIT_MATRIARCH_LURKING_ON_CEILING        = 865,
    ANIMKIT_MATRIARCH_HANGING_BY_WEB            = 866
};

// 44906 - Skitterweb Matriarch
// Lurks on the ceiling of the Skittering Dark and drops down on whoever engages her; climbs back up when she gets home. She needs
// CREATURE_FLAG_EXTRA_NO_MOVE_FLAGS_UPDATE, or the movement flag update gives her gravity back.
struct npc_silverpine_skitterweb_matriarch : public ScriptedAI
{
    npc_silverpine_skitterweb_matriarch(Creature* creature) : ScriptedAI(creature), _onCeiling(false) { }

    void JustAppeared() override
    {
        me->SetDisableGravity(true);
        me->SetHover(true);
        _ceilingPos = me->GetPosition();
        _onCeiling = true;

        me->SetAIAnimKitId(ANIMKIT_MATRIARCH_LURKING_ON_CEILING);
        me->CastSpell(me, SPELL_SKITTERWEB, true);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void JustReachedHome() override
    {
        me->SetDisableGravity(true);
        me->SetHover(true);
        me->CastSpell(me, SPELL_SKITTERWEB, true);

        me->SetAIAnimKitId(ANIMKIT_MATRIARCH_HANGING_BY_WEB);
        me->GetMotionMaster()->MoveJump(_ceilingPos, 8.0f, 8.0f);

        _events.ScheduleEvent(EVENT_MATRIARCH_BACK_ON_CEILING, 1s + 500ms);
    }

    // On the ceiling she must not chase anyone through the air
    void AttackStart(Unit* who) override
    {
        if (_onCeiling)
            AttackStartNoMove(who);
        else
            ScriptedAI::AttackStart(who);
    }

    // Creature::CanStartAttack turns down anyone far below a creature that cannot fly, and the ceiling can block line of sight,
    // so she drops on whoever comes within her aggro radius by herself
    void MoveInLineOfSight(Unit* who) override
    {
        if (!_onCeiling || me->IsInCombat())
        {
            ScriptedAI::MoveInLineOfSight(who);
            return;
        }

        if (who->IsControlledByPlayer() && me->IsHostileTo(who) && me->IsValidAttackTarget(who)
            && me->IsWithinDist2d(who, me->GetAttackDistance(who)))
            AttackStart(who);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (!_onCeiling)
        {
            ScheduleCombatEvents();
            return;
        }

        me->SetAIAnimKitId(ANIMKIT_RESET);
        me->PlayOneShotAnimKitId(ANIMKIT_MATRIARCH_HANGING_BY_WEB);
        _events.ScheduleEvent(EVENT_MATRIARCH_DROP, 3s + 500ms);
    }

    void JustSummoned(Creature* summon) override
    {
        if (Unit* victim = me->GetVictim())
            if (summon->IsAIEnabled)
                summon->AI()->AttackStart(victim);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MATRIARCH_DROP:
                    me->SetDisableGravity(false);
                    me->SetHover(false);
                    me->GetMotionMaster()->MoveFall();
                    _events.ScheduleEvent(EVENT_MATRIARCH_LAND, 1s);
                    break;
                case EVENT_MATRIARCH_LAND:
                    me->PlayOneShotAnimKitId(ANIMKIT_MATRIARCH_INTERACT);
                    me->SetHomePosition(me->GetPosition());
                    _onCeiling = false;
                    _events.ScheduleEvent(EVENT_MATRIARCH_FIGHT, 1s + 500ms);
                    break;
                case EVENT_MATRIARCH_FIGHT:
                    if (Unit* victim = me->GetVictim())
                        ScriptedAI::AttackStart(victim);
                    ScheduleCombatEvents();
                    break;
                case EVENT_MATRIARCH_BACK_ON_CEILING:
                    me->SetFacingTo(_ceilingPos.GetOrientation());
                    me->SetHomePosition(_ceilingPos);
                    me->SetAIAnimKitId(ANIMKIT_MATRIARCH_LURKING_ON_CEILING);
                    _onCeiling = true;
                    break;
                case EVENT_MATRIARCH_SUMMON_SPIDERLINGS:
                    DoCastSelf(SPELL_SUMMON_SPIDERLINGS);
                    break;
                case EVENT_MATRIARCH_VENOM_SPLASH:
                    DoCastVictim(SPELL_VENOM_SPLASH);
                    _events.ScheduleEvent(EVENT_MATRIARCH_VENOM_SPLASH, 15s, 18s);
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        if (!_onCeiling)
            DoMeleeAttackIfReady();
    }

private:
    void ScheduleCombatEvents()
    {
        _events.ScheduleEvent(EVENT_MATRIARCH_SUMMON_SPIDERLINGS, 5s);
        _events.ScheduleEvent(EVENT_MATRIARCH_VENOM_SPLASH, 9s, 14s);
    }

    EventMap _events;
    Position _ceilingPos;
    bool _onCeiling;
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

enum Lordaeron
{
    QUEST_LORDAERON                             = 27098,

    NPC_SYLVANAS_LORDAERON                      = 45051,
    NPC_FORSAKEN_WARHORSE_LORDAERON             = 45057,

    SPELL_SUMMON_SYLVANAS_AND_HORSE             = 84128,
    SPELL_RIDE_VEHICLE_HARDCODED_LORDAERON      = 46598,

    EVENT_LORDAERON_WAIT_FOR_RIDER              = 1,
    EVENT_LORDAERON_TALK                        = 3,
    EVENT_LORDAERON_TALK_DONE                   = 4,
    EVENT_LORDAERON_FINISH                      = 5,
    EVENT_LORDAERON_BOARD                       = 6,

    // Point ids of the ride are this plus the index in the route
    POINT_LORDAERON_ROUTE                       = 100
};

// Where Summon Sylvanas + Horse brings them (spell_target_position): Sylvanas at Forsaken High Command
Position const LordaeronStartPos = { 1381.35f, 1040.73f, 54.4004f };

// The ride to the Sepulcher. Nothing was sniffed: the nodes follow the flight path from Forsaken High Command to the Sepulcher (TaxiPath
// 2436) at its flight height, and the ground height under them is taken when the ride starts
Position const LordaeronRoute[] =
{
    { 1364.59f, 1018.76f,  63.5146f },
    { 1339.20f, 1014.64f,  65.9663f },
    { 1322.84f, 1013.92f,  67.7506f },
    { 1305.81f, 1031.70f,  69.7352f },
    { 1278.24f, 1066.53f,  72.4136f },
    { 1199.84f, 1156.39f,  72.4136f },
    { 1097.92f, 1229.40f,  71.4961f },
    {  994.83f, 1303.97f,  61.7403f },
    {  898.08f, 1349.14f,  78.4122f },
    {  844.41f, 1370.97f,  81.4349f },
    {  796.46f, 1357.22f,  81.4349f },
    {  739.80f, 1358.21f,  83.0237f },
    {  649.94f, 1359.79f,  97.9831f },
    {  594.89f, 1391.85f, 117.8470f },
    {  556.30f, 1449.92f, 122.5310f },
    {  517.38f, 1496.27f, 134.1770f },
    {  493.86f, 1521.97f, 137.7760f },
    {  505.00f, 1558.00f, 135.0000f }
};

// How long each of Sylvanas' 22 lines lasts before the next one
Milliseconds const LordaeronTextDelays[] =
{
    6s, 6s, 9s, 5s, 7s, 8s, 10s, 11s, 11s, 7s, 6s, 11s, 6s, 12s, 14s, 10s, 12s, 6s, 8s, 14s, 5s, 6s
};

float const LordaeronMaxStep            = 30.0f;
// The horse goes on to the next step this close to the current one, so that it never stops on the way
float const LordaeronStepLead           = 8.0f;
float const WarhorseLordaeronSpeedRate  = 1.0f;
float const SylvanasLordaeronSpeedRate  = 1.1f;

// Sylvanas and the warhorse of the ride are the player's personal summons, so they are found from the player's side
static Creature* FindLordaeronSummonOf(Player* player, uint32 entry)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, 100.0f);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return creature;
    return nullptr;
}

enum ToForsakenForwardCommand
{
    QUEST_TO_FORSAKEN_FORWARD_COMMAND           = 27290,

    NPC_ARTHURA_FORWARD_COMMAND                 = 45318,

    SPELL_FLIGHT_OF_THE_VALKYR_FORWARD_COMMAND  = 84695,

    GOSSIP_MENU_SYLVANAS                        = 11823,
    GOSSIP_OPTION_FORWARD_COMMAND               = 1,

    EVENT_FORWARD_COMMAND_WAIT_FOR_RIDER        = 1,
    EVENT_FORWARD_COMMAND_TAKE_OFF              = 2,
    EVENT_FORWARD_COMMAND_LAND                  = 3,
    EVENT_FORWARD_COMMAND_SET_DOWN              = 4,
    EVENT_FORWARD_COMMAND_LEAVE                 = 5,

    POINT_FORWARD_COMMAND_FLIGHT                = 1,
    POINT_FORWARD_COMMAND_LANDING               = 2
};

// Where Flight of the Val'kyr (84695) summons Arthura (spell_target_position): beside Sylvanas at the Sepulcher
Position const ForwardCommandSepulcherPos = { 505.193f, 1574.03f, 132.098f };

// 44365 - Lady Sylvanas Windrunner
// Gives the player Agatha (Bond of the Val'kyr) on accepting Rise, Forsaken and dismisses her when No Escape is turned in, and at
// Forsaken High Command takes the player to the Sepulcher on accepting Lordaeron. Her lines in The Warchief Cometh are played by
// Grand Executor Mortuus through this AI.
struct npc_silverpine_sylvanas_windrunner_high_command : public ScriptedAI
{
    npc_silverpine_sylvanas_windrunner_high_command(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_RISE_FORSAKEN && !player->HasAura(SPELL_BOND_OF_THE_VALKYR))
            player->CastSpell(player, SPELL_BOND_OF_THE_VALKYR, true);
        else if (quest->GetQuestId() == QUEST_LORDAERON)
            StartLordaeron(player);
    }

    // A ride that was cut short (the player left the horse, died or logged out) starts again by talking to her
    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_LORDAERON) == QUEST_STATUS_INCOMPLETE)
            StartLordaeron(player);
        return false;
    }

    // To Forsaken Forward Command: "Arthura will escort you to Forward Command. Tell me when you are ready and I will send you on
    // your way." The option (shown at the Sepulcher while the quest is in the log) calls Arthura, Flight of the Val'kyr. The quest
    // has no objectives, so it is complete as soon as it is accepted.
    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_SYLVANAS || gossipListId != GOSSIP_OPTION_FORWARD_COMMAND)
            return false;

        CloseGossipMenuFor(player);

        QuestStatus status = player->GetQuestStatus(QUEST_TO_FORSAKEN_FORWARD_COMMAND);
        if ((status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_INCOMPLETE) && player->IsAlive() && !player->GetVehicle()
            && !player->IsInFlight() && me->GetExactDist2d(ForwardCommandSepulcherPos.GetPositionX(), ForwardCommandSepulcherPos.GetPositionY()) < 40.0f)
            player->CastSpell(player, SPELL_FLIGHT_OF_THE_VALKYR_FORWARD_COMMAND, true);
        return true;
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        // Despawn All Summons, done directly: without the bond Agatha leaves
        if (quest->GetQuestId() == QUEST_NO_ESCAPE)
            player->RemoveAurasDueToSpell(SPELL_BOND_OF_THE_VALKYR);
    }

private:
    void StartLordaeron(Player* player)
    {
        // Only the Sylvanas at Forsaken High Command leads the ride; the one at the Sepulcher is where it ends
        if (me->GetExactDist2d(LordaeronStartPos.GetPositionX(), LordaeronStartPos.GetPositionY()) > 30.0f)
            return;

        if (!player->IsAlive() || player->GetVehicle() || FindLordaeronSummonOf(player, NPC_SYLVANAS_LORDAERON))
            return;

        player->CastSpell(player, SPELL_SUMMON_SYLVANAS_AND_HORSE, true);
    }
};

// 45051 - Lady Sylvanas Windrunner
// Summoned with the warhorse by Summon Sylvanas + Horse on accepting Lordaeron: once the player sits on the horse, it carries them to
// the Sepulcher with Sylvanas riding at its side, telling the story of the Forsaken, and when they have arrived and she has finished,
// the player is credited and set down. The horse leads, as the player's camera is on it.
struct npc_silverpine_sylvanas_windrunner_lordaeron : public ScriptedAI
{
    npc_silverpine_sylvanas_windrunner_lordaeron(Creature* creature) : ScriptedAI(creature), _routePoint(0), _textId(0), _boardTries(0),
        _started(false), _arrived(false), _talkDone(false), _finished(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetWalk(false);
        me->SetSpeed(MOVE_RUN, SylvanasLordaeronSpeedRate);

        BuildRoute();
        _events.ScheduleEvent(EVENT_LORDAERON_WAIT_FOR_RIDER, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_finished)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        Creature* horse = ObjectAccessor::GetCreature(*me, _horseGUID);
        if (!player || !player->IsInWorld() || !player->IsAlive() || (_started && (!horse || player->GetVehicleBase() != horse)))
        {
            EndRide(player, horse, false);
            return;
        }

        _events.Update(diff);

        if (_started && !_arrived)
            UpdateRoute(horse);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_LORDAERON_WAIT_FOR_RIDER:
                    horse = FindLordaeronSummonOf(player, NPC_FORSAKEN_WARHORSE_LORDAERON);
                    if (horse && player->GetVehicleBase() == horse)
                        StartRide(horse);
                    else if (++_boardTries < 10)
                        _events.ScheduleEvent(EVENT_LORDAERON_WAIT_FOR_RIDER, 500ms);
                    else
                    {
                        EndRide(player, horse, false);
                        return;
                    }
                    break;
                case EVENT_LORDAERON_TALK:
                    Talk(_textId, player);
                    if (_textId + 1 < std::size(LordaeronTextDelays))
                        _events.ScheduleEvent(EVENT_LORDAERON_TALK, LordaeronTextDelays[_textId]);
                    else
                        _events.ScheduleEvent(EVENT_LORDAERON_TALK_DONE, LordaeronTextDelays[_textId]);
                    ++_textId;
                    break;
                case EVENT_LORDAERON_TALK_DONE:
                    _talkDone = true;
                    TryFinish();
                    break;
                case EVENT_LORDAERON_FINISH:
                    EndRide(player, horse, true);
                    return;
                default:
                    break;
            }
        }
    }

private:
    // Splits the route into short steps, so that each one finds a ground path, at the ground height under the flight path
    void BuildRoute()
    {
        _route.clear();

        Position from = me->GetPosition();
        for (Position const& node : LordaeronRoute)
        {
            float dist = from.GetExactDist2d(node.GetPositionX(), node.GetPositionY());
            uint32 steps = std::max<uint32>(1, uint32(std::ceil(dist / LordaeronMaxStep)));
            for (uint32 i = 1; i <= steps; ++i)
            {
                float t = float(i) / float(steps);
                float x = from.GetPositionX() + (node.GetPositionX() - from.GetPositionX()) * t;
                float y = from.GetPositionY() + (node.GetPositionY() - from.GetPositionY()) * t;
                float z = from.GetPositionZ() + (node.GetPositionZ() - from.GetPositionZ()) * t;

                float ground = me->GetMap()->GetHeight(x, y, z + 20.0f, false);
                if (ground > INVALID_HEIGHT)
                    z = ground;

                _route.push_back({ x, y, z });
            }
            from = node;
        }
    }

    void StartRide(Creature* horse)
    {
        _started = true;
        _horseGUID = horse->GetGUID();

        // The ride is a story: nothing on the way picks a fight with the rider
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);

        horse->SetWalk(false);
        horse->SetSpeed(MOVE_RUN, WarhorseLordaeronSpeedRate);
        horse->GetMotionMaster()->Clear();
        MoveHorseTo(horse, 0);

        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(horse, 3.0f, float(M_PI / 2.0f));

        _events.ScheduleEvent(EVENT_LORDAERON_TALK, 3s);
    }

    // Each step is a pathed move of the horse, started before the previous one ends; one that ended early (no path) is started again
    void UpdateRoute(Creature* horse)
    {
        bool const lastPoint = _routePoint + 1 >= _route.size();
        float const dist = horse->GetExactDist2d(_route[_routePoint].GetPositionX(), _route[_routePoint].GetPositionY());
        bool const moving = horse->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE;

        if (lastPoint)
        {
            if (!moving)
            {
                _arrived = true;
                TryFinish();
            }
        }
        else if (dist < LordaeronStepLead || !moving)
            MoveHorseTo(horse, _routePoint + 1);
    }

    void MoveHorseTo(Creature* horse, uint32 routePoint)
    {
        _routePoint = routePoint;
        horse->GetMotionMaster()->MovePoint(POINT_LORDAERON_ROUTE + _routePoint, _route[_routePoint], true);
    }

    void TryFinish()
    {
        if (_arrived && _talkDone)
            _events.ScheduleEvent(EVENT_LORDAERON_FINISH, 1s);
    }

    // Lordaeron Complete, done directly: the credit, then the player is set down and the ride leaves
    void EndRide(Player* player, Creature* horse, bool completed)
    {
        _finished = true;
        _events.Reset();

        if (player)
        {
            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            if (completed)
                player->KilledMonsterCredit(NPC_SYLVANAS_LORDAERON);
        }

        // The horse leaves as soon as the player gets off
        if (player && horse && player->GetVehicleBase() == horse)
            player->ExitVehicle();
        else if (horse)
            horse->DespawnOrUnsummon(0);

        me->DespawnOrUnsummon(completed ? 3000 : 0);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _horseGUID;
    std::vector<Position> _route;
    uint32 _routePoint;
    uint32 _textId;
    uint32 _boardTries;
    bool _started;
    bool _arrived;
    bool _talkDone;
    bool _finished;
};

// 45057 - Forsaken Warhorse
// Summoned with Sylvanas for Lordaeron: seats the player and carries them along her route; leaves when the player gets off
struct npc_silverpine_forsaken_warhorse_lordaeron : public ScriptedAI
{
    npc_silverpine_forsaken_warhorse_lordaeron(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        _events.ScheduleEvent(EVENT_LORDAERON_BOARD, 500ms);
    }

    void PassengerBoarded(Unit* /*passenger*/, int8 /*seatId*/, bool apply) override
    {
        if (!apply)
            me->DespawnOrUnsummon(500);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (_events.ExecuteEvent() == EVENT_LORDAERON_BOARD)
        {
            // Ride Sylvanas Horse picks any warhorse nearby; the player's own one is boarded directly
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (player && player->IsAlive() && !player->GetVehicle())
                player->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED_LORDAERON, true);
            else
                me->DespawnOrUnsummon(0);
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
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

enum NowhereToRun
{
    QUEST_NOWHERE_TO_RUN                        = 27195,

    NPC_VETERAN_FORSAKEN_TROOPER                = 45225,
    NPC_PACKLEADER_IVAR_BLOODFANG_MINE          = 45236,
    NPC_BOMB_MINE                               = 45237,

    SPELL_EXPLOSION_MINE                        = 84474,

    TALK_FORTESKI_START                         = 0,
    TALK_FORTESKI_ONWARD                        = 1,
    TALK_FORTESKI_WHATS_THIS                    = 2,
    TALK_FORTESKI_TRAP                          = 3,
    TALK_FORTESKI_RUN                           = 4,
    TALK_BLOODFANG_YOUR_DOOM                    = 0,

    EVENT_FORTESKI_CHECK_PLAYER                 = 1,
    EVENT_FORTESKI_ONWARD                       = 2,
    EVENT_FORTESKI_WHATS_THIS                   = 3,
    EVENT_FORTESKI_YOUR_DOOM                    = 4,
    EVENT_FORTESKI_TRAP                         = 5,
    EVENT_FORTESKI_RUN                          = 6,
    EVENT_FORTESKI_EXPLOSION                    = 7,
    EVENT_FORTESKI_DIE                          = 8,

    POINT_FORTESKI_MINE_END                     = 1
};

// In front of where Bloodfang appears (his sniffed spawn faces up the tunnel), facing him
Position const ForteskiMineEndPos   = { 409.39f, 1024.73f, 107.6f, 5.54f };
Position const BloodfangMinePos     = { 413.818f, 1020.68f, 107.745f, 2.4f };
Position const BombMinePos          = { 410.661f, 1011.75f, 107.928f, 0.0f };

float const ForteskiMaxPlayerDistance = 60.0f;

// 45228 - Master Forteski
// Nowhere to Run: leads his troopers into the Deep Elem Mine, where Packleader Ivar Bloodfang springs a trap; Forteski sends the
// player back to Sylvanas (the quest event) before the bomb goes off and kills him and his troopers, who return a little later
struct npc_silverpine_master_forteski : public ScriptedAI
{
    npc_silverpine_master_forteski(Creature* creature) : ScriptedAI(creature), _escorting(false), _creditGiven(false) { }

    void Reset() override
    {
        if (_escorting)
            return;

        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, me->GetCreatureTemplate()->npcflag);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_NOWHERE_TO_RUN)
            StartEscort(player);
    }

    // An escort that was reset (the player died, left or abandoned the quest) starts again by talking to him
    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_NOWHERE_TO_RUN) == QUEST_STATUS_INCOMPLETE)
            StartEscort(player);
        return false;
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_FORTESKI_MINE_END && _escorting)
            _events.ScheduleEvent(EVENT_FORTESKI_WHATS_THIS, 0ms);
    }

    void JustReachedHome() override
    {
        // Sent home by a fight on the way
        if (_escorting && !_creditGiven)
            ResetEscort();
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Killed before the explosion: the troopers go home
        if (_escorting)
            ResetEscort();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_escorting)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);

            switch (eventId)
            {
                case EVENT_FORTESKI_CHECK_PLAYER:
                    if (!player || !player->IsInWorld() || !player->IsAlive() || !me->IsWithinDistInMap(player, ForteskiMaxPlayerDistance) ||
                        player->GetQuestStatus(QUEST_NOWHERE_TO_RUN) != QUEST_STATUS_INCOMPLETE)
                    {
                        ResetEscort();
                        return;
                    }
                    _events.ScheduleEvent(EVENT_FORTESKI_CHECK_PLAYER, 1s);
                    break;
                case EVENT_FORTESKI_ONWARD:
                {
                    Talk(TALK_FORTESKI_ONWARD);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(POINT_FORTESKI_MINE_END, ForteskiMineEndPos, true);

                    // The troopers follow in rows of three behind him
                    uint32 index = 0;
                    for (Creature* trooper : GetTroopers())
                    {
                        trooper->SetWalk(false);
                        trooper->GetMotionMaster()->Clear();
                        trooper->GetMotionMaster()->MoveFollow(me, 2.5f + float(index / 3) * 2.5f, float(M_PI) + (float(index % 3) - 1.0f) * 0.6f);
                        ++index;
                    }
                    break;
                }
                case EVENT_FORTESKI_WHATS_THIS:
                    me->SetFacingTo(ForteskiMineEndPos.GetOrientation());
                    Talk(TALK_FORTESKI_WHATS_THIS);
                    if (Creature* bloodfang = me->SummonCreature(NPC_PACKLEADER_IVAR_BLOODFANG_MINE, BloodfangMinePos, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        // The trap is sprung, not fought
                        bloodfang->SetReactState(REACT_PASSIVE);
                        bloodfang->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        bloodfang->SetFacingToObject(me);
                        _bloodfangGUID = bloodfang->GetGUID();
                    }
                    _events.ScheduleEvent(EVENT_FORTESKI_YOUR_DOOM, 3s);
                    break;
                case EVENT_FORTESKI_YOUR_DOOM:
                    if (Creature* bloodfang = ObjectAccessor::GetCreature(*me, _bloodfangGUID))
                    {
                        me->SetFacingToObject(bloodfang);
                        bloodfang->AI()->Talk(TALK_BLOODFANG_YOUR_DOOM);
                        // Toss Bomb, done with its throw: its nearby-entry destination is searched within its self range here
                        bloodfang->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_THROWN);
                    }
                    if (Creature* bomb = me->SummonCreature(NPC_BOMB_MINE, BombMinePos, TEMPSUMMON_TIMED_DESPAWN, 20s))
                    {
                        bomb->SetReactState(REACT_PASSIVE);
                        bomb->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                        _bombGUID = bomb->GetGUID();
                    }
                    _events.ScheduleEvent(EVENT_FORTESKI_TRAP, 2s);
                    break;
                case EVENT_FORTESKI_TRAP:
                    Talk(TALK_FORTESKI_TRAP);
                    _events.ScheduleEvent(EVENT_FORTESKI_RUN, 3s);
                    break;
                case EVENT_FORTESKI_RUN:
                    _creditGiven = true;
                    _events.CancelEvent(EVENT_FORTESKI_CHECK_PLAYER);
                    if (player)
                    {
                        Talk(TALK_FORTESKI_RUN, player);
                        player->GroupEventHappens(QUEST_NOWHERE_TO_RUN, me);
                    }
                    // Bloodfang slips away before the blast
                    if (Creature* bloodfang = ObjectAccessor::GetCreature(*me, _bloodfangGUID))
                        bloodfang->DespawnOrUnsummon(1500);
                    _events.ScheduleEvent(EVENT_FORTESKI_EXPLOSION, 4s);
                    break;
                case EVENT_FORTESKI_EXPLOSION:
                    if (Creature* bomb = ObjectAccessor::GetCreature(*me, _bombGUID))
                        bomb->CastSpell(bomb, SPELL_EXPLOSION_MINE, true);
                    // Once they have landed from the blast
                    _events.ScheduleEvent(EVENT_FORTESKI_DIE, 2500ms);
                    break;
                case EVENT_FORTESKI_DIE:
                {
                    // They come back shortly for the next player
                    Unit* killer = ObjectAccessor::GetCreature(*me, _bombGUID);
                    if (!killer)
                        killer = me;

                    for (Creature* trooper : GetTroopers())
                    {
                        trooper->SetReactState(REACT_AGGRESSIVE);
                        trooper->SetRespawnDelay(30);
                        trooper->SetCorpseDelay(15);
                        killer->Kill(trooper);
                    }
                    _trooperGUIDs.clear();

                    _escorting = false;
                    me->SetRespawnDelay(30);
                    me->SetCorpseDelay(15);
                    killer->Kill(me);
                    return;
                }
                default:
                    break;
            }
        }
    }

private:
    std::vector<Creature*> GetTroopers() const
    {
        std::vector<Creature*> troopers;
        for (ObjectGuid const& guid : _trooperGUIDs)
            if (Creature* trooper = ObjectAccessor::GetCreature(*me, guid))
                if (trooper->IsAlive())
                    troopers.push_back(trooper);
        return troopers;
    }

    void StartEscort(Player* player)
    {
        if (_escorting || me->IsInCombat() || !player->IsAlive())
            return;

        _escorting = true;
        _creditGiven = false;
        _playerGUID = player->GetGUID();
        _events.Reset();

        // One escort at a time
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        me->SetReactState(REACT_PASSIVE);
        me->SetFacingToObject(player);
        Talk(TALK_FORTESKI_START, player);

        _trooperGUIDs.clear();
        std::list<Creature*> troopers;
        me->GetCreatureListWithEntryInGrid(troopers, NPC_VETERAN_FORSAKEN_TROOPER, 30.0f);
        for (Creature* trooper : troopers)
        {
            if (!trooper->IsAlive() || trooper->IsInCombat())
                continue;

            trooper->SetReactState(REACT_PASSIVE);
            _trooperGUIDs.push_back(trooper->GetGUID());
        }

        _events.ScheduleEvent(EVENT_FORTESKI_CHECK_PLAYER, 1s);
        _events.ScheduleEvent(EVENT_FORTESKI_ONWARD, 4s);
    }

    // Everyone walks back without the explosion, and the quest can be started again
    void ResetEscort()
    {
        _escorting = false;
        _events.Reset();
        DespawnTrap();

        for (Creature* trooper : GetTroopers())
        {
            trooper->SetReactState(REACT_AGGRESSIVE);
            trooper->GetMotionMaster()->Clear();
            trooper->GetMotionMaster()->MoveTargetedHome();
        }
        _trooperGUIDs.clear();

        if (me->IsAlive())
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveTargetedHome();
            Reset();
        }
    }

    void DespawnTrap()
    {
        if (Creature* bloodfang = ObjectAccessor::GetCreature(*me, _bloodfangGUID))
            bloodfang->DespawnOrUnsummon(0);
        if (Creature* bomb = ObjectAccessor::GetCreature(*me, _bombGUID))
            bomb->DespawnOrUnsummon(0);
        _bloodfangGUID.Clear();
        _bombGUID.Clear();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _bloodfangGUID;
    ObjectGuid _bombGUID;
    std::vector<ObjectGuid> _trooperGUIDs;
    bool _escorting;
    bool _creditGiven;
};

// A personal summon of the player (or an actor of the player's scene) of an entry, alive and in reach. Lookups from the
// player's side find only the player's own: group members can see each other's actors standing on the same spots.
// Takes a creature out of Permanent Feign Death so it can be seen getting up: removing the aura alone left it lying on the client,
// as the stand state (already standing under the feign death) did not change. Clear the dead state outright and kneel; the
// caller stands it up a moment later (SetStandState(UNIT_STAND_STATE_STAND)), which the client shows as rising.
static void BeginRisingFromFeignDeath(Creature* creature)
{
    creature->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
    creature->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
    creature->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
    creature->ClearUnitState(UNIT_STATE_DIED);
    creature->SetStandState(UNIT_STAND_STATE_KNEEL);
}

static Creature* FindPersonalSummonOf(Player* player, uint32 entry, float range)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, range);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return creature;
    return nullptr;
}

/*######
## The Great Escape (27438)
######*/

enum TheGreatEscape
{
    QUEST_THE_GREAT_ESCAPE                      = 27438,

    NPC_ARTHURA_GREAT_ESCAPE                    = 45555,
    NPC_GODFREY_CORPSE_GREAT_ESCAPE             = 45556,
    NPC_BELMONT_GREAT_ESCAPE                    = 45557,
    NPC_BELMONT_FORWARD_COMMAND                 = 45312,
    NPC_BELMONT_FORWARD_COMMAND_2               = 45474,

    SPELL_THE_GREAT_ESCAPE_INITIALIZE           = 89225,
    SPELL_FLIGHT_OF_THE_VALKYR                  = 85073,
    SPELL_SUMMON_BELMONT_GREAT_ESCAPE           = 85079,
    SPELL_QUEST_CREDIT_THE_GREAT_ESCAPE         = 85091,

    EVENT_GREAT_ESCAPE_BOARD_CORPSE             = 1,
    EVENT_GREAT_ESCAPE_WAIT_FOR_RIDER           = 2,
    // The farewell steps follow each other from here up to EVENT_GREAT_ESCAPE_TAKE_OFF
    EVENT_GREAT_ESCAPE_TALK                     = 3,
    EVENT_GREAT_ESCAPE_TAKE_OFF                 = 8,
    EVENT_GREAT_ESCAPE_LAND                     = 9,
    EVENT_GREAT_ESCAPE_CREDIT                   = 10,
    EVENT_GREAT_ESCAPE_LEAVE                    = 11,

    TALK_ARTHURA_WHAT_ABOUT_YOU                 = 0,
    TALK_ARTHURA_FAREWELL                       = 1,
    TALK_ARTHURA_HANG_ON                        = 2,
    TALK_BELMONT_ILL_BE_FINE                    = 0,
    TALK_BELMONT_MAKE_IT_BACK                   = 1,
    TALK_BELMONT_MOVE_OUT                       = 2,

    POINT_GREAT_ESCAPE_FLIGHT                   = 1,
    POINT_GREAT_ESCAPE_LANDING                  = 2,

    SEAT_GREAT_ESCAPE_GODFREY                   = 1
};

// Where Flight of the Val'kyr and Summon Belmont put Arthura and Belmont (spell_target_position): Tempest's Reach, where the
// quest's Belmont kneels by Godfrey's corpse (seen once A Man Named Godfrey is completed, Detect: Quest Invis Zone 11).
Position const GreatEscapeTempestsReachPos = { -2121.2f, 931.754f, 3.18012f };

// Nothing was sniffed. The flight leaves Forsaken Forward Command eastwards over the Greymane Wall (its towers reach 172 yards, from
// the vmaps) and the Battlefront at a height that clears them, and lands at the Forsaken Front beside Sylvanas (wiki: "you pass the Forsaken Forward Command ... the 7th
// Legion marches on the road").
Position const GreatEscapeFlightPath[] =
{
    {  -860.0f, 1600.0f, 195.0f },
    {  -760.0f, 1560.0f, 200.0f },
    {  -690.0f, 1530.0f, 170.0f },
    {  -560.0f, 1440.0f, 130.0f },
    {  -420.0f, 1360.0f, 105.0f },
    {  -300.0f, 1300.0f,  85.0f },
    {  -215.0f, 1268.0f,  62.0f }
};

// From Tempest's Reach the flight first climbs out of the bay over the cliffs behind it and crosses the Ruins of Gilneas to
// Forward Command, clearing the terrain by 20 yards or more
Position const GreatEscapeTempestsReachPath[] =
{
    { -2112.0f,  940.0f, 110.0f },
    { -2000.0f, 1030.0f, 165.0f },
    { -1500.0f, 1300.0f, 175.0f },
    { -1000.0f, 1580.0f, 165.0f }
};

Position const GreatEscapeLandingPos = { -188.5f, 1263.0f, 45.0f };

float const GreatEscapeFlightSpeedRate = 2.5f;

// 89225 - The Great Escape Initialize
// The quest's start spell (and talking to Belmont while the escape has not been made): Flight of the Val'kyr
class spell_silverpine_the_great_escape_initialize : public SpellScript
{
    PrepareSpellScript(spell_silverpine_the_great_escape_initialize);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FLIGHT_OF_THE_VALKYR });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !player->IsAlive() || player->GetVehicle() || player->IsInFlight())
            return;

        if (player->GetQuestStatus(QUEST_THE_GREAT_ESCAPE) != QUEST_STATUS_INCOMPLETE)
            return;

        if (FindPersonalSummonOf(player, NPC_ARTHURA_GREAT_ESCAPE, 100.0f))
            return;

        player->CastSpell(player, SPELL_FLIGHT_OF_THE_VALKYR, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_silverpine_the_great_escape_initialize::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 85073 - Flight of the Val'kyr
// Brings Arthura where the player is when the escape does not start at Tempest's Reach
class spell_silverpine_flight_of_the_valkyr : public SpellScript
{
    PrepareSpellScript(spell_silverpine_flight_of_the_valkyr);

    void SetDest(SpellDestination& dest)
    {
        Unit* caster = GetCaster();
        if (caster->GetExactDist2d(GreatEscapeTempestsReachPos.GetPositionX(), GreatEscapeTempestsReachPos.GetPositionY()) > 50.0f)
            dest.Relocate(caster->GetPosition());
    }

    void Register() override
    {
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_silverpine_flight_of_the_valkyr::SetDest, EFFECT_0, TARGET_DEST_DB);
    }
};

// 85079 - Summon Belmont
// Belmont stays behind: the Belmont who gave the quest kneels on the very spot this would summon one, so none is summoned
class spell_silverpine_summon_belmont_great_escape : public SpellScript
{
    PrepareSpellScript(spell_silverpine_summon_belmont_great_escape);

    void HandleSummon(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_silverpine_summon_belmont_great_escape::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};

// 45312, 45474 - Deathstalker Commander Belmont
// The escape starts with the quest's start spell; talking to him again starts an escape that was cut short (the player logged out
// or was taken off the val'kyr)
struct npc_silverpine_belmont_great_escape : public ScriptedAI
{
    npc_silverpine_belmont_great_escape(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_THE_GREAT_ESCAPE) == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(player, SPELL_THE_GREAT_ESCAPE_INITIALIZE, true);
        return false;
    }
};

// 45555 - Arthura
// Summoned by Flight of the Val'kyr with the player on her (its ride aura also gives the Gilneas Act II phase) and Lord Godfrey's
// corpse in her other hand. Belmont sees them off, she flies them over the Greymane Wall to the Forsaken Front, where the player is
// credited and set down beside Sylvanas, and she leaves.
struct npc_silverpine_arthura_great_escape : public VehicleAI
{
    npc_silverpine_arthura_great_escape(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _arrived(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);

        // Lord Godfrey's corpse rides in the second seat; the player gets the first one from the ride aura
        if (TempSummon* corpse = me->SummonCreature(NPC_GODFREY_CORPSE_GREAT_ESCAPE, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, _playerGUID))
            _corpseGUID = corpse->GetGUID();

        _events.ScheduleEvent(EVENT_GREAT_ESCAPE_BOARD_CORPSE, 500ms);
        _events.ScheduleEvent(EVENT_GREAT_ESCAPE_WAIT_FOR_RIDER, 1s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player is off: the val'kyr leaves
        if (!_arrived)
            Leave(0);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_GREAT_ESCAPE_FLIGHT)
            _events.ScheduleEvent(EVENT_GREAT_ESCAPE_LAND, 100ms);
        else if (pointId == POINT_GREAT_ESCAPE_LANDING)
            _events.ScheduleEvent(EVENT_GREAT_ESCAPE_CREDIT, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || (_started && !_arrived && player->GetVehicleBase() != me))
        {
            Leave(0);
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_GREAT_ESCAPE_BOARD_CORPSE:
                    if (Creature* corpse = ObjectAccessor::GetCreature(*me, _corpseGUID))
                        corpse->EnterVehicle(me, SEAT_GREAT_ESCAPE_GODFREY);
                    break;
                case EVENT_GREAT_ESCAPE_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                    {
                        _started = true;
                        _events.ScheduleEvent(EVENT_GREAT_ESCAPE_TALK, 1s);
                    }
                    else if (++_boardTries < 10)
                        _events.ScheduleEvent(EVENT_GREAT_ESCAPE_WAIT_FOR_RIDER, 500ms);
                    else
                    {
                        Leave(0);
                        return;
                    }
                    break;
                case EVENT_GREAT_ESCAPE_TALK:
                    Talk(TALK_ARTHURA_WHAT_ABOUT_YOU, player);
                    next = 4s;
                    break;
                case EVENT_GREAT_ESCAPE_TALK + 1:
                    BelmontTalk(player, TALK_BELMONT_ILL_BE_FINE);
                    next = 6s;
                    break;
                case EVENT_GREAT_ESCAPE_TALK + 2:
                    BelmontTalk(player, TALK_BELMONT_MAKE_IT_BACK);
                    next = 5s;
                    break;
                case EVENT_GREAT_ESCAPE_TALK + 3:
                    BelmontTalk(player, TALK_BELMONT_MOVE_OUT);
                    next = 4s;
                    break;
                case EVENT_GREAT_ESCAPE_TALK + 4:
                    Talk(TALK_ARTHURA_FAREWELL, player);
                    next = 3s;
                    break;
                case EVENT_GREAT_ESCAPE_TAKE_OFF:
                    Talk(TALK_ARTHURA_HANG_ON, player);
                    TakeOff();
                    break;
                case EVENT_GREAT_ESCAPE_LAND:
                {
                    Position landing = GreatEscapeLandingPos;
                    float ground = me->GetMap()->GetHeight(landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ() + 10.0f);
                    if (ground > INVALID_HEIGHT)
                        landing.m_positionZ = ground;
                    me->GetMotionMaster()->MoveLand(POINT_GREAT_ESCAPE_LANDING, landing);
                    break;
                }
                case EVENT_GREAT_ESCAPE_CREDIT:
                    // Quest Credit: The Great Escape (85091) targets the val'kyr's master, the player; the credit is given directly
                    // so it does not depend on the spell reaching them
                    _arrived = true;
                    if (player->GetQuestStatus(QUEST_THE_GREAT_ESCAPE) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(QUEST_THE_GREAT_ESCAPE);
                    // Getting off despawns the val'kyr (a vehicle minion leaves with its owner), so the corpse she carries,
                    // which the player does not own, goes first
                    if (Creature* corpse = ObjectAccessor::GetCreature(*me, _corpseGUID))
                        corpse->DespawnOrUnsummon();
                    if (player->GetVehicleBase() == me)
                        player->ExitVehicle();
                    _events.ScheduleEvent(EVENT_GREAT_ESCAPE_LEAVE, 2s);
                    break;
                case EVENT_GREAT_ESCAPE_LEAVE:
                    Leave(0);
                    return;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    // Belmont stays behind: the Belmont who gave the quest, kneeling at Tempest's Reach (or wherever an escape cut short was
    // started again)
    void BelmontTalk(Player* player, uint8 textId)
    {
        Creature* belmont = FindPersonalSummonOf(player, NPC_BELMONT_GREAT_ESCAPE, 50.0f);
        if (!belmont)
            belmont = me->FindNearestCreature(NPC_BELMONT_FORWARD_COMMAND, 40.0f);
        if (!belmont)
            belmont = me->FindNearestCreature(NPC_BELMONT_FORWARD_COMMAND_2, 40.0f);
        if (belmont && belmont->IsAIEnabled)
            belmont->AI()->Talk(textId, player);
    }

    void TakeOff()
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());
        path.push_back({ me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20.0f });

        if (me->GetExactDist2d(GreatEscapeTempestsReachPos.GetPositionX(), GreatEscapeTempestsReachPos.GetPositionY()) < 100.0f)
            for (Position const& node : GreatEscapeTempestsReachPath)
                path.push_back(node);

        for (Position const& node : GreatEscapeFlightPath)
            path.push_back(node);

        me->SetSpeed(MOVE_RUN, GreatEscapeFlightSpeedRate);
        me->SetSpeed(MOVE_FLIGHT, GreatEscapeFlightSpeedRate);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_GREAT_ESCAPE_FLIGHT, path.data(), path.size(), false, true);
    }

    // Godfrey's corpse leaves with her; a player still on board is set down first
    void Leave(uint32 delay)
    {
        _events.Reset();
        _started = false;
        _arrived = true;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();

        if (Creature* corpse = ObjectAccessor::GetCreature(*me, _corpseGUID))
            corpse->DespawnOrUnsummon(delay);
        me->DespawnOrUnsummon(delay);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _corpseGUID;
    uint32 _boardTries;
    bool _started;
    bool _arrived;
};

// 84695 - Flight of the Val'kyr (To Forsaken Forward Command)
// One Arthura per player
class spell_silverpine_flight_of_the_valkyr_forward_command : public SpellScript
{
    PrepareSpellScript(spell_silverpine_flight_of_the_valkyr_forward_command);

    SpellCastResult CheckCast()
    {
        if (Player* player = GetCaster()->ToPlayer())
            if (FindPersonalSummonOf(player, NPC_ARTHURA_FORWARD_COMMAND, 200.0f))
                return SPELL_FAILED_DONT_REPORT;
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_silverpine_flight_of_the_valkyr_forward_command::CheckCast);
    }
};

// Nothing was sniffed. From the Sepulcher the val'kyr climbs over the hills west of it, crosses the Battlefront and the Greymane Wall
// above its towers (172 yards, from the vmaps) and sets the player down beside Deathstalker Commander Belmont at Forsaken Forward
// Command. Every node clears the terrain and the models under it by 20 yards or more.
Position const ForwardCommandFlightPath[] =
{
    {  470.0f, 1575.0f, 205.0f },
    {  380.0f, 1580.0f, 215.0f },
    {  250.0f, 1586.0f, 215.0f },
    {  120.0f, 1591.0f, 230.0f },
    {    0.0f, 1595.0f, 240.0f },
    { -150.0f, 1605.0f, 205.0f },
    { -350.0f, 1610.0f, 150.0f },
    { -600.0f, 1616.0f, 170.0f },
    { -700.0f, 1620.0f, 195.0f },
    { -820.0f, 1624.0f, 195.0f },
    { -880.0f, 1626.0f, 120.0f }
};

Position const ForwardCommandLandingPos = { -922.0f, 1630.0f, 68.5f };

float const ForwardCommandFlightSpeedRate = 2.5f;

// 45318 - Arthura
// Summoned by Flight of the Val'kyr with the player on her (its ride aura, 84691, also gives the Gilneas Act I phase). She flies the
// player to Forsaken Forward Command, sets them down and leaves; the quest ends at Belmont there.
struct npc_silverpine_arthura_forward_command : public VehicleAI
{
    npc_silverpine_arthura_forward_command(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _arrived(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);

        _events.ScheduleEvent(EVENT_FORWARD_COMMAND_WAIT_FOR_RIDER, 1s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player is off: the val'kyr leaves
        if (!_arrived)
            Leave();
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_FORWARD_COMMAND_FLIGHT)
            _events.ScheduleEvent(EVENT_FORWARD_COMMAND_LAND, 100ms);
        else if (pointId == POINT_FORWARD_COMMAND_LANDING)
            _events.ScheduleEvent(EVENT_FORWARD_COMMAND_SET_DOWN, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || (_started && !_arrived && player->GetVehicleBase() != me))
        {
            Leave();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FORWARD_COMMAND_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                    {
                        _started = true;
                        _events.ScheduleEvent(EVENT_FORWARD_COMMAND_TAKE_OFF, 1500ms);
                    }
                    else if (++_boardTries < 10)
                        _events.ScheduleEvent(EVENT_FORWARD_COMMAND_WAIT_FOR_RIDER, 500ms);
                    else
                    {
                        Leave();
                        return;
                    }
                    break;
                case EVENT_FORWARD_COMMAND_TAKE_OFF:
                    TakeOff();
                    break;
                case EVENT_FORWARD_COMMAND_LAND:
                {
                    Position landing = ForwardCommandLandingPos;
                    float ground = me->GetMap()->GetHeight(landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ() + 10.0f);
                    if (ground > INVALID_HEIGHT)
                        landing.m_positionZ = ground;
                    me->GetMotionMaster()->MoveLand(POINT_FORWARD_COMMAND_LANDING, landing);
                    break;
                }
                case EVENT_FORWARD_COMMAND_SET_DOWN:
                    // Getting off despawns the val'kyr (a vehicle minion leaves with its owner)
                    _arrived = true;
                    if (player->GetVehicleBase() == me)
                        player->ExitVehicle();
                    _events.ScheduleEvent(EVENT_FORWARD_COMMAND_LEAVE, 2s);
                    break;
                case EVENT_FORWARD_COMMAND_LEAVE:
                    Leave();
                    return;
                default:
                    break;
            }
        }
    }

private:
    void TakeOff()
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());
        path.push_back({ me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20.0f });
        for (Position const& node : ForwardCommandFlightPath)
            path.push_back(node);

        me->SetSpeed(MOVE_RUN, ForwardCommandFlightSpeedRate);
        me->SetSpeed(MOVE_FLIGHT, ForwardCommandFlightSpeedRate);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_FORWARD_COMMAND_FLIGHT, path.data(), path.size(), false, true);
    }

    // A player still on board is set down first
    void Leave()
    {
        _events.Reset();
        _started = false;
        _arrived = true;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();

        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    uint32 _boardTries;
    bool _started;
    bool _arrived;
};

// 45525 - Lady Sylvanas Windrunner (the Forsaken Front, around Godfrey's body)
// Ends The Great Escape. A player who got here with the escape still open (the flight was cut short) has made it all the same.
struct npc_silverpine_sylvanas_forsaken_front : public ScriptedAI
{
    npc_silverpine_sylvanas_forsaken_front(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_THE_GREAT_ESCAPE) == QUEST_STATUS_INCOMPLETE && !player->GetVehicle()
            && !FindPersonalSummonOf(player, NPC_ARTHURA_GREAT_ESCAPE, 100.0f))
            player->AreaExploredOrEventHappens(QUEST_THE_GREAT_ESCAPE);
        return false;
    }
};

/*######
## Rise, Godfrey (27472)
######*/

enum RiseGodfrey
{
    QUEST_RISE_GODFREY                          = 27472,

    NPC_SYLVANAS_RISE_GODFREY                   = 45605,
    NPC_GODFREY_RISE_GODFREY                    = 45606,
    NPC_ASHBURY_RISE_GODFREY                    = 45607,
    NPC_WALDEN_RISE_GODFREY                     = 45608,
    NPC_AGATHA_RISE_GODFREY                     = 45609,
    NPC_DASCHLA_RISE_GODFREY                    = 45610,
    NPC_ARTHURA_RISE_GODFREY                    = 45611,
    NPC_CROMUSH_RISE_GODFREY                    = 45612,

    SPELL_SUMMON_FORSAKEN_FRONT_ACTORS          = 85149,
    SPELL_SUMMON_SYLVANAS_RISE_GODFREY          = 85150,
    SPELL_SUMMON_CROMUSH_RISE_GODFREY           = 85151,
    SPELL_SUMMON_AGATHA_RISE_GODFREY            = 85152,
    SPELL_SUMMON_DASCHLA_RISE_GODFREY           = 85153,
    SPELL_SUMMON_ARTHURA_RISE_GODFREY           = 85154,
    SPELL_SUMMON_WALDEN_RISE_GODFREY            = 85155,
    SPELL_SUMMON_GODFREY_RISE_GODFREY           = 85156,
    SPELL_SUMMON_ASHBURY_RISE_GODFREY           = 85157,
    SPELL_GODFREY_KILL_CREDIT                   = 85177,
    SPELL_RAISE_FORSAKEN_RISE_GODFREY           = 85197,
    SPELL_GODFREY_TRANSFORM                     = 85198,
    SPELL_ASHBURY_TRANSFORM                     = 85200,
    SPELL_WALDEN_TRANSFORM                      = 85201,
    SPELL_PERMANENT_FEIGN_DEATH                 = 29266,

    // The scene steps follow each other from here up to EVENT_RISE_GODFREY_CREDIT
    EVENT_RISE_GODFREY_START                    = 1,
    EVENT_RISE_GODFREY_RAISE                    = 2,
    EVENT_RISE_GODFREY_TRANSFORM                = 3,
    EVENT_RISE_GODFREY_CREDIT                   = 15,
    EVENT_RISE_GODFREY_END                      = 16,

    TALK_SYLVANAS_RISE_GODFREY_0                = 0,
    TALK_SYLVANAS_RISE_GODFREY_1                = 1,
    TALK_SYLVANAS_RISE_GODFREY_2                = 2,
    TALK_SYLVANAS_RISE_GODFREY_3                = 3,
    TALK_SYLVANAS_RISE_GODFREY_4                = 4,
    TALK_SYLVANAS_RISE_GODFREY_5                = 5,
    TALK_GODFREY_RISE_GODFREY_0                 = 0,
    TALK_GODFREY_RISE_GODFREY_1                 = 1,
    TALK_GODFREY_RISE_GODFREY_2                 = 2,
    TALK_GODFREY_RISE_GODFREY_3                 = 3,
    TALK_GODFREY_RISE_GODFREY_4                 = 4,
    TALK_CROMUSH_RISE_GODFREY_0                 = 0,

    RISE_GODFREY_ACTOR_COUNT                    = 8
};

// The eight spells of Summon Forsaken Front Actors and what they summon, on the spots of the camp around Godfrey's body
// (spell_target_position), facing as that camp does
uint32 const RiseGodfreyActorSummonSpells[RISE_GODFREY_ACTOR_COUNT] =
{
    SPELL_SUMMON_SYLVANAS_RISE_GODFREY, SPELL_SUMMON_CROMUSH_RISE_GODFREY, SPELL_SUMMON_AGATHA_RISE_GODFREY, SPELL_SUMMON_DASCHLA_RISE_GODFREY,
    SPELL_SUMMON_ARTHURA_RISE_GODFREY, SPELL_SUMMON_WALDEN_RISE_GODFREY, SPELL_SUMMON_GODFREY_RISE_GODFREY, SPELL_SUMMON_ASHBURY_RISE_GODFREY
};

uint32 const RiseGodfreyActorEntries[RISE_GODFREY_ACTOR_COUNT] =
{
    NPC_SYLVANAS_RISE_GODFREY, NPC_CROMUSH_RISE_GODFREY, NPC_AGATHA_RISE_GODFREY, NPC_DASCHLA_RISE_GODFREY,
    NPC_ARTHURA_RISE_GODFREY, NPC_WALDEN_RISE_GODFREY, NPC_GODFREY_RISE_GODFREY, NPC_ASHBURY_RISE_GODFREY
};

float const RiseGodfreyActorOrientations[RISE_GODFREY_ACTOR_COUNT] =
{
    3.12414f, 3.12414f, 3.83972f, 2.19912f, 6.19592f, 5.20108f, 0.0f, 0.820305f
};

enum RiseGodfreyActorIndex
{
    RISE_ACTOR_SYLVANAS                         = 0,
    RISE_ACTOR_CROMUSH                          = 1,
    RISE_ACTOR_ARTHURA                          = 4,
    RISE_ACTOR_WALDEN                           = 5,
    RISE_ACTOR_GODFREY                          = 6,
    RISE_ACTOR_ASHBURY                          = 7
};

// Godfrey's body at the Forsaken Front; the scene is played to a player this close to it
Position const RiseGodfreyBodyPos = { -198.587f, 1275.19f, 40.0797f };
float const RiseGodfreySceneRange = 60.0f;

static void DespawnForsakenFrontActors(Player* player)
{
    for (uint32 entry : RiseGodfreyActorEntries)
    {
        std::list<Creature*> actors;
        player->GetCreatureListWithEntryInGrid(actors, entry, 200.0f);
        for (Creature* actor : actors)
            if (actor->IsSummon() && actor->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
                actor->DespawnOrUnsummon(0);
    }
}

// Summon Forsaken Front Actors, done afresh: any actors of an earlier try leave first
static void SummonForsakenFrontActors(Player* player)
{
    DespawnForsakenFrontActors(player);
    for (uint32 spellId : RiseGodfreyActorSummonSpells)
        player->CastSpell(player, spellId, true);
}

static bool CanSeeRiseGodfrey(Player* player)
{
    return player->IsInWorld() && player->IsAlive() && !player->GetVehicle() && !player->IsInFlight()
        && player->GetQuestStatus(QUEST_RISE_GODFREY) == QUEST_STATUS_INCOMPLETE
        && player->GetExactDist2d(RiseGodfreyBodyPos.GetPositionX(), RiseGodfreyBodyPos.GetPositionY()) < RiseGodfreySceneRange;
}

// 85149 - Summon Forsaken Front Actors
// The start spell of Rise, Godfrey: Sylvanas and the others around Godfrey's body, as the player's own actors
class spell_silverpine_summon_forsaken_front_actors : public SpellScript
{
    PrepareSpellScript(spell_silverpine_summon_forsaken_front_actors);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_SYLVANAS_RISE_GODFREY, SPELL_SUMMON_CROMUSH_RISE_GODFREY, SPELL_SUMMON_AGATHA_RISE_GODFREY,
            SPELL_SUMMON_DASCHLA_RISE_GODFREY, SPELL_SUMMON_ARTHURA_RISE_GODFREY, SPELL_SUMMON_WALDEN_RISE_GODFREY,
            SPELL_SUMMON_GODFREY_RISE_GODFREY, SPELL_SUMMON_ASHBURY_RISE_GODFREY });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Player* player = GetCaster()->ToPlayer())
            if (CanSeeRiseGodfrey(player))
                SummonForsakenFrontActors(player);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_silverpine_summon_forsaken_front_actors::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 85149 - Summon Forsaken Front Actors
// The aura stays while Rise, Godfrey is in progress (the quest's reward spell, Cancel 85149, takes it away) and brings the actors
// back for a scene that was cut short (the player died, left or logged out); abandoning the quest takes it and the actors away
class spell_silverpine_summon_forsaken_front_actors_aura : public AuraScript
{
    PrepareAuraScript(spell_silverpine_summon_forsaken_front_actors_aura);

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 3 * IN_MILLISECONDS;
    }

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        switch (player->GetQuestStatus(QUEST_RISE_GODFREY))
        {
            case QUEST_STATUS_INCOMPLETE:
                break;
            case QUEST_STATUS_COMPLETE:
            case QUEST_STATUS_REWARDED:
                return;
            default:
                Remove();
                return;
        }

        // The aura lasts ten minutes; keep it while the quest is open
        if (GetDuration() >= 0 && GetDuration() < 60 * IN_MILLISECONDS)
            RefreshDuration();

        if (CanSeeRiseGodfrey(player) && !FindPersonalSummonOf(player, NPC_SYLVANAS_RISE_GODFREY, 100.0f))
            SummonForsakenFrontActors(player);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
            if (player->IsInWorld())
                DespawnForsakenFrontActors(player);
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_silverpine_summon_forsaken_front_actors_aura::CalcPeriodic, EFFECT_1, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_silverpine_summon_forsaken_front_actors_aura::OnPeriodic, EFFECT_1, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_silverpine_summon_forsaken_front_actors_aura::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 85197 - Raise Forsaken (Rise, Godfrey), 85950 - Raise Forsaken (Cities in Dust)
// The val'kyr raise the dead of their own scene. The spells have no radius here, so the dead are picked directly: the actors of the
// same player, as group members' scenes play on the same spots.
class spell_silverpine_raise_forsaken_actors : public SpellScript
{
    PrepareSpellScript(spell_silverpine_raise_forsaken_actors);

    void SelectTargets(std::list<WorldObject*>& targets)
    {
        targets.clear();

        std::vector<uint32> entries;
        if (GetSpellInfo()->Id == SPELL_RAISE_FORSAKEN_RISE_GODFREY)
            entries = { NPC_GODFREY_RISE_GODFREY, NPC_WALDEN_RISE_GODFREY, NPC_ASHBURY_RISE_GODFREY };
        else
            entries = { 46026 }; // Lady Sylvanas Windrunner (Cities in Dust)

        Unit* caster = GetCaster();
        for (uint32 entry : entries)
        {
            std::list<Creature*> creatures;
            caster->GetCreatureListWithEntryInGrid(creatures, entry, 30.0f);
            for (Creature* creature : creatures)
                if (creature->IsAlive() && creature->GetPrivateObjectOwner() == caster->GetPrivateObjectOwner())
                    targets.push_back(creature);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_silverpine_raise_forsaken_actors::SelectTargets, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 45605 - Lady Sylvanas Windrunner (Rise, Godfrey)
// The first of the Forsaken Front actors: plays the scene from the wiki's order of the broadcast texts 45740-45751. Arthura raises
// Godfrey, Walden and Ashbury, they take their undead forms and stand, and Sylvanas sends the player on with Godfrey Kill Credit.
struct npc_silverpine_sylvanas_rise_godfrey : public ScriptedAI
{
    npc_silverpine_sylvanas_rise_godfrey(Creature* creature) : ScriptedAI(creature), _startTries(0), _credited(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetFacingTo(RiseGodfreyActorOrientations[RISE_ACTOR_SYLVANAS]);

        // The other actors are summoned right after her
        _events.ScheduleEvent(EVENT_RISE_GODFREY_START, 2s);
    }

    void UpdateAI(uint32 diff) override
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || !player->IsAlive()
            || player->GetExactDist2d(RiseGodfreyBodyPos.GetPositionX(), RiseGodfreyBodyPos.GetPositionY()) > RiseGodfreySceneRange + 20.0f
            || (!_credited && player->GetQuestStatus(QUEST_RISE_GODFREY) != QUEST_STATUS_INCOMPLETE))
        {
            // The player left or abandoned the quest: the start spell's aura brings the scene back when they return
            EndScene(0);
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_RISE_GODFREY_START)
            {
                bool allThere = true;
                for (uint8 i = 0; i < RISE_GODFREY_ACTOR_COUNT; ++i)
                {
                    Creature* actor = FindPersonalSummonOf(player, RiseGodfreyActorEntries[i], 100.0f);
                    _actorGUIDs[i] = actor ? actor->GetGUID() : ObjectGuid::Empty;
                    if (!actor)
                        allThere = false;
                }

                if (!allThere)
                {
                    if (++_startTries < 5)
                        _events.ScheduleEvent(EVENT_RISE_GODFREY_START, 1s);
                    else
                        EndScene(0);
                    return;
                }
            }
            else if (eventId == EVENT_RISE_GODFREY_END)
            {
                EndScene(0);
                return;
            }

            Creature* actors[RISE_GODFREY_ACTOR_COUNT];
            for (uint8 i = 0; i < RISE_GODFREY_ACTOR_COUNT; ++i)
            {
                actors[i] = ObjectAccessor::GetCreature(*me, _actorGUIDs[i]);
                if (!actors[i] || !actors[i]->IsAIEnabled)
                {
                    // An actor is missing: the scene starts again with new ones
                    EndScene(0);
                    return;
                }
            }

            Creature* godfrey = actors[RISE_ACTOR_GODFREY];
            Creature* walden = actors[RISE_ACTOR_WALDEN];
            Creature* ashbury = actors[RISE_ACTOR_ASHBURY];

            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_RISE_GODFREY_START:
                    Talk(TALK_SYLVANAS_RISE_GODFREY_0, player);
                    next = 5s;
                    break;
                case EVENT_RISE_GODFREY_RAISE:
                    actors[RISE_ACTOR_ARTHURA]->CastSpell(actors[RISE_ACTOR_ARTHURA], SPELL_RAISE_FORSAKEN_RISE_GODFREY, false);
                    next = 8s;
                    break;
                case EVENT_RISE_GODFREY_TRANSFORM:
                    godfrey->CastSpell(godfrey, SPELL_GODFREY_TRANSFORM, true);
                    walden->CastSpell(walden, SPELL_WALDEN_TRANSFORM, true);
                    ashbury->CastSpell(ashbury, SPELL_ASHBURY_TRANSFORM, true);
                    for (Creature* raised : { godfrey, walden, ashbury })
                        BeginRisingFromFeignDeath(raised);
                    next = 3s;
                    break;
                case 4:
                    for (Creature* raised : { godfrey, walden, ashbury })
                        raised->SetStandState(UNIT_STAND_STATE_STAND);
                    godfrey->AI()->Talk(TALK_GODFREY_RISE_GODFREY_0, player);
                    next = 4s;
                    break;
                case 5:
                    godfrey->SetFacingToObject(me);
                    Talk(TALK_SYLVANAS_RISE_GODFREY_1, player);
                    next = 8s;
                    break;
                case 6:
                    actors[RISE_ACTOR_CROMUSH]->AI()->Talk(TALK_CROMUSH_RISE_GODFREY_0, player);
                    next = 3s;
                    break;
                case 7:
                    Talk(TALK_SYLVANAS_RISE_GODFREY_2, player);
                    next = 3s;
                    break;
                case 8:
                    godfrey->AI()->Talk(TALK_GODFREY_RISE_GODFREY_1, player);
                    next = 5s;
                    break;
                case 9:
                    godfrey->AI()->Talk(TALK_GODFREY_RISE_GODFREY_2, player);
                    next = 5s;
                    break;
                case 10:
                    godfrey->AI()->Talk(TALK_GODFREY_RISE_GODFREY_3, player);
                    next = 4s;
                    break;
                case 11:
                    Talk(TALK_SYLVANAS_RISE_GODFREY_3, player);
                    next = 9s;
                    break;
                case 12:
                    godfrey->AI()->Talk(TALK_GODFREY_RISE_GODFREY_4, player);
                    next = 3s;
                    break;
                case 13:
                    me->SetFacingToObject(player);
                    Talk(TALK_SYLVANAS_RISE_GODFREY_4, player);
                    next = 6s;
                    break;
                case 14:
                    Talk(TALK_SYLVANAS_RISE_GODFREY_5, player);
                    next = 4s;
                    break;
                case EVENT_RISE_GODFREY_CREDIT:
                    // Godfrey Kill Credit goes to her summoner; the camp after Rise, Godfrey takes the actors' place
                    _credited = true;
                    DoCast(me, SPELL_GODFREY_KILL_CREDIT, true);
                    next = 2s;
                    break;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    void EndScene(uint32 delay)
    {
        _events.Reset();
        for (ObjectGuid const& guid : _actorGUIDs)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                if (actor != me)
                    actor->DespawnOrUnsummon(delay);
        me->DespawnOrUnsummon(delay);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _actorGUIDs[RISE_GODFREY_ACTOR_COUNT];
    uint32 _startTries;
    bool _credited;
};

// 45606 - Lord Godfrey, 45607 - Baron Ashbury, 45608 - Lord Walden, 45609 - Agatha, 45610 - Daschla, 45611 - Arthura,
// 45612 - High Warlord Cromush
// The other Forsaken Front actors, played by Sylvanas; they leave with her
struct npc_silverpine_forsaken_front_actor : public ScriptedAI
{
    npc_silverpine_forsaken_front_actor(Creature* creature) : ScriptedAI(creature), _checkTimer(5000) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        for (uint8 i = 0; i < RISE_GODFREY_ACTOR_COUNT; ++i)
            if (RiseGodfreyActorEntries[i] == me->GetEntry())
                me->SetFacingTo(RiseGodfreyActorOrientations[i]);

        // The val'kyr hover over the dead
        if (me->GetEntry() == NPC_AGATHA_RISE_GODFREY || me->GetEntry() == NPC_DASCHLA_RISE_GODFREY || me->GetEntry() == NPC_ARTHURA_RISE_GODFREY)
            me->SetDisableGravity(true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 1000;

        Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID);
        if (!player || !FindPersonalSummonOf(player, NPC_SYLVANAS_RISE_GODFREY, 100.0f))
            me->DespawnOrUnsummon(0);
    }

private:
    ObjectGuid _summonerGUID;
    uint32 _checkTimer;
};

/*######
## The Battlefront: Godfrey, Walden and Ashbury go with the player (Taking the Battlefront to On Her Majesty's Secret Service)
######*/

enum BattlefrontCompanions
{
    QUEST_TAKING_THE_BATTLEFRONT                = 27542,
    QUEST_ON_HER_MAJESTYS_SECRET_SERVICE        = 27594,

    NPC_GODFREY_BATTLEFRONT                     = 45878,
    NPC_WALDEN_BATTLEFRONT                      = 45879,
    NPC_ASHBURY_BATTLEFRONT                     = 45880,

    SPELL_SUMMON_GODFREY_BATTLEFRONT            = 85517,
    SPELL_SUMMON_WALDEN_BATTLEFRONT             = 85518,
    SPELL_SUMMON_ASHBURY_BATTLEFRONT            = 85519,

    BATTLEFRONT_COMPANION_COUNT                 = 3
};

uint32 const BattlefrontCompanionSummonSpells[BATTLEFRONT_COMPANION_COUNT] =
{
    SPELL_SUMMON_GODFREY_BATTLEFRONT, SPELL_SUMMON_WALDEN_BATTLEFRONT, SPELL_SUMMON_ASHBURY_BATTLEFRONT
};

uint32 const BattlefrontCompanionEntries[BATTLEFRONT_COMPANION_COUNT] =
{
    NPC_GODFREY_BATTLEFRONT, NPC_WALDEN_BATTLEFRONT, NPC_ASHBURY_BATTLEFRONT
};

// They walk behind the player
float const BattlefrontCompanionFollowAngles[BATTLEFRONT_COMPANION_COUNT] =
{
    float(M_PI), float(M_PI * 3 / 4), float(M_PI * 5 / 4)
};

// 85516 - Battlefront Summon Master Aura
// Given in Silverpine Forest from Taking the Battlefront until On Her Majesty's Secret Service is turned in (spell_area). Godfrey,
// Walden and Ashbury are the player's guardians: they are summoned when the aura comes and whenever one of them is missing (after a
// login, a death, a flight or a teleport) or left far behind, and leave with it. Battlefront Broadcast is not needed for that.
class spell_silverpine_battlefront_summon_master_aura : public AuraScript
{
    PrepareAuraScript(spell_silverpine_battlefront_summon_master_aura);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_GODFREY_BATTLEFRONT, SPELL_SUMMON_WALDEN_BATTLEFRONT, SPELL_SUMMON_ASHBURY_BATTLEFRONT });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
            if (player->IsInWorld())
                SummonMissingCompanions(player);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        for (uint32 entry : BattlefrontCompanionEntries)
            GetTarget()->RemoveAllMinionsByEntry(entry);
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        // spell_area only looks at the quests on area changes: the companions leave once On Her Majesty's Secret Service is turned in
        if (player->GetQuestStatus(QUEST_TAKING_THE_BATTLEFRONT) != QUEST_STATUS_REWARDED
            || player->GetQuestStatus(QUEST_ON_HER_MAJESTYS_SECRET_SERVICE) == QUEST_STATUS_REWARDED)
        {
            Remove();
            return;
        }

        SummonMissingCompanions(player);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_silverpine_battlefront_summon_master_aura::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_silverpine_battlefront_summon_master_aura::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_silverpine_battlefront_summon_master_aura::CalcPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_silverpine_battlefront_summon_master_aura::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }

private:
    // One of each, the player's own: a companion that is dead or left far behind is replaced
    static void SummonMissingCompanions(Player* player)
    {
        if (!player->IsAlive() || player->IsInFlight() || player->GetVehicle())
            return;

        for (uint8 i = 0; i < BATTLEFRONT_COMPANION_COUNT; ++i)
        {
            std::list<Creature*> companions;
            player->GetAllMinionsByEntry(companions, BattlefrontCompanionEntries[i]);

            bool found = false;
            for (Creature* companion : companions)
            {
                if (!found && companion->IsAlive() && companion->IsInMap(player) && companion->IsWithinDist(player, 100.0f))
                    found = true;
                else
                    companion->ToTempSummon()->UnSummon();
            }

            if (found)
                continue;

            player->CastSpell(player, BattlefrontCompanionSummonSpells[i], true);

            if (Unit* companion = player->GetFirstMinionByEntry(BattlefrontCompanionEntries[i]))
                if (!companion->IsInCombat())
                    companion->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, BattlefrontCompanionFollowAngles[i]);
        }
    }
};

/*######
## Cities in Dust (27601)
######*/

enum CitiesInDust
{
    QUEST_CITIES_IN_DUST                        = 27601,

    NPC_SYLVANAS_FORSAKEN_FRONT_RAISED          = 45617,
    NPC_CITIES_IN_DUST_QUEST_CREDIT             = 46019,
    NPC_SYLVANAS_CITIES_IN_DUST                 = 46026,
    NPC_LORNA_CITIES_IN_DUST                    = 46027,
    NPC_GODFREY_CITIES_IN_DUST                  = 46028,
    NPC_WALDEN_CITIES_IN_DUST                   = 46029,
    NPC_ASHBURY_CITIES_IN_DUST                  = 46030,
    NPC_CROMUSH_CITIES_IN_DUST                  = 46031,
    NPC_ARTHURA_CITIES_IN_DUST                  = 46032,
    NPC_DASCHLA_CITIES_IN_DUST                  = 46033,
    NPC_AGATHA_CITIES_IN_DUST                   = 46034,
    NPC_BLOODFANG_CITIES_IN_DUST                = 46035,
    NPC_CROWLEY_CITIES_IN_DUST                  = 46036,

    SPELL_SUMMON_SYLVANAS_CITIES_IN_DUST        = 85864,
    SPELL_SUMMON_CROMUSH_CITIES_IN_DUST         = 85868,
    SPELL_SUMMON_ARTHURA_CITIES_IN_DUST         = 85869,
    SPELL_SUMMON_AGATHA_CITIES_IN_DUST          = 85870,
    SPELL_SUMMON_DASCHLA_CITIES_IN_DUST         = 85871,
    SPELL_SONG_OF_SYLVANAS                      = 85874,
    SPELL_SUMMON_CROWLEY_CITIES_IN_DUST         = 85877,
    SPELL_SUMMON_BLOODFANG_CITIES_IN_DUST       = 85878,
    SPELL_SUMMON_GODFREY_CITIES_IN_DUST         = 85925,
    SPELL_SUMMON_WALDEN_CITIES_IN_DUST          = 85926,
    SPELL_SUMMON_ASHBURY_CITIES_IN_DUST         = 85927,
    SPELL_SHIELD_OF_THE_VALKYR                  = 85934,
    SPELL_EXECUTION                             = 85944,
    SPELL_RAISE_FORSAKEN_CITIES_IN_DUST         = 85950,

    GOSSIP_MENU_SYLVANAS_FORSAKEN_FRONT         = 11823,
    GOSSIP_OPTION_ACCOMPANY_SYLVANAS            = 0,

    EVENT_CITIES_START                          = 1,
    // The scene at the Greymane Wall: its steps follow each other from here up to EVENT_CITIES_END
    EVENT_CITIES_SCENE                          = 2,
    EVENT_CITIES_CROWLEY_ARRIVED                = 3,
    EVENT_CITIES_BRING_HER                      = 14,
    EVENT_CITIES_GODFREY_ARRIVED                = 15,
    EVENT_CITIES_EXECUTION                      = 25,
    EVENT_CITIES_FIGHT                          = 29,
    EVENT_CITIES_FALL_BACK                      = 33,
    EVENT_CITIES_RAISE                          = 40,
    EVENT_CITIES_SACRIFICE                      = 41,
    EVENT_CITIES_CREDIT                         = 46,
    EVENT_CITIES_END                            = 47,

    // Point ids of the march are this plus the step in the route
    POINT_CITIES_ROUTE                          = 100,
    POINT_CITIES_SPOT                           = 1,

    SEAT_GODFREY_LORNA                          = 0
};

// Lady Sylvanas Windrunner (46026)
enum CitiesInDustSylvanasTexts
{
    TALK_CID_SYLVANAS_ONWARD                    = 0,  // 46139
    TALK_CID_SYLVANAS_CITIES_IN_DUST            = 1,  // 46141
    TALK_CID_SYLVANAS_SURRENDER                 = 2,  // 46142
    TALK_CID_SYLVANAS_SCURRY                    = 3,  // 46143
    TALK_CID_SYLVANAS_GREYMANE_WALL             = 4,  // 46144
    TALK_CID_SYLVANAS_YOU_HAVE_LOST             = 5,  // 46188
    TALK_CID_SYLVANAS_FRIVOLOUSLY               = 6,  // 46190
    TALK_CID_SYLVANAS_GILNEAS_WORTH             = 7,  // 46191
    TALK_CID_SYLVANAS_YOUR_DAUGHTER             = 8,  // 46194
    TALK_CID_SYLVANAS_NOTHING_YET               = 9,  // 46196
    TALK_CID_SYLVANAS_A_CHOICE                  = 10, // 46197
    TALK_CID_SYLVANAS_LIFE_OF_LORNA             = 11, // 46198
    TALK_CID_SYLVANAS_CHOOSE_WISELY             = 12, // 46199
    TALK_CID_SYLVANAS_BRING_HER                 = 13, // 46200
    TALK_CID_SYLVANAS_RELEASE_HER               = 14, // 46206
    TALK_CID_SYLVANAS_LEAVE_HERE                = 15, // 46208
    TALK_CID_SYLVANAS_LORDAERON_BELONGS         = 16, // 46209
    TALK_CID_SYLVANAS_VICTORIOUS                = 17, // 46211
    TALK_CID_SYLVANAS_DIES                      = 18, // 46212
    TALK_CID_SYLVANAS_ONLY_DARKNESS             = 19, // 46224
    TALK_CID_SYLVANAS_BRILLIANT_LIGHT           = 20, // 46225
    TALK_CID_SYLVANAS_VALKYR_FUTURE             = 21, // 46226
    TALK_CID_SYLVANAS_NEVER_STOP                = 22  // 46227
};

enum CitiesInDustActorTexts
{
    TALK_CID_CROWLEY_NOT_OVER                   = 0,  // 46189
    TALK_CID_CROWLEY_DIE_TRYING                 = 1,  // 46193
    TALK_CID_CROWLEY_LORNA                      = 2,  // 46195
    TALK_CID_CROWLEY_MAGGOT                     = 3,  // 46203
    TALK_CID_CROWLEY_RETREAT                    = 4,  // 46204
    TALK_CID_CROWLEY_FORGIVE_ME                 = 5,  // 46210
    TALK_CID_BLOODFANG_CANT_BE_SERIOUS          = 0,  // 46205
    TALK_CID_GODFREY_OLD_FRIEND                 = 0,  // 46202
    TALK_CID_GODFREY_OF_COURSE                  = 1,  // 46207
    TALK_CID_GODFREY_LONG_AGO                   = 2,  // 46214
    TALK_CID_GODFREY_BELONGS_TO_ME              = 3,  // 46215
    TALK_CID_GODFREY_JOIN_YOUR_MISTRESS         = 4,  // 46216
    TALK_CID_GODFREY_FALL_BACK                  = 5,  // 46217
    TALK_CID_GODFREY_QUIVER                     = 6,  // 46218
    TALK_CID_CROMUSH_WHAT_HAVE_YOU_DONE         = 0,  // 46213
    TALK_CID_CROMUSH_FIX_HER                    = 1,  // 46219
    TALK_CID_AGATHA_BOUND_TO_HER                = 0,  // 46220
    TALK_CID_ARTHURA_DESTROY_US                 = 0,  // 46221
    TALK_CID_ARTHURA_LET_IT_BE_DONE             = 1,  // 46223
    TALK_CID_DASCHLA_SACRED_DUTY                = 0   // 46222
};

enum CitiesInDustActorIndex
{
    CID_CROMUSH,
    CID_ARTHURA,
    CID_AGATHA,
    CID_DASCHLA,
    CID_CROWLEY,
    CID_BLOODFANG,
    CID_GODFREY,
    CID_WALDEN,
    CID_ASHBURY,
    CID_LORNA,
    CID_ACTOR_COUNT
};

uint32 const CitiesInDustActorEntries[CID_ACTOR_COUNT] =
{
    NPC_CROMUSH_CITIES_IN_DUST, NPC_ARTHURA_CITIES_IN_DUST, NPC_AGATHA_CITIES_IN_DUST, NPC_DASCHLA_CITIES_IN_DUST,
    NPC_CROWLEY_CITIES_IN_DUST, NPC_BLOODFANG_CITIES_IN_DUST, NPC_GODFREY_CITIES_IN_DUST, NPC_WALDEN_CITIES_IN_DUST,
    NPC_ASHBURY_CITIES_IN_DUST, NPC_LORNA_CITIES_IN_DUST
};

// The march from the Forsaken Front across the Battlefront to the Greymane Wall. Nothing was sniffed: the nodes keep to the open
// ground between the catapults and Pyrewood, and the ground height under them is taken when the march starts. It ends where
// Sylvanas (46124) and Cromush (46125) stand after the quest.
Position const CitiesInDustRoute[] =
{
    { -215.0f, 1288.0f, 44.0f },
    { -270.0f, 1298.0f, 42.0f },
    { -350.0f, 1300.0f, 39.0f },
    { -420.0f, 1340.0f, 37.0f },
    { -500.0f, 1385.0f, 30.0f },
    { -580.0f, 1440.0f, 27.0f },
    { -640.0f, 1500.0f, 28.0f },
    { -690.0f, 1540.0f, 28.0f },
    { -698.4f, 1546.6f, 28.1f }
};

float const CitiesInDustMaxStep         = 25.0f;
float const CitiesInDustStepLead        = 5.0f;
// The march waits for a player this far behind, and goes on once they are this close again
float const CitiesInDustWaitDistance    = 35.0f;
float const CitiesInDustResumeDistance  = 15.0f;
float const CitiesInDustAbortDistance   = 150.0f;

// Spots at the Greymane Wall: the val'kyr hover around Sylvanas, Crowley and Bloodfang face her from the wall, Godfrey's men stand
// between them (Ashbury on his sniffed spot) and later run for Shadowfang Keep
Position const CitiesSylvanasFacingWall = { -698.4f, 1546.6f, 28.1f, float(M_PI) };
Position const CitiesCromushSpot        = { -694.8f, 1545.6f, 28.1f };
Position const CitiesArthuraSpot        = { -695.5f, 1552.5f, 28.1f };
Position const CitiesAgathaSpot         = { -695.5f, 1540.5f, 28.1f };
Position const CitiesDaschlaSpot        = { -692.0f, 1547.5f, 28.1f };
Position const CitiesCrowleySpot        = { -722.0f, 1548.5f, 28.1f };
Position const CitiesBloodfangSpot      = { -724.0f, 1542.0f, 28.1f };
Position const CitiesGodfreySpot        = { -707.5f, 1550.5f, 28.1f };
Position const CitiesWaldenSpot         = { -707.0f, 1541.5f, 28.1f };
Position const CitiesAshburySpot        = { -708.249f, 1546.1f, 28.0089f };
Position const CitiesLornaSpot          = { -718.0f, 1546.0f, 28.1f };
Position const CitiesGilneasRetreat     = { -800.0f, 1546.0f, 28.0f };
Position const CitiesShadowfangFlight   = { -640.0f, 1600.0f, 28.0f };
float const CitiesValkyrHoverHeight     = 3.0f;

static Position CitiesGroundPos(Map* map, Position const& pos, float above = 0.0f)
{
    Position ground = pos;
    float z = map->GetHeight(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10.0f, false);
    if (z > INVALID_HEIGHT)
        ground.m_positionZ = z;
    ground.m_positionZ += above;
    return ground;
}

// 85877 - Summon Crowley, 85878 - Summon Bloodfang, 85925 - Summon Godfrey, 85926 - Summon Walden, 85927 - Summon Asbhury
// Their spell_target_position is on the ground of the Battlefront; keep the summon on it
class spell_silverpine_cities_in_dust_summon : public SpellScript
{
    PrepareSpellScript(spell_silverpine_cities_in_dust_summon);

    void SetDest(SpellDestination& dest)
    {
        dest.Relocate(CitiesGroundPos(GetCaster()->GetMap(), dest._position));
    }

    void Register() override
    {
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_silverpine_cities_in_dust_summon::SetDest, EFFECT_0, TARGET_DEST_DB);
    }
};

// 85944 - Execution
// Godfrey's shot only fells Sylvanas: she lies dead until her val'kyr raise her, and plays on
class spell_silverpine_execution : public SpellScript
{
    PrepareSpellScript(spell_silverpine_execution);

    void HandleHit(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target || target->GetEntry() != NPC_SYLVANAS_CITIES_IN_DUST)
            return;

        PreventHitDefaultEffect(effIndex);
        target->CastSpell(target, SPELL_PERMANENT_FEIGN_DEATH, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_execution::HandleHit, EFFECT_0, SPELL_EFFECT_INSTAKILL);
    }
};

// 46026 - Lady Sylvanas Windrunner (Cities in Dust)
// Summoned by Summon Sylvanas when the player tells Sylvanas at the Forsaken Front that they are ready. Brings her Cromush and the
// three val'kyr (Silverpine Finale Master Summon Script, done directly), sings Song of Sylvanas and marches them with the player
// across the Battlefront to the Greymane Wall, and plays the finale of broadcast_text 46139-46227 with the speakers of
// warcraft.wiki.gg: Crowley and Bloodfang come to parley, Godfrey brings Lorna and the worgen retreat, Godfrey shoots Sylvanas,
// fights Cromush and the val'kyr and flees for Shadowfang, and the val'kyr give their lives to raise her. The player is credited at
// the end. The actors are all the player's personal summons; the crowd of the Battlefront leaves the player alone meanwhile.
struct npc_silverpine_sylvanas_cities_in_dust : public ScriptedAI
{
    npc_silverpine_sylvanas_cities_in_dust(Creature* creature) : ScriptedAI(creature), _routePoint(0), _startTries(0),
        _marching(false), _waiting(false), _credited(false), _ended(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetWalk(false);

        // Silverpine Finale Master Summon Script, done directly
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            for (uint32 spellId : { SPELL_SUMMON_CROMUSH_CITIES_IN_DUST, SPELL_SUMMON_ARTHURA_CITIES_IN_DUST, SPELL_SUMMON_AGATHA_CITIES_IN_DUST,
                SPELL_SUMMON_DASCHLA_CITIES_IN_DUST })
                player->CastSpell(player, spellId, true);

        BuildRoute();
        _events.ScheduleEvent(EVENT_CITIES_START, 1500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_ended)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || !player->IsAlive() || !me->IsWithinDist(player, CitiesInDustAbortDistance)
            || (!_credited && player->GetQuestStatus(QUEST_CITIES_IN_DUST) != QUEST_STATUS_INCOMPLETE))
        {
            // Sylvanas at the Forsaken Front starts it again
            EndEvent(player, 0);
            return;
        }

        _events.Update(diff);

        if (_marching)
            UpdateMarch(player);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_CITIES_START)
            {
                StartMarch(player);
                continue;
            }

            Creature* cromush = Actor(CID_CROMUSH);
            Creature* crowley = Actor(CID_CROWLEY);
            Creature* godfrey = Actor(CID_GODFREY);

            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_CITIES_SCENE:
                    // At the wall: Crowley and Bloodfang come out to parley
                    me->SetFacingTo(CitiesSylvanasFacingWall.GetOrientation());
                    player->CastSpell(player, SPELL_SUMMON_CROWLEY_CITIES_IN_DUST, true);
                    player->CastSpell(player, SPELL_SUMMON_BLOODFANG_CITIES_IN_DUST, true);
                    FindActor(player, CID_CROWLEY);
                    FindActor(player, CID_BLOODFANG);
                    MoveActor(CID_CROWLEY, CitiesCrowleySpot);
                    MoveActor(CID_BLOODFANG, CitiesBloodfangSpot);
                    next = 8s;
                    break;
                case EVENT_CITIES_CROWLEY_ARRIVED:
                    FaceMe(CID_CROWLEY);
                    FaceMe(CID_BLOODFANG);
                    Talk(TALK_CID_SYLVANAS_YOU_HAVE_LOST, player);
                    next = 5s;
                    break;
                case 4:
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_NOT_OVER, player);
                    next = 4s;
                    break;
                case 5:
                    Talk(TALK_CID_SYLVANAS_FRIVOLOUSLY, player);
                    next = 7s;
                    break;
                case 6:
                    Talk(TALK_CID_SYLVANAS_GILNEAS_WORTH, player);
                    next = 6s;
                    break;
                case 7:
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_DIE_TRYING, player);
                    next = 4s;
                    break;
                case 8:
                    Talk(TALK_CID_SYLVANAS_YOUR_DAUGHTER, player);
                    next = 7s;
                    break;
                case 9:
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_LORNA, player);
                    next = 4s;
                    break;
                case 10:
                    Talk(TALK_CID_SYLVANAS_NOTHING_YET, player);
                    next = 3s;
                    break;
                case 11:
                    Talk(TALK_CID_SYLVANAS_A_CHOICE, player);
                    next = 5s;
                    break;
                case 12:
                    Talk(TALK_CID_SYLVANAS_LIFE_OF_LORNA, player);
                    next = 5s;
                    break;
                case 13:
                    Talk(TALK_CID_SYLVANAS_CHOOSE_WISELY, player);
                    next = 6s;
                    break;
                case EVENT_CITIES_BRING_HER:
                {
                    // AICast Godfrey+ Summon and its Silverpine Finale Master Summon Script (Godfrey+), done directly. Godfrey carries
                    // Lorna, who has no summon spell.
                    Talk(TALK_CID_SYLVANAS_BRING_HER, player);
                    player->CastSpell(player, SPELL_SUMMON_GODFREY_CITIES_IN_DUST, true);
                    player->CastSpell(player, SPELL_SUMMON_WALDEN_CITIES_IN_DUST, true);
                    player->CastSpell(player, SPELL_SUMMON_ASHBURY_CITIES_IN_DUST, true);
                    FindActor(player, CID_GODFREY);
                    FindActor(player, CID_WALDEN);
                    FindActor(player, CID_ASHBURY);
                    if (Creature* summonedGodfrey = Actor(CID_GODFREY))
                        if (TempSummon* lorna = player->SummonCreature(NPC_LORNA_CITIES_IN_DUST, summonedGodfrey->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, player->GetGUID()))
                        {
                            _actorGUIDs[CID_LORNA] = lorna->GetGUID();
                            lorna->EnterVehicle(summonedGodfrey, SEAT_GODFREY_LORNA);
                        }
                    MoveActor(CID_GODFREY, CitiesGodfreySpot);
                    MoveActor(CID_WALDEN, CitiesWaldenSpot);
                    MoveActor(CID_ASHBURY, CitiesAshburySpot);
                    next = 14s;
                    break;
                }
                case EVENT_CITIES_GODFREY_ARRIVED:
                    FaceActor(CID_GODFREY, CID_CROWLEY);
                    FaceActor(CID_WALDEN, CID_CROWLEY);
                    FaceActor(CID_ASHBURY, CID_CROWLEY);
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_OLD_FRIEND, player);
                    next = 3s;
                    break;
                case 16:
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_MAGGOT, player);
                    next = 4s;
                    break;
                case 17:
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_RETREAT, player);
                    next = 6s;
                    break;
                case 18:
                    // Bloodfang will not have it and runs back to Gilneas
                    ActorTalk(CID_BLOODFANG, TALK_CID_BLOODFANG_CANT_BE_SERIOUS, player);
                    MoveActor(CID_BLOODFANG, CitiesGilneasRetreat);
                    DespawnActor(CID_BLOODFANG, 10000);
                    next = 4s;
                    break;
                case 19:
                    Talk(TALK_CID_SYLVANAS_RELEASE_HER, player);
                    next = 3s;
                    break;
                case 20:
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_OF_COURSE, player);
                    if (Creature* lorna = Actor(CID_LORNA))
                    {
                        lorna->ExitVehicle();
                        MoveActor(CID_LORNA, CitiesLornaSpot);
                    }
                    next = 4s;
                    break;
                case 21:
                    Talk(TALK_CID_SYLVANAS_LEAVE_HERE, player);
                    next = 4s;
                    break;
                case 22:
                    Talk(TALK_CID_SYLVANAS_LORDAERON_BELONGS, player);
                    next = 4s;
                    break;
                case 23:
                    // Crowley takes his daughter home
                    ActorTalk(CID_CROWLEY, TALK_CID_CROWLEY_FORGIVE_ME, player);
                    MoveActor(CID_CROWLEY, CitiesGilneasRetreat);
                    MoveActor(CID_LORNA, CitiesGilneasRetreat);
                    DespawnActor(CID_CROWLEY, 12000);
                    DespawnActor(CID_LORNA, 12000);
                    next = 6s;
                    break;
                case 24:
                    // She turns to her army
                    me->SetFacingTo(0.0f);
                    Talk(TALK_CID_SYLVANAS_VICTORIOUS, player);
                    next = 3s;
                    break;
                case EVENT_CITIES_EXECUTION:
                    if (godfrey)
                    {
                        godfrey->SetFacingToObject(me);
                        godfrey->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_RIFLE);
                        godfrey->CastSpell(me, SPELL_EXECUTION, true);
                    }
                    // She falls even if the shot could not hit her
                    if (!me->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                        me->CastSpell(me, SPELL_PERMANENT_FEIGN_DEATH, true);
                    Talk(TALK_CID_SYLVANAS_DIES, player);
                    next = 3s;
                    break;
                case 26:
                    if (cromush)
                        cromush->SetFacingToObject(godfrey ? godfrey : me);
                    ActorTalk(CID_CROMUSH, TALK_CID_CROMUSH_WHAT_HAVE_YOU_DONE, player);
                    next = 3s;
                    break;
                case 27:
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_LONG_AGO, player);
                    next = 5s;
                    break;
                case 28:
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_BELONGS_TO_ME, player);
                    next = 4s;
                    break;
                case EVENT_CITIES_FIGHT:
                    // The fight is played, not fought: the actors can't be hurt and the player stays out of it
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_JOIN_YOUR_MISTRESS, player);
                    if (cromush)
                    {
                        cromush->SetFacingToObject(godfrey ? godfrey : me);
                        cromush->SetEmoteState(EMOTE_STATE_READY1H);
                    }
                    for (uint8 i : { CID_GODFREY, CID_WALDEN, CID_ASHBURY })
                        if (Creature* traitor = Actor(i))
                        {
                            if (cromush)
                                traitor->SetFacingToObject(cromush);
                            traitor->SetEmoteState(i == CID_GODFREY ? EMOTE_STATE_READY_RIFLE : EMOTE_STATE_READY1H);
                        }
                    for (uint8 i : { CID_ARTHURA, CID_AGATHA, CID_DASCHLA })
                        if (Creature* valkyr = Actor(i))
                            valkyr->CastSpell(valkyr, SPELL_SHIELD_OF_THE_VALKYR, true);
                    next = 2s;
                    break;
                case 30:
                case 31:
                case 32:
                    if (cromush)
                        cromush->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    if (godfrey)
                        godfrey->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_RIFLE);
                    for (uint8 i : { CID_WALDEN, CID_ASHBURY })
                        if (Creature* traitor = Actor(i))
                            traitor->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    next = 2s;
                    break;
                case EVENT_CITIES_FALL_BACK:
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_FALL_BACK, player);
                    if (cromush)
                        cromush->SetEmoteState(EMOTE_ONESHOT_NONE);
                    for (uint8 i : { CID_GODFREY, CID_WALDEN, CID_ASHBURY })
                    {
                        if (Creature* traitor = Actor(i))
                            traitor->SetEmoteState(EMOTE_ONESHOT_NONE);
                        MoveActor(i, CitiesShadowfangFlight);
                    }
                    next = 3s;
                    break;
                case 34:
                    ActorTalk(CID_GODFREY, TALK_CID_GODFREY_QUIVER, player);
                    for (uint8 i : { CID_GODFREY, CID_WALDEN, CID_ASHBURY })
                        DespawnActor(i, 8000);
                    next = 4s;
                    break;
                case 35:
                    // Cromush kneels at her side
                    if (cromush)
                    {
                        cromush->SetFacingToObject(me);
                        cromush->SetStandState(UNIT_STAND_STATE_KNEEL);
                    }
                    ActorTalk(CID_CROMUSH, TALK_CID_CROMUSH_FIX_HER, player);
                    next = 4s;
                    break;
                case 36:
                    ActorTalk(CID_AGATHA, TALK_CID_AGATHA_BOUND_TO_HER, player);
                    next = 4s;
                    break;
                case 37:
                    ActorTalk(CID_ARTHURA, TALK_CID_ARTHURA_DESTROY_US, player);
                    next = 4s;
                    break;
                case 38:
                    ActorTalk(CID_DASCHLA, TALK_CID_DASCHLA_SACRED_DUTY, player);
                    next = 5s;
                    break;
                case 39:
                    ActorTalk(CID_ARTHURA, TALK_CID_ARTHURA_LET_IT_BE_DONE, player);
                    next = 2s;
                    break;
                case EVENT_CITIES_RAISE:
                    for (uint8 i : { CID_ARTHURA, CID_AGATHA, CID_DASCHLA })
                        if (Creature* valkyr = Actor(i))
                        {
                            valkyr->SetFacingToObject(me);
                            valkyr->CastSpell(valkyr, SPELL_RAISE_FORSAKEN_CITIES_IN_DUST, false);
                        }
                    next = 8s;
                    break;
                case EVENT_CITIES_SACRIFICE:
                    // The val'kyr give their lives for hers
                    for (uint8 i : { CID_ARTHURA, CID_AGATHA, CID_DASCHLA })
                        if (Creature* valkyr = Actor(i))
                        {
                            valkyr->Kill(valkyr, false);
                            valkyr->DespawnOrUnsummon(6000);
                        }
                    BeginRisingFromFeignDeath(me);
                    if (cromush)
                        cromush->SetStandState(UNIT_STAND_STATE_STAND);
                    next = 3s;
                    break;
                case 42:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    me->SetFacingToObject(player);
                    Talk(TALK_CID_SYLVANAS_ONLY_DARKNESS, player);
                    next = 4s;
                    break;
                case 43:
                    Talk(TALK_CID_SYLVANAS_BRILLIANT_LIGHT, player);
                    next = 8s;
                    break;
                case 44:
                    Talk(TALK_CID_SYLVANAS_VALKYR_FUTURE, player);
                    next = 6s;
                    break;
                case 45:
                    Talk(TALK_CID_SYLVANAS_NEVER_STOP, player);
                    next = 4s;
                    break;
                case EVENT_CITIES_CREDIT:
                    // Cities in Dust (85952) targets her master, the player; the credit is given directly so it does not depend on
                    // the spell reaching them. Sylvanas and Cromush of the Battlefront (46124, 46125) take the actors' place.
                    _credited = true;
                    player->KilledMonsterCredit(NPC_CITIES_IN_DUST_QUEST_CREDIT);
                    next = 1s;
                    break;
                case EVENT_CITIES_END:
                    EndEvent(player, 0);
                    return;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    Creature* Actor(uint8 index) const
    {
        return ObjectAccessor::GetCreature(*me, _actorGUIDs[index]);
    }

    void FindActor(Player* player, uint8 index)
    {
        if (Creature* actor = FindPersonalSummonOf(player, CitiesInDustActorEntries[index], 200.0f))
            _actorGUIDs[index] = actor->GetGUID();
    }

    void ActorTalk(uint8 index, uint8 textId, Player* player)
    {
        if (Creature* actor = Actor(index))
            if (actor->IsAIEnabled)
                actor->AI()->Talk(textId, player);
    }

    void MoveActor(uint8 index, Position const& pos)
    {
        if (Creature* actor = Actor(index))
        {
            actor->SetWalk(false);
            actor->GetMotionMaster()->Clear();
            actor->GetMotionMaster()->MovePoint(POINT_CITIES_SPOT, CitiesGroundPos(me->GetMap(), pos), true);
        }
    }

    void FaceMe(uint8 index)
    {
        if (Creature* actor = Actor(index))
            actor->SetFacingToObject(me);
    }

    void FaceActor(uint8 index, uint8 targetIndex)
    {
        if (Creature* actor = Actor(index))
            if (Creature* target = Actor(targetIndex))
                actor->SetFacingToObject(target);
    }

    void DespawnActor(uint8 index, uint32 delay)
    {
        if (Creature* actor = Actor(index))
            actor->DespawnOrUnsummon(delay);
    }

    // Splits the route into short steps, so that each one finds a ground path, at the ground height under it
    void BuildRoute()
    {
        _route.clear();
        _routeNodes.clear();

        Position from = me->GetPosition();
        for (uint8 node = 0; node < std::size(CitiesInDustRoute); ++node)
        {
            Position const& to = CitiesInDustRoute[node];
            float dist = from.GetExactDist2d(to.GetPositionX(), to.GetPositionY());
            uint32 steps = std::max<uint32>(1, uint32(std::ceil(dist / CitiesInDustMaxStep)));
            for (uint32 i = 1; i <= steps; ++i)
            {
                float t = float(i) / float(steps);
                Position step = { from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * t,
                    from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * t,
                    from.GetPositionZ() + (to.GetPositionZ() - from.GetPositionZ()) * t };
                _route.push_back(CitiesGroundPos(me->GetMap(), step));
                _routeNodes.push_back(node);
            }
            from = to;
        }
    }

    void StartMarch(Player* player)
    {
        for (uint8 i : { CID_CROMUSH, CID_ARTHURA, CID_AGATHA, CID_DASCHLA })
            FindActor(player, i);

        if (!Actor(CID_CROMUSH) || !Actor(CID_ARTHURA) || !Actor(CID_AGATHA) || !Actor(CID_DASCHLA))
        {
            if (++_startTries < 5)
                _events.ScheduleEvent(EVENT_CITIES_START, 1s);
            else
                EndEvent(player, 0);
            return;
        }

        // Her army marches with her
        float const angles[] = { float(M_PI * 3 / 4), float(M_PI / 2), float(M_PI * 3 / 2), float(M_PI) };
        uint8 const followers[] = { CID_CROMUSH, CID_ARTHURA, CID_AGATHA, CID_DASCHLA };
        for (uint8 i = 0; i < std::size(followers); ++i)
            if (Creature* follower = Actor(followers[i]))
            {
                follower->GetMotionMaster()->Clear();
                follower->GetMotionMaster()->MoveFollow(me, 3.0f, angles[i]);
            }

        DoCast(me, SPELL_SONG_OF_SYLVANAS, true);
        Talk(TALK_CID_SYLVANAS_ONWARD, player);

        _marching = true;
        MoveTo(0);
    }

    // Each step is a pathed move, started before the previous one ends; the march waits for a player left behind
    void UpdateMarch(Player* player)
    {
        float const playerDist = me->GetExactDist2d(player);
        if (_waiting)
        {
            if (playerDist < CitiesInDustResumeDistance)
            {
                _waiting = false;
                MoveTo(_routePoint);
            }
            return;
        }

        if (playerDist > CitiesInDustWaitDistance)
        {
            _waiting = true;
            me->GetMotionMaster()->Clear();
            me->StopMoving();
            return;
        }

        bool const lastPoint = _routePoint + 1 >= _route.size();
        float const dist = me->GetExactDist2d(_route[_routePoint].GetPositionX(), _route[_routePoint].GetPositionY());
        bool const moving = me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE;

        if (lastPoint)
        {
            if (!moving)
                ArriveAtWall();
        }
        else if (dist < CitiesInDustStepLead || !moving)
            MoveTo(_routePoint + 1);
    }

    void MoveTo(uint32 routePoint)
    {
        if (routePoint > 0 && routePoint != _routePoint && _routeNodes[routePoint] != _routeNodes[routePoint - 1])
            NodeReached(_routeNodes[routePoint - 1]);

        _routePoint = routePoint;
        me->GetMotionMaster()->MovePoint(POINT_CITIES_ROUTE + _routePoint, _route[_routePoint], true);
    }

    // Her demands across the Battlefront
    void NodeReached(uint8 node)
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        switch (node)
        {
            case 2: Talk(TALK_CID_SYLVANAS_CITIES_IN_DUST, player); break;
            case 3: Talk(TALK_CID_SYLVANAS_SURRENDER, player); break;
            case 4: Talk(TALK_CID_SYLVANAS_SCURRY, player); break;
            case 5: Talk(TALK_CID_SYLVANAS_GREYMANE_WALL, player); break;
            default: break;
        }
    }

    void ArriveAtWall()
    {
        _marching = false;
        me->RemoveAurasDueToSpell(SPELL_SONG_OF_SYLVANAS);

        MoveActor(CID_CROMUSH, CitiesCromushSpot);
        if (Creature* arthura = Actor(CID_ARTHURA))
            arthura->GetMotionMaster()->MovePoint(POINT_CITIES_SPOT, CitiesGroundPos(me->GetMap(), CitiesArthuraSpot, CitiesValkyrHoverHeight), false);
        if (Creature* agatha = Actor(CID_AGATHA))
            agatha->GetMotionMaster()->MovePoint(POINT_CITIES_SPOT, CitiesGroundPos(me->GetMap(), CitiesAgathaSpot, CitiesValkyrHoverHeight), false);
        if (Creature* daschla = Actor(CID_DASCHLA))
            daschla->GetMotionMaster()->MovePoint(POINT_CITIES_SPOT, CitiesGroundPos(me->GetMap(), CitiesDaschlaSpot, CitiesValkyrHoverHeight), false);

        _events.ScheduleEvent(EVENT_CITIES_SCENE, 2s);
    }

    void EndEvent(Player* player, uint32 delay)
    {
        _ended = true;
        _marching = false;
        _events.Reset();

        if (player)
        {
            for (uint32 spellId : { SPELL_SUMMON_SYLVANAS_CITIES_IN_DUST, SPELL_SUMMON_CROMUSH_CITIES_IN_DUST, SPELL_SUMMON_ARTHURA_CITIES_IN_DUST,
                SPELL_SUMMON_AGATHA_CITIES_IN_DUST, SPELL_SUMMON_DASCHLA_CITIES_IN_DUST })
                player->RemoveAurasDueToSpell(spellId);
        }

        for (ObjectGuid const& guid : _actorGUIDs)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                actor->DespawnOrUnsummon(delay);
        me->DespawnOrUnsummon(delay);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _actorGUIDs[CID_ACTOR_COUNT];
    std::vector<Position> _route;
    std::vector<uint8> _routeNodes;
    uint32 _routePoint;
    uint32 _startTries;
    bool _marching;
    bool _waiting;
    bool _credited;
    bool _ended;
};

// 46027 - Commander Lorna Crowley, 46028 - Lord Godfrey, 46029 - Lord Walden, 46030 - Baron Ashbury, 46031 - High Warlord Cromush,
// 46032 - Arthura, 46033 - Daschla, 46034 - Agatha, 46035 - Packleader Ivar Bloodfang, 46036 - Lord Darius Crowley
// The other actors of Cities in Dust, played by Sylvanas; they leave with her
struct npc_silverpine_cities_in_dust_actor : public ScriptedAI
{
    npc_silverpine_cities_in_dust_actor(Creature* creature) : ScriptedAI(creature), _checkTimer(3000) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        if (me->GetEntry() == NPC_ARTHURA_CITIES_IN_DUST || me->GetEntry() == NPC_DASCHLA_CITIES_IN_DUST || me->GetEntry() == NPC_AGATHA_CITIES_IN_DUST)
        {
            me->SetCanFly(true);
            me->SetDisableGravity(true);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 1000;

        Player* player = ObjectAccessor::GetPlayer(*me, _summonerGUID);
        if (!player || !FindPersonalSummonOf(player, NPC_SYLVANAS_CITIES_IN_DUST, 200.0f))
            me->DespawnOrUnsummon(0);
    }

private:
    ObjectGuid _summonerGUID;
    uint32 _checkTimer;
};

// 45617 - Lady Sylvanas Windrunner (the Forsaken Front, after Rise, Godfrey)
// Gives Cities in Dust; told that the player is ready (gossip, while the quest is in progress), she summons her march across the
// Battlefront (Summon Sylvanas). Choosing it again after the march was cut short starts it again.
struct npc_silverpine_sylvanas_forsaken_front_finale : public ScriptedAI
{
    npc_silverpine_sylvanas_forsaken_front_finale(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_SYLVANAS_FORSAKEN_FRONT || gossipListId != GOSSIP_OPTION_ACCOMPANY_SYLVANAS)
            return false;

        CloseGossipMenuFor(player);

        if (player->GetQuestStatus(QUEST_CITIES_IN_DUST) != QUEST_STATUS_INCOMPLETE || !player->IsAlive() || player->GetVehicle())
            return true;

        if (!FindPersonalSummonOf(player, NPC_SYLVANAS_CITIES_IN_DUST, 200.0f))
            player->CastSpell(player, SPELL_SUMMON_SYLVANAS_CITIES_IN_DUST, true);
        return true;
    }
};

/*######
## On Whose Orders? (27364)
######*/

// 84870 - Start Quest: Master Sergeant Zaren
// On Whose Orders? comes over the F.C.D. ("<The F.C.D. buzzes.>") as the player takes Vengeance for Our Soldiers from Captain
// Rutsak. The quest is accepted automatically; show it in its window, as a quest started by an area trigger is, instead of only
// adding it to the log.
class spell_silverpine_start_quest_master_sergeant_zaren : public SpellScript
{
    PrepareSpellScript(spell_silverpine_start_quest_master_sergeant_zaren);

    void HandleQuestStart(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Player* player = GetHitPlayer();
        Quest const* quest = sObjectMgr->GetQuestTemplate(uint32(GetSpellInfo()->Effects[effIndex].MiscValue));
        if (!player || !quest || !player->CanTakeQuest(quest, false) || !player->CanAddQuest(quest, false))
            return;

        player->AddQuest(quest, nullptr);
        player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, player->GetGUID(), true, true);
        if (player->CanCompleteQuest(quest->GetQuestId()))
            player->CompleteQuest(quest->GetQuestId());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_silverpine_start_quest_master_sergeant_zaren::HandleQuestStart, EFFECT_0, SPELL_EFFECT_QUEST_START);
    }
};

/*######
## What Tomorrow Brings (27401)
######*/

enum WhatTomorrowBrings
{
    QUEST_WHAT_TOMORROW_BRINGS                  = 27401,

    NPC_GILNEAS_CAMERA_TARGET                   = 45430,

    EVENT_CAMERA_WAIT_FOR_RIDER                 = 1,
    EVENT_CAMERA_PAN                            = 2,
    EVENT_CAMERA_CREDIT                         = 3,
    EVENT_CAMERA_RETURN                         = 4,

    POINT_CAMERA_HARBOR                         = 1
};

// Nothing was sniffed. The camera rises from the 7th Legion Telescope and glides out over the harbour, where the 7th Legion lands
// under High Commander Halford Wyrmbane, to stop short of the camera target (45430, hovering above it).
Position const GilneasCameraPath[] =
{
    { -1440.0f, 1745.0f, 60.0f },
    { -1380.0f, 1830.0f, 75.0f },
    { -1310.0f, 1920.0f, 85.0f },
    { -1260.0f, 1985.0f, 85.0f }
};

Position const GilneasCameraTargetPos = { -1200.5f, 2063.86f, 82.5887f };

float const GilneasCameraSpeedRate = 1.2f;

// 45427 - Gilneas Camera 01
// Summoned on the player by the 7th Legion Telescope (Summon Gilneas Camera) with the player in its seat. It takes the player's
// view out over the harbour, holds it there, gives the credit (Quest Credit: What Tomorrow Brings targets the camera target,
// so it is given directly) and brings the player back to the telescope.
struct npc_silverpine_gilneas_camera : public VehicleAI
{
    npc_silverpine_gilneas_camera(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _done(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        _home = me->GetPosition();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);

        _events.ScheduleEvent(EVENT_CAMERA_WAIT_FOR_RIDER, 500ms);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player left early: back to the telescope
        if (!_done)
            Return();
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == EFFECT_MOTION_TYPE && pointId == POINT_CAMERA_HARBOR)
        {
            me->SetFacingTo(me->GetAngle(GilneasCameraTargetPos.GetPositionX(), GilneasCameraTargetPos.GetPositionY()));
            _events.ScheduleEvent(EVENT_CAMERA_CREDIT, 6s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            _done = true;
            me->DespawnOrUnsummon();
            return;
        }

        if (_started && !_done && player->GetVehicleBase() != me)
        {
            Return();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CAMERA_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                    {
                        _started = true;
                        _events.ScheduleEvent(EVENT_CAMERA_PAN, 1s);
                    }
                    else if (++_boardTries < 10)
                        _events.ScheduleEvent(EVENT_CAMERA_WAIT_FOR_RIDER, 500ms);
                    else
                    {
                        _done = true;
                        me->DespawnOrUnsummon();
                        return;
                    }
                    break;
                case EVENT_CAMERA_PAN:
                {
                    std::vector<Position> path;
                    path.push_back(me->GetPosition());
                    for (Position const& node : GilneasCameraPath)
                        path.push_back(node);

                    me->SetSpeed(MOVE_RUN, GilneasCameraSpeedRate);
                    me->SetSpeed(MOVE_FLIGHT, GilneasCameraSpeedRate);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveSmoothPath(POINT_CAMERA_HARBOR, path.data(), path.size(), false, true);
                    break;
                }
                case EVENT_CAMERA_CREDIT:
                    if (player->GetQuestStatus(QUEST_WHAT_TOMORROW_BRINGS) == QUEST_STATUS_INCOMPLETE)
                        player->KilledMonsterCredit(NPC_GILNEAS_CAMERA_TARGET);
                    _events.ScheduleEvent(EVENT_CAMERA_RETURN, 2s);
                    break;
                case EVENT_CAMERA_RETURN:
                    Return();
                    return;
                default:
                    break;
            }
        }
    }

private:
    // Sets the player down at the telescope again, wherever the camera is. The teleport waits until getting off is over: from
    // PassengerBoarded it would run before the exit's jump, which would then carry the player back out over the harbour.
    void Return()
    {
        _events.Reset();
        _done = true;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();

            Position home = _home;
            player->m_Events.AddLambdaEventAtOffset([player, home]()
            {
                if (player->IsInWorld() && !player->GetVehicle() && player->GetExactDist(&home) > 5.0f)
                    player->NearTeleportTo(home.GetPositionX(), home.GetPositionY(), home.GetPositionZ(), player->GetOrientation());
            }, 300);
        }

        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    Position _home;
    uint32 _boardTries;
    bool _started;
    bool _done;
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
    new creature_script<npc_silverpine_skitterweb_matriarch>("npc_silverpine_skitterweb_matriarch");
    new aura_script<spell_silverpine_bond_of_the_valkyr>("spell_silverpine_bond_of_the_valkyr");
    new creature_script<npc_silverpine_sylvanas_windrunner_high_command>("npc_silverpine_sylvanas_windrunner_high_command");
    new creature_script<npc_silverpine_sylvanas_windrunner_lordaeron>("npc_silverpine_sylvanas_windrunner_lordaeron");
    new creature_script<npc_silverpine_forsaken_warhorse_lordaeron>("npc_silverpine_forsaken_warhorse_lordaeron");
    new creature_script<npc_silverpine_agatha_fenris_isle>("npc_silverpine_agatha_fenris_isle");
    new spell_script<spell_silverpine_forsaken_trooper_masterscript_fenris_isle>("spell_silverpine_forsaken_trooper_masterscript_fenris_isle");
    new creature_script<npc_silverpine_hillsbrad_refugee>("npc_silverpine_hillsbrad_refugee");
    new creature_script<npc_silverpine_forsaken_trooper_fenris_isle>("npc_silverpine_forsaken_trooper_fenris_isle");
    new creature_script<npc_silverpine_worgen_sentry>("npc_silverpine_worgen_sentry");
    new at_silverpine_no_escape();
    new creature_script<npc_silverpine_fenris_keep_camera>("npc_silverpine_fenris_keep_camera");
    new creature_script<npc_silverpine_fenris_keep_actor>("npc_silverpine_fenris_keep_actor");
    new creature_script<npc_silverpine_master_forteski>("npc_silverpine_master_forteski");
    new spell_script<spell_silverpine_the_great_escape_initialize>("spell_silverpine_the_great_escape_initialize");
    new spell_script<spell_silverpine_flight_of_the_valkyr>("spell_silverpine_flight_of_the_valkyr");
    new spell_script<spell_silverpine_summon_belmont_great_escape>("spell_silverpine_summon_belmont_great_escape");
    new creature_script<npc_silverpine_belmont_great_escape>("npc_silverpine_belmont_great_escape");
    new creature_script<npc_silverpine_arthura_great_escape>("npc_silverpine_arthura_great_escape");
    new creature_script<npc_silverpine_sylvanas_forsaken_front>("npc_silverpine_sylvanas_forsaken_front");
    new spell_script<spell_silverpine_summon_forsaken_front_actors>("spell_silverpine_summon_forsaken_front_actors");
    new aura_script<spell_silverpine_summon_forsaken_front_actors_aura>("spell_silverpine_summon_forsaken_front_actors_aura");
    new spell_script<spell_silverpine_raise_forsaken_actors>("spell_silverpine_raise_forsaken_actors");
    new creature_script<npc_silverpine_sylvanas_rise_godfrey>("npc_silverpine_sylvanas_rise_godfrey");
    new creature_script<npc_silverpine_forsaken_front_actor>("npc_silverpine_forsaken_front_actor");
    new aura_script<spell_silverpine_battlefront_summon_master_aura>("spell_silverpine_battlefront_summon_master_aura");
    new spell_script<spell_silverpine_cities_in_dust_summon>("spell_silverpine_cities_in_dust_summon");
    new spell_script<spell_silverpine_execution>("spell_silverpine_execution");
    new creature_script<npc_silverpine_sylvanas_cities_in_dust>("npc_silverpine_sylvanas_cities_in_dust");
    new creature_script<npc_silverpine_cities_in_dust_actor>("npc_silverpine_cities_in_dust_actor");
    new creature_script<npc_silverpine_sylvanas_forsaken_front_finale>("npc_silverpine_sylvanas_forsaken_front_finale");
    new spell_script<spell_silverpine_flight_of_the_valkyr_forward_command>("spell_silverpine_flight_of_the_valkyr_forward_command");
    new creature_script<npc_silverpine_arthura_forward_command>("npc_silverpine_arthura_forward_command");
    new creature_script<npc_silverpine_gilneas_camera>("npc_silverpine_gilneas_camera");
    new spell_script<spell_silverpine_start_quest_master_sergeant_zaren>("spell_silverpine_start_quest_master_sergeant_zaren");
}
