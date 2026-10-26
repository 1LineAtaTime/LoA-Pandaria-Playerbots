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
#include "GameObject.h"
#include "Item.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <list>
#include <tuple>
#include <unordered_map>

// Kezan (map 648) creature scripts.
//
// Quest 14071 "Rolling with my Homies": the player summons the Hot Rod 34840
// ("Keys to the Hot Rod", item 46856, spell 66393) and drives it past Izzy
// 34890, Ace 34892 and Gobber 34954. Each one then summons its SUMMON-flagged
// copy (34959/34957/34958) onto the player, which gives the objective credit
// and boards a rear seat of the Hot Rod.
//
// Ported from the GPL-licensed AshamaneCore kezan.cpp
// (npc_hot_rod_rolling_my_homies / npc_homies / npc_homies_summon /
// spell_radio) and adapted to this core; see
// the substitutions. The key
// fix over the old npc_original_hot_rod: CreatureAI::OnCharmed disables the
// AI as soon as the driver charms the vehicle, so PassengerBoarded never ran.
//
// spell_klaxon (61178) is not ported as its own class: 61178 is already bound to
// spell_wintergrasp_grab_passenger, whose homie branch (below) dispatches
// ACTION_GRAB_PASSENGER to npc_homies for caster 34840. The car really honks
// with 66298 ("Honk Horn"), which is a plain creature spell and needs no script.

enum QuestRollingMyHomies
{
    QUEST_ROLLING_WITH_MY_HOMIES            = 14071,
    OBJECTIVE_IZZY_PICKED_UP                = 265516,
    OBJECTIVE_ACE_PICKED_UP                 = 265517,
    OBJECTIVE_GOBBER_PICKED_UP              = 265518,

    NPC_HOT_ROD                             = 34840,

    NPC_IZZY                                = 34890,
    NPC_ACE                                 = 34892,
    NPC_GOBBER                              = 34954,

    NPC_IZZY_SUMMON                         = 34959,
    NPC_ACE_SUMMON                          = 34957,
    NPC_GOBBER_SUMMON                       = 34958,

    SPELL_GENERIC_INVISIBILITY_DETECTION_1  = 49416,
    SPELL_GENERIC_INVISIBILITY_DETECTION_2  = 49417,
    SPELL_GENERIC_INVISIBILITY_DETECTION_3  = 60922,

    SPELL_SUMMON_ACE                        = 66597,
    SPELL_SUMMON_GOBBER                     = 66599,
    SPELL_SUMMON_IZZY                       = 66600,

    SPELL_RESUMMON_ACE                      = 66644,
    SPELL_RESUMMON_GOBBER                   = 66645,
    SPELL_RESUMMON_IZZY                     = 66646,

    SPELL_HOT_ROD_KNOCKBACK                 = 66301,

    SOUND_HOT_ROD_RADIO                     = 23406,

    ACTION_HOMIES_ENTER_VEHICLE             = 1,

    // #207: dispatched by zone_wintergrasp.cpp's spell_wintergrasp_grab_passenger
    // (spell 61178 "Grab Passenger", Hot Rod vehicle bar) to npc_homies. That file
    // repeats the value as a file-local constant - keep the two in sync.
    ACTION_GRAB_PASSENGER                   = 2,
};

// #207: 66301 "...Hot Rod - Knockback" (12 yd) is design intent, not a bug -
// AshamaneCore's npc_citoyen_gob (kezan.cpp:768-801)
// makes the citizen/looter crowd react to it, and it is part of quest 14121. An
// unfiltered 4 Hz cast pushed the quest NPCs around (the homies and Megs all sit
// inside 12 yd, further out than the 10 yd proximity pickup radius), so it was OFF
// until npc_citoyen_gob was ported.
// #438: npc_citoyen_gob is ported below (KezanGoblin::npc_citoyen_gob), so the cast
// is back ON, restricted to a driver who has "Robbing Hoods" (14121) active - the
// only quest that needs it (Wowpedia 14121: run over 12 Hired Looters).
static bool const HOT_ROD_KNOCKBACK_ENABLED = true;
static uint32 const QUEST_ROBBING_HOODS_KNOCKBACK = 14121;

// summon entry -> { Hot Rod seat, resummon spell }
std::unordered_map<uint32, std::tuple<int8, uint32>> const seatPerHomie =
{
    { NPC_ACE_SUMMON,       { 1, SPELL_RESUMMON_ACE }       },
    { NPC_GOBBER_SUMMON,    { 2, SPELL_RESUMMON_GOBBER }    },
    { NPC_IZZY_SUMMON,      { 3, SPELL_RESUMMON_IZZY }      },
};

// in-world homie entry -> { quest objective, summon spell, invisibility detection spell }
std::unordered_map<uint32, std::tuple<uint32, uint32, uint32>> const objectiveAndSpellsPerHomie =
{
    { NPC_ACE,      { OBJECTIVE_ACE_PICKED_UP,      SPELL_SUMMON_ACE,       SPELL_GENERIC_INVISIBILITY_DETECTION_1 } },
    { NPC_GOBBER,   { OBJECTIVE_GOBBER_PICKED_UP,   SPELL_SUMMON_GOBBER,    SPELL_GENERIC_INVISIBILITY_DETECTION_2 } },
    { NPC_IZZY,     { OBJECTIVE_IZZY_PICKED_UP,     SPELL_SUMMON_IZZY,      SPELL_GENERIC_INVISIBILITY_DETECTION_3 } },
};

// The player's own homie summon of the given entry, if one is nearby. Searched
// by summoner rather than through m_Controlled, so it works whether the summon
// spell makes a minion or a plain temporary summon.
static Creature* FindHomieSummon(Player* player, uint32 entry)
{
    std::list<Creature*> summons;
    player->GetCreatureListWithEntryInGrid(summons, entry, 50.0f);
    for (Creature* summon : summons)
        if (summon->IsSummon() && summon->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return summon;

    return nullptr;
}

struct npc_sister_goldskimmer : public ScriptedAI
{
    npc_sister_goldskimmer(Creature* creature) : ScriptedAI(creature) { }

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->GetTypeId() == TYPEID_PLAYER && me->IsFriendlyTo(who) && me->isInFrontInMap(who, 7) && !who->HasAura(74973))
        {
            Talk(0, who);
            me->CastSpell(who, 74973, true);
        }
    }
};

