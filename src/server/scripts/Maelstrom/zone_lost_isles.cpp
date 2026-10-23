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

#include "ScriptPCH.h"
#include "GameObjectAI.h"
#include "Item.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ScriptedEscortAI.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <list>

// Lost Isles (map 648, zone 4720), part 1: crash landing -> naga arc (#438).
//
// Quests: 14239 Don't Go Into the Light!, 14021 Miner Troubles, 14236 Weed Whacker, 14238 Infrared = Infradead,
// 14240 To the Cliffs, 14242 Precious Cargo, 14243 Warchief's Revenge, 14244 Up, Up & Away!, 24864 Irresistible
// Pool Pony, 24868 Surrender or Else!.  The DB half is sql/updates/world/2026_10_04_15_world_goblin_lost_isles.sql.
//
// Ported from the GPL-licensed MoPCore lost_isles.cpp (the MoP lineage) and AshamaneCore
// lost_isles.cpp, adapted to this core; the ScriptNames follow the MoP ones where the entry is bound.
//
// Every type here lives in an anonymous namespace (zone_lost_isles_volcano.cpp and zone_kezan.cpp are separate
// translation units that also define script structs).
//
// Core traps handled here:
//  * Creature::Update runs AI()->JustAppeared() on the FIRST update of a summon, i.e. after SummonCreature returned and
//    after a synchronous vehicle boarding. npc_escortAI::JustAppeared() resets the escort state, so an escort Start() issued
//    from PassengerBoarded / right after SummonCreature is lost. loi_escort_base therefore only queues the start and
//    performs it from UpdateEscortAI (which runs after JustAppeared).
//  * Vehicle AIs override OnCharmed (CreatureAI::OnCharmed disables the AI when a driver boards, #207).

namespace
{

enum LostIslesIds
{
    // 14239 Don't Go Into the Light!
    QUEST_DONT_GO_INTO_THE_LIGHT        = 14239,
    NPC_GEARGRINDER_GIZMO               = 36600,
    SPELL_NEAR_DEATH                    = 69010,
    SPELL_JUMPER_CABLES                 = 69022,

    // 14021 Miner Troubles
    QUEST_MINER_TROUBLES                = 14021,
    NPC_FRIGHTENED_MINER                = 35813,
    NPC_MINER_ORE_CART                  = 35814,
    NPC_SMART_MINING_MONKEY             = 35812,
    NPC_MINER_TROUBLES_CREDIT           = 35816,
    SPELL_ORE_CART_CHAIN                = 68122,

    // 14236 Weed Whacker
    QUEST_WEED_WHACKER                  = 14236,
    SPELL_WEED_WHACKER_AURA             = 68212,

    // 14238 / 14240 Kilag Gorefang (Wild Overlook)
    QUEST_INFRARED_INFRADEAD            = 14238,
    QUEST_TO_THE_CLIFFS                 = 14240,
    NPC_ORC_SCOUT_COMPANION             = 36100,
    NPC_BASTIA_RIDE                     = 36585,
    SPELL_INFRARED_QUEST_ACCEPT         = 68344,
    SPELL_INFRARED_ORC_SCOUT            = 68338,
    SPELL_INFRARED_HEAT_FOCALS          = 69303,

    // 14242 Precious Cargo
    QUEST_PRECIOUS_CARGO                = 14242,
    NPC_GYROCHOPPA_VEHICLE              = 36143,

    // 14243 Warchief's Revenge
    QUEST_WARCHIEFS_REVENGE             = 14243,
    NPC_CYCLONE_OF_THE_ELEMENTS         = 36178,
    SPELL_CYCLONE_RIDE                  = 68436,

    // 14244 Up, Up & Away!
    QUEST_UP_UP_AND_AWAY                = 14244,
    NPC_SLING_ROCKET_PLAYER             = 36505,
    NPC_UP_UP_AND_AWAY_CREDIT           = 50046,

