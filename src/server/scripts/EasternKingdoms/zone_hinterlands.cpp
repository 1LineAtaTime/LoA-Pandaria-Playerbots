/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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
SDName: Hinterlands
SD%Complete: 100
SDComment: Quest support: 836, 26419, 26532. Sharpbeak taxi
SDCategory: The Hinterlands
EndScriptData */

/* ContentData
npc_00x09hl
npc_hinterlands_shadra
npc_hinterlands_shadra_lookout
npc_sharpbeak
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

/*######
## npc_00x09hl
######*/

enum eOOX
{
    SAY_OOX_START           = 0,
    SAY_OOX_AGGRO           = 1,
    SAY_OOX_AMBUSH          = 2,
    SAY_OOX_END             = 3,

    QUEST_RESQUE_OOX_09     = 836,

    NPC_MARAUDING_OWL       = 7808,
    NPC_VILE_AMBUSHER       = 7809,

    FACTION_ESCORTEE_A      = 774,
    FACTION_ESCORTEE_H      = 775
};

class npc_00x09hl : public CreatureScript
{
public:
    npc_00x09hl() : CreatureScript("npc_00x09hl") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest) override
    {
        if (quest->GetQuestId() == QUEST_RESQUE_OOX_09)
        {
            creature->SetStandState(UNIT_STAND_STATE_STAND);

            if (player->GetTeam() == ALLIANCE)
                creature->SetFaction(FACTION_ESCORTEE_A);
            else if (player->GetTeam() == HORDE)
                creature->SetFaction(FACTION_ESCORTEE_H);

            creature->AI()->Talk(SAY_OOX_START, player);

            if (npc_00x09hlAI* pEscortAI = CAST_AI(npc_00x09hl::npc_00x09hlAI, creature->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), quest);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_00x09hlAI(creature);
    }

    struct npc_00x09hlAI : public npc_escortAI
    {
        npc_00x09hlAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() override { }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 26:
                    Talk(SAY_OOX_AMBUSH);
                    break;
                case 43:
                    Talk(SAY_OOX_AMBUSH);
                    break;
                case 64:
                    Talk(SAY_OOX_END);
                    if (Player* player = GetPlayerForEscort())
                        player->GroupEventHappens(QUEST_RESQUE_OOX_09, me);
                    break;
            }
        }

        void WaypointStart(uint32 uiPointId) override
        {
            switch (uiPointId)
            {
                case 27:
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        const Position src = {147.927444f, -3851.513428f, 130.893f, 0};
                        Position dst = me->GetRandomPoint(src, 7.0f);
                        DoSummon(NPC_MARAUDING_OWL, dst, 25000ms, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                    }
                    break;
                case 44:
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        const Position src = {-141.151581f, -4291.213867f, 120.130f, 0};
                        Position dst = me->GetRandomPoint(src, 7.0f);
                        me->SummonCreature(NPC_VILE_AMBUSHER, dst, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000ms);
                    }
                    break;
            }
        }

        void JustEngagedWith(Unit* who) override
        {
            if (who->GetEntry() == NPC_MARAUDING_OWL || who->GetEntry() == NPC_VILE_AMBUSHER)
                return;

            Talk(SAY_OOX_AGGRO);
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        }
    };
};

/*######
## npc_rinji
######*/

enum Rinji
{
    SAY_RIN_BY_OUTRUNNER    = 0,
    SAY_RIN_FREE            = 0,    // from here
    SAY_RIN_HELP            = 1,
    SAY_RIN_COMPLETE        = 2,
    SAY_RIN_PROGRESS_1      = 3,
    SAY_RIN_PROGRESS_2      = 4,

    QUEST_RINJI_TRAPPED     = 2742,
    NPC_RANGER              = 2694,
    NPC_OUTRUNNER           = 2691,
    GO_RINJI_CAGE           = 142036
};

struct RinjiLocationXYZ
{
    float x, y, z;
};

RinjiLocationXYZ const AmbushSpawn[] =
{
    { 191.296204f, -2839.329346f, 107.388f },
    { 70.972466f,  -2848.674805f, 109.459f }
};

RinjiLocationXYZ const AmbushMoveTo[] =
{
    { 166.630386f, -2824.780273f, 108.153f },
    { 70.886589f,  -2874.335449f, 116.675f }
};

