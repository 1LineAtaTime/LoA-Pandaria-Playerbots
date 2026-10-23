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

// #680 (2026-10-23): Shockwave and Bad Intentions are back on, ported from The-Cataclysm-Preservation-Project TrinityCore 4.3.4
// (src/server/scripts/Kalimdor/LostCityOfTheTolvir/boss_general_husam.cpp + sql/old/4.3.4/world 2017_11_23_01 / 2020_07_09_00).
// Why both were commented out ("To do: fix client crash"):
//  - Bad Intentions did the whole charge -> ride Husam -> Hurl -> ride the wall target -> Hard Impact chain inside ONE server call.
//    This core boards synchronously (Vehicle::AddPassenger calls PassengerBoarded before it returns), and the old PassengerBoarded
//    cast Hurl at once, whose script called ExitVehicle() and the next ride. So the player left Husam's seat while that seat's
//    CONTROL_VEHICLE aura was still being applied, and the client got the transport-enter spline, the exit and a second
//    vehicle's enter in one packet burst. Husam's seat (VehicleSeat 8374, vehicle 1036) has the
//    WAITING_FOR_VEH_TRANSITION_ANIM_ENTER/EXIT flags (0x08000000 | 0x10000000), so the client was still waiting on the enter
//    animation when the exit arrived. Wowpedia "General Husam" notes a live-game disconnect on this very throw.
//    Now: Husam throws 1 s after the passenger is seated (EVENT_THROW_PLAYER). Hurl moves the passenger to the wall target
//    through Unit::_EnterVehicle's own vehicle switch, with no manual ExitVehicle. The wall target hits and ejects 400 ms after
//    boarding, from its own UpdateAI. This is the TC 4.3.4 timing.
//  - Shockwave: the stalkers summoned a visual every 250 ms 4 yd from Husam, so about 80 visuals stacked at four points.
//    They never despawned, and they were handed to Siamat through instance GetGuidData(DATA_GENERAL_HUSAM), which returned
//    Siamat's guid. Each Shockwave then fired about 20 stacked 83454 hits per point. Now the four stalkers come from the DBC
//    summons 83131-83134 (4 yd N/S/E/W, 10 s). They run 40 yd out at 8 yd/s, leaving a visual every 2 yd (TC: aura 83129 ->
//    83466 -> 83128). Each visual does its own 4 yd 83454 hit 4.6 s after it appears, then despawns at 12 s.
// Text group ids follow the creature_text rows (0 aggro, 1 slay, 2 "Murkash!", 3 Shockwave emote, 4 "Tread Lightly", 5 death).

#include "ScriptPCH.h"
#include "lost_city_of_the_tolvir.h"
#include "MoveSplineInit.h"
#include "PathGenerator.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_HAMMER_FIST                              = 83655,
    // Mystic Trap
    SPELL_MYSTIC_TRAP_FIND_TARGET                  = 83644,
    SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER        = 83111,
    SPELL_THROW_LAND_MINES                         = 83122,
    SPELL_TOLVIR_LAND_MINE_VISUAL                  = 83110,
    SPELL_TOLVIR_LAND_MIVE_PERIODIC                = 85523,
    SPELL_MYSTIC_TRAP                              = 83171,
    SPELL_DETONATE_TRAPS                           = 91263,
    // Bad Intentions
    SPELL_BAD_INTENTIONS                           = 83113,  // eff0 charge, eff1 script: the target rides Husam (46598)
    SPELL_THROW_VISUAL                             = 83371,
    SPELL_HARD_IMPACT                              = 83339,
    SPELL_RIDE_VEHICLE_HARDCODED                   = 46598,
    SPELL_HURL_SCRIPT                              = 83236,  // eff0 script (bp 83235)
    SPELL_HURL_RIDE                                = 83235,  // CONTROL_VEHICLE on the nearest Bad Intentions Target (conditions)
    // Shockwave
    SPELL_SHOCKWAVE                                = 83445,  // 5 s cast, hits the stalkers (conditions 83445 -> 44711)
    SPELL_SHOCKWAVE_VISUAL                         = 83130,
    SPELL_SHOCKWAVE_STALKER_VISUAL                 = 83127,
    SPELL_SHOCKWAVE_DAMAGE                         = 83454,  // 4 yd around a Shockwave Visual
    SPELL_SUMMON_SHOCKWAVE                         = 83128,  // summons a Shockwave Visual (44712, 15 s)
    SPELL_SUMMON_SHOCKWAVE_TARGET_N                = 83131,  // the four stalkers (44711, 10 s), 4 yd from Husam
    SPELL_SUMMON_SHOCKWAVE_TARGET_S                = 83132,
    SPELL_SUMMON_SHOCKWAVE_TARGET_E                = 83133,
    SPELL_SUMMON_SHOCKWAVE_TARGET_W                = 83134,
};