    // 24864 / 24868 naga arc
    QUEST_IRRESISTIBLE_POOL_PONY        = 24864,
    QUEST_SURRENDER_OR_ELSE             = 24868,
    QUEST_GET_BACK_TO_TOWN              = 24897,
    NPC_NAGA_HATCHLING_COMPANION        = 44589,
    NPC_NAGA_HATCHLING_CREDIT           = 38413,
    NPC_ACE_COMPANION                   = 39198,
    NPC_FACELESS_OF_THE_DEEP            = 38448,
    SPELL_POOL_PONY_AURA                = 71914,
    SPELL_HATCHLING_PROXIMITY_MASTER    = 71917,
    SPELL_PHASE_QUEST_ZONE_06           = 68481,
};

// Summons of 'owner' with the given entry (works whether the summon spell made a minion or a plain summon).
static std::list<Creature*> FindSummonsOf(Unit* owner, uint32 entry, float range)
{
    std::list<Creature*> found;
    std::list<Creature*> all;
    owner->GetCreatureListWithEntryInGrid(all, entry, range);
    for (Creature* creature : all)
        if (creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == owner->GetGUID())
            found.push_back(creature);

    return found;
}

// Base of every escorted summon below (see the file comment: Start() has to wait for the first update).
struct loi_escort_base : public npc_escortAI
{
    explicit loi_escort_base(Creature* creature) : npc_escortAI(creature), _startPending(false), _startRun(true), _startLoop(false) { }

    void AttackStart(Unit* /*who*/) override { }
    void EnterCombat(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }
    void OnCharmed(bool /*apply*/) override { }

    void QueueStart(ObjectGuid playerGuid, bool run, bool loop)
    {
        _startPending = true;
        _startGuid = playerGuid;
        _startRun = run;
        _startLoop = loop;
    }

    void UpdateEscortAI(uint32 diff) override
    {
        if (_startPending && !HasEscortState(STATE_ESCORT_ESCORTING))
        {
            _startPending = false;
            Start(false, _startRun, _startGuid, nullptr, false, _startLoop);
        }

        EscortTick(diff);
    }

protected:
    virtual void EscortTick(uint32 /*diff*/) { }

private:
    bool _startPending;
    bool _startRun;
    bool _startLoop;
    ObjectGuid _startGuid;
};

// ---------------------------------------------------------------------------------------------------------------
// 14239 Don't Go Into the Light!   (Doc Zapnozzle 36608, static spawn in phase 2)
// The player wakes up with 69010 "Near Death!". Doc revives him with jumper cables while talking to Gizmo, then the
// quest is offered; the reward spell 69013 removes the aura. After the turn-in he swims away.
// MoPCore npc_Zapnozzle, timings/lines from AshamaneCore npc_zapnozzle (lost_isles.cpp:67-178), text from Wowpedia.
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_zapnozzle : public ScriptedAI
{
    enum State
    {
        STATE_IDLE,
        STATE_INTRO,
        STATE_WAIT_REWARD,
        STATE_OUTRO,
        STATE_LEAVING
    };

    explicit loi_npc_zapnozzle(Creature* creature) : ScriptedAI(creature), _state(STATE_IDLE), _timer(1000), _step(0) { }

    void Reset() override
    {
        _state = STATE_IDLE;
        _timer = 1000;
        _step = 0;
        _playerGuid.Clear();
        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() != QUEST_DONT_GO_INTO_THE_LIGHT)
            return;

        _playerGuid = player->GetGUID();
        _state = STATE_OUTRO;
        _step = 0;
        _timer = 1000;
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id == 1 && _state == STATE_LEAVING)
            me->DespawnOrUnsummon(500);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_timer > diff)
        {
            _timer -= diff;
            return;
        }

        switch (_state)
        {
            case STATE_IDLE:
            {
                _timer = 1000;
                std::list<Player*> players;
                me->GetPlayerListInGrid(players, 20.0f);
                for (Player* player : players)
                {
                    if (player->HasAura(SPELL_NEAR_DEATH) && player->GetQuestStatus(QUEST_DONT_GO_INTO_THE_LIGHT) == QUEST_STATUS_NONE)
                    {
                        _playerGuid = player->GetGUID();
                        _state = STATE_INTRO;
                        _step = 0;
                        _timer = 0;
                        me->RemoveNpcFlag(UNIT_NPC_FLAG_QUESTGIVER);
                        break;
                    }
                }
                break;
            }
            case STATE_INTRO:
                RunIntroStep();
                break;
            case STATE_WAIT_REWARD:
                // nobody took the quest: start over for the next player
                Reset();
                break;
            case STATE_OUTRO:
                RunOutroStep();
                break;
            case STATE_LEAVING:
                me->DespawnOrUnsummon();
                break;
        }
    }

