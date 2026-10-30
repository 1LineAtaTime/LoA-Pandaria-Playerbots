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
#include "Log.h"
#include "ScriptedEscortAI.h"
#include "SmartAI.h"
#include "ObjectAccessor.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <list>

// Lost Isles (map 648), second half: Oomlot -> volcano -> Gallywix (#438, dev B2).
//
// Ported from the MoPCore script lineage (mopcore lost_isles.cpp:1495-2420) and its cleaned-up
// AshamaneCore copy (lost_isles.cpp:1409-2358), adapted to this core:
//   * ScriptNames bound by the matching SQL (2026_10_04_16_world_goblin_lost_isles_volcano.sql)
//     are the MoP ones: npc_canon_gob, npc_oom_isle_gob, spell_boot_damage,
//     npc_izzy_airplane, npc_avion_gob, npc_killag_2, npc_Chariot, npc_Chariot2, npc_Prince.
//     npc_ceint is a MoP-only class of the same lineage (bound by the SQL to Coach Crosscheck).
//   * Every vehicle AI overrides OnCharmed with an empty body (CreatureAI::OnCharmed disables the AI
//     when a driver boards, #207).
//   * Seat numbers are 0-based seat indexes, as Unit::EnterVehicle(base, seatId) takes them (the
//     old CastCustomSpell(46598, BASE_POINT0, n) used 1-based n).
//   * Quests that ride or fly with a spell/SAI only (Pride of Kezan 25066, Final Confrontation
//     25251, Rocket Boot Boost 24952, Volcanoth 24958, Big Red Button 25207) need no C++ here.
//
// Everything file-local lives in an anonymous namespace, struct names are prefixed "lovolc_".

namespace
{
    enum LovolcQuests
    {
        QUEST_OLD_FRIENDS                   = 25023,
        QUEST_ZOMBIES_VS_ROCKET_BOOTS       = 24942,
        QUEST_LETS_RIDE                     = 25100,
        QUEST_WILD_MINE_CART_RIDE           = 25184
    };

    enum LovolcCreatures
    {
        NPC_OOMLOT_WARRIOR                  = 38531,
        NPC_GOBLIN_ZOMBIE                   = 38753,
        NPC_GOBLIN_ZOMBIE_2                 = 38816,    // second "Goblin Zombie" entry of the same field; also credits 38807 (SAI id0)
        NPC_SUPER_BOOSTER_ROCKET_BOOTS      = 38802,
        NPC_FLYING_BOMBER                   = 38929,
        NPC_SASSY_BOMBER_PASSENGER          = 38928,    // Sassy Hardwrench, seat 0 of the Flying Bomber (#457)
        NPC_BASTIA                          = 39152,
        NPC_MINE_CART_VEHICLE               = 39329,
        NPC_CAULDRON_TRIGGER                = 41505,
        NPC_THRALL_GALLYWIX_DOCKS           = 39594,
        NPC_ULTIMATE_FOOTBOMB_UNIFORM_VEH   = 39598,

        // Ace/Izzy/Gobber follower copies that ride along in the Mine Cart (MoPCore npc_Chariot)
        NPC_HOMIE_ACE_A                     = 34957,
        NPC_HOMIE_IZZY_A                    = 34959,
        NPC_HOMIE_ACE_B                     = 39199,
        NPC_HOMIE_IZZY_B                    = 39205
    };

    enum LovolcSpells
    {
        SPELL_CANNON_BLAST                  = 72207,    // B.C. Eliminator "Fire Cannon" (72206) trigger
        SPELL_BOOTS_DAMAGE_TRIGGER          = 72885,
        SPELL_BOOTS_VEHICLE_AURA            = 72887,
        SPELL_BOOTS_QUEST_AURA              = 72889,
        SPELL_OLD_FRIENDS_BIND              = 73135,    // "Quest Accept & Bind" (spell_target_position row exists)
        SPELL_CAULDRON_EFFECT_1             = 93569,
        SPELL_CAULDRON_EFFECT_2             = 71094,