enum eCreatures
{
    NPC_MYSTIC_TRAP_TARGET                         = 44840,
    NPC_BAD_INTENTIONS_TARGET                      = 44586,
    NPC_SHOCKWAVE_STALKER                          = 44711,
    NPC_SHOCKWAVE_VISUAL                           = 44712,
    NPC_LAND_MINE_VEHICLE                          = 44798,
};

enum eEvents
{
    EVENT_SUMMON_LAND_MINES                        = 1,
    EVENT_COUNTDOWN_LAND_MINES                     = 2,
    EVENT_SUMMON_SHOCKWAVE                         = 3,
    EVENT_HAMMER_FIST                              = 4,
    EVENT_BAD_INTENTIONS                           = 5,
    EVENT_THROW_PLAYER                             = 6,
    EVENT_BAD_INTENTIONS_CHECK                     = 7,  // #680: did the prepared 83113 really go off?
};

enum ePhases
{
    LAND_MINE_STATE_JUSTADDED                      = 0x1,
    LAND_MINE_STATE_ACTIVATED                      = 0x2,
    LAND_MINE_STATE_COUNTDOWN                      = 0x4,
    LAND_MINE_STATE_DETONATED                      = 0x8,
};

enum Texts
{
    SAY_AGGRO                                      = 0,
    SAY_SLAY                                       = 1,
    SAY_SHOCKWAVE                                  = 2,
    SAY_ANNOUNCE_SHOCKWAVE                         = 3,
    SAY_DETONATE_MINES                             = 4,
    SAY_DEATH                                      = 5,
};

enum eMisc
{
    SEAT_PLAYER                                    = 0,
};

class boss_general_husam : public CreatureScript
{
    public:
        boss_general_husam() : CreatureScript("boss_general_husam") { }

        struct boss_general_husamAI : public BossAI
        {
            boss_general_husamAI(Creature* creature) : BossAI(creature, DATA_GENERAL_HUSAM)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->setActive(true);
            }

            uint8 uiHammerFistCount;
            bool _badIntentionsCast = false;     // #680: set by SpellCasted() when 83113 really went off
            uint8 _badIntentionsTries = 0;
            ObjectGuid _badIntentionsTarget;

            void Reset() override
            {
                _Reset();
                _badIntentionsTries = 0;

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (Vehicle* vehicle = me->GetVehicleKit())
                    vehicle->RemoveAllPassengers();

                summons.DespawnAll();
                uiHammerFistCount = 0;
            }