// 14071
class quest_rolling_with_my_homies : public QuestScript
{
public:
    quest_rolling_with_my_homies() : QuestScript("quest_rolling_with_my_homies") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus == QUEST_STATUS_INCOMPLETE)
        {
            player->CastSpell(player, SPELL_GENERIC_INVISIBILITY_DETECTION_1, true);
            player->CastSpell(player, SPELL_GENERIC_INVISIBILITY_DETECTION_2, true);
            player->CastSpell(player, SPELL_GENERIC_INVISIBILITY_DETECTION_3, true);
        }
    }
};

// 34840
struct npc_hot_rod_rolling_my_homies : public ScriptedAI
{
    npc_hot_rod_rolling_my_homies(Creature* creature) : ScriptedAI(creature) { }

    // Keep the AI running while the driver charms the vehicle, otherwise
    // Vehicle::AddPassenger skips PassengerBoarded.
    void OnCharmed(bool /*apply*/) override { }

    void AttackStart(Unit* /*who*/) override { }
    void EnterCombat(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply || !me->IsSummon())
            return;

        Player* player = who->ToPlayer();
        if (!player)
            return;

        // Knock citizens aside while driving forward. The task holds the GUID,
        // not the Player*, and stops once the driver is gone or out of the seat.
        // #207: gated off by default - see HOT_ROD_KNOCKBACK_ENABLED above.
        if (HOT_ROD_KNOCKBACK_ENABLED)
        {
            ObjectGuid driverGuid = player->GetGUID();
            scheduler.CancelAll();
            scheduler.Schedule(Milliseconds(250), [this, driverGuid](TaskContext context)
            {
                Player* driver = ObjectAccessor::GetPlayer(*me, driverGuid);
                if (!driver || driver->GetVehicleBase() != me)
                    return;

                if (driver->GetQuestStatus(QUEST_ROBBING_HOODS_KNOCKBACK) == QUEST_STATUS_INCOMPLETE &&
                    me->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FORWARD))
                    driver->CastSpell(driver, SPELL_HOT_ROD_KNOCKBACK, true);

                context.Repeat();
            });
        }

        // Homies already picked up ride along again; after the quest they are resummoned.
        for (auto const& itr : seatPerHomie)
        {
            if (Creature* homie = FindHomieSummon(player, itr.first))
                homie->AI()->DoAction(ACTION_HOMIES_ENTER_VEHICLE);
            else if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) == QUEST_STATUS_REWARDED)
                player->CastSpell(player, std::get<1>(itr.second), true);
        }
    }

private:
    TaskScheduler scheduler;
};

// 66299
class spell_radio : public SpellScript
{
    PrepareSpellScript(spell_radio);

    void HandleOnHit()
    {
        if (Unit* owner = GetCaster()->GetCharmerOrOwner())
            if (Player* player = owner->ToPlayer())
                GetCaster()->PlayDistanceSound(SOUND_HOT_ROD_RADIO, player);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_radio::HandleOnHit);
    }
};

// 34892 Ace
// 34954 Gobber
// 34890 Izzy
struct npc_homies : public ScriptedAI
{
    npc_homies(Creature* creature) : ScriptedAI(creature) { }

    void MoveInLineOfSight(Unit* who) override
    {
        // Notified either by the Hot Rod itself or by the player riding it.
        Unit* hotRod = who->GetEntry() == NPC_HOT_ROD ? who : who->GetVehicleBase();
        if (!hotRod || hotRod->GetEntry() != NPC_HOT_ROD || !hotRod->GetVehicleKit())
            return;

        Unit* driver = hotRod->GetVehicleKit()->GetPassenger(0);
        if (!driver)
            return;

        Player* player = driver->ToPlayer();
        if (!player)
            return;

        if (me->GetDistance(player) > 10.0f)
            return;

        PickUp(player);
    }

    // #207: 61178 "Grab Passenger" from the Hot Rod's vehicle bar, dispatched by
    // zone_wintergrasp.cpp's spell_wintergrasp_grab_passenger (it cannot seat the
    // homie itself: the objective is credited by the SUMMON-flagged copy, not by
    // the in-world one). Same effect as the 10 yd proximity pickup above, minus
    // the range gate - the button is its long-range version. 61178 reaches 150 yd.
    void DoAction(int32 action) override
    {
        if (action != ACTION_GRAB_PASSENGER)
            return;

        std::list<Creature*> hotRods;
        me->GetCreatureListWithEntryInGrid(hotRods, NPC_HOT_ROD, 150.0f);
        for (Creature* hotRod : hotRods)
        {
            Vehicle* kit = hotRod->GetVehicleKit();
            if (!kit)
                continue;

            Unit* driver = kit->GetPassenger(0);
            if (!driver)
                continue;

            if (Player* player = driver->ToPlayer())
            {
                PickUp(player);
                return;
            }
        }
    }

private:
    // Reveal this homie and summon the copy that takes the objective credit and
    // boards a rear seat (npc_homies_summon). No-op once credited.
    void PickUp(Player* player)
    {
        if (player->GetQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES) != QUEST_STATUS_INCOMPLETE)
            return;

        uint32 objectiveId, summonSpell, invisibilityDetectionSpell;
        std::tie(objectiveId, summonSpell, invisibilityDetectionSpell) = objectiveAndSpellsPerHomie.at(me->GetEntry());

        if (player->GetQuestObjectiveCounter(objectiveId))
            return;

        player->RemoveAurasDueToSpell(invisibilityDetectionSpell);
        player->CastSpell(player, summonSpell, true);
    }
};

// 34957 Ace Summon
// 34958 Gobber Summon
// 34959 Izzy Summon
struct npc_homies_summon : public ScriptedAI
{
    npc_homies_summon(Creature* creature) : ScriptedAI(creature), _enterVehicleTimer(0) { }

    void Reset() override
    {
        _enterVehicleTimer = 0;

        if (Player* player = GetSummonerPlayer())
        {
            player->KilledMonsterCredit(me->GetEntry());

            // Board a moment later rather than from inside the summon/add-to-map call.
            _enterVehicleTimer = 500;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_enterVehicleTimer)
            return;

        if (_enterVehicleTimer <= diff)
        {
            _enterVehicleTimer = 0;
            DoAction(ACTION_HOMIES_ENTER_VEHICLE);
        }
        else
            _enterVehicleTimer -= diff;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_HOMIES_ENTER_VEHICLE || me->GetVehicle())
            return;

        Player* player = GetSummonerPlayer();
        if (!player)
            return;

        Vehicle* vehicle = player->GetVehicle();
        int8 seatIdBefore = seatPerHomie.count(me->GetEntry())
            ? std::get<0>(seatPerHomie.at(me->GetEntry())) : int8(-99);