        // Trade Prince Gallywix abilities (creature_template spell1..4 = 81000,74003,74005,74000; 74004
        // is the fifth one MoPCore/Ashamane cast)
        SPELL_PRINCE_CALL_A_MEETING         = 81000,
        SPELL_PRINCE_DOWNSIZING             = 74003,
        SPELL_PRINCE_YOURE_FIRED            = 74004,
        SPELL_PRINCE_REVENUE_STREAM         = 74005,
        SPELL_PRINCE_UNLOAD_TOXIC_ASSETS    = 74000
    };

    enum LovolcCredits
    {
        CREDIT_MINE_CART_RIDDEN             = 39335,
        CREDIT_TRADE_PRINCE_DEALT_WITH      = 39582
    };

    // Sassy Hardwrench 38928 (the passenger in seat 0 of the Flying Bomber) - creature_text groups
    // added by the SQL file, wording from Wowpedia "Old Friends" (25023).
    enum LovolcSassyTexts
    {
        SASSY_TEXT_HANG_ON                  = 1,
        SASSY_TEXT_BEHIND_YOU               = 2,
        SASSY_TEXT_ON_FIRE                  = 3,
        SASSY_TEXT_YEE_HAW                  = 4,
        SASSY_TEXT_TOWN_IN_A_BOX            = 5,
        SASSY_TEXT_BACK_SEAT                = 6,
        SASSY_TEXT_TOAST                    = 7
    };

    // Trade Prince Gallywix 39582 / Thrall 39594 creature_text groups (already in the DB)
    enum LovolcPrinceTexts
    {
        PRINCE_TEXT_ARRIVAL                 = 3,    // "I SEE THE TRAITOR IS HERE TO RESCUE YOU, WARCHIEF..."
        PRINCE_TEXT_MOOLAH_BACK             = 2,
        PRINCE_TEXT_TRIM_FAT                = 5,
        PRINCE_TEXT_TOXIC_SUCKER            = 4,
        PRINCE_TEXT_TOXIC_UNLOAD            = 6,
        PRINCE_TEXT_DOWNSIZING_CARTEL       = 7,
        PRINCE_TEXT_UNCLE                   = 8,
        PRINCE_TEXT_BEATEN                  = 9,
        PRINCE_TEXT_IM_YOUR_GOBLIN          = 10,
        PRINCE_TEXT_LONG_LIVE_CARTEL        = 11,

        THRALL_TEXT_THINKS                  = 0,
        THRALL_TEXT_REMAIN_TRADE_PRINCE     = 1,
        THRALL_TEXT_SEND_REPRESENTATIVE     = 2,
        THRALL_TEXT_NEW_HOME                = 3
    };

    // B.C. Eliminator 38526 (quest 24901). The cannon is only a seat with a spell bar; the kill credit
    // comes from the Oomlot Warrior (KillCredit1 38536 set by the SQL file).
    struct lovolc_npc_canon_gob : public ScriptedAI
    {
        lovolc_npc_canon_gob(Creature* creature) : ScriptedAI(creature) { }

        void OnCharmed(bool /*apply*/) override { }
        void AttackStart(Unit* /*who*/) override { }
        void EnterCombat(Unit* /*who*/) override { }
        void EnterEvadeMode(EvadeReason /*why*/) override { }
        void DamageTaken(Unit* /*attacker*/, uint32& damage) override { damage = 0; }
        void UpdateAI(uint32 /*diff*/) override { }
    };

    // Oomlot Warrior 38531: the cannon's "Cannon Blast" (72207) kills it outright, the quest credit
    // (38536) is the creature_template KillCredit1, which Unit::Kill gives to the vehicle's driver.
    struct lovolc_npc_oom_isle_gob : public ScriptedAI
    {
        lovolc_npc_oom_isle_gob(Creature* creature) : ScriptedAI(creature) { }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_CANNON_BLAST && caster && me->IsAlive())
                caster->Kill(me);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    // Coach Crosscheck 38738: turning in "Zombies vs. Super Booster Rocket Boots" takes the boots away.
    // No AI is supplied (GetAI is not overridden), so his SmartAI/gossip keep working.
    class lovolc_npc_ceint : public CreatureScript
    {
    public:
        lovolc_npc_ceint() : CreatureScript("npc_ceint") { }

