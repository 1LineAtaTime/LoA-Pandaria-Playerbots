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
#include "halls_of_origination.h"

// Sun-Touched Spriteling 39370
// Sun-Touched Sprite 39369

enum Spells
{
    // Rajh
    SPELL_SUN_STRIKE                    = 73872, 
    SPELL_SUN_STRIKE_DMG                = 73874,
    // #691 (Wowpedia Rajh 2026-10-23; TC 4.3.4 TCPP boss_rajh.cpp; 5.4.8 DBC). TCPP rides Rajh on the Inferno Leap vehicle
    // (87653, an area CONTROL_VEHICLE with no conditions row here) and drops the orb through the serverside 76374; both are
    // done by the script below instead, with the DBC spells for the damage.
    SPELL_INFERNO_LEAP                  = 87647, // landing: 32.4k (heroic 120k) within 10 yd + knock back
    SPELL_INFERNO_LEAP_SPEED            = 87645, // on the landing marker: 87657 +50% speed within 10 yd every 1 s
    SPELL_SUMMON_SUN_ORB                = 80352, // 3 s channel (interruptible); its tick casts 76338 (summons npc 40835)
    SPELL_SUMMON_SUN_ORB_SUMMON         = 76338,
    SPELL_BLAZING_INFERNO               = 76195, // the orb lands: 18.5k within 13 yd + knock back
    SPELL_BLESSING_OF_THE_SUN           = 76352,
    SPELL_BLESSING_OF_THE_SUN_AURA      = 76355,
    SPELL_ZERO_POWER                    = 72242,
    SPELL_SUMMON_SOLAR_WINDS            = 74104,
    SPELL_SOLAR_WINDS_AURA              = 74107,
    SPELL_SOLAR_WINDS_DMG               = 74108,
    SPELL_SOLAR_WINDS_VISUAL            = 74109,
    SPELL_SOLAR_FIRE                    = 89131,
    SPELL_SOLAR_FIRE_DMG                = 89133,
};

enum Texts
{
    SAY_DEATH    = 0,
    SAY_AGGRO    = 1,
    SAY_ENERGIZE = 2,
    SAY_KILL     = 3, 
};

const Position pointcenter = {-319.68f, 193.42f, 343.94f, 4.76f};

enum Events
{
    EVENT_SUN_STRIKE           = 1,
    EVENT_SUMMON_SOLAR_WINDS   = 2,
    EVENT_SUMMON_SUN_ORB       = 3,
    EVENT_INFERNO_LEAP         = 4,
    EVENT_INFERNO_LEAP_JUMP    = 5,
};

enum RajhMisc
{
    POINT_INFERNO_LEAP         = 2,
    SUN_ORB_ENERGY             = 20,    // Wowpedia: "costs Rajh 20 Sun Energy"; TCPP charges it when the channel completes
    INFERNO_LEAP_ENERGY        = 10,    // 87653 (the leap cast) SpellPower: 10 energy
    INFERNO_LEAP_CAST_MS       = 1500,  // 87653 SpellCastTimes 16
    SUN_ORB_CHANNEL_MS         = 3000,
    SUN_ORB_HEIGHT             = 15,    // TCPP spell_rajh_summon_sun_orb: dest + 15 yd
    SUN_ORB_FALL_MS            = 1500,
    SUN_ORB_LAND_MS            = 3500,
};

enum Adds
{
    NPC_SOLAR_WINDS1            = 39634,
    NPC_SOLAR_WINDS2            = 39635, // 74109
    NPC_SOLAR_WINDS3            = 47922,
    NPC_SUN_TOUCHED_SPRITELING  = 39370, // 76159
    NPC_SUN_TOUCHED_SPRITE      = 39369, // 76158,
    NPC_ORB_OF_SUN              = 40835, // 73658
    NPC_INFERNO_LEAP            = 47040,
    NPC_BLAZING_INFERNO         = 40927,
};

class boss_rajh : public CreatureScript
{
    public:
        boss_rajh() : CreatureScript("boss_rajh") { }

        struct boss_rajhAI : public BossAI
        {
            boss_rajhAI(Creature* creature) : BossAI(creature, DATA_RAJH)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            uint8 phase;
            uint8 timerCase = 0;            // TCPP _randomTimerCase: two timer sets for the orb and Sun Strike
            bool orbChanneling = false;
            uint32 orbChannelStart = 0;
            uint32 orbLaunchedAt = 0;
            float leapX = 0.0f;
            float leapY = 0.0f;
            float leapZ = 0.0f;
            bool leaping = false;
            uint32 leapStart = 0;