class npc_rinji : public CreatureScript
{
public:
    npc_rinji() : CreatureScript("npc_rinji") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_RINJI_TRAPPED)
        {
            // Rin'ji is stored immune to players (and npcs) while he sits in the cage; escort Start() drops the npc immunity.
            creature->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);

            if (GameObject* go = creature->FindNearestGameObject(GO_RINJI_CAGE, INTERACTION_DISTANCE))
                go->UseDoorOrButton();

            if (npc_rinjiAI* pEscortAI = CAST_AI(npc_rinji::npc_rinjiAI, creature->AI()))
                pEscortAI->Start(false, false, player->GetGUID(), quest);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_rinjiAI(creature);
    }

    struct npc_rinjiAI : public npc_escortAI
    {
        npc_rinjiAI(Creature* creature) : npc_escortAI(creature)
        {
            _IsByOutrunner = false;
            spawnId = 0;
            postEventCount = 0;
            postEventTimer = 3000;
        }

        void Reset() override
        {
            postEventCount = 0;
            postEventTimer = 3000;
            _IsByOutrunner = false;
            spawnId = 0;
        }

        void JustEngagedWith(Unit* who) override
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (who->GetEntry() == NPC_OUTRUNNER && !_IsByOutrunner)
                {
                    if (Creature* talker = who->ToCreature())
                        talker->AI()->Talk(SAY_RIN_BY_OUTRUNNER);
                    _IsByOutrunner = true;
                }

                if (urand(0, 3))
                    return;

                // only if attacked and escorter is not in combat?
                Talk(SAY_RIN_HELP);
            }
        }

        void DoSpawnAmbush(bool _first)
        {
            if (!_first)
                spawnId = 1;

            me->SummonCreature(NPC_RANGER, AmbushSpawn[spawnId].x, AmbushSpawn[spawnId].y, AmbushSpawn[spawnId].z, 0.0f,
                               TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60s);

            for (int i = 0; i < 2; ++i)
            {
                me->SummonCreature(NPC_OUTRUNNER, AmbushSpawn[spawnId].x, AmbushSpawn[spawnId].y, AmbushSpawn[spawnId].z, 0.0f,
                                   TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60s);
            }
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(0, AmbushMoveTo[spawnId].x, AmbushMoveTo[spawnId].y, AmbushMoveTo[spawnId].z);
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    Talk(SAY_RIN_FREE, player);
                    break;
                case 7:
                    DoSpawnAmbush(true);
                    break;
                case 13:
                    DoSpawnAmbush(false);
                    break;
                case 17:
                    Talk(SAY_RIN_COMPLETE, player);
                    player->GroupEventHappens(QUEST_RINJI_TRAPPED, me);
                    me->SetWalk(false);
                    postEventCount = 1;
                    break;
            }
        }

        void UpdateEscortAI(uint32 diff) override
        {
            // Check if we have a current target
            if (!UpdateVictim())
            {
                if (HasEscortState(STATE_ESCORT_ESCORTING) && postEventCount)
                {
                    if (postEventTimer <= diff)
                    {
                        postEventTimer = 3000;

                        if (Player* player = GetPlayerForEscort())
                        {
                            switch (postEventCount)
                            {
                                case 1:
                                    Talk(SAY_RIN_PROGRESS_1, player);
                                    ++postEventCount;
                                    break;
                                case 2:
                                    Talk(SAY_RIN_PROGRESS_2, player);
                                    postEventCount = 0;
                                    break;
                            }
                        }
                        else
                        {
                            me->DespawnOrUnsummon();
                            return;
                        }
                    }
                    else
                        postEventTimer -= diff;
                }
                return;
            }
            DoMeleeAttackIfReady();
        }

    private:
        uint32 postEventCount;
        uint32 postEventTimer;
        int    spawnId;
        bool   _IsByOutrunner;
    };
};

/*######
## Shadra the Venom Queen (26419 Horde, 26532 Alliance)
######*/