        if (!vehicle || vehicle->GetBase()->GetEntry() != NPC_HOT_ROD)
            return;

        int8 seatId = seatIdBefore;
        if (!vehicle->HasEmptySeat(seatId))
            seatId = -1;

        me->EnterVehicle(vehicle->GetBase(), seatId);
    }

private:
    Player* GetSummonerPlayer() const
    {
        if (!me->IsSummon())
            return nullptr;

        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
            return summoner->ToPlayer();

        return nullptr;
    }

    uint32 _enterVehicleTimer;
};

// ============================================================================
// #438 dev A - the rest of the Kezan (zone 4737) goblin quests, level 1-5.
//
// Ported from the GPL-licensed AshamaneCore kezan.cpp and cross-checked against
// MoPCore's kezan.cpp (the MoP lineage). Every script keeps its MoP
// ScriptName where the entry is bound, so the two DBs stay
// comparable. All creature scripts that drive a vehicle override OnCharmed
// (CreatureAI::OnCharmed disables the AI when a driver boards, #207).
// ============================================================================
namespace KezanGoblin
{
enum Footbomb
{
    QUEST_THE_REPLACEMENTS              = 24488,
    QUEST_NECESSARY_ROUGHNESS           = 24502,
    QUEST_FOURTH_AND_GOAL_ALT           = 24503,
    QUEST_FOURTH_AND_GOAL               = 28414,
    QUEST_GIVE_SASSY_THE_NEWS           = 24520,

    NPC_COACH_CROSSCHECK                = 37106,
    NPC_BILGEWATER_BUCCANEER            = 37179,
    NPC_BILGEWATER_BUCCANEER_GOAL       = 37213,
    NPC_STEAMWHEEDLE_SHARK              = 37114,
    NPC_FOURTH_AND_GOAL_TARGET          = 37203,

    SPELL_SUMMON_BUCCANEER              = 70015, // Necessary Roughness: summons 37179, rider casts 70016 (ride + credit 48271)
    SPELL_SUMMON_BUCCANEER_GOAL         = 70075, // Fourth and Goal: summons 37213, rider boards with 46598
    SPELL_GROUND_RUMBLE_EARTHQUAKE      = 78607,
    SPELL_SUMMON_DEATHWING              = 66322,
    SPELL_THROW_FOOTBOMB                = 69993,
    SPELL_KICK_FOOTBOMB                 = 70052,

    TALK_COACH_REPLACEMENTS             = 0,
    TALK_COACH_ROUGHNESS                = 1,
    TALK_COACH_NEWS                     = 2,
    TALK_COACH_THROW_HINT               = 3,
    TALK_COACH_KICK_HINT                = 4,

    EVENT_EJECT_DELAY                   = 6000, // ms between "Fourth and Goal" complete and leaving the shredder
};

Position const SharkSpawnPos[8] =
{
    { -8278.00f, 1485.00f, 43.14f, 6.28f },
    { -8285.00f, 1485.00f, 43.90f, 6.28f },
    { -8285.61f, 1489.94f, 43.90f, 6.28f },
    { -8285.76f, 1480.03f, 43.90f, 6.28f },
    { -8290.68f, 1475.38f, 43.95f, 6.28f },
    { -8291.06f, 1480.00f, 43.98f, 6.28f },
    { -8290.98f, 1487.85f, 43.99f, 6.28f },
    { -8290.90f, 1492.98f, 43.90f, 6.28f },
};

// Talk from the Coach standing next to the pitch (the boss-emote hints of 24502/28414).
static void CoachHint(Player* player, uint8 group)
{
    if (Creature* coach = player->FindNearestCreature(NPC_COACH_CROSSCHECK, 200.0f))
        coach->AI()->Talk(group, player);
}

// 37106 - bound as npc_gw (the MoP name).
// Quest accept lines (Wowpedia "On accept") and the Necessary Roughness boarding:
// MoPCore's npc_gw summoned the shredder on accept; retail (and the DB: creature
// 48526 + npc_spellclick_spells 70015/70075) lets the player click the parked
// shredder. Both are supported - the auto-cast needs 48526 near the Coach, if
// the dest-nearby search misses it the player simply clicks the shredder.
class npc_gw : public CreatureScript
{
public:
    npc_gw() : CreatureScript("npc_gw") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        switch (quest->GetQuestId())
        {
            case QUEST_THE_REPLACEMENTS:
                creature->AI()->Talk(TALK_COACH_REPLACEMENTS, player);
                break;
            case QUEST_NECESSARY_ROUGHNESS:
                creature->AI()->Talk(TALK_COACH_ROUGHNESS, player);
                if (!player->IsOnVehicle())
                    player->CastSpell(player, SPELL_SUMMON_BUCCANEER, true);
                break;
            case QUEST_GIVE_SASSY_THE_NEWS:
                creature->AI()->Talk(TALK_COACH_NEWS, player);
                break;
            default:
                break;
        }

        return false;
    }
};

// 28414 / 24503 "Fourth and Goal" (bound through quest_template_addon.ScriptName).
// "Necessary Roughness" (24502) auto-submits and 28414 auto-accepts: swap the
// shredder for the goal one (37213), then shake the ground and fly Deathwing
// over on completion (Wowpedia 24520 / Ashamane quest_fourth_and_goal).
class quest_fourth_and_goal : public QuestScript
{
public:
    quest_fourth_and_goal() : QuestScript("quest_fourth_and_goal") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (newStatus == QUEST_STATUS_INCOMPLETE && oldStatus == QUEST_STATUS_NONE)
        {
            if (player->IsOnVehicle())
                player->ExitVehicle();

            player->m_Events.Schedule(1000, [player]()
            {
                player->NearTeleportTo(-8250.910156f, 1484.290039f, 41.499901f, 3.124140f);
                player->m_Events.Schedule(1000, [player]()
                {
                    player->CastSpell(player, SPELL_SUMMON_BUCCANEER_GOAL, true);
                });
            });
        }
        else if (newStatus == QUEST_STATUS_COMPLETE)
        {
            player->CastSpell(player, SPELL_GROUND_RUMBLE_EARTHQUAKE, true);
            player->CastSpell(player, SPELL_SUMMON_DEATHWING, true);
        }
        else if (newStatus == QUEST_STATUS_REWARDED)
        {
            player->CastSpell(player, SPELL_GROUND_RUMBLE_EARTHQUAKE, true);
        }
    }
};