private:
    void RunIntroStep()
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        switch (_step)
        {
            case 0:
                Talk(0, player);
                _timer = 5000;
                break;
            case 1:
                Talk(1, player);
                _timer = 4000;
                break;
            case 2:
                if (Creature* gizmo = me->FindNearestCreature(NPC_GEARGRINDER_GIZMO, 20.0f))
                    gizmo->AI()->Talk(1, player);
                _timer = 6000;
                break;
            case 3:
                Talk(2, player);
                _timer = 6000;
                break;
            case 4:
                if (player)
                    me->CastSpell(player, SPELL_JUMPER_CABLES, true);
                _timer = 6000;
                break;
            case 5:
                Talk(3, player);
                _timer = 4000;
                break;
            case 6:
                if (player)
                    me->CastSpell(player, SPELL_JUMPER_CABLES, true);
                _timer = 6000;
                break;
            default:
                Talk(4, player);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                _state = STATE_WAIT_REWARD;
                _timer = 120000;
                return;
        }

        ++_step;
    }

    void RunOutroStep()
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        switch (_step)
        {
            case 0:
                Talk(5, player);
                _timer = 4000;
                break;
            case 1:
                Talk(6, player);
                _timer = 2000;
                break;
            default:
                me->SetSpeed(MOVE_SWIM, 2.0f);
                me->GetMotionMaster()->MovePoint(1, 578.49f, 3132.37f, 0.26f);
                _state = STATE_LEAVING;
                _timer = 20000;
                return;
        }

        ++_step;
    }

    State _state;
    uint32 _timer;
    uint32 _step;
    ObjectGuid _playerGuid;
};

// ---------------------------------------------------------------------------------------------------------------
// 14021 Miner Troubles   (Foreman Dampwick 35769 = npc_Mechumide, miner 35813 = npc_mineur_gob)
// Accept summons the Frightened Miner and his ore cart and walks the miner through the Kaja'mite Cavern along the
// script_waypoint path; the walk pauses while a Smart Mining Monkey is within 10 yd. Credit 35816 at the end.
// Quest-miner entry 35813 (the DBC summon 68059 and the entry that has the creature_text) instead of MoPCore's 35810
// (the static scenery miners share that entry).
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_frightened_miner : public loi_escort_base
{
    explicit loi_npc_frightened_miner(Creature* creature) : loi_escort_base(creature), _holdTimer(1000) { }

    void WaypointReached(uint32 pointId) override
    {
        Player* player = GetPlayerForEscort();
        switch (pointId)
        {
            case 6:
            case 12:
                me->HandleEmoteCommand(467);
                if (pointId == 12)
                    Talk(2, player);
                break;
            case 9:
                Talk(1, player);
                me->HandleEmoteCommand(467);
                break;
            case 17:
                Talk(3, player);
                me->HandleEmoteCommand(467);
                break;
            case 18:
                Talk(4, player);
                if (player)
                    player->KilledMonsterCredit(NPC_MINER_TROUBLES_CREDIT);
                if (Creature* cart = me->FindNearestCreature(NPC_MINER_ORE_CART, 15.0f))
                    cart->DespawnOrUnsummon();
                break;
            default:
                break;
        }
    }

protected:
    void EscortTick(uint32 diff) override
    {
        if (_holdTimer > diff)
        {
            _holdTimer -= diff;
            return;
        }

        _holdTimer = 1000;
        // wait for the player to clear the monkeys around the miner
        Creature* monkey = me->FindNearestCreature(NPC_SMART_MINING_MONKEY, 10.0f, true);
        SetEscortPaused(monkey != nullptr);
    }

private:
    uint32 _holdTimer;
};

struct loi_npc_foreman_dampwick : public ScriptedAI
{
    explicit loi_npc_foreman_dampwick(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_MINER_TROUBLES)
            return;