            // #680: only schedule the throw here. Casting Hurl inside the boarding call is what crashed the client (see the top of the file).
            void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool apply) override
            {
                if (apply)
                    events.ScheduleEvent(EVENT_THROW_PLAYER, 1000);
            }

            // #680: Unit::CastSpell returns nothing. A non-triggered instant spell is only PREPARED by it (Spell::prepare puts it in
            // CURRENT_GENERIC_SPELL; it is cast on the next spell update unless TRIGGERED_CAST_DIRECTLY, Spell.cpp prepare), and
            // Spell::HandleAfterCast -> AI()->SpellCasted is the one sign that it really went off.
            void SpellCasted(Spell const* spell) override
            {
                if (spell->GetSpellInfo()->Id != SPELL_BAD_INTENTIONS)
                    return;

                _badIntentionsCast = true;
                Unit* target = ObjectAccessor::GetUnit(*me, _badIntentionsTarget);
                TC_LOG_INFO("scripts", "[LCT-HUSAM] bad intentions cast on %s dist=%.1f try=%u", target ? target->GetName().c_str() : "?",
                    target ? me->GetDistance(target) : -1.0f, uint32(_badIntentionsTries) + 1);
                _badIntentionsTries = 0;
            }

            // #680 (w106): Bad Intentions went off 0 times in a whole fight and nothing said why. Every failed attempt now leaves one
            // [LCT-HUSAM] scripts line with the checks Spell::CheckCast makes for its charge effect (Spell.cpp SPELL_EFFECT_CHARGE:
            // target alive, LOS, caster not rooted, a complete path), whether it failed in prepare or was prepared and then never
            // cast, and it retries 2 s later (up to 5 tries) instead of silently waiting for the next Shockwave.
            void LogBadIntentionsFailure(Unit* target, char const* stage)
            {
                if (!target)
                {
                    TC_LOG_INFO("scripts", "[LCT-HUSAM] bad intentions %s: no non-tank player on the threat list (victim=%s threat=%u) try=%u",
                        stage, me->GetVictim() ? me->GetVictim()->GetName().c_str() : "-",
                        uint32(me->GetThreatManager().GetThreatListSize()), uint32(_badIntentionsTries) + 1);
                    return;
                }

                PathGenerator path(me);
                bool const pathOk = path.CalculatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false);
                Spell const* current = me->GetCurrentSpell(CURRENT_GENERIC_SPELL);
                TC_LOG_INFO("scripts", "[LCT-HUSAM] bad intentions %s on %s dist=%.1f alive=%d los=%d path=%d pathtype=0x%X rooted=%d "
                    "generic=%u victim=%s threat=%u try=%u", stage, target->GetName().c_str(), me->GetDistance(target),
                    int(target->IsAlive()), int(target->IsWithinLOSInMap(me)), int(pathOk), uint32(path.GetPathType()),
                    int(me->HasUnitState(UNIT_STATE_ROOT)), current ? current->GetSpellInfo()->Id : 0u,
                    me->GetVictim() ? me->GetVictim()->GetName().c_str() : "-", uint32(me->GetThreatManager().GetThreatListSize()),
                    uint32(_badIntentionsTries) + 1);
            }

            void RetryBadIntentions()
            {
                if (++_badIntentionsTries < 5)
                    events.ScheduleEvent(EVENT_BAD_INTENTIONS, 2000);
                else
                    _badIntentionsTries = 0;
            }

            void DoBadIntentions()
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me));
                _badIntentionsCast = false;
                _badIntentionsTarget = target ? target->GetGUID() : ObjectGuid::Empty;
                if (!target)
                {
                    LogBadIntentionsFailure(nullptr, "skipped");
                    RetryBadIntentions();
                    return;
                }

                me->CastSpell(target, SPELL_BAD_INTENTIONS, false);
                if (_badIntentionsCast)
                    return;     // went off inside the call (SpellCasted logged it)

                Spell const* current = me->GetCurrentSpell(CURRENT_GENERIC_SPELL);
                if (current && current->GetSpellInfo()->Id == SPELL_BAD_INTENTIONS)
                {
                    events.ScheduleEvent(EVENT_BAD_INTENTIONS_CHECK, 1500);   // prepared: it should go off on the next spell update
                    return;
                }

                LogBadIntentionsFailure(target, "FAILED-prepare");
                RetryBadIntentions();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk(SAY_AGGRO);
                _badIntentionsTries = 0;
                events.ScheduleEvent(EVENT_SUMMON_LAND_MINES, 3000);
                events.ScheduleEvent(EVENT_HAMMER_FIST, 5000);
                // #680: TC 4.3.4 first timers (Shockwave 15 s heroic / 18 s normal, Bad Intentions 12-13 s)
                events.ScheduleEvent(EVENT_SUMMON_SHOCKWAVE, IsHeroic() ? 15000 : 18000);
                events.ScheduleEvent(EVENT_BAD_INTENTIONS, urand(12000, 13000));

                if (IsHeroic())
                    events.ScheduleEvent(EVENT_COUNTDOWN_LAND_MINES, 15000);
            }

            void JustSummoned(Creature* summon) override
            {
                switch (summon->GetEntry())
                {
                    case NPC_LAND_MINE_VEHICLE:
                    {
                        float x, y, z;
                        summon->GetPosition(x, y, z);

                        if (Creature* mine = me->SummonCreature(44796, x, y, z))
                            mine->EnterVehicle(summon);
                        break;
                    }
                    case NPC_SHOCKWAVE_STALKER:
                    {
                        // TC 4.3.4: each stalker runs 40 yd straight out from Husam at 8 yd/s (the visual trail is its own AI)
                        float const angle = me->GetAngle(summon);
                        float const x = summon->GetPositionX() + std::cos(angle) * 40.0f;
                        float const y = summon->GetPositionY() + std::sin(angle) * 40.0f;
                        Movement::MoveSplineInit init(summon);
                        init.MoveTo(x, y, summon->GetPositionZ(), false, true);
                        init.SetVelocity(8.0f);
                        init.Launch();
                        break;
                    }
                    default:
                        break;
                }

                summons.Summon(summon);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                // nobody stays in Husam's hand when he dies between the pick-up and the throw
                if (Vehicle* vehicle = me->GetVehicleKit())
                    vehicle->RemoveAllPassengers();

                Talk(SAY_DEATH);
                summons.DespawnAll();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_COUNTDOWN_LAND_MINES:
                            Talk(SAY_DETONATE_MINES);
                            events.ScheduleEvent(EVENT_COUNTDOWN_LAND_MINES, 15000);
                            me->CastSpell(me, SPELL_DETONATE_TRAPS, false);
                            break;
                        case EVENT_SUMMON_LAND_MINES:
                            {
                                me->CastSpell(me, SPELL_MYSTIC_TRAP_FIND_TARGET, false);
                                std::list<Creature*> lTriggers;
                                me->GetCreatureListWithEntryInGrid(lTriggers, NPC_MYSTIC_TRAP_TARGET, 50.0f);

                                if (!lTriggers.empty())
                                    for (std::list<Creature*>::const_iterator itr = lTriggers.begin(); itr != lTriggers.end(); ++itr)
                                        if (*itr)
                                            me->CastSpell((*itr), SPELL_THROW_LAND_MINES, false);

                                events.ScheduleEvent(EVENT_SUMMON_LAND_MINES, 11000);
                            }
                            break;
                        case EVENT_SUMMON_SHOCKWAVE:
                            // TC 4.3.4 EVENT_SHOCKWAVE: emote + yell, the four stalkers, then the 5 s Shockwave cast
                            Talk(SAY_ANNOUNCE_SHOCKWAVE);
                            Talk(SAY_SHOCKWAVE);
                            me->CastSpell(me, SPELL_SUMMON_SHOCKWAVE_TARGET_N, true);
                            me->CastSpell(me, SPELL_SUMMON_SHOCKWAVE_TARGET_S, true);
                            me->CastSpell(me, SPELL_SUMMON_SHOCKWAVE_TARGET_E, true);
                            me->CastSpell(me, SPELL_SUMMON_SHOCKWAVE_TARGET_W, true);
                            me->CastSpell(me, SPELL_SHOCKWAVE, false);

                            uiHammerFistCount = 0;
                            events.RescheduleEvent(EVENT_SUMMON_LAND_MINES, urand(8000, 10000));
                            events.RescheduleEvent(EVENT_HAMMER_FIST, urand(10000, 12000));
                            events.RescheduleEvent(EVENT_BAD_INTENTIONS, 17000);
                            events.ScheduleEvent(EVENT_SUMMON_SHOCKWAVE, 39000);
                            break;
                        case EVENT_HAMMER_FIST:
                            {
                                ++uiHammerFistCount;
                                me->CastSpell(me->GetVictim(), SPELL_HAMMER_FIST, false);

                                if (uiHammerFistCount < 4)
                                    events.ScheduleEvent(EVENT_HAMMER_FIST, 500);
                                else
                                {
                                    uiHammerFistCount = 0;
                                    events.ScheduleEvent(EVENT_HAMMER_FIST, urand(5000, 15000));
                                }
                            }
                            break;
                        case EVENT_BAD_INTENTIONS:
                            // TC 4.3.4: a random non-tank; the next one comes from the Shockwave reschedule (17 s after each Shockwave)
                            DoBadIntentions();
                            break;
                        case EVENT_BAD_INTENTIONS_CHECK:
                            if (!_badIntentionsCast)
                            {
                                LogBadIntentionsFailure(ObjectAccessor::GetUnit(*me, _badIntentionsTarget), "FAILED-not-cast");
                                RetryBadIntentions();
                            }
                            break;
                        case EVENT_THROW_PLAYER:
                            // 1 s after the pick-up: Hurl moves the passenger onto the nearest Bad Intentions Target (spell_hurl)
                            me->CastSpell(me, SPELL_HURL_SCRIPT, true);
                            me->CastSpell(me, SPELL_THROW_VISUAL, true);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_general_husamAI>(creature);
        }
};