// Sourced: the quest texts (26419: "the lake below roils! Shadra is emerging!"; 26532: "She's comin' out of the water right now! Gryphon
// Master Talonaxe is comin' right now with the boys"; 26558: "The Deathstalkers and I will be along shortly"), Wowhead comments (accepting
// the quest brings her out of the lake and the helpers in; "four elite undead"; "no matter what, she engages the Forsaken adds"; "the
// aide of the Wildhammer dwarves"; the lookout's option "I'm ready to face Shadra", broadcast text 50642, brings her out again), the
// emergence path (mop's waypoints of the old static Shadra, a straight walk from the lake bed to the shore), the helpers' entries
// (Darkcleric Marnal 43018 and the Deathstalker Bodychoppers 43016, which mop had standing by the Deathstalker Lookout; the Alliance
// twins Gryphon Master Talonaxe 43215 and the Wildhammer Spirithunters 43213, spawned by no source) and Marnal's Shadra Taunt Aura 82121.
// Ours: one Shadra at a time (a player who accepts while she fights joins that fight), the kill for everyone nearby who holds either
// quest when she dies (the helpers can kill her before a player has tapped her), four helpers plus a leader on both sides, the Alliance
// spots behind the Wildhammer Lookout, the taunt aura on the leader (Talonaxe too) only from the charge on, when the helpers charge,
// ending a fight with no player in it after 3 minutes, Chop/Burn Shadra on the corpse, and every timing.
enum ShadraEvent
{
    QUEST_SHADRA_HORDE                  = 26419,
    QUEST_SHADRA_ALLIANCE               = 26532,

    NPC_SHADRA                          = 43007,
    NPC_DEATHSTALKER_LOOKOUT            = 43299,
    NPC_WILDHAMMER_LOOKOUT              = 43298,
    NPC_DARKCLERIC_MARNAL_EVENT         = 43018,
    NPC_DEATHSTALKER_BODYCHOPPER        = 43016,
    NPC_GRYPHON_MASTER_TALONAXE_EVENT   = 43215,
    NPC_WILDHAMMER_SPIRITHUNTER         = 43213,

    SPELL_SHADRA_TAUNT_AURA             = 82121,    // periodic Taunt Pulse 82122 on Shadra (conditions)
    SPELL_BURN_SHADRA                   = 80753,
    SPELL_CHOP_SHADRA                   = 80755,

    BROADCAST_READY_TO_FACE_SHADRA      = 50642,    // "I'm ready to face Shadra!"

    POINT_SHADRA_SHORE                  = 1,

    DATA_SHADRA_INVOKER                 = 1,
    DATA_SHADRA_TEAM                    = 2,
    DATA_SHADRA_BUSY                    = 3,

    ACTION_SHADRA_END                   = 1,

    SHADRA_TEAM_HORDE                   = 1,
    SHADRA_TEAM_ALLIANCE                = 2
};

namespace
{
    // mop's waypoints 312702 (points 1-24), the walk of the old static Shadra from the lake bed to the shore
    Position const ShadraEmergePath[] =
    {
        { -358.817f, -2885.98f, 61.4881f }, { -358.109f, -2886.68f, 61.4881f }, { -356.246f, -2887.41f, 62.1131f },
        { -355.315f, -2887.78f, 62.7381f }, { -352.52f,  -2888.88f, 64.1131f }, { -350.656f, -2889.6f,  64.9881f },
        { -347.861f, -2890.7f,  65.7381f }, { -344.135f, -2892.16f, 66.4881f }, { -342.272f, -2892.89f, 67.1131f },
        { -340.408f, -2893.62f, 67.8631f }, { -339.477f, -2893.99f, 68.4881f }, { -336.682f, -2895.08f, 69.7381f },
        { -334.818f, -2895.81f, 70.8631f }, { -333.887f, -2896.18f, 71.8631f }, { -332.955f, -2896.54f, 72.4628f },
        { -332.023f, -2896.91f, 73.7128f }, { -331.092f, -2897.27f, 74.3378f }, { -330.16f,  -2897.64f, 74.9628f },
        { -328.297f, -2898.37f, 75.8378f }, { -326.434f, -2899.1f,  76.9628f }, { -324.57f,  -2899.83f, 77.7128f },
        { -321.775f, -2900.93f, 78.2608f }, { -314.322f, -2903.85f, 78.8858f }, { -312.785f, -2904.45f, 79.0577f }
    };
    size_t const ShadraEmergePathSize = std::extent<decltype(ShadraEmergePath)>::value;
    Position const ShadraSummonPos = { -358.817f, -2885.98f, 61.4881f, 5.90956f };