        Talk(0, player);

        // a miner left over from an earlier accept would walk the path a second time
        for (Creature* old : FindSummonsOf(player, NPC_FRIGHTENED_MINER, 200.0f))
            old->DespawnOrUnsummon();
        for (Creature* old : FindSummonsOf(player, NPC_MINER_ORE_CART, 200.0f))
            old->DespawnOrUnsummon();

        TempSummon* miner = player->SummonCreature(NPC_FRIGHTENED_MINER, player->GetPositionX() + 2.0f, player->GetPositionY(),
            player->GetPositionZ() + 1.0f, player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
        TempSummon* cart = player->SummonCreature(NPC_MINER_ORE_CART, player->GetPositionX() - 2.0f, player->GetPositionY(),
            player->GetPositionZ() + 1.0f, player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
        if (!miner || !cart)
        {
            if (miner)
                miner->DespawnOrUnsummon();
            if (cart)
                cart->DespawnOrUnsummon();
            return;
        }

        miner->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        cart->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        miner->CastSpell(cart, SPELL_ORE_CART_CHAIN, true);
        cart->GetMotionMaster()->MoveFollow(miner, 1.0f, float(M_PI));

        if (loi_npc_frightened_miner* minerAI = dynamic_cast<loi_npc_frightened_miner*>(miner->AI()))
            minerAI->QueueStart(player->GetGUID(), false, false);

        miner->AI()->Talk(0, player);
    }
};

// ---------------------------------------------------------------------------------------------------------------
// 14236 Weed Whacker: item 49108 casts 68211, which toggles the whacker form (aura 68212 = vehicle id 493 + periodic
// 68213 that damages the plants around). The kill credit is the plain kill of Freezya 35897. The script is bound to 68211.
// ---------------------------------------------------------------------------------------------------------------
class loi_spell_weed_whacker : public SpellScript
{
    PrepareSpellScript(loi_spell_weed_whacker);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Item* item = GetCastItem();
        if (!item)
            return;

        Player* player = item->GetOwner();
        if (!player)
            return;

        if (player->HasAura(SPELL_WEED_WHACKER_AURA))
            player->RemoveAurasDueToSpell(SPELL_WEED_WHACKER_AURA);
        else if (player->GetQuestStatus(QUEST_WEED_WHACKER) == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(player, SPELL_WEED_WHACKER_AURA, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(loi_spell_weed_whacker::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// ---------------------------------------------------------------------------------------------------------------
// 14238 Infrared = Infradead / 14240 To the Cliffs   (Kilag Gorefang 35917 = npc_killag_sangrecroc)
// 14238: accept casts 68344 (-> aura 68338 -> summons the Orc Scout 36100 companion); the reward removes the aura
//        (AshamaneCore lost_isles.cpp:541-588, MoPCore :543-582).
// 14240: accept puts the player on Bastia 36585 who runs the path to Scout Brax.
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_kilag_overlook : public ScriptedAI
{
    explicit loi_npc_kilag_overlook(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        switch (quest->GetQuestId())
        {
            case QUEST_INFRARED_INFRADEAD:
                Talk(0, player);
                me->CastSpell(player, SPELL_INFRARED_QUEST_ACCEPT, true);
                break;
            case QUEST_TO_THE_CLIFFS:
            {
                Talk(1, player);
                for (Creature* old : FindSummonsOf(player, NPC_BASTIA_RIDE, 100.0f))
                    old->DespawnOrUnsummon();

                TempSummon* bastia = player->SummonCreature(NPC_BASTIA_RIDE, player->GetPositionX(), player->GetPositionY(),
                    player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(300000));
                if (bastia)
                    player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, bastia, true);
                break;
            }
            default:
                break;
        }
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() == QUEST_INFRARED_INFRADEAD)
        {
            player->RemoveAurasDueToSpell(SPELL_INFRARED_ORC_SCOUT);
            player->RemoveAurasDueToSpell(SPELL_INFRARED_HEAT_FOCALS);
        }
    }
};

// 68338: the scout goes away with the aura
class loi_aura_infrared_orc_scout : public AuraScript
{
    PrepareAuraScript(loi_aura_infrared_orc_scout);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        for (Creature* scout : FindSummonsOf(target, NPC_ORC_SCOUT_COMPANION, 200.0f))
            scout->DespawnOrUnsummon();
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(loi_aura_infrared_orc_scout::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 36585 Bastia (npc_pant_gob): rides the 18 script_waypoint points, the last one ends the ride
struct loi_npc_bastia : public loi_escort_base
{
    explicit loi_npc_bastia(Creature* creature) : loi_escort_base(creature) { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (apply && who->GetTypeId() == TYPEID_PLAYER)
            QueueStart(who->GetGUID(), true, false);
    }

    void WaypointReached(uint32 pointId) override
    {
        if (pointId == 17)
            me->DespawnOrUnsummon();
    }
};

// ---------------------------------------------------------------------------------------------------------------
// 14242 Precious Cargo   (Gyrochoppa 36127 = npc_gyrocoptere_quest_giver, flying seat 36143 = npc_girocoptere)
// Accept puts the player into a Gyrochoppa that flies the six points to the Vengeance Wake; the credit is the
// Alliance Wizard kill (SAI 36621). MoPCore summoned 39074 here, which is the 25066 bomber.
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_gyrochoppa_giver : public ScriptedAI
{
    explicit loi_npc_gyrochoppa_giver(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_PRECIOUS_CARGO)
            return;

        for (Creature* old : FindSummonsOf(player, NPC_GYROCHOPPA_VEHICLE, 200.0f))
            old->DespawnOrUnsummon();

        TempSummon* copter = player->SummonCreature(NPC_GYROCHOPPA_VEHICLE, me->GetPositionX(), me->GetPositionY(),
            me->GetPositionZ() + 1.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(300000));
        if (copter)
            player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, copter, true);
    }
};

struct loi_npc_gyrochoppa : public loi_escort_base
{
    explicit loi_npc_gyrochoppa(Creature* creature) : loi_escort_base(creature) { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply || who->GetTypeId() != TYPEID_PLAYER)
            return;

        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_FLIGHT, 3.0f);
        Talk(0, who);
        QueueStart(who->GetGUID(), true, false);
    }