// 37179 Bilgewater Buccaneer (Necessary Roughness shredder)
// 37213 Bilgewater Buccaneer (Fourth and Goal shredder)
struct npc_boucanier_gob : public ScriptedAI
{
    npc_boucanier_gob(Creature* creature) : ScriptedAI(creature), _boardTimer(0), _ejectTimer(0) { }

    void Reset() override
    {
        _ejectTimer = 0;
    }

    // #438 run 1 (build 4, leg L02): "Fourth and Goal" summons 37213 with 70075, which has no ride effect.
    // MoP 5.4.8 seats the summoner from 37213's SmartAI actionlist 3721300 (cross-cast 46598), but this
    // ScriptName replaces that SAI - so seat the summoner here, after the shredder's first update.
    void IsSummonedBy(Unit* summoner) override
    {
        if (me->GetEntry() != NPC_BILGEWATER_BUCCANEER_GOAL || !summoner->IsPlayer())
            return;

        _summonerGuid = summoner->GetGUID();
        _boardTimer = 1000;
    }

    void OnCharmed(bool /*apply*/) override { }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        Player* player = who->ToPlayer();
        if (!player)
            return;

        if (apply)
        {
            // The shredder is a stationary firing platform.
            me->SetSpeed(MOVE_RUN, 0.001f);

            if (me->GetEntry() == NPC_BILGEWATER_BUCCANEER)
            {
                if (player->GetQuestStatus(QUEST_NECESSARY_ROUGHNESS) == QUEST_STATUS_INCOMPLETE)
                {
                    CoachHint(player, TALK_COACH_THROW_HINT);
                    for (uint8 i = 0; i < 8; ++i)
                        me->SummonCreature(NPC_STEAMWHEEDLE_SHARK, SharkSpawnPos[i], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(120000));
                }
            }
            else if (player->GetQuestStatus(QUEST_FOURTH_AND_GOAL) == QUEST_STATUS_INCOMPLETE ||
                     player->GetQuestStatus(QUEST_FOURTH_AND_GOAL_ALT) == QUEST_STATUS_INCOMPLETE)
            {
                CoachHint(player, TALK_COACH_KICK_HINT);
            }
        }
        else
        {
            DespawnSharks();
            me->DespawnOrUnsummon(1000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->GetEntry() != NPC_BILGEWATER_BUCCANEER_GOAL)
            return;

        if (_boardTimer)
        {
            if (_boardTimer <= diff)
            {
                _boardTimer = 0;
                if (Player* summoner = ObjectAccessor::GetPlayer(*me, _summonerGuid))
                    if (!summoner->GetVehicle() && summoner->IsWithinDistInMap(me, 30.0f))
                        summoner->EnterVehicle(me, 0);
            }
            else
                _boardTimer -= diff;
        }

        Vehicle* kit = me->GetVehicleKit();
        Unit* passenger = kit ? kit->GetPassenger(0) : nullptr;
        Player* player = passenger ? passenger->ToPlayer() : nullptr;
        if (!player)
            return;

        // The goal is scored (quest COMPLETE): let the earthquake and Deathwing play, then drop the player off.
        QuestStatus s28414 = player->GetQuestStatus(QUEST_FOURTH_AND_GOAL);
        QuestStatus s24503 = player->GetQuestStatus(QUEST_FOURTH_AND_GOAL_ALT);
        bool scored = s28414 == QUEST_STATUS_COMPLETE || s28414 == QUEST_STATUS_REWARDED ||
                      s24503 == QUEST_STATUS_COMPLETE || s24503 == QUEST_STATUS_REWARDED;
        if (!scored)
        {
            _ejectTimer = 0;
            return;
        }

        _ejectTimer += diff;
        if (_ejectTimer >= EVENT_EJECT_DELAY)
        {
            _ejectTimer = 0;
            player->ExitVehicle();
        }
    }

private:
    // The sharks are summoned by this vehicle.
    void DespawnSharks()
    {
        std::list<Creature*> sharks;
        me->GetCreatureListWithEntryInGrid(sharks, NPC_STEAMWHEEDLE_SHARK, 100.0f);
        for (Creature* shark : sharks)
            if (shark->IsSummon() && shark->ToTempSummon()->GetSummonerGUID() == me->GetGUID())
                shark->DespawnOrUnsummon();
    }

    ObjectGuid _summonerGuid;
    uint32 _boardTimer;
    uint32 _ejectTimer;
};

// 37114 Steamwheedle Shark: swims at the shredder, dies to the footbomb (69993).
struct npc_shark_gob : public ScriptedAI
{
    npc_shark_gob(Creature* creature) : ScriptedAI(creature), _startTimer(0), _started(false) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetWalk(true);
        _startTimer = 3000;
        _started = false;
    }

    void AttackStart(Unit* /*who*/) override { }
    void JustReachedHome() override { }

    void UpdateAI(uint32 diff) override
    {
        if (_started)
            return;

        if (_startTimer <= diff)
        {
            _started = true;
            me->GetMotionMaster()->MovePoint(1, -8260.0f, me->GetPositionY(), me->GetPositionZ());
        }
        else
            _startTimer -= diff;
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id != SPELL_THROW_FOOTBOMB || !me->IsAlive())
            return;

        // The footbomb is a vehicle spell: the caster is the shredder, the thrower its driver.
        if (Vehicle* kit = caster->GetVehicleKit())
            if (Unit* passenger = kit->GetPassenger(0))
                if (Player* player = passenger->ToPlayer())
                    player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());

        caster->Kill(me);
    }

private:
    uint32 _startTimer;
    bool _started;
};

// 37203 Fourth And Goal Target: the invisible target between the smokestacks.
struct npc_fourth_and_goal_target : public ScriptedAI
{
    npc_fourth_and_goal_target(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->setActive(true);
    }

    void JustReachedHome() override { }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id != SPELL_KICK_FOOTBOMB)
            return;

        if (Vehicle* kit = caster->GetVehicleKit())
            if (Unit* passenger = kit->GetPassenger(0))
                if (Player* player = passenger->ToPlayer())
                    player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
    }
};

// 70052 Kick Footbomb. The DB's implicit-target condition for this spell names
// 37213 (the shredder) instead of the goal target, so the footbomb can miss the
// 37203 SpellHit above; like AshamaneCore, credit the kick itself.
class spell_fourth_and_goal_kick_footbomb : public SpellScript
{
    PrepareSpellScript(spell_fourth_and_goal_kick_footbomb);