    // Where mop had Marnal and four of the Bodychoppers standing (guids 312696-312700)
    Position const MarnalPos = { -275.217f, -2940.7f, 87.9364f, 2.54818f };
    Position const BodychopperPos[] =
    {
        { -271.641f, -2943.36f, 89.2776f, 2.70526f },
        { -271.832f, -2944.78f, 89.6094f, 2.70526f },
        { -270.266f, -2942.79f, 89.4478f, 2.70526f },
        { -270.359f, -2945.01f, 90.0193f, 2.70526f }
    };

    // Behind the Wildhammer Lookout, away from the lake: distance and angle from its facing; Talonaxe first
    float const WildhammerOffsets[5][2] = { { 6.0f, 0.0f }, { 8.0f, -0.6f }, { 8.0f, 0.6f }, { 10.0f, -0.3f }, { 10.0f, 0.3f } };

    float const ShadraSearchRange               = 200.0f;   // the lookouts stand 75 yd from the lake bed
    float const ShadraCreditRange               = 80.0f;
    uint32 const ShadraIdleDespawn              = 2 * MINUTE * IN_MILLISECONDS;
    uint32 const ShadraEmergeTimeout            = MINUTE * IN_MILLISECONDS;
    uint32 const ShadraNoPlayerDespawn          = 3 * MINUTE * IN_MILLISECONDS;
    uint32 const ShadraTauntCheck               = 2 * IN_MILLISECONDS;
    Milliseconds const ShadraHelperLifetime     = 15min;
    uint32 const ShadraHelperLinger             = 8 * IN_MILLISECONDS;

    bool HoldsShadraQuest(Player* player)
    {
        return player->GetQuestStatus(QUEST_SHADRA_HORDE) == QUEST_STATUS_INCOMPLETE
            || player->GetQuestStatus(QUEST_SHADRA_ALLIANCE) == QUEST_STATUS_INCOMPLETE;
    }

    // Brings Shadra out of the lake for the player, unless she is out already (walking out or fighting: the player joins in)
    void StartShadra(Player* player)
    {
        if (!player->IsInWorld() || !player->IsAlive())
            return;

        bool const horde = player->GetTeam() == HORDE;
        Creature* lookout = player->FindNearestCreature(horde ? NPC_DEATHSTALKER_LOOKOUT : NPC_WILDHAMMER_LOOKOUT, ShadraSearchRange);
        if (!lookout)
            return;

        if (Creature* shadra = lookout->FindNearestCreature(NPC_SHADRA, ShadraSearchRange))
        {
            if (!shadra->IsAIEnabled || shadra->AI()->GetData(DATA_SHADRA_BUSY))
                return;

            // Idle back in the lake after an evade: she and her helpers go, and a new Shadra comes
            shadra->AI()->DoAction(ACTION_SHADRA_END);
        }

        // Helpers lingering after the last fight would fight the other side's
        std::list<Creature*> helpers;
        for (uint32 entry : { NPC_DARKCLERIC_MARNAL_EVENT, NPC_DEATHSTALKER_BODYCHOPPER, NPC_GRYPHON_MASTER_TALONAXE_EVENT, NPC_WILDHAMMER_SPIRITHUNTER })
            lookout->GetCreatureListWithEntryInGridAppend(helpers, entry, ShadraSearchRange);
        for (Creature* helper : helpers)
            if (helper->IsSummon() && !helper->IsDestroyedObject())
                helper->DespawnOrUnsummon();

        TempSummon* shadra = lookout->SummonCreature(NPC_SHADRA, ShadraSummonPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30s);
        if (!shadra || !shadra->IsAIEnabled)
            return;

        TC_LOG_DEBUG("scripts", "Hinterlands: Shadra emerges for %s (%s)", player->GetName().c_str(), horde ? "Horde" : "Alliance");
        shadra->AI()->SetGUID(player->GetGUID(), DATA_SHADRA_INVOKER);
        shadra->AI()->SetData(DATA_SHADRA_TEAM, horde ? SHADRA_TEAM_HORDE : SHADRA_TEAM_ALLIANCE);
    }
}