    void WaypointReached(uint32 pointId) override
    {
        if (pointId != 6)
            return;

        if (Vehicle* kit = me->GetVehicleKit())
            kit->RemoveAllPassengers();
        me->DespawnOrUnsummon(1000);
    }
};

// ---------------------------------------------------------------------------------------------------------------
// 14243 Warchief's Revenge   (Thrall 36161, cyclone 36178 = npc_tornade_gob)
// Accept turns the player into the Cyclone of the Elements (vehicle bar: Lightning Strike 68445), which circles the
// Alliance ships along 32 script_waypoint points while the player slays 50 Alliance Sailors. When the quest is
// complete the cyclone flies up to the Wild Overlook (AshamaneCore lost_isles.cpp:715-781, MoPCore :741-845).
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_thrall_warchief : public ScriptedAI
{
    explicit loi_npc_thrall_warchief(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_WARCHIEFS_REVENGE)
            return;

        Talk(uint8(urand(0, 1)), player);

        for (Creature* old : FindSummonsOf(player, NPC_CYCLONE_OF_THE_ELEMENTS, 300.0f))
            old->DespawnOrUnsummon();

        // spawn point from smart_scripts 36161 id 3
        TempSummon* cyclone = player->SummonCreature(NPC_CYCLONE_OF_THE_ELEMENTS, 980.924f, 3823.16f, 11.27f, 0.0f,
            TEMPSUMMON_TIMED_DESPAWN, Milliseconds(900000));
        if (cyclone)
            player->CastSpell(cyclone, SPELL_CYCLONE_RIDE, true);
    }
};

struct loi_npc_cyclone : public loi_escort_base
{
    enum Points
    {
        POINT_FINISH_1  = 1001,
        POINT_FINISH_2  = 1002
    };

    explicit loi_npc_cyclone(Creature* creature) : loi_escort_base(creature), _checkTimer(1000), _finishing(false), _hadPassenger(false) { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply || who->GetTypeId() != TYPEID_PLAYER)
            return;

