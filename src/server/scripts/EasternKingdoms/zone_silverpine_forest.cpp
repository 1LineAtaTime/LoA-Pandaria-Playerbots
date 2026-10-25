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
}