        // 72889 "Summon Spell" is a vehicle-category summon of 38802 whose effect value is the ride aura
        // 72887, so one cast spawns the boots and boards the player (which also starts the damage ticks
        // 72885 through spell_linked_spell). The spell is restricted by its area group to the Oostan
        // areas, where Crosscheck stands for this quest; nothing in the quest data cast it before.
        bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_ZOMBIES_VS_ROCKET_BOOTS && !player->HasAura(SPELL_BOOTS_VEHICLE_AURA))
                player->CastSpell(player, SPELL_BOOTS_QUEST_AURA, true);

            return false;
        }

        bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/) override
        {
            if (quest->GetQuestId() == QUEST_ZOMBIES_VS_ROCKET_BOOTS)
            {
                player->RemoveAurasDueToSpell(SPELL_BOOTS_VEHICLE_AURA);
                player->RemoveAurasDueToSpell(SPELL_BOOTS_DAMAGE_TRIGGER);
                if (Creature* boots = player->FindNearestCreature(NPC_SUPER_BOOSTER_ROCKET_BOOTS, 10.0f))
                    boots->DespawnOrUnsummon();
            }

            return false;
        }
    };

    // 72886 "Zombies vs. Super Booster Rocket Boots: Damage - Effect": the boots only hurt the
    // plain Goblin Zombies 38753 and 38816 (not the headhunters or the named pygmies, Wowpedia 24942;
    // #461 lane Y2: both entries share the zombie field and both credit 38807 on death in the
    // world data - t461 B01 build 87: 25 boot hits, all on 38753, 77 of the 162 zombies there are 38816) and stop
    // ticking once the caster no longer carries the boots' vehicle aura.
    class lovolc_spell_boot_damage : public SpellScript
    {
        PrepareSpellScript(lovolc_spell_boot_damage);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* object)
            {
                Creature* creature = object ? object->ToCreature() : nullptr;
                return !creature || !creature->IsAlive() ||
                    (creature->GetEntry() != NPC_GOBLIN_ZOMBIE && creature->GetEntry() != NPC_GOBLIN_ZOMBIE_2);
            });

            if (Unit* caster = GetCaster())
            {
                if (!caster->HasAura(SPELL_BOOTS_VEHICLE_AURA))
                {
                    caster->RemoveAurasDueToSpell(SPELL_BOOTS_DAMAGE_TRIGGER);
                    targets.clear();
                }
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(lovolc_spell_boot_damage::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    // Flying Bomber 38929 (quest 25023 "Old Friends"). VehicleId 657: seat 0 Sassy (accessory 38928),
    // seat 1 the player, seat 2 the rear seat the player is moved to after the first pass.
    // Waypoints: script_waypoint 38929 (26 rows).
    struct lovolc_npc_avion_gob : public npc_escortAI
    {
        lovolc_npc_avion_gob(Creature* creature) : npc_escortAI(creature)
        {
            SetDespawnAtFar(false);
        }

        void OnCharmed(bool /*apply*/) override { }
        void AttackStart(Unit* /*who*/) override { }
        void EnterCombat(Unit* /*who*/) override { }
        void EnterEvadeMode(EvadeReason /*why*/) override { }

        void Reset() override
        {
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 6.0f);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            Player* player = who->ToPlayer();
            if (!apply || !player || HasEscortState(STATE_ESCORT_ESCORTING))
                return;

            // The static bomber can be clicked by anyone; only fly players that took the quest.
            if (player->GetQuestStatus(QUEST_OLD_FRIENDS) != QUEST_STATUS_INCOMPLETE)
                return;

            // #438 lead: do not Start() here. The player boards synchronously inside the summon, before
            // the bomber's first Creature::Update, and that update runs npc_escortAI::JustAppeared()
            // (ScriptedEscortAI.cpp:159-173), which resets the escort state. Start from
            // UpdateEscortAI instead, which always runs after JustAppeared.
            _pendingStart = player->GetGUID();
        }

        void UpdateEscortAI(uint32 /*diff*/) override
        {
            if (!_pendingStart.IsEmpty() && !HasEscortState(STATE_ESCORT_ESCORTING))
            {
                ObjectGuid const playerGuid = _pendingStart;
                _pendingStart.Clear();
                Start(false, true, playerGuid);
            }
        }

        void WaypointReached(uint32 pointId) override
        {
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 6.0f);

            switch (pointId)
            {
                case 1:
                    EnsureSassy();
                    SassyTalk(SASSY_TEXT_HANG_ON);
                    break;
                case 4:
                    SassyTalk(SASSY_TEXT_BEHIND_YOU);
                    break;
                case 8:
                    SassyTalk(SASSY_TEXT_ON_FIRE);
                    break;
                case 11:
                    SassyTalk(SASSY_TEXT_YEE_HAW);
                    break;
                case 15:
                    SassyTalk(SASSY_TEXT_TOWN_IN_A_BOX);
                    break;
                case 19:
                {
                    SassyTalk(SASSY_TEXT_BACK_SEAT);
                    if (Vehicle* kit = me->GetVehicleKit())
                        if (Unit* passenger = kit->GetPassenger(1))
                            if (passenger->GetTypeId() == TYPEID_PLAYER)
                                passenger->ChangeSeat(2, false);
                    me->RemoveAurasDueToSpell(73149);
                    break;
                }
                case 23:
                    SassyTalk(SASSY_TEXT_TOAST);
                    break;
                case 25:
                    // End of the path: drop the passengers at the Warchief's Lookout and vanish.
                    if (Vehicle* kit = me->GetVehicleKit())
                        kit->RemoveAllPassengers();
                    if (Creature* sassy = ObjectAccessor::GetCreature(*me, _sassyGuid))
                        if (sassy->IsSummon())
                            sassy->DespawnOrUnsummon(1000);
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }

    private:
        ObjectGuid _pendingStart;
        ObjectGuid _sassyGuid;

        // #457: no Sassy line ever reached the client on the L17 flights (w16b/w17: the seat change at waypoint 19 ran, so
        // the waypoint handler fires, but there was no MONSTER_YELL from her), i.e. seat 0 held no Sassy for SassyTalk.
        // The 38928 seat-0 accessory (vehicle_template_accessory) is installed while this personal bomber is still being
        // added to the map by the player's SummonCreature; the port source does not rely on it for this flight: MoPCore
        // npc_izzy_airplane summons its own Sassy (38869) and seats her (mopcore lost_isles.cpp:1687-1692; AshamaneCore
        // keeps the same code commented out, ashamane lost_isles.cpp:1611-1617). Same here, once, at the first waypoint (every join has run by then): keep
        // whatever sits in seat 0, otherwise seat a fresh 38928 (her creature_text groups 1-7). The log line records
        // which case it was, so the next L17 run shows whether the accessory ever arrived (ERROR level: the test server
        // logs Logger.root at 5 unless verbose-loggers is on; one line per flight).
        void EnsureSassy()
        {
            if (!_sassyGuid.IsEmpty())
                return;

            Vehicle* kit = me->GetVehicleKit();
            if (!kit)
                return;

            Unit* seat0 = kit->GetPassenger(0);
            TC_LOG_ERROR("scripts", "lost_isles #457: Flying Bomber %u seat0=%u at waypoint 1%s", me->GetGUID().GetCounter(),
                seat0 ? seat0->GetEntry() : 0, seat0 ? "" : " - seating a summoned Sassy 38928");
            if (seat0)
            {
                _sassyGuid = seat0->GetGUID();
                return;
            }

            if (TempSummon* sassy = me->SummonCreature(NPC_SASSY_BOMBER_PASSENGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(300000)))
            {
                _sassyGuid = sassy->GetGUID();
                sassy->EnterVehicle(me, 0);
            }
        }

        void SassyTalk(uint8 group)
        {
            Player* player = GetPlayerForEscort();
            Creature* sassy = ObjectAccessor::GetCreature(*me, _sassyGuid);
            if (!sassy)
                if (Vehicle* kit = me->GetVehicleKit())
                    if (Unit* seat0 = kit->GetPassenger(0))
                        sassy = seat0->ToCreature();

            if (sassy && sassy->AI())
                sassy->AI()->Talk(group, player);
        }
    };

    // Sassy Hardwrench 38928 (quest 25023 "Old Friends" giver, also the bomber's seat-0 passenger).
    // On accept the player gets the hearthstone bind (73135) and a personal Flying Bomber.
    class lovolc_npc_izzy_airplane : public CreatureScript
    {
    public:
        lovolc_npc_izzy_airplane() : CreatureScript("npc_izzy_airplane") { }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new ScriptedAI(creature);
        }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() != QUEST_OLD_FRIENDS)
                return false;

            player->CastSpell(player, SPELL_OLD_FRIENDS_BIND, true);

            // Summoned by the player so that it shares the player's phase; the 38928 accessory
            // (vehicle_template_accessory) joins seat 0 by itself.
            if (TempSummon* bomber = player->SummonCreature(NPC_FLYING_BOMBER, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                player->EnterVehicle(bomber, 1);

            return false;
        }
    };

    // Kilag Gorefang 39066 (quest 25100 "Let's Ride"): the player rides Bastia 39152; Bastia's SmartAI
    // (passenger boarded -> waypoint path 39152, 37 points, despawn at the end) runs the trip.
    class lovolc_npc_killag_2 : public CreatureScript
    {
    public:
        lovolc_npc_killag_2() : CreatureScript("npc_killag_2") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() != QUEST_LETS_RIDE)
                return false;

            if (TempSummon* bastia = player->SummonCreature(NPC_BASTIA, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                player->EnterVehicle(bastia, 0);
                if (creature->AI())
                    creature->AI()->Talk(0, player);

                // #438 run 1 (build 4, leg L19): boarding inside the summon call fires Bastia's SmartAI
                // "passenger boarded -> start path 39152" BEFORE her first Creature::Update, and that update runs
                // SmartAI::JustAppeared() (SmartAI.cpp:589-603), which resets mEscortState - she stopped at
                // waypoint 1 with the player on her back. Re-start the path once that first update has happened.
                EnsureBastiaPath(player, bastia->GetGUID(), 0);
            }

            return false;
        }

    private:
        // Polls every 500 ms for up to 6 s: once Bastia carries the player but is no longer escorting, the
        // JustAppeared reset has happened - start the path again (same path/run flag as her SAI row 0).
        static void EnsureBastiaPath(Player* player, ObjectGuid bastiaGuid, uint32 tries)
        {
            player->m_Events.Schedule(500, [player, bastiaGuid, tries]()
            {
                Creature* bastia = ObjectAccessor::GetCreature(*player, bastiaGuid);
                if (!bastia || player->GetVehicleBase() != bastia)
                    return;

                if (SmartAI* ai = dynamic_cast<SmartAI*>(bastia->AI()))
                {
                    if (!ai->HasEscortState(SMART_ESCORT_ESCORTING))
                    {
                        ai->StartPath(true, NPC_BASTIA, false, player);
                        return;
                    }
                }

                if (tries < 12)
                    EnsureBastiaPath(player, bastiaGuid, tries + 1);
            });
        }
    };

    // Mine Cart 39341 (quest 25184 "Wild Mine Cart Ride" giver): the player (and the follower copies
    // of Ace/Izzy/Gobber, if any) ride the Mine Cart vehicle 39329 down its 13-point path.
    class lovolc_npc_chariot : public CreatureScript
    {
    public:
        lovolc_npc_chariot() : CreatureScript("npc_Chariot") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() != QUEST_WILD_MINE_CART_RIDE)
                return false;

            TempSummon* cart = player->SummonCreature(NPC_MINE_CART_VEHICLE, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
            if (!cart)
                return false;

            cart->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            player->EnterVehicle(cart, 0);

            int8 seat = 1;
            std::list<Unit*> followers;
            for (Unit* controlled : player->m_Controlled)
                if (controlled->GetTypeId() == TYPEID_UNIT)
                    switch (controlled->GetEntry())
                    {
                        case NPC_HOMIE_ACE_A:
                        case NPC_HOMIE_ACE_B:
                        case NPC_HOMIE_IZZY_A:
                        case NPC_HOMIE_IZZY_B:
                            followers.push_back(controlled);
                            break;
                        default:
                            break;
                    }

            for (Unit* follower : followers)
                follower->EnterVehicle(cart, seat++);

            // #438 lead: queue the escort start; a Start() here is wiped by the cart's first-update
            // npc_escortAI::JustAppeared() (ScriptedEscortAI.cpp:159-173). See lovolc_npc_chariot2::SetGUID.
            if (cart->AI())
                cart->AI()->SetGUID(player->GetGUID(), 1);

            return false;
        }
    };

    // Mine Cart vehicle 39329, script_waypoint 39329 (13 rows).
    struct lovolc_npc_chariot2 : public npc_escortAI
    {
        lovolc_npc_chariot2(Creature* creature) : npc_escortAI(creature)
        {
            SetDespawnAtFar(false);
        }

        void OnCharmed(bool /*apply*/) override { }
        void AttackStart(Unit* /*who*/) override { }
        void EnterCombat(Unit* /*who*/) override { }
        void EnterEvadeMode(EvadeReason /*why*/) override { }

        // id 1: start the ride for this player (queued by lovolc_npc_chariot::OnQuestAccept).
        void SetGUID(ObjectGuid guid, int32 id) override
        {
            if (id == 1)
                _pendingStart = guid;
        }

        void UpdateEscortAI(uint32 /*diff*/) override
        {
            if (!_pendingStart.IsEmpty() && !HasEscortState(STATE_ESCORT_ESCORTING))
            {
                ObjectGuid const playerGuid = _pendingStart;
                _pendingStart.Clear();
                Start(false, true, playerGuid);
            }
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            // Follower copies leaving the cart are not needed any more.
            if (!apply && who->GetTypeId() == TYPEID_UNIT && who->ToCreature()->IsSummon())
                who->ToCreature()->DespawnOrUnsummon();
        }

        void WaypointReached(uint32 pointId) override
        {
            if (pointId != 12)
                return;

            Player* player = nullptr;
            if (TempSummon* summon = me->ToTempSummon())
                if (Unit* summoner = summon->GetSummoner())
                    player = summoner->ToPlayer();

            if (Vehicle* kit = me->GetVehicleKit())
                kit->RemoveAllPassengers();

            if (Creature* fx = me->SummonCreature(NPC_CAULDRON_TRIGGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, Milliseconds(10000)))
            {
                fx->CastSpell(fx, SPELL_CAULDRON_EFFECT_1, true);
                fx->CastSpell(fx, SPELL_CAULDRON_EFFECT_2, true);
            }

            if (player)
            {
                player->GetMotionMaster()->MoveJump(2354.36f, 1943.21f, 24.0f, 20.0f, 20.0f);
                player->KilledMonsterCredit(CREDIT_MINE_CART_RIDDEN);
            }

            me->DespawnOrUnsummon();
        }

    private:
        ObjectGuid _pendingStart;
    };

    // Trade Prince Gallywix 39582 (quest 25251 "Final Confrontation"). The fight starts when a player
    // in the Ultimate Footbomb Uniform (vehicle 39598) comes close; a lethal blow does not kill him,
    // it ends the fight, gives the credit and plays the surrender dialogue with Thrall.
    struct lovolc_npc_prince : public ScriptedAI
    {
        lovolc_npc_prince(Creature* creature) : ScriptedAI(creature), _started(false), _ended(false), _spellTimer(5000) { }

        void Reset() override
        {
            scheduler.CancelAll();
            _started = false;
            _ended = false;
            _spellTimer = 5000;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (_started || _ended || !who || !me->IsWithinDistInMap(who, 40.0f))
                return;

            Player* player = who->GetCharmerOrOwnerPlayerOrPlayerItself();
            if (!player)
                return;

            Unit* base = who->GetTypeId() == TYPEID_PLAYER ? who->GetVehicleBase() : who;
            if (!base || base->GetEntry() != NPC_ULTIMATE_FOOTBOMB_UNIFORM_VEH)
                return;

            _started = true;
            Talk(PRINCE_TEXT_ARRIVAL, player);
            AttackStart(player);
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            if (_ended)
            {
                damage = 0;
                return;
            }

            if (damage < me->GetHealth())
                return;

            damage = 0;
            _ended = true;
            me->SetFaction(35);
            me->CombatStop(true);
            me->GetThreatManager().ClearAllThreat();

            if (attacker)
                if (Player* killer = attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
                    killer->KilledMonsterCredit(CREDIT_TRADE_PRINCE_DEALT_WITH);

            // Anyone else still in a uniform close by gets the credit too.
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                if (Player* player = itr->GetSource())
                    if (player->IsAlive() && me->IsWithinDistInMap(player, 80.0f))
                        if (Unit* base = player->GetVehicleBase())
                            if (base->GetEntry() == NPC_ULTIMATE_FOOTBOMB_UNIFORM_VEH)
                                player->KilledMonsterCredit(CREDIT_TRADE_PRINCE_DEALT_WITH);

            ScheduleSurrender();
        }

        void UpdateAI(uint32 diff) override
        {
            scheduler.Update(diff);

            if (!_started || _ended)
                return;

            if (!UpdateVictim())
                return;

            if (_spellTimer <= diff)
            {
                switch (urand(0, 4))
                {
                    case 0:
                        me->CastSpell(me->GetVictim(), SPELL_PRINCE_REVENUE_STREAM, true);
                        Talk(PRINCE_TEXT_TOXIC_SUCKER);
                        break;
                    case 1:
                        me->CastSpell(me->GetVictim(), SPELL_PRINCE_UNLOAD_TOXIC_ASSETS, true);
                        Talk(PRINCE_TEXT_TOXIC_UNLOAD);
                        break;
                    case 2:
                        me->CastSpell(me->GetVictim(), SPELL_PRINCE_YOURE_FIRED, true);
                        Talk(PRINCE_TEXT_TRIM_FAT);
                        break;
                    case 3:
                        me->CastSpell(me->GetVictim(), SPELL_PRINCE_CALL_A_MEETING, true);
                        Talk(PRINCE_TEXT_MOOLAH_BACK);
                        break;
                    default:
                        me->CastSpell(me->GetVictim(), SPELL_PRINCE_DOWNSIZING, true);
                        Talk(PRINCE_TEXT_DOWNSIZING_CARTEL);
                        break;
                }
                _spellTimer = 5000;
            }
            else
                _spellTimer -= diff;

            DoMeleeAttackIfReady();
        }

    private:
        void ScheduleSurrender()
        {
            scheduler.Schedule(Milliseconds(2000), [this](TaskContext /*context*/) { Talk(PRINCE_TEXT_UNCLE); });
            scheduler.Schedule(Milliseconds(7000), [this](TaskContext /*context*/) { Talk(PRINCE_TEXT_BEATEN); });
            scheduler.Schedule(Milliseconds(12000), [this](TaskContext /*context*/) { Talk(PRINCE_TEXT_IM_YOUR_GOBLIN); });
            scheduler.Schedule(Milliseconds(15000), [this](TaskContext /*context*/) { ThrallTalk(THRALL_TEXT_THINKS); });
            scheduler.Schedule(Milliseconds(19000), [this](TaskContext /*context*/) { ThrallTalk(THRALL_TEXT_REMAIN_TRADE_PRINCE); });
            scheduler.Schedule(Milliseconds(26000), [this](TaskContext /*context*/) { ThrallTalk(THRALL_TEXT_SEND_REPRESENTATIVE); });
            scheduler.Schedule(Milliseconds(33000), [this](TaskContext /*context*/) { ThrallTalk(THRALL_TEXT_NEW_HOME); });
            scheduler.Schedule(Milliseconds(40000), [this](TaskContext /*context*/) { Talk(PRINCE_TEXT_LONG_LIVE_CARTEL); });
            scheduler.Schedule(Milliseconds(46000), [this](TaskContext /*context*/)
            {
                // The Prince is a static spawn: bring him back soon so the next player can do the quest.
                me->SetRespawnDelay(30);
                me->DespawnOrUnsummon();
            });
        }

        void ThrallTalk(uint8 group)
        {
            if (Creature* thrall = me->FindNearestCreature(NPC_THRALL_GALLYWIX_DOCKS, 60.0f))
                if (thrall->AI())
                    thrall->AI()->Talk(group);
        }

        TaskScheduler scheduler;
        bool _started;
        bool _ended;
        uint32 _spellTimer;
    };
}

void AddSC_lost_isles_volcano()
{
    new creature_script<lovolc_npc_canon_gob>("npc_canon_gob");
    new creature_script<lovolc_npc_oom_isle_gob>("npc_oom_isle_gob");
    new lovolc_npc_ceint();
    new spell_script<lovolc_spell_boot_damage>("spell_boot_damage");
    new creature_script<lovolc_npc_avion_gob>("npc_avion_gob");
    new lovolc_npc_izzy_airplane();
    new lovolc_npc_killag_2();
    new lovolc_npc_chariot();
    new creature_script<lovolc_npc_chariot2>("npc_Chariot2");
    new creature_script<lovolc_npc_prince>("npc_Prince");
}