        _hadPassenger = true;
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_FLIGHT, 3.0f);
        QueueStart(who->GetGUID(), true, true);
    }

    void WaypointReached(uint32 /*pointId*/) override { }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (id == POINT_FINISH_1)
        {
            me->GetMotionMaster()->MovePoint(POINT_FINISH_2, 863.672180f, 2778.023193f, 126.243454f, false);
            return;
        }

        if (id == POINT_FINISH_2)
        {
            Finish();
            return;
        }

        npc_escortAI::MovementInform(type, id);
    }

protected:
    void EscortTick(uint32 diff) override
    {
        if (!_hadPassenger || _finishing)
            return;

        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }

        _checkTimer = 1000;

        Vehicle* kit = me->GetVehicleKit();
        Unit* passenger = kit ? kit->GetPassenger(0) : nullptr;
        Player* player = passenger ? passenger->ToPlayer() : nullptr;
        if (!player)
        {
            // the driver left (dismounted, died, logged out)
            me->DespawnOrUnsummon();
            return;
        }

        if (player->GetQuestStatus(QUEST_WARCHIEFS_REVENGE) == QUEST_STATUS_COMPLETE)
        {
            _finishing = true;
            SetEscortPaused(true);
            me->SetSpeed(MOVE_FLIGHT, 5.0f);
            me->GetMotionMaster()->MovePoint(POINT_FINISH_1, 885.869568f, 3104.882324f, 175.560944f, false);
        }
    }

private:
    void Finish()
    {
        if (Vehicle* kit = me->GetVehicleKit())
        {
            if (Unit* passenger = kit->GetPassenger(0))
            {
                if (Player* player = passenger->ToPlayer())
                {
                    kit->RemoveAllPassengers();
                    // the Wild Overlook landing the old hack used (LoA 36161 smart_scripts id 1)
                    player->NearTeleportTo(871.797f, 2768.51f, 119.17f, 3.0f);
                }
            }
        }

        me->DespawnOrUnsummon(500);
    }

    uint32 _checkTimer;
    bool _finishing;
    bool _hadPassenger;
};

// ---------------------------------------------------------------------------------------------------------------
// 14244 Up, Up & Away!   (GO 196439 = gob_fronde_gobelin, rockets 36505 / 36514 = npc_fusee_gob)
// The Rocket Sling gossip option (menu 10808 "Up, Up & Away!") seats the player in a Sling Rocket 36505 that flies the
// four points to the Landing Site; the credit (50046) is given on arrival. Gallywix's own rocket 36514
// (summoned by his SAI via 68819, npc_fusee_gob) flies the same path without a passenger.
// ---------------------------------------------------------------------------------------------------------------
struct loi_go_rocket_sling : public GameObjectAI
{
    explicit loi_go_rocket_sling(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action != 0)
            return false;

        player->PlayerTalkClass->SendCloseGossip();

        if (player->GetQuestStatus(QUEST_UP_UP_AND_AWAY) == QUEST_STATUS_NONE || player->GetVehicle())
            return true;

        TempSummon* rocket = player->SummonCreature(NPC_SLING_ROCKET_PLAYER, me->GetPositionX(), me->GetPositionY(),
            me->GetPositionZ() + 3.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(120000));
        if (rocket)
            player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, rocket, true);

        return true;
    }
};

struct loi_npc_sling_rocket : public loi_escort_base
{
    explicit loi_npc_sling_rocket(Creature* creature) : loi_escort_base(creature) { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply || who->GetTypeId() != TYPEID_PLAYER)
            return;

        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_FLIGHT, 5.0f);
        QueueStart(who->GetGUID(), true, false);
    }

    void WaypointReached(uint32 pointId) override
    {
        if (pointId != 3)
            return;

        if (Player* player = GetPlayerForEscort())
            player->KilledMonsterCredit(NPC_UP_UP_AND_AWAY_CREDIT);

        if (Vehicle* kit = me->GetVehicleKit())
            kit->RemoveAllPassengers();
        me->DespawnOrUnsummon(1000);
    }
};

struct loi_npc_gallywix_rocket : public loi_escort_base
{
    explicit loi_npc_gallywix_rocket(Creature* creature) : loi_escort_base(creature)
    {
        QueueStart(ObjectGuid::Empty, true, false);
    }