    void CreditKick()
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsVehicle())
            return;

        if (Unit* passenger = caster->GetVehicleKit()->GetPassenger(0))
            if (Player* player = passenger->ToPlayer())
                player->KilledMonsterCredit(NPC_FOURTH_AND_GOAL_TARGET);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_fourth_and_goal_kick_footbomb::CreditKick);
    }
};

// ---------------------------------------------------------------------------
// 14122 "The Great Bank Heist"
//
// Right-clicking one of the First Bank of Kezan vault doors (gameobject 195449,
// type 22, spell 67555) summons a private vault vehicle (35486) the player boards
// with "Vault Cracking Toolset" (67476). The vault asks for one of five tools at
// a time (Blastcrackers, Kaja'mite Drill, Ear-O-Scope, Infinifold Lockpick,
// Amazing G-Ray - the vehicle bar spells 67508/67522/67524/67525/67526); the
// right tool is +10 on the 0-100 "Vault Breaking" bar, a wrong one -10, a missed
// 5 second window nothing. At 100 the player gets the credit and 67492 creates
// the Personal Riches. Ported from AshamaneCore (spell_great_bank_heist_vault_interact
// / npc_first_bank_of_kezan_vault). The "use X now" prompt is a raid-boss whisper
// to the rider (creature_text 35486 groups 5-9, BroadcastText 35576-35579/35700,
// "Use your <tool>!" + icon), followed by "Correct!"/"Incorrect!" (groups 10/11,
// BroadcastText 35702/35704); the retail "Bunny - ... Aura" hint spell 67495-67499
// stays on the vault as a server-side marker only (the client shows nothing for it,
// #443). The vault seat controls the vehicle (VehicleSeat 5326 flags 0x62000806,
// CAN_CONTROL), so the bar spells are cast BY the vault (SpellHandler: mover has the
// spell) and their TARGET_UNIT_VEHICLE (94) resolves to the vault's own vehicle base,
// i.e. nothing - SpellHit never fires. spell_great_bank_heist_vault_tool hands the
// cast to the vault AI instead (AfterCast).
// ---------------------------------------------------------------------------
enum GreatBankHeist
{
    QUEST_THE_GREAT_BANK_HEIST      = 14122,
    NPC_FBOK_VAULT                  = 35486,

    SPELL_VAULT_CONTROL_VEHICLE     = 67476,
    SPELL_VAULT_BLASTCRACKERS       = 67508,
    SPELL_VAULT_KAJAMITE_DRILL      = 67522,
    SPELL_VAULT_EAR_O_SCOPE         = 67524,
    SPELL_VAULT_INFINIFOLD_LOCKPICK = 67525,
    SPELL_VAULT_AMAZIN_G_RAY        = 67526,

    SPELL_HINT_DRILL                = 67495,
    SPELL_HINT_EXPLOSIVES           = 67496,
    SPELL_HINT_LISTEN               = 67497,
    SPELL_HINT_LOCKPICK             = 67498,
    SPELL_HINT_G_RAY                = 67499,

    SPELL_VAULT_TIMER               = 67502,
    SPELL_VAULT_SUCCESS_CREATE_ITEM = 67492,
    ITEM_PERSONAL_RICHES            = 46858,

    TALK_VAULT_BREAKING_IN          = 0,
    TALK_VAULT_USE_TOOLS            = 1,
    TALK_VAULT_PROGRESS_BAR         = 2,
    TALK_VAULT_GOOD_LUCK            = 3,
    TALK_VAULT_SUCCESS              = 4,
    TALK_VAULT_USE_BLASTCRACKERS    = 5,    // BroadcastText 35577
    TALK_VAULT_USE_KAJAMITE_DRILL   = 6,    // BroadcastText 35576
    TALK_VAULT_USE_EAR_O_SCOPE      = 7,    // BroadcastText 35578
    TALK_VAULT_USE_LOCKPICK         = 8,    // BroadcastText 35579
    TALK_VAULT_USE_G_RAY            = 9,    // BroadcastText 35700
    TALK_VAULT_CORRECT              = 10,   // BroadcastText 35702
    TALK_VAULT_INCORRECT            = 11,   // BroadcastText 35704

    DATA_VAULT_TOOL_USED            = 1,

    VAULT_STEP                      = 10,
    VAULT_MAX                       = 100,
    VAULT_INTRO_END                 = 23000,
    VAULT_PROMPT_DELAY              = 1000,
    VAULT_PROMPT_WINDOW             = 5000,
};