// 43007 - Shadra; her spells stay in her SmartAI
struct npc_hinterlands_shadra : public SmartAI
{
    npc_hinterlands_shadra(Creature* creature) : SmartAI(creature), _emerged(false), _helpersReleased(false), _ending(false),
        _idleTimer(0), _emergeTimer(ShadraEmergeTimeout), _noPlayerTimer(0), _tauntTimer(ShadraTauntCheck) { }

    void IsSummonedBy(Unit* summoner) override
    {
        SmartAI::IsSummonedBy(summoner);

        me->setActive(true, ActiveFlags::Script);   // the fight goes on when nobody stands near
        me->SetWalk(true);
        me->GetMotionMaster()->MoveSmoothPath(POINT_SHADRA_SHORE, ShadraEmergePath, ShadraEmergePathSize, true);
    }

    void SetGUID(ObjectGuid guid, int32 id = 0) override
    {
        if (id == DATA_SHADRA_INVOKER)
            _invokerGUID = guid;
        else
            SmartAI::SetGUID(guid, id);
    }

    void SetData(uint32 id, uint32 value) override
    {
        if (id != DATA_SHADRA_TEAM)
        {
            SmartAI::SetData(id, value);
            return;
        }

        if (!_helperGUIDs.empty())
            return;

        if (value == SHADRA_TEAM_HORDE)
        {
            SummonHelper(NPC_DARKCLERIC_MARNAL_EVENT, MarnalPos);
            for (Position const& pos : BodychopperPos)
                SummonHelper(NPC_DEATHSTALKER_BODYCHOPPER, pos);
        }
        else if (Creature* lookout = me->FindNearestCreature(NPC_WILDHAMMER_LOOKOUT, ShadraSearchRange))
        {
            for (uint8 i = 0; i < 5; ++i)
            {
                Position pos = lookout->GetNearPosition(WildhammerOffsets[i][0], float(M_PI) + WildhammerOffsets[i][1]);
                lookout->UpdateGroundPositionZ(pos.GetPositionX(), pos.GetPositionY(), pos.m_positionZ);
                pos.SetOrientation(lookout->GetOrientation());
                SummonHelper(i ? NPC_WILDHAMMER_SPIRITHUNTER : NPC_GRYPHON_MASTER_TALONAXE_EVENT, pos);
            }
        }
    }

    uint32 GetData(uint32 id) const override
    {
        if (id == DATA_SHADRA_BUSY)
            return (!_ending && me->IsAlive() && (!_emerged || me->IsInCombat())) ? 1 : 0;
        return SmartAI::GetData(id);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_SHADRA_END)
            End();
        else
            SmartAI::DoAction(action);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        SmartAI::MovementInform(type, id);

        if (type == EFFECT_MOTION_TYPE && id == POINT_SHADRA_SHORE)
            Emerged();
    }

    void JustEngagedWith(Unit* who) override
    {
        SmartAI::JustEngagedWith(who);
        _emerged = true;
        ReleaseHelpers();
    }

    void JustDied(Unit* killer) override
    {
        SmartAI::JustDied(killer);

        // The helpers may have killed her untapped: everyone nearby who holds either quest gets the kill
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, ShadraCreditRange);
        for (Player* player : players)
            if (HoldsShadraQuest(player))
                player->KilledMonsterCredit(NPC_SHADRA, me->GetGUID());

        // "subdue and harvest her body", then they leave
        for (ObjectGuid const& guid : _helperGUIDs)
        {
            Creature* helper = ObjectAccessor::GetCreature(*me, guid);
            if (!helper || !helper->IsAlive())
                continue;

            if (helper->GetEntry() == NPC_DEATHSTALKER_BODYCHOPPER)
                helper->CastSpell(me, SPELL_CHOP_SHADRA, true);
            else if (helper->GetEntry() == NPC_DARKCLERIC_MARNAL_EVENT)
                helper->CastSpell(me, SPELL_BURN_SHADRA, true);
            helper->DespawnOrUnsummon(ShadraHelperLinger);
        }
        _helperGUIDs.clear();
    }

    void UpdateAI(uint32 diff) override
    {
        SmartAI::UpdateAI(diff);

        if (_ending || !me->IsAlive())
            return;

        if (!_emerged)
        {
            // The walk ended without its inform
            if (_emergeTimer <= diff)
                Emerged();
            else
                _emergeTimer -= diff;
            return;
        }

        if (me->IsInCombat())
        {
            _idleTimer = 0;

            // A fight with no player in it (the helpers' taunt holds her, or she fights an elite helper alone) ends after a while
            if (me->GetThreatManager().GetThreatListPlayerCount())
                _noPlayerTimer = 0;
            else if ((_noPlayerTimer += diff) >= ShadraNoPlayerDespawn)
            {
                End();
                return;
            }

            // A leader who evaded lost the taunt aura with his other auras
            if (_tauntTimer <= diff)
            {
                _tauntTimer = ShadraTauntCheck;
                ApplyTaunt();
            }
            else
                _tauntTimer -= diff;
            return;
        }

        _noPlayerTimer = 0;

        // Nobody fights her (players gone, helpers dead): she and her helpers go
        _idleTimer += diff;
        if (_idleTimer >= ShadraIdleDespawn)
            End();
    }