    void Reset() override
    {
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_FLIGHT, 5.0f);
    }

    void WaypointReached(uint32 /*pointId*/) override { }
};

// ---------------------------------------------------------------------------------------------------------------
// 24864 Irresistible Pool Pony   (hatchlings 38412 / 44578 / 44579 / 44580 = npc_young_naga_gob)
// A player carrying the Pool Pony aura 71914 within 10 yd makes the hatchling follow him as the summoned companion
// 44589 and credits 38413 (spellclick 71916 is not in the DBC set, so the LoA SAI could never fire).
// Megs 38432 (npc_megs_isle_gob) dismisses the companions on turn-in.
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_young_naga : public ScriptedAI
{
    explicit loi_npc_young_naga(Creature* creature) : ScriptedAI(creature), _timer(1000) { }

    void Reset() override
    {
        _timer = 1000;
        me->GetMotionMaster()->MoveRandom(5.0f);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_timer > diff)
        {
            _timer -= diff;
            return;
        }

        _timer = 1000;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 10.0f);
        for (Player* player : players)
        {
            if (!player->HasAura(SPELL_POOL_PONY_AURA) || player->GetQuestStatus(QUEST_IRRESISTIBLE_POOL_PONY) != QUEST_STATUS_INCOMPLETE)
                continue;

            if (TempSummon* naga = player->SummonCreature(NPC_NAGA_HATCHLING_COMPANION, me->GetPositionX(), me->GetPositionY(),
                me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, Milliseconds(40000)))
            {
                naga->CastSpell(naga, SPELL_HATCHLING_PROXIMITY_MASTER, true);
                naga->GetMotionMaster()->MoveFollow(player, float(urand(1, 4)), float(urand(0, 6)));
                player->KilledMonsterCredit(NPC_NAGA_HATCHLING_CREDIT);
            }

            me->DespawnOrUnsummon();
            break;
        }
    }

private:
    uint32 _timer;
};

// ---------------------------------------------------------------------------------------------------------------
// 24868 Surrender or Else!   (Megs 38432 = npc_megs_isle_gob, Ace 39198 = npc_crack_isle_gob)
// Megs: 24868 accept summons Ace, 24864 turn-in dismisses the hatchlings, 24897 accept casts the phase aura (MoPCore).
// Ace follows the player to the Faceless of the Deep Void Zone (38450 spawn at 131.56 1938.32 8.62), calls out the naga
// leader, the Faceless (summoned copy of 38448, immune and friendly until then) talks, Ace runs and the Faceless becomes
// hostile and attackable (AshamaneCore lost_isles.cpp:1273-1406, MoPCore :1365-1492).
// ---------------------------------------------------------------------------------------------------------------
struct loi_npc_megs : public ScriptedAI
{
    explicit loi_npc_megs(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        switch (quest->GetQuestId())
        {
            case QUEST_SURRENDER_OR_ELSE:
            {
                Talk(0, player);
                for (Creature* old : FindSummonsOf(player, NPC_ACE_COMPANION, 300.0f))
                    old->DespawnOrUnsummon();

                player->SummonCreature(NPC_ACE_COMPANION, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
                    player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(900000));
                break;
            }
            case QUEST_IRRESISTIBLE_POOL_PONY:
                Talk(1, player);
                break;
            case QUEST_GET_BACK_TO_TOWN:
                Talk(2, player);
                player->CastSpell(player, SPELL_PHASE_QUEST_ZONE_06, true);
                break;
            default:
                break;
        }
    }

    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() != QUEST_IRRESISTIBLE_POOL_PONY)
            return;

        for (Creature* hatchling : FindSummonsOf(player, NPC_NAGA_HATCHLING_COMPANION, 900.0f))
            hatchling->DespawnOrUnsummon();
    }
};

struct loi_npc_crack : public ScriptedAI
{
    enum Stage
    {
        STAGE_WALK,
        STAGE_NAGA_TALK,
        STAGE_NAGA_ATTACK,
        STAGE_LEAVE
    };