            void Reset() override
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                me->GetMap()->SetWorldState(WORLDSTATE_SUN_OF_A, 1);

                phase = 0;
                orbChanneling = false;
                orbLaunchedAt = 0;
                leaping = false;

                SetEquipmentSlots(false, 1728, 0, 0);

                DoCast(me, SPELL_ZERO_POWER, true);
                me->SetPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetPower(POWER_ENERGY, 100);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_SUN_STRIKE, urand(5000, 10000));
                events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(10000, 15000));
                ScheduleOrbAndLeap();

                DoZoneInCombat();
            }

            // #691: TCPP boss_rajh.cpp JustEngagedWith / EVENT_REENGAGE timers - Sun Orb at 10 s or 25 s, Inferno Leap at 14 s.
            void ScheduleOrbAndLeap()
            {
                timerCase = urand(0, 1);
                events.ScheduleEvent(EVENT_SUMMON_SUN_ORB, timerCase == 0 ? 10000 : 25000);
                events.ScheduleEvent(EVENT_INFERNO_LEAP, 14000);
            }

            void JustSummoned(Creature* summon) override
            {
                switch (summon->GetEntry())
                {
                    case NPC_ORB_OF_SUN:
                        LaunchSunOrb(summon);
                        break;
                    case NPC_INFERNO_LEAP:
                        // the landing marker: held, never put in combat (BossAI::JustSummoned would make it chase)
                        summons.Summon(summon);
                        summon->SetReactState(REACT_PASSIVE);
                        summon->SetControlled(true, UNIT_STATE_ROOT);
                        break;
                    default:
                        BossAI::JustSummoned(summon);
                        break;
                }
            }

            // Wowpedia Summon Sun Orb: "summons a Sun Orb above the location of a random player. The Sun Orb then falls, and when
            // it lands the Orb deals 18500 to 21500 Fire damage to any player within 13 yards and knocks them back". TCPP: the
            // orb appears 15 yd up, lands ~4.5 s later and the Blazing Inferno (npc 40927) casts 76195 on it.
            void LaunchSunOrb(Creature* orb)
            {
                // the channel's own tick and the completion fallback (UpdateAI) can both summon one: keep the first
                if (orbLaunchedAt && GetMSTimeDiffToNow(orbLaunchedAt) < 2000)
                {
                    orb->DespawnOrUnsummon();
                    return;
                }

                orbLaunchedAt = getMSTime();
                summons.Summon(orb);
                orb->SetReactState(REACT_PASSIVE);

                float x, y, z;
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    target->GetPosition(x, y, z);
                else
                    me->GetPosition(x, y, z);

                orb->SetDisableGravity(true);
                orb->NearTeleportTo(x, y, z + float(SUN_ORB_HEIGHT), orb->GetOrientation());
                orb->m_Events.Schedule(SUN_ORB_FALL_MS, [orb, x, y, z]()
                {
                    orb->GetMotionMaster()->MovePoint(0, x, y, z, false);
                });
                orb->m_Events.Schedule(SUN_ORB_LAND_MS, [orb, x, y, z]()
                {
                    if (Creature* inferno = orb->SummonCreature(NPC_BLAZING_INFERNO, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(3000)))
                    {
                        inferno->SetReactState(REACT_PASSIVE);
                        inferno->CastSpell(orb, SPELL_BLAZING_INFERNO, true);
                    }
                    orb->DespawnOrUnsummon(500);
                });
            }

            void JustReachedHome() override
            {
                _JustReachedHome();
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                // #691 Inferno Leap lands: 87647 hits within 10 yd of the landing spot
                if (type == EFFECT_MOTION_TYPE && pointId == POINT_INFERNO_LEAP)
                {
                    leaping = false;
                    DoCast(me, SPELL_INFERNO_LEAP, true);
                    if (phase == 0)
                        me->SetReactState(REACT_AGGRESSIVE);
                    return;
                }

                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                        case 1:
                            Talk(SAY_ENERGIZE);
                            DoCastAOE(SPELL_BLESSING_OF_THE_SUN);
                            break;
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                // #691: the Sun Orb channel has ended. Only a full channel lands an orb and costs the energy (an interrupt saves
                // both - Wowpedia strategy); its last tick normally summons the orb, the completion summons it if the tick did not.
                if (orbChanneling && !me->HasUnitState(UNIT_STATE_CASTING))
                {
                    orbChanneling = false;
                    if (GetMSTimeDiffToNow(orbChannelStart) + 150 >= SUN_ORB_CHANNEL_MS)
                    {
                        me->ModifyPower(POWER_ENERGY, -int32(SUN_ORB_ENERGY));
                        if (!orbLaunchedAt || GetMSTimeDiffToNow(orbLaunchedAt) >= 2000)
                            DoCast(me, SPELL_SUMMON_SUN_ORB_SUMMON, true);
                    }
                }

                // a leap whose jump never reported its landing must not leave him passive for the rest of the fight
                if (leaping && GetMSTimeDiffToNow(leapStart) > INFERNO_LEAP_CAST_MS + 4500)
                {
                    leaping = false;
                    if (phase == 0)
                        me->SetReactState(REACT_AGGRESSIVE);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                // #691: TCPP / Wowpedia - he recharges below 10 Sun Energy (was < 1: a 10 left over by Inferno Leap stalled him)
                if (me->GetPower(POWER_ENERGY) <= 10 && phase == 0)
                {
                    phase = 1;
                    leaping = false;
                    events.Reset();
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(1, pointcenter);
                    return;
                }
                else if (me->GetPower(POWER_ENERGY) > 99 && phase == 1)
                {
                    phase = 0;
                    me->GetMap()->SetWorldState(WORLDSTATE_SUN_OF_A, 0);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.ScheduleEvent(EVENT_SUN_STRIKE, urand(5000, 10000));
                    events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(6000, 11000));
                    ScheduleOrbAndLeap();
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
                    return;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUN_STRIKE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_SUN_STRIKE);
                            events.ScheduleEvent(EVENT_SUN_STRIKE, urand(20000, 25000));
                            break;
                        case EVENT_SUMMON_SOLAR_WINDS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_SUMMON_SOLAR_WINDS);
                            events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(20000, 25000));
                            break;
                        case EVENT_SUMMON_SUN_ORB:
                            me->StopMoving();
                            DoCast(me, SPELL_SUMMON_SUN_ORB);
                            orbChanneling = me->HasUnitState(UNIT_STATE_CASTING);
                            orbChannelStart = getMSTime();
                            events.ScheduleEvent(EVENT_SUMMON_SUN_ORB, timerCase == 0 ? urand(35000, 36000) : urand(31000, 37000));
                            return;
                        case EVENT_INFERNO_LEAP:
                            // Wowpedia Inferno Leap: "prepares to leap to the location of a random player ... players within 10 yards
                            // of the chosen location have their movement speed increased by 50%" (87645 on the marker npc 47040).
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                target->GetPosition(leapX, leapY, leapZ);
                                if (Creature* marker = me->SummonCreature(NPC_INFERNO_LEAP, leapX, leapY, leapZ, 0.0f,
                                    TEMPSUMMON_TIMED_DESPAWN, Milliseconds(INFERNO_LEAP_CAST_MS + 2500)))
                                    marker->CastSpell(marker, SPELL_INFERNO_LEAP_SPEED, true);
                                me->AttackStop();
                                me->StopMoving();
                                me->SetReactState(REACT_PASSIVE);
                                me->SetFacingTo(me->GetAngle(leapX, leapY));
                                leaping = true;
                                leapStart = getMSTime();
                                events.ScheduleEvent(EVENT_INFERNO_LEAP_JUMP, INFERNO_LEAP_CAST_MS);
                            }
                            events.ScheduleEvent(EVENT_INFERNO_LEAP, urand(59000, 60000));
                            break;
                        case EVENT_INFERNO_LEAP_JUMP:
                            me->ModifyPower(POWER_ENERGY, -int32(INFERNO_LEAP_ENERGY));
                            me->GetMotionMaster()->MoveJump(leapX, leapY, leapZ, 25.0f, 10.0f, POINT_INFERNO_LEAP);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_rajhAI>(creature);
        }
};

class npc_rajh_solar_wind : public CreatureScript
{
    public:
        npc_rajh_solar_wind() : CreatureScript("npc_rajh_solar_wind") { }

        struct npc_rajh_solar_windAI : public ScriptedAI
        {
            npc_rajh_solar_windAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                DoCast(me, SPELL_SOLAR_WINDS_VISUAL, true);
                DoCast(me, SPELL_SOLAR_WINDS_AURA, true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_rajh_solar_windAI>(creature);
        }
};

void AddSC_boss_rajh()
{
    new boss_rajh();
    new npc_rajh_solar_wind();
}