// 67555 "The Great Bank Heist: Vault Interact" (cast by the player through gameobject 195449)
class spell_great_bank_heist_vault_interact : public SpellScript
{
    PrepareSpellScript(spell_great_bank_heist_vault_interact);

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || player->IsOnVehicle())
            return;

        if (player->GetQuestStatus(QUEST_THE_GREAT_BANK_HEIST) != QUEST_STATUS_INCOMPLETE)
            return;

        // Private summon: only this player sees and uses his vault.
        if (Creature* vault = player->SummonCreature(NPC_FBOK_VAULT, player->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, Milliseconds(0), 0, player->GetGUID()))
            player->CastSpell(vault, SPELL_VAULT_CONTROL_VEHICLE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_great_bank_heist_vault_interact::HandleHit, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 35486 First Bank of Kezan Vault
struct npc_first_bank_of_kezan_vault : public ScriptedAI
{
    enum State
    {
        STATE_IDLE,
        STATE_INTRO,
        STATE_GAP,
        STATE_PROMPT,
        STATE_DONE,
    };

    npc_first_bank_of_kezan_vault(Creature* creature) : ScriptedAI(creature), _state(STATE_IDLE), _timer(0), _introStep(0), _wanted(0), _progress(0) { }

    void Reset() override
    {
        _state = STATE_IDLE;
        _timer = 0;
        _introStep = 0;
        _progress = 0;
    }

    void OnCharmed(bool /*apply*/) override { }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!apply)
        {
            // #938: the toolset spell also left a dummy aura on the rider; without this, leaving the vault early blocked the
            // vault spell (67555, TARGET_AURASTATE) until relog (and for good before 67476 became unsaveable).
            who->RemoveAurasDueToSpell(SPELL_VAULT_CONTROL_VEHICLE);

            if (who->GetGUID() == _riderGuid)
            {
                _state = STATE_DONE;
                me->DespawnOrUnsummon(2000);
            }
            return;
        }

        if (!who->IsPlayer() || _state != STATE_IDLE)
            return;

        _riderGuid = who->GetGUID();
        _progress = 0;
        me->SetMaxPower(POWER_MANA, VAULT_MAX);
        me->SetPower(POWER_MANA, 0);
        me->SetControlled(true, UNIT_STATE_ROOT);
        _timer = 0;
        _introStep = 0;
        _state = STATE_INTRO;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_state == STATE_IDLE || _state == STATE_DONE)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _riderGuid);
        if (!player)
        {
            _state = STATE_DONE;
            me->DespawnOrUnsummon(1000);
            return;
        }

        _timer += diff;
        switch (_state)
        {
            case STATE_INTRO:
            {
                static uint32 const introTime[4]  = { 1000, 6000, 13000, 20000 };
                static uint8 const introGroup[4]  = { TALK_VAULT_BREAKING_IN, TALK_VAULT_USE_TOOLS, TALK_VAULT_PROGRESS_BAR, TALK_VAULT_GOOD_LUCK };
                while (_introStep < 4 && _timer >= introTime[_introStep])
                {
                    Talk(introGroup[_introStep], player);
                    ++_introStep;
                }
                if (_timer >= VAULT_INTRO_END)
                {
                    _state = STATE_GAP;
                    _timer = 0;
                }
                break;
            }
            case STATE_GAP:
                if (_timer >= VAULT_PROMPT_DELAY)
                    Prompt(player);
                break;
            case STATE_PROMPT:
                if (_timer >= VAULT_PROMPT_WINDOW)
                {
                    // Too slow: nothing lost, ask again.
                    ClearPrompt(player);
                    _state = STATE_GAP;
                    _timer = 0;
                }
                break;
            default:
                break;
        }
    }

    // A tool spell that did reach the vault as its target (a passenger-cast variant).
    // The normal path is SetData from spell_great_bank_heist_vault_tool (vault-cast).
    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (caster != me)
            ToolUsed(spell->Id);
    }

    void SetData(uint32 type, uint32 value) override
    {
        if (type == DATA_VAULT_TOOL_USED)
            ToolUsed(value);
    }

    void ToolUsed(uint32 spellId)
    {
        if (_state != STATE_PROMPT)
            return;

        int8 used = -1;
        for (uint8 i = 0; i < 5; ++i)
            if (ToolSpells[i] == spellId)
                used = int8(i);

        if (used < 0)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _riderGuid);
        if (!player)
            return;

        ClearPrompt(player);

        if (used == _wanted)
        {
            _progress = std::min<int32>(_progress + VAULT_STEP, VAULT_MAX);
            if (_progress < VAULT_MAX)
                Talk(TALK_VAULT_CORRECT, player);
        }
        else
        {
            _progress = std::max<int32>(_progress - VAULT_STEP, 0);
            Talk(TALK_VAULT_INCORRECT, player);
        }

        me->SetPower(POWER_MANA, _progress);

        if (_progress >= VAULT_MAX)
        {
            _state = STATE_DONE;
            Talk(TALK_VAULT_SUCCESS, player);
            player->KilledMonsterCredit(me->GetEntry());
            me->CastSpell(player, SPELL_VAULT_SUCCESS_CREATE_ITEM, true);
            if (!player->HasItemCount(ITEM_PERSONAL_RICHES, 1))
                player->AddItem(ITEM_PERSONAL_RICHES, 1);
            player->ExitVehicle();
            me->DespawnOrUnsummon(2000);
            return;
        }

        _state = STATE_GAP;
        _timer = 0;
    }

private:
    static uint32 const ToolSpells[5];
    static uint32 const HintSpells[5];
    static uint8 const PromptTexts[5];

    void Prompt(Player* player)
    {
        _wanted = urand(0, 4);
        Talk(PromptTexts[_wanted], player);
        me->CastSpell(me, HintSpells[_wanted], true);
        me->CastSpell(player, SPELL_VAULT_TIMER, true);
        _state = STATE_PROMPT;
        _timer = 0;
    }

    void ClearPrompt(Player* player)
    {
        for (uint8 i = 0; i < 5; ++i)
            me->RemoveAurasDueToSpell(HintSpells[i]);
        player->RemoveAurasDueToSpell(SPELL_VAULT_TIMER);
    }

    ObjectGuid _riderGuid;
    uint8 _state;
    uint32 _timer;
    uint8 _introStep;
    uint8 _wanted;
    int32 _progress;
};

uint32 const npc_first_bank_of_kezan_vault::ToolSpells[5] =
{
    SPELL_VAULT_BLASTCRACKERS, SPELL_VAULT_KAJAMITE_DRILL, SPELL_VAULT_EAR_O_SCOPE, SPELL_VAULT_INFINIFOLD_LOCKPICK, SPELL_VAULT_AMAZIN_G_RAY
};

uint32 const npc_first_bank_of_kezan_vault::HintSpells[5] =
{
    SPELL_HINT_EXPLOSIVES, SPELL_HINT_DRILL, SPELL_HINT_LISTEN, SPELL_HINT_LOCKPICK, SPELL_HINT_G_RAY
};

uint8 const npc_first_bank_of_kezan_vault::PromptTexts[5] =
{
    TALK_VAULT_USE_BLASTCRACKERS, TALK_VAULT_USE_KAJAMITE_DRILL, TALK_VAULT_USE_EAR_O_SCOPE, TALK_VAULT_USE_LOCKPICK, TALK_VAULT_USE_G_RAY
};

// 67508 Blastcrackers, 67522 Kaja'mite Drill, 67524 Ear-O-Scope, 67525 Infinifold Lockpick,
// 67526 Amazing G-Ray (the vault's vehicle bar). The vault casts them itself (control seat) and
// TARGET_UNIT_VEHICLE finds no target, so tell the vault AI which tool was used. A rider-cast
// variant (caster = player) hits the vault and is handled by its SpellHit instead.
class spell_great_bank_heist_vault_tool : public SpellScript
{
    PrepareSpellScript(spell_great_bank_heist_vault_tool);

    void HandleAfterCast()
    {
        Creature* vault = GetCaster() ? GetCaster()->ToCreature() : nullptr;
        if (!vault || vault->GetEntry() != NPC_FBOK_VAULT || !vault->IsAIEnabled)
            return;

        vault->AI()->SetData(DATA_VAULT_TOOL_USED, GetSpellInfo()->Id);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_great_bank_heist_vault_tool::HandleAfterCast);
    }
};