class npc_land_mine : public CreatureScript
{
    public:
        npc_land_mine() : CreatureScript("npc_land_mine") { }

        struct npc_land_mineAI : public ScriptedAI
        {
            npc_land_mineAI(Creature* creature) : ScriptedAI(creature)
            {
                uiState = LAND_MINE_STATE_JUSTADDED;
                uiDespawnTimer = 1000;
                uiActivationTimer = 2000;
                uiCountdownTimer = urand(20000, 35000);
                me->SetInCombatWithZone();
            }
            
            uint32 uiActivationTimer;
            uint32 uiCountdownTimer;
            uint32 uiDespawnTimer;
            uint8 uiState;

            void AddMineState(uint32 state)
            {
                uiState |= state;
            }

            bool HasMineState(const uint32 state) const
            {
                return (uiState & state);
            }

            void ClearMineState(uint32 state)
            {
                uiState &= ~state;
            }

            void StartCountDown()
            {
                if (HasMineState(LAND_MINE_STATE_DETONATED | LAND_MINE_STATE_COUNTDOWN) || !HasMineState(LAND_MINE_STATE_ACTIVATED))
                    return;

                AddMineState(LAND_MINE_STATE_DETONATED);
                AddMineState(LAND_MINE_STATE_COUNTDOWN);
                me->AddAura(SPELL_TOLVIR_LAND_MIVE_PERIODIC, me);
                uiDespawnTimer = 5500;
            }