private:
    void SummonHelper(uint32 entry, Position const& pos)
    {
        TempSummon* helper = me->SummonCreature(entry, pos, TEMPSUMMON_TIMED_DESPAWN, ShadraHelperLifetime);
        if (!helper)
            return;

        _helperGUIDs.push_back(helper->GetGUID());
    }

    // The leader (Marnal or Talonaxe, summoned first) taunts her once the helpers charge: the pulse reaches 100 yd without line of
    // sight and would pull her off the lake bed before she is out
    void ApplyTaunt()
    {
        if (!_helpersReleased || _helperGUIDs.empty())
            return;

        if (Creature* leader = ObjectAccessor::GetCreature(*me, _helperGUIDs.front()))
            if (leader->IsAlive() && !leader->HasAura(SPELL_SHADRA_TAUNT_AURA))
                leader->AddAura(SPELL_SHADRA_TAUNT_AURA, leader);
    }

    void Emerged()
    {
        if (_emerged)
            return;

        _emerged = true;
        _idleTimer = 0;

        Player* invoker = ObjectAccessor::GetPlayer(*me, _invokerGUID);
        if (invoker && (!invoker->IsAlive() || !me->IsWithinDistInMap(invoker, ShadraCreditRange)))
            invoker = nullptr;

        // The one who brought her out taps her
        if (invoker && !me->HasLootRecipient())
            me->SetLootRecipient(invoker);

        // She goes for the helpers' leader
        if (!me->IsInCombat())
        {
            Unit* target = nullptr;
            if (!_helperGUIDs.empty())
                target = ObjectAccessor::GetCreature(*me, _helperGUIDs.front());
            if (!target || !target->IsAlive())
                target = invoker;
            if (target)
                AttackStart(target);
        }

        ReleaseHelpers();
    }

    void ReleaseHelpers()
    {
        if (_helpersReleased)
            return;

        _helpersReleased = true;
        for (ObjectGuid const& guid : _helperGUIDs)
            if (Creature* helper = ObjectAccessor::GetCreature(*me, guid))
                if (helper->IsAlive() && helper->IsAIEnabled)
                    helper->AI()->AttackStart(me);
        ApplyTaunt();
    }

    void End()
    {
        if (_ending)
            return;

        _ending = true;
        for (ObjectGuid const& guid : _helperGUIDs)
            if (Creature* helper = ObjectAccessor::GetCreature(*me, guid))
                helper->DespawnOrUnsummon();
        _helperGUIDs.clear();
        me->DespawnOrUnsummon();
    }

    ObjectGuid _invokerGUID;
    GuidVector _helperGUIDs;
    bool _emerged;
    bool _helpersReleased;
    bool _ending;
    uint32 _idleTimer;
    uint32 _emergeTimer;
    uint32 _noPlayerTimer;
    uint32 _tauntTimer;
};