// ---------------------------------------------------------------------------
// 14124 "Liberate the Kaja'mite": Kablooey Bombs (item 48768, spell 67682)
// blow a Kaja'mite Deposit (195488) into three Kaja'mite Chunks (195492).
// The deposit's own linked trap (195493 -> spell 67507) is not in the DBC, so
// the chunks are spawned here (AshamaneCore spell_kabummbomb).
// #716: effect 1 is a DUMMY on TARGET_UNIT_DEST_AREA_ENEMY (5 yd) - the slot
// where the engineering bombs (e.g. Rough Copper Bomb 4064) carry their 1 s
// MOD_STUN aura. The item text says the bombs "stun those unruly Rebellious
// Trolls" and Wowpedia (Liberate the Kaja'mite, Notes) says "if for a second
// or so". No public core handles it, so the dummy applies the 1 s generic Stun
// (23454, "Stuns target for $d") to each Rebellious Troll it hits. AddAura, not
// CastSpell: 23454 has a 5 yd combat range, the troll can be 10 yd away (5 yd
// toss + 5 yd radius).
// ---------------------------------------------------------------------------
enum LiberateKajamite
{
    QUEST_LIBERATE_THE_KAJAMITE     = 14124,
    GO_KAJAMITE_DEPOSIT             = 195488,
    GO_KAJAMITE_CHUNK               = 195492,
    KAJAMITE_CHUNK_LIFETIME         = 300,
    NPC_REBELLIOUS_TROLL            = 35294,
    SPELL_KABLOOEY_TROLL_STUN       = 23454,
};

// 67682
class spell_kabummbomb : public SpellScript
{
    PrepareSpellScript(spell_kabummbomb);

    void HandleActivateGameobject(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        GameObject* deposit = GetHitGObj();
        if (!player || !deposit || deposit->GetEntry() != GO_KAJAMITE_DEPOSIT)
            return;

        if (player->GetQuestStatus(QUEST_LIBERATE_THE_KAJAMITE) != QUEST_STATUS_INCOMPLETE)
            return;

        float x = deposit->GetPositionX();
        float y = deposit->GetPositionY();
        float z = deposit->GetPositionZ() + 1.0f;

        player->SummonGameObject(GO_KAJAMITE_CHUNK, x - 1.0f, y,        z, 0.0f, { }, KAJAMITE_CHUNK_LIFETIME);
        player->SummonGameObject(GO_KAJAMITE_CHUNK, x - 1.0f, y - 1.0f, z, 0.0f, { }, KAJAMITE_CHUNK_LIFETIME);
        player->SummonGameObject(GO_KAJAMITE_CHUNK, x - 1.0f, y + 1.0f, z, 0.0f, { }, KAJAMITE_CHUNK_LIFETIME);

        // Gone for now; it respawns on its normal timer for the next player.
        deposit->ForcedDespawn();
    }

    void HandleStunTroll(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Creature* troll = GetHitCreature();
        if (!caster || !troll || troll->GetEntry() != NPC_REBELLIOUS_TROLL || !troll->IsAlive())
            return;

        caster->AddAura(SPELL_KABLOOEY_TROLL_STUN, troll);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_kabummbomb::HandleActivateGameobject, EFFECT_0, SPELL_EFFECT_ACTIVATE_OBJECT);
        OnEffectHitTarget += SpellEffectFn(spell_kabummbomb::HandleStunTroll, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// ---------------------------------------------------------------------------
// 14121 "Robbing Hoods": the Hot Rod's knockback (66301, see
// HOT_ROD_KNOCKBACK_ENABLED above) runs over Hired Looters, who die and hand
// over Stolen Loot (47530); the citizens are shoved aside and walk home
// (AshamaneCore npc_citoyen_gob, bound to 35075 and 35234).
// ---------------------------------------------------------------------------
enum RobbingHoods
{
    QUEST_ROBBING_HOODS             = 14121,
    NPC_HIRED_LOOTER                = 35234,
    ITEM_STOLEN_LOOT                = 47530,
    STOLEN_LOOT_REQUIRED            = 12,
};

struct npc_citoyen_gob : public ScriptedAI
{
    npc_citoyen_gob(Creature* creature) : ScriptedAI(creature), _homeTimer(0) { }

    void Reset() override
    {
        _homeTimer = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_homeTimer)
        {
            if (_homeTimer <= diff)
            {
                _homeTimer = 0;
                if (me->IsAlive())
                    me->GetMotionMaster()->MoveTargetedHome();
            }
            else
                _homeTimer -= diff;
        }

        // The Hired Looters are hostile: keep the stock victim/melee handling.
        ScriptedAI::UpdateAI(diff);
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id != SPELL_HOT_ROD_KNOCKBACK)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !me->IsAlive())
            return;

        if (me->GetEntry() == NPC_HIRED_LOOTER)
        {
            if (player->GetQuestStatus(QUEST_ROBBING_HOODS) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(ITEM_STOLEN_LOOT, STOLEN_LOOT_REQUIRED))
                player->AddItem(ITEM_STOLEN_LOOT, 1);

            me->setDeathState(JUST_DIED);
            return;
        }

        _homeTimer = 3000;
    }

private:
    uint32 _homeTimer;
};

// ---------------------------------------------------------------------------
// 14125 "447": Gasbot Control Panel (gameobject 201736, type 22, spell 70253)
// summons the Gasbot (70252 -> 37598); it flies to KTC Headquarters, blows up
// and credits the player (AshamaneCore spell_summon_gasbot / npc_kezan_gasbot).
// The Defective Generator / Leaky Stove / Flammable Bed objectives are plain
// quest goobers (data1 = 14125) and need no script.
// ---------------------------------------------------------------------------
enum Quest447
{
    QUEST_447                       = 14125,
    NPC_GASBOT                      = 37598,

    SPELL_GASBOT_SUMMON             = 70252,
    SPELL_GASBOT_SUMMON_VISUAL      = 70254,
    SPELL_GASBOT_GAS_STREAM         = 70256,
    SPELL_GASBOT_BIG_FIRE           = 70259,
    SPELL_GASBOT_EXPLOSION          = 70262,

    POINT_GASBOT_HEADQUARTERS       = 1,
};