    explicit loi_npc_crack(Creature* creature) : ScriptedAI(creature), _stage(STAGE_WALK), _timer(1000), _step(0), _lineTimer(4000), _lineCount(0) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner ? summoner->ToPlayer() : nullptr;
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGuid = player->GetGUID();
        me->GetMotionMaster()->MoveFollow(player, 1.0f, 1.0f);
        Talk(1, player);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_playerGuid.IsEmpty())
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (_timer > diff)
            _timer -= diff;
        else
        {
            _timer = 1000;
            RunStage(player);
        }

        if (_stage == STAGE_WALK)
        {
            if (_lineTimer > diff)
                _lineTimer -= diff;
            else
            {
                // "You ready to make their leader surrender?" once, then the sales-pitch yells along the way
                if (_lineCount == 0)
                    Talk(2, player);
                else
                    Talk(3, player);
                ++_lineCount;
                _lineTimer = 9000;
            }
        }
    }

private:
    void RunStage(Player* player)
    {
        switch (_stage)
        {
            case STAGE_WALK:
            {
                if (me->GetDistance(131.564f, 1938.32f, 8.62286f) > 6.0f)
                    break;

                Talk(4, player);
                if (TempSummon* naga = player->SummonCreature(NPC_FACELESS_OF_THE_DEEP, 131.564f, 1938.32f, 10.6f, 0.0f,
                    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(600000)))
                {
                    naga->SetFaction(35);
                    _nagaGuid = naga->GetGUID();
                }

                _stage = STAGE_NAGA_TALK;
                _step = 0;
                _timer = 2000;
                break;
            }
            case STAGE_NAGA_TALK:
            {
                if (Creature* naga = ObjectAccessor::GetCreature(*me, _nagaGuid))
                    naga->AI()->Talk(uint8(_step), player);

                ++_step;
                _timer = 4000;
                if (_step > 3)
                {
                    Talk(5, player);
                    _stage = STAGE_NAGA_ATTACK;
                    _timer = 2000;
                }
                break;
            }
            case STAGE_NAGA_ATTACK:
            {
                if (Creature* naga = ObjectAccessor::GetCreature(*me, _nagaGuid))
                {
                    naga->SetFaction(14);
                    naga->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
                    naga->AI()->AttackStart(player);
                }

                float x, y, z, o;
                me->GetHomePosition(x, y, z, o);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
                _stage = STAGE_LEAVE;
                _timer = 8000;
                break;
            }
            case STAGE_LEAVE:
                me->DespawnOrUnsummon();
                break;
        }
    }

    Stage _stage;
    uint32 _timer;
    uint32 _step;
    uint32 _lineTimer;
    uint32 _lineCount;
    ObjectGuid _playerGuid;
    ObjectGuid _nagaGuid;
};

} // namespace

void AddSC_lost_isles()
{
    new creature_script<loi_npc_zapnozzle>("npc_Zapnozzle");
    new creature_script<loi_npc_foreman_dampwick>("npc_Mechumide");
    new creature_script<loi_npc_frightened_miner>("npc_mineur_gob");
    new spell_script<loi_spell_weed_whacker>("spell_weed_whacker");
    new creature_script<loi_npc_kilag_overlook>("npc_killag_sangrecroc");
    new aura_script<loi_aura_infrared_orc_scout>("aura_infrared_orc_scout");
    new creature_script<loi_npc_bastia>("npc_pant_gob");
    new creature_script<loi_npc_gyrochoppa_giver>("npc_gyrocoptere_quest_giver");
    new creature_script<loi_npc_gyrochoppa>("npc_girocoptere");
    new creature_script<loi_npc_thrall_warchief>("npc_li_thrall_warchief");
    new creature_script<loi_npc_cyclone>("npc_tornade_gob");
    new GenericGameObjectScript<loi_go_rocket_sling>("gob_fronde_gobelin");
    new creature_script<loi_npc_sling_rocket>("npc_li_sling_rocket");
    new creature_script<loi_npc_gallywix_rocket>("npc_fusee_gob");
    new creature_script<loi_npc_young_naga>("npc_young_naga_gob");
    new creature_script<loi_npc_megs>("npc_megs_isle_gob");
    new creature_script<loi_npc_crack>("npc_crack_isle_gob");
}