// 43299 - Deathstalker Lookout, 43298 - Wildhammer Lookout: "I'm ready to face Shadra!" while either quest is in the log
struct npc_hinterlands_shadra_lookout : public ScriptedAI
{
    npc_hinterlands_shadra_lookout(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        if (!HoldsShadraQuest(player))
            return false;

        player->PrepareGossipMenu(me, me->GetGossipMenuId(), true);
        if (BroadcastText const* text = sObjectMgr->GetBroadcastText(BROADCAST_READY_TO_FACE_SHADRA))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, text->GetText(player->GetSession()->GetSessionDbLocaleIndex(), player->GetGender()),
                GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SendPreparedGossip(me);
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        if (GetGossipActionFor(player, gossipListId) != GOSSIP_ACTION_INFO_DEF + 1)
            return false;

        CloseGossipMenuFor(player);
        if (HoldsShadraQuest(player))
            StartShadra(player);
        return true;
    }
};

// 26419, 26532 - Shadra the Venom Queen: accepting brings her out of the lake
class quest_hinterlands_shadra_the_venom_queen : public QuestScript
{
public:
    quest_hinterlands_shadra_the_venom_queen() : QuestScript("quest_hinterlands_shadra_the_venom_queen") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus == QUEST_STATUS_NONE && newStatus == QUEST_STATUS_INCOMPLETE)
            StartShadra(player);
    }
};

/*######
## Sharpbeak (43161, 51125): Doran Steelwing's gryphon between the Steelwing camp and the top of Jintha'Alor
######*/

// Sourced: the spells (83486 force-casts 80657 Sharpbeak Taxi - Camp to Jintha'Alor, 94120 Sharpbeak Taxi - Jintha'Alor to Camp; both
// summon a vehicle with SummonProperties 161, 5.4.8 DBC), Vehicle 1434 (one seat, no exit at will), Doran's gossip option and SmartAI,
// Sharpbeak 44776's spellclick and the vehicles' data (TDB 442 and 1210), and the two flight paths (TCPP npc_sharpbeak, 4.3.4 sniff).
// The spline replaces each path's first point with Sharpbeak's own position (TCPP's first points are those placeholders). He flies
// at his run speed, the TDBs' rate 3.23.
// Ours: setting the rider down on the ground at the end (TCPP ejects with 50630, which this core does not handle), and the despawns.
enum Sharpbeak
{
    NPC_SHARPBEAK_CAMP                  = 43161,
    NPC_SHARPBEAK_JINTHAALOR            = 51125,

    POINT_SHARPBEAK_END                 = 1
};

namespace
{
    Position const SharpbeakCampPath[] =
    {
        { -75.40077f, -4037.111f, 114.6418f },
        { -68.80193f, -4034.235f, 123.6844f },
        { -62.2031f,  -4031.36f,  132.727f  },
        { -48.5851f,  -4008.04f,  156.977f  },
        { -26.2691f,  -3987.88f,  176.755f  },
        { 11.5087f,   -3960.86f,  203.561f  },
        { 45.0087f,   -3922.58f,  236.672f  },
        { 75.4427f,   -3856.91f,  255.672f  },
        { 74.8351f,   -3768.84f,  279.839f  },
        { -53.0104f,  -3582.62f,  287.755f  },
        { -169.123f,  -3582.08f,  282.866f  },
        { -241.8403f, -3625.01f,  247.4203f }
    };

    Position const SharpbeakJinthaalorPath[] =
    {
        { -249.4681f, -3632.487f, 232.6947f },
        { -241.606f,  -3627.713f, 236.6187f },
        { -235.6163f, -3624.076f, 239.6081f },
        { -226.8698f, -3623.929f, 244.8882f },
        { -193.6406f, -3618.776f, 244.8882f },
        { -149.7292f, -3613.349f, 244.8882f },
        { -103.8976f, -3623.828f, 238.0368f },
        { -41.33681f, -3710.568f, 232.4109f },
        { 6.201389f,  -3739.243f, 214.2869f },
        { 37.44097f,  -3773.431f, 189.4650f },
        { 44.21875f,  -3884.991f, 177.7446f },
        { 39.81424f,  -3934.679f, 168.1627f },
        { 32.17535f,  -3983.781f, 166.1228f },
        { 21.34896f,  -4005.293f, 162.9598f },
        { -5.734375f, -4028.695f, 149.0161f },
        { -23.23611f, -4040.689f, 140.1189f },
        { -35.45139f, -4047.543f, 133.2071f },
        { -59.21181f, -4051.257f, 128.0297f },
        { -76.90625f, -4040.207f, 126.0433f },
        { -77.51563f, -4022.026f, 123.2135f }
    };