// 70253
class spell_summon_gasbot : public SpellScript
{
    PrepareSpellScript(spell_summon_gasbot);

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_447) != QUEST_STATUS_INCOMPLETE)
            return;

        player->CastSpell(player, SPELL_GASBOT_SUMMON, true);
        player->CastSpell(player, SPELL_GASBOT_SUMMON_VISUAL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_summon_gasbot::HandleHit, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 37598 Gasbot (only the summoned copy moves; the parked spawn stays decoration)
struct npc_kezan_gasbot : public ScriptedAI
{
    npc_kezan_gasbot(Creature* creature) : ScriptedAI(creature), _startTimer(0), _despawnTimer(0) { }

    void Reset() override
    {
        _startTimer = 0;
        _despawnTimer = 0;
    }

    void OnCharmed(bool /*apply*/) override { }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGuid = summoner->GetGUID();
        me->CastSpell(me, SPELL_GASBOT_GAS_STREAM, true);
        _startTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_startTimer)
        {
            if (_startTimer <= diff)
            {
                _startTimer = 0;
                me->GetMotionMaster()->MovePoint(POINT_GASBOT_HEADQUARTERS, -8423.85f, 1365.88f, 104.7f);
            }
            else
                _startTimer -= diff;
        }

        if (_despawnTimer)
        {
            if (_despawnTimer <= diff)
            {
                _despawnTimer = 0;
                me->DespawnOrUnsummon();
            }
            else
                _despawnTimer -= diff;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != POINT_GASBOT_HEADQUARTERS)
            return;

        me->CastSpell(me, SPELL_GASBOT_BIG_FIRE, true);
        me->CastSpell(me, SPELL_GASBOT_EXPLOSION, true);
        _despawnTimer = 15000;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGuid))
            player->KilledMonsterCredit(me->GetEntry());
    }

private:
    ObjectGuid _summonerGuid;
    uint32 _startTimer;
    uint32 _despawnTimer;
};

// ---------------------------------------------------------------------------
// 66912 "Fireworks" (party bar, "Life of the Party"): effect 1 launches one of
// the three cosmetic firework spells on the caster (AshamaneCore
// spell_kezan_fireworks). The partygoers' needs are plain SmartAI.
// ---------------------------------------------------------------------------
class spell_kezan_fireworks : public SpellScript
{
    PrepareSpellScript(spell_kezan_fireworks);

    void HandleFireworks(SpellEffIndex /*effIndex*/)
    {
        static uint32 const fireworks[3] = { 66917, 66918, 66919 }; // blue, green, red
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, fireworks[urand(0, 2)], true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_kezan_fireworks::HandleFireworks, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// ---------------------------------------------------------------------------
// 14126 "Life Savings": the reward spell 91847 plays movie 22 (core effect 45)
// and puts the screen effect on; when the client reports the movie finished
// (Player::GetMovie() is reset by CMSG_COMPLETE_MOVIE) the player is bound and
// teleported to the Lost Isles with 74100 (AshamaneCore spell_life_saving_complete,
// which used a movie-complete hook this core does not have, so the movie id is
// polled). Fallback after 4 minutes in case the client never answers.
// ---------------------------------------------------------------------------
enum LifeSavings
{
    SPELL_LIFE_SAVINGS_COMPLETE         = 91847,
    SPELL_LIFE_SAVINGS_TELEPORT         = 74100,
    LIFE_SAVINGS_POLL_MS                = 500,
    LIFE_SAVINGS_TIMEOUT_MS             = 240000,
};

static void FinishLifeSavings(Player* player, uint32 elapsed)
{
    if (elapsed < LIFE_SAVINGS_TIMEOUT_MS && player->GetMovie())
    {
        player->m_Events.Schedule(LIFE_SAVINGS_POLL_MS, [player, elapsed]()
        {
            FinishLifeSavings(player, elapsed + LIFE_SAVINGS_POLL_MS);
        });
        return;
    }

    player->RemoveAurasDueToSpell(SPELL_LIFE_SAVINGS_COMPLETE);
    player->CastSpell(player, SPELL_LIFE_SAVINGS_TELEPORT, true);
}

// 91847
class spell_life_saving_complete : public SpellScript
{
    PrepareSpellScript(spell_life_saving_complete);

    void HandleAfterHit()
    {
        Unit* target = GetHitUnit();
        Player* player = target ? target->ToPlayer() : nullptr;
        if (!player)
            return;

        // Give the movie packet a second to start before polling for its end.
        player->m_Events.Schedule(1000, [player]()
        {
            FinishLifeSavings(player, 1000);
        });
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_life_saving_complete::HandleAfterHit);
    }
};
} // namespace KezanGoblin

// #438: the Lost Isles half of the Goblin start (same map 648) lives in its own
// files but registers through AddSC_kezan, so ScriptLoader.cpp needs no edit.
void AddSC_lost_isles();          // zone_lost_isles.cpp (crash landing -> Town-In-A-Box defence)
void AddSC_lost_isles_volcano();  // zone_lost_isles_volcano.cpp (Oomlot -> Gallywix -> Orgrimmar)

void AddSC_kezan()
{
    AddSC_lost_isles();
    AddSC_lost_isles_volcano();

    new creature_script<npc_sister_goldskimmer>("npc_sister_goldskimmer");
    new quest_rolling_with_my_homies();
    new creature_script<npc_hot_rod_rolling_my_homies>("npc_hot_rod_rolling_my_homies");
    new spell_script<spell_radio>("spell_radio");
    new creature_script<npc_homies>("npc_homies");
    new creature_script<npc_homies_summon>("npc_homies_summon");

    // #438 dev A: Kezan level 1-5 quests (see namespace KezanGoblin above).
    new KezanGoblin::npc_gw();
    new KezanGoblin::quest_fourth_and_goal();
    new creature_script<KezanGoblin::npc_boucanier_gob>("npc_boucanier_gob");
    new creature_script<KezanGoblin::npc_shark_gob>("npc_shark_gob");
    new creature_script<KezanGoblin::npc_fourth_and_goal_target>("npc_fourth_and_goal_target");
    new spell_script<KezanGoblin::spell_fourth_and_goal_kick_footbomb>("spell_fourth_and_goal_kick_footbomb");
    new spell_script<KezanGoblin::spell_great_bank_heist_vault_interact>("spell_great_bank_heist_vault_interact");
    new creature_script<KezanGoblin::npc_first_bank_of_kezan_vault>("npc_first_bank_of_kezan_vault");
    new spell_script<KezanGoblin::spell_great_bank_heist_vault_tool>("spell_great_bank_heist_vault_tool");
    new spell_script<KezanGoblin::spell_kabummbomb>("spell_kabummbomb");
    new creature_script<KezanGoblin::npc_citoyen_gob>("npc_citoyen_gob");
    new spell_script<KezanGoblin::spell_summon_gasbot>("spell_summon_gasbot");
    new creature_script<KezanGoblin::npc_kezan_gasbot>("npc_kezan_gasbot");
    new spell_script<KezanGoblin::spell_kezan_fireworks>("spell_kezan_fireworks");
    new spell_script<KezanGoblin::spell_life_saving_complete>("spell_life_saving_complete");
}