            void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
            {
                if (spell->Id == SPELL_DETONATE_TRAPS)
                    StartCountDown();
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                if (spell->Id == 83112)
                {
                    float x, y, _x, _y;
                    me->GetPosition(x, y);
                    target->GetPosition(_x, _y);

                    if (sqrt(pow(x - _x, 2) + pow(y - _y, 2)) < 1.4f)
                    {
                        if (!HasMineState(LAND_MINE_STATE_ACTIVATED))
                            return;

                        ClearMineState(LAND_MINE_STATE_ACTIVATED);
                        AddMineState(LAND_MINE_STATE_DETONATED);
                        uiDespawnTimer = 1000;
                        me->CastSpell(target, SPELL_MYSTIC_TRAP, true);
                        me->RemoveAura(SPELL_TOLVIR_LAND_MINE_VISUAL);
                        me->RemoveAura(SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (HasMineState(LAND_MINE_STATE_JUSTADDED))
                {
                    if (uiActivationTimer <= diff)
                    {
                        ClearMineState(LAND_MINE_STATE_JUSTADDED);
                        AddMineState(LAND_MINE_STATE_ACTIVATED);
                        me->AddAura(SPELL_TOLVIR_LAND_MINE_VISUAL, me);
                        me->AddAura(SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER, me);
                    }
                    else
                        uiActivationTimer -= diff;
                }

                if (HasMineState(LAND_MINE_STATE_ACTIVATED) && !HasMineState(LAND_MINE_STATE_COUNTDOWN))
                {
                    if (uiCountdownTimer <= diff)
                        StartCountDown();
                    else
                        uiCountdownTimer -= diff;
                }

                if (HasMineState(LAND_MINE_STATE_DETONATED))
                {
                    if (uiDespawnTimer <= diff)
                    {
                        if (Creature* pMineVehicle = me->GetVehicleCreatureBase())
                            pMineVehicle->DespawnOrUnsummon();

                        me->DespawnOrUnsummon();
                    }
                    else
                        uiDespawnTimer -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_land_mineAI>(creature);
        }
};

// #680: TC 4.3.4 Shockwave Stalker (44711). Husam's JustSummoned starts it on its 40 yd run. Every 250 ms it summons a
// Shockwave Visual (83128) unless one already stands within 2 yd. That is the TC aura 83129 -> 83466 -> 83128 chain, done
// here so no creature_template_addon / spell_script_names / conditions rows are needed. It despawns with its 10 s summon
// duration (83131-83134).
class npc_shockwave_stalker : public CreatureScript
{
    public:
        npc_shockwave_stalker() : CreatureScript("npc_shockwave_stalker") { }

        struct npc_shockwave_stalkerAI : public ScriptedAI
        {
            npc_shockwave_stalkerAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                instance = creature->GetInstanceScript();
                uiTrailTimer = 0;
            }

            InstanceScript* instance;
            uint32 uiTrailTimer;

            void JustSummoned(Creature* summon) override
            {
                if (summon->GetEntry() != NPC_SHOCKWAVE_VISUAL)
                    return;

                summon->SetReactState(REACT_PASSIVE);
                summon->AddAura(SPELL_SHOCKWAVE_STALKER_VISUAL, summon);
                // TC 4.3.4 npc_husam_shockwave_visual: 83454 (4 yd) 4.6 s after it appears, auras off at 7 s, gone at 12 s
                summon->m_Events.AddLambdaEventAtOffset([summon]() { summon->CastSpell(summon, SPELL_SHOCKWAVE_DAMAGE, true); }, 4600);
                summon->m_Events.AddLambdaEventAtOffset([summon]() { summon->RemoveAllAuras(); }, 7000);
                summon->DespawnOrUnsummon(12000);

                // Husam tracks it too, so it goes with his death / evade (instance guid fixed by #680)
                if (instance)
                    if (Creature* husam = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_GENERAL_HUSAM)))
                        if (husam->IsAIEnabled)
                            husam->AI()->JustSummoned(summon);
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_SHOCKWAVE && caster)
                    caster->CastSpell(me, SPELL_SHOCKWAVE_VISUAL, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (uiTrailTimer > diff)
                {
                    uiTrailTimer -= diff;
                    return;
                }

                uiTrailTimer = 250;
                if (!me->FindNearestCreature(NPC_SHOCKWAVE_VISUAL, 2.0f))
                    me->CastSpell(me, SPELL_SUMMON_SHOCKWAVE, true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_shockwave_stalkerAI>(creature);
        }
};

// #680: Bad Intentions Target (44586, the wall spots). Hard Impact, then the eject, 400 ms after the passenger is seated, from
// UpdateAI and not inside the boarding call (TC 4.3.4 npc_husam_bad_intentions_target). 50630 Eject All Passengers has no
// handler on this core, so the eject is RemoveAllPassengers.
class npc_bad_intentios_target : public CreatureScript
{
    public:
        npc_bad_intentios_target() : CreatureScript("npc_bad_intentios_target") { }

        struct npc_bad_intentios_targetAI : public ScriptedAI
        {
            npc_bad_intentios_targetAI(Creature* creature) : ScriptedAI(creature)
            {
                uiImpactTimer = 0;
            }

            ObjectGuid passengerGUID;
            uint32 uiImpactTimer;

            void Reset() override
            {
                uiImpactTimer = 0;
                passengerGUID = ObjectGuid::Empty;

                if (Vehicle* vehicle = me->GetVehicleKit())
                    vehicle->RemoveAllPassengers();
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
            {
                if (!apply)
                    return;

                passengerGUID = who->GetGUID();
                uiImpactTimer = 400;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!uiImpactTimer)
                    return;

                if (uiImpactTimer > diff)
                {
                    uiImpactTimer -= diff;
                    return;
                }

                uiImpactTimer = 0;
                if (Unit* passenger = ObjectAccessor::GetUnit(*me, passengerGUID))
                    if (passenger->GetVehicleBase() == me)
                        me->CastSpell(passenger, SPELL_HARD_IMPACT, true);

                passengerGUID = ObjectGuid::Empty;
                if (Vehicle* vehicle = me->GetVehicleKit())
                    vehicle->RemoveAllPassengers();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_bad_intentios_targetAI>(creature);
        }
};

// 83113 Bad Intentions, eff1 script: the charged player rides Husam (TC 4.3.4 spell_husam_bad_intentions: stop the cast,
// triggered ride, so a casting player is still picked up)
class spell_bad_intentions : public SpellScriptLoader
{
    public:
        spell_bad_intentions() : SpellScriptLoader("spell_bad_intentions") { }

        class spell_bad_intentions_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_bad_intentions_SpellScript)

            void HandleScript(SpellEffIndex /*effect*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!caster || !target || target->GetVehicle())
                    return;

                target->CastStop();
                target->CastSpell(caster, SPELL_RIDE_VEHICLE_HARDCODED, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_bad_intentions_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_bad_intentions_SpellScript();
        }
};

// 83236 Hurl, cast by Husam on himself 1 s after the pick-up. TC 4.3.4 spell_husam_hurl: the seated passenger rides the nearest
// Bad Intentions Target (83235). Unit::_EnterVehicle leaves Husam's seat itself, so there is no ExitVehicle() before it (the old
// code exited first, inside the boarding call). If no target is found, the passenger is just let go.
class spell_hurl : public SpellScriptLoader
{
    public:
        spell_hurl() : SpellScriptLoader("spell_hurl") { }

        class spell_hurl_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hurl_SpellScript)

            void HandleScript(SpellEffIndex /*effect*/)
            {
                Unit* caster = GetCaster();
                Vehicle* vehicle = caster ? caster->GetVehicleKit() : nullptr;
                if (!vehicle)
                    return;

                Unit* passenger = vehicle->GetPassenger(SEAT_PLAYER);
                if (!passenger)
                    return;

                if (Creature* wall = caster->FindNearestCreature(NPC_BAD_INTENTIONS_TARGET, 100.0f))
                    passenger->CastSpell(wall, SPELL_HURL_RIDE, true);
                else
                    passenger->ExitVehicle();
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_hurl_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hurl_SpellScript();
        }
};

void AddSC_boss_general_husam()
{
    new boss_general_husam();
    new npc_land_mine();
    new npc_shockwave_stalker();
    new npc_bad_intentios_target();

    new spell_bad_intentions();
    new spell_hurl();
}