    uint32 const SharpbeakTakeOffDelay      = 500;                      // not inside the boarding call
    uint32 const SharpbeakNoRiderDespawn    = 5 * IN_MILLISECONDS;
}

struct npc_sharpbeak : public ScriptedAI
{
    npc_sharpbeak(Creature* creature) : ScriptedAI(creature), _state(STATE_WAITING), _timer(SharpbeakNoRiderDespawn) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->setActive(true, ActiveFlags::Script);
    }

    // Not the follow-on-spawn of CreatureAI::JustAppeared
    void JustAppeared() override { }
    void EnterEvadeMode(EvadeReason /*why*/ = EVADE_REASON_OTHER) override { }
    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }

    void PassengerBoarded(Unit* /*passenger*/, int8 /*seatId*/, bool apply) override
    {
        if (apply)
        {
            if (_state == STATE_WAITING)
            {
                _state = STATE_TAKE_OFF;
                _timer = SharpbeakTakeOffDelay;
            }
            return;
        }

        // The rider is off (logged out, died): Sharpbeak goes. A rider set down at the end is handled there.
        if (_state == STATE_DONE)
            return;

        _state = STATE_DONE;
        me->DespawnOrUnsummon(1000);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == POINT_SHARPBEAK_END && _state == STATE_FLYING)
        {
            _state = STATE_LANDED;
            _timer = 0;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        switch (_state)
        {
            case STATE_WAITING:
                // Never boarded
                if (_timer <= diff)
                {
                    _state = STATE_DONE;
                    me->DespawnOrUnsummon();
                }
                else
                    _timer -= diff;
                break;
            case STATE_TAKE_OFF:
                if (_timer > diff)
                {
                    _timer -= diff;
                    break;
                }
                _state = STATE_FLYING;
                if (me->GetEntry() == NPC_SHARPBEAK_CAMP)
                    me->GetMotionMaster()->MoveSmoothPath(POINT_SHARPBEAK_END, SharpbeakCampPath, std::extent<decltype(SharpbeakCampPath)>::value, false, true);
                else
                    me->GetMotionMaster()->MoveSmoothPath(POINT_SHARPBEAK_END, SharpbeakJinthaalorPath, std::extent<decltype(SharpbeakJinthaalorPath)>::value, false, true);
                break;
            case STATE_LANDED:
                _state = STATE_DONE;
                SetRiderDown();
                me->DespawnOrUnsummon(1000);
                break;
            default:
                break;
        }
    }

private:
    enum SharpbeakState
    {
        STATE_WAITING,
        STATE_TAKE_OFF,
        STATE_FLYING,
        STATE_LANDED,
        STATE_DONE
    };

    // The summit end hangs 10 yd over the terrace: the rider gets down to the ground under Sharpbeak
    void SetRiderDown()
    {
        Vehicle* vehicle = me->GetVehicleKit();
        if (!vehicle)
            return;

        Unit* rider = vehicle->GetPassenger(0);
        if (!rider)
        {
            vehicle->RemoveAllPassengers();
            return;
        }

        // Unit::ExitVehicle ignores the position (the control aura's removal exits at the vehicle's): exit to the ground first; the
        // aura's removal then finds the rider out of the vehicle
        Position ground = me->GetPosition();
        me->UpdateGroundPositionZ(ground.GetPositionX(), ground.GetPositionY(), ground.m_positionZ);
        ObjectGuid const riderGUID = rider->GetGUID();
        rider->_ExitVehicle(&ground);
        me->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE, riderGUID);
    }

    SharpbeakState _state;
    uint32 _timer;
};

void AddSC_hinterlands()
{
    new npc_00x09hl();
    new npc_rinji();
    new creature_script<npc_hinterlands_shadra>("npc_hinterlands_shadra");
    new creature_script<npc_hinterlands_shadra_lookout>("npc_hinterlands_shadra_lookout");
    new quest_hinterlands_shadra_the_venom_queen();
    new creature_script<npc_sharpbeak>("npc_sharpbeak");
}
