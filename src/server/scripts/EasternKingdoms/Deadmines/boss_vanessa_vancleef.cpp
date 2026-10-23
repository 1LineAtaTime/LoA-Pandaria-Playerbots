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
#include "deadmines.h"
#include "LFGMgr.h"
#include "Group.h"
#include "Vehicle.h"

// #681 / #826: the boss fight is ported from The-Cataclysm-Preservation-Project TrinityCore 4.3.4 boss_vanessa_van_cleef.cpp
// (e503f96, 2025-08-02): Deflection, the 50% / 25% detonations (Fiery Blaze on the deck, the ropes), Vengeance of VanCleef and the
// 1% Powder Explosion; the intro now ends in the Nightmare Gauntlet (instance_deadmines.cpp) instead of showing the boss at once.
// Intended design: Wowpedia "Vanessa VanCleef (tactics)" (MediaWiki API, 2026-10-23).

// creature_text groups of the MoP test world DB (49541 = CPP boss_vanessa_van_cleef.cpp:35-46; the old ids said the slay line on
// death and the adds line on a kill)
enum ScriptedTexts
{
    SAY_AGGRO                   = 0,
    SAY_KILL                    = 1,
    SAY_ADDS                    = 2,
    SAY_DETONATION_1            = 3,
    SAY_ANNOUNCE_DETONATION_1   = 4,
    SAY_DETONATION_2            = 5,
    SAY_ANNOUNCE_DETONATION_2   = 6,
    SAY_SUICIDE_1               = 7,
    SAY_SUICIDE_2               = 8,
    SAY_ANNOUNCE_POWDER         = 9,
    SAY_DEATH                   = 10,

    // the sitting Vanessa 49429 (groups 8-12 repeat 1-5)
    SAY_NOISE_FROM_ABOVE        = 0,
    SAY_ANNOUNCE_NIGHTMARE_ELIXIR = 6,
    SAY_INTRO_1                 = 8,
    SAY_INTRO_2                 = 9,
    SAY_INTRO_3                 = 10,
    SAY_INTRO_4                 = 11,
    SAY_INTRO_5                 = 12,
};

enum Spells
{
    SPELL_DEADLY_BLADES         = 92622,
    SPELL_BACKSLASH_TARGETING   = 92620,
    SPELL_BACKSLASH             = 92619,
    SPELL_DEFLECTION            = 92614,
    SPELL_VENGEANCE_OF_VANCLEEF = 95542,
    SPELL_POWDER_EXPLOSION      = 96283,   // 5.4.8 SpellEffect.dbc: eff0 instakill self, eff1 185000 Fire, radius index 13 = 10 yd
    SPELL_SITTING               = 89279,
    SPELL_NOXIOUS_CONCOCTION    = 92100,
    SPELL_VANESSA_ACHIEVEMENT   = 95654,
    SPELL_SUMMON_ENFORCER       = 92616,
    SPELL_SUMMON_SHADOWGUARD    = 92618,
    SPELL_SUMMON_BLOOD_WIZARD   = 92617,

    // #681 / #826
    SPELL_CLEAR_ALL_DEBUFFS     = 34098,
    SPELL_COSMETIC_BOMB_STATE   = 96280,
    SPELL_FIERY_BLAZE           = 93484,   // periodic 1 s -> 93485, 46250 Fire within 5 yd (radius index 8)
    SPELL_CLICK_ME              = 95527,   // the Rope's addon aura
    SPELL_ROPE_BEAM             = 43785,
    SPELL_NIGHTMARE_ELIXIR      = 92113,
    SPELL_VANESSAS_BLACKOUT     = 92120,

    // Defias Enforcer
    SPELL_CHARGE                = 90928,
    SPELL_BLOODBATH             = 90925,
    SPELL_RECKLESSNESS          = 90929,

    // Defias Blood Wizard
    SPELL_BLOODWASH             = 90946,
    SPELL_BLOODBOLT             = 90938,
    SPELL_RAGEZONE              = 90932,
};

enum Adds
{
    NPC_DEFIAS_ENFORCER             = 49850,
    NPC_DEFIAS_SHADOWGUARD          = 49852,
    NPC_DEFIAS_BLOOD_WIZARD         = 49854,

    NPC_ROPE                        = 49550, // 95527
};

enum Events
{
    EVENT_INTRO_1       = 1,
    EVENT_INTRO_2       = 2,
    EVENT_INTRO_3       = 3,
    EVENT_INTRO_4       = 4,
    EVENT_INTRO_5       = 5,
    EVENT_INTRO_6       = 6,
    EVENT_INTRO_7       = 7,
    EVENT_ADDS_1        = 8,
    EVENT_ADDS_2        = 9,
    EVENT_ADDS_3        = 10,
    EVENT_BACKSLASH     = 11,
    EVENT_DEFLECTION    = 12,
    EVENT_CONTINUE      = 13,

    // #681 (CPP boss_vanessa_van_cleef.cpp:86-108)
    EVENT_ENTRANCE_FIRE = 14,
    EVENT_FACE_TO_DECK,
    EVENT_TALK_DETONATION,
    EVENT_ANNOUNCE_DETONATION,
    EVENT_SUMMON_DETONATION_FIRE,
    EVENT_SUMMON_ROPES,
    EVENT_CAST_FIERY_BLAZE,
    EVENT_EXTINGUISH_FIRES,
    EVENT_JUMP_ON_DECK,
    EVENT_FACE_TO_ROPES,
    EVENT_VENGEANCE_OF_VANCLEEF,
    EVENT_TALK_SUICIDE_1,
    EVENT_COSMETIC_BOMB_STATE,
    EVENT_TALK_SUICIDE_2,
    EVENT_ANNOUNCE_POWDER,
    EVENT_TALK_LAST_WORDS,
    EVENT_POWDER_EXPLOSION,

    // the rope (CPP :131-132)
    EVENT_START_SWING,
};

#define ACHIEVEMENT_EVENT 27527

const Position addsPos[3] =
{
    {-67.115807f, -829.708740f, 40.959011f, 1.344201f},
    {-69.161903f, -814.125977f, 40.722912f, 1.611237f},
    {-57.057121f, -816.067200f, 41.852272f, 5.565719f}
};

// CPP boss_vanessa_van_cleef.cpp:161-164
const Position vanessaDetonationJumpPos      = { -53.09028f, -819.5538f, 51.42455f };
const Position vanessaAfterDetonationJumpPos = { -66.64063f, -815.1962f, 40.94127f };

uint32 const NPC_VANESSAS_ROPE_ANCHOR_ENTRY = 49552;   // five DB spawns high above the deck (MoP test world 326765-326769)
uint32 const MODEL_ID_INVISIBLE = 11686;

// CPP deadmines.h:399-523 (e503f96), verbatim
Position const VanessaEntranceFirePos[] =
{
    { -38.11458f, -795.2952f, 39.45137f, 3.926991f },
    { -64.02257f, -797.4323f, 39.12309f, 3.926991f },
    { -46.48438f, -791.8073f, 39.12098f, 3.926991f },
    { -75.50694f, -787.1597f, 39.16325f, 3.926991f },
    { -66.18229f, -789.2778f, 39.78834f, 3.926991f },
    { -70.9809f, -795.5f, 39.14447f, 3.926991f }
};
Position const RopePos[] =
{
    { -62.17014f, -839.8438f, 41.48514f, 5.044002f },
    { -64.97049f, -840.0087f, 41.22668f, 4.834562f },
    { -67.79166f, -840.1736f, 40.96664f, 4.712389f },
    { -70.62153f, -840.0104f, 40.71846f, 4.537856f },
    { -73.42361f, -839.8646f, 40.47259f, 4.34587f  }
};
Position const RopePath1[] =
{
    { -59.18403f, -877.3125f, 35.78355f },
    { -58.08333f, -923.9757f, 50.83706f },
    { -52.68403f, -922.8768f, 50.83706f },
    { -58.59201f, -877.1684f, 38.01407f },
    { -62.86285f, -836.3472f, 46.62854f }
};
Position const RopePath2[] =
{
    { -62.23438f, -877.2969f, 35.78355f },
    { -66.63021f, -924.0417f, 50.83706f },
    { -60.85938f, -924.0399f, 50.83706f },
    { -62.03472f, -877.4514f, 38.01407f },
    { -64.96875f, -836.7847f, 46.62854f }
};
Position const RopePath3[] =
{
    { -67.98264f, -877.3785f, 35.78355f },
    { -75.66493f, -924.3958f, 50.83706f },
    { -69.01215f, -923.9114f, 50.83706f },
    { -67.80903f, -877.0972f, 38.01407f },
    { -68.02778f, -836.6268f, 46.62854f }
};
Position const RopePath4[] =
{
    { -74.47049f, -878.3611f, 35.78355f },
    { -82.86111f, -922.4688f, 50.83706f },
    { -77.72396f, -924.1285f, 50.83706f },
    { -71.76563f, -877.132f, 38.01407f },
    { -70.55209f, -836.6754f, 46.62854f }
};
Position const RopePath5[] =
{
    { -77.625f, -877.75f, 35.78355f },
    { -85.46528f, -922.4965f, 50.83706f },
    { -91.52431f, -921.3768f, 50.83706f },
    { -78.06944f, -875.9445f, 38.01407f },
    { -73.34549f, -836.3733f, 46.62854f }
};
Position const VanessaNightmareFirePos[] =
{
    { -85.18403f, -822.6042f, 39.48589f, 3.926991f },
    { -62.18924f, -834.6563f, 41.49004f, 3.193953f },
    { -76.88889f, -806.1927f, 40.12347f, 3.926991f },
    { -60.28472f, -806.1979f, 41.61631f, 2.75762f },
    { -41.65799f, -834.1059f, 34.51352f, 3.926991f },
    { -79.61111f, -794.7726f, 38.50275f, 3.926991f },
    { -54.19097f, -789.9566f, 38.85344f, 3.926991f },
    { -59.27778f, -787.6597f, 39.37201f, 3.926991f },
    { -44.94792f, -801.1077f, 40.703f, 3.926991f },
    { -75.33681f, -833.434f, 40.29444f, 3.193953f },
    { -62.4375f, -825.9114f, 41.42738f, 3.193953f },
    { -70.38889f, -824.9722f, 40.70635f, 3.193953f },
    { -66.54514f, -815.118f, 41.0335f, 3.193953f },
    { -57.90972f, -816.8611f, 41.85588f, 2.75762f },
    { -54.5382f, -833.9739f, 42.20368f, 2.75762f },  
    { -81.48264f, -832.3143f, 39.79344f, 2.75762f },
    { -76.38541f, -821.6371f, 40.15092f, 2.75762f },
    { -84.03125f, -817.4323f, 39.54358f, 2.75762f },
    { -34.31597f, -825.7864f, 35.14482f, 3.926991f },
    { -101.6024f, -822.2257f, 38.22025f, 3.926991f },
    { -110.0f, -821.908f, 30.27846f, 3.926991f },
    { -54.32118f, -840.3108f, 33.4867f, 3.926991f },
    { -92.3125f, -835.5712f, 31.01794f, 3.926991f },
    { -95.20313f, -826.7986f, 38.69661f, 3.926991f },
    { -81.6875f, -791.7795f, 25.86889f, 3.926991f },
    { -75.52952f, -788.3438f, 26.0653f, 3.926991f },
    { -104.3299f, -806.0121f, 30.58083f, 3.926991f },
    { -87.6875f, -830.8108f, 39.29154f, 2.75762f },
    { -46.73611f, -821.1771f, 52.5815f, 3.193953f },
    { -69.06944f, -834.368f, 40.86119f, 3.193953f },
    { -68.94444f, -805.1528f, 40.82362f, 3.926991f },
    { -38.67708f, -816.1389f, 43.68202f, 3.926991f },
    { -46.09722f, -830.2518f, 43.00029f, 2.75762f },
    { -46.44965f, -809.7448f, 42.93429f, 3.926991f },
    { -54.49479f, -826.5833f, 42.19642f, 3.926991f },
    { -37.77257f, -824.493f, 43.78171f, 3.926991f },
    { -106.7882f, -827.8351f, 30.3655f, 3.926991f },
    { -85.05209f, -808.5243f, 39.47224f, 3.926991f },
    { -108.5208f, -840.7136f, 17.11709f, 3.926991f },
    { -74.0f, -813.632f, 40.37197f, 3.926991f },
    { -101.724f, -815.1875f, 38.20302f, 3.926991f },
    { -109.6736f, -832.7049f, 20.99191f, 3.926991f },
    { -93.49653f, -809.6979f, 38.80095f, 3.926991f },
    { -92.09202f, -843.8507f, 21.32712f, 3.926991f },
    { -82.74479f, -853.8368f, 17.44633f, 3.926991f },
    { -83.51563f, -847.0851f, 18.62521f, 3.926991f },
    { -77.3316f, -848.7101f, 16.88625f, 3.926991f },
    { -47.97917f, -851.7882f, 18.46356f, 3.926991f },
    { -54.41667f, -846.2222f, 22.62641f, 3.926991f },
    { -46.15799f, -843.5104f, 23.03055f, 3.926991f },
    { -32.24479f, -844.8195f, 19.15213f, 3.926991f },
    { -28.87847f, -840.0226f, 19.30344f, 3.926991f },
    { -31.54861f, -830.5191f, 23.84564f, 3.926991f },
    { -53.39583f, -808.4184f, 42.27031f, 2.75762f },
    { -59.65799f, -786.8472f, 18.00813f, 3.926991f },
    { -51.8559f, -788.4774f, 18.27221f, 3.926991f },
    { -115.3142f, -827.2327f, 20.94427f, 3.926991f },
    { -116.1042f, -835.6597f, 17.07181f, 3.926991f },
    { -17.43924f, -821.8785f, 19.89213f, 3.926991f },
    { -22.3125f, -807.7292f, 19.61356f, 3.926991f },
    { -17.77431f, -814.7327f, 19.87694f, 3.926991f },
    { -116.9809f, -803.1337f, 17.05497f, 3.926991f },
    { -122.8872f, -816.658f, 17.01834f, 3.926991f },
    { -65.53993f, -768.6545f, 28.01228f, 3.926991f },
    { -123.3351f, -807.2153f, 16.98381f, 3.926991f }
};

static bool IsExtinguishableFire(Position const& pos)
{
    // CPP :375-377: Blizzard's orientations mark the fires that go out after 11 s (the others burn on)
    return std::fabs(pos.GetOrientation() - 2.75762f) < 0.001f || std::fabs(pos.GetOrientation() - 3.193953f) < 0.001f;
}

class boss_vanessa_vancleef : public CreatureScript
{
    public:
        boss_vanessa_vancleef() : CreatureScript("boss_vanessa_vancleef") { }

        struct boss_vanessa_vancleefAI : public BossAI
        {
            boss_vanessa_vancleefAI(Creature* creature) : BossAI(creature, DATA_VANESSA)
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
                me->setActive(true);
                me->SetReactState(REACT_DEFENSIVE);
            }

            void Reset() override
            {
                me->SetReactState(REACT_DEFENSIVE);
                _Reset();

                // by default right now cuz poison phase doesn`t work...
                me->GetMap()->SetWorldState(WORLDSTATE_VANCLEEF_VINDICATION, 1);

                if (instance)
                {
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                    if (instance->GetData(DATA_VANESSA_EVENT) == DONE)
                        me->SetVisible(true);
                    else
                        me->SetVisible(false);
                }
                phase = 0;
                _detonationCounter = 0;
                _prepareForReEngage = false;
                _allowDeath = false;
                _talkedAdds = false;
                _nightmareFlames.clear();
                _extinguishableFlames.clear();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                // by default right now cuz poison phase doesn`t work...
                me->GetMap()->SetWorldState(WORLDSTATE_VANCLEEF_VINDICATION, 1);

                _JustEngagedWith();

                if (instance)
                {
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_VANESSA_ACHIEVEMENT);
                }

                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_BACKSLASH, urand(10000, 15000));
                // #681: Wowpedia "Vanessa VanCleef (tactics)" (MediaWiki API, 2026-10-14): "While Vanessa VanCleef's remaining health
                // is above 50%, she will periodically call on a Defias Enforcer" (then a Shadowguard and a Blood Wizard). The fork only
                // started calling them below 50%; same 1 s / 5 s / 60 s timers, now from the pull until 50%.
                events.ScheduleEvent(EVENT_ADDS_1, 1000);
                // #681: Deflection (92614, 10 s: +100% parry, ranged attacks miss, spells deflected), "only while her remaining health
                // is greater than 25%" (Wowpedia). CPP boss_vanessa_van_cleef.cpp:202 / :308-311: 11 s into the pull, again 0.4 s
                // after each re-engage (DamageTaken below).
                events.ScheduleEvent(EVENT_DEFLECTION, 11000);
                // #681: CPP :204 / :342-345 - 12.5 s in, six fires block the ship's entrance ramp (General Purpose Bunny JMF +
                // Fiery Blaze, CPP JustSummoned :283-285)
                events.ScheduleEvent(EVENT_ENTRANCE_FIRE, 12500);

                DoZoneInCombat();
            }

            // #826: a wipe sends the group back through the Nightmare (CPP :215-222 -> instance ResetVanessasNightmare); the
            // instance hides this DB spawn again (Reset reads DATA_VANESSA_EVENT) and returns the Note after 30 s
            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                if (instance && instance->GetBossState(DATA_VANESSA) != DONE)
                    instance->SetData(DATA_VANESSA_EVENT, FAIL);
                BossAI::EnterEvadeMode(why);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim && victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                instance->DoFinishLFGDungeon(326);
                TC_LOG_INFO("scripts", "[DM-VANESSA] dead (powder explosion %s)", _allowDeath ? "yes" : "no");
            }

            void JustSummoned(Creature* summon) override
            {
                // the fires and the ropes are scenery (no zone-in), the adds fight
                if (summon->GetEntry() == NPC_GENERAL_PURPOSE_DUMMY_JMF || summon->GetEntry() == NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY ||
                    summon->GetEntry() == NPC_ROPE)
                {
                    summons.Summon(summon);
                    summon->SetIsCombatDisallowed(true);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);   // Fiery Blaze is area-entry (any unit)
                    if (summon->GetEntry() == NPC_GENERAL_PURPOSE_DUMMY_JMF)
                        summon->CastSpell(summon, SPELL_FIERY_BLAZE, true);
                    return;
                }
                BossAI::JustSummoned(summon);
            }

            // #681 CPP :230-270: no death before the Powder Explosion; the 50% / 25% detonations; the 1% suicide; the re-engage
            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (damage >= me->GetHealth() && !_allowDeath)
                    damage = me->GetHealth() - 1;

                uint32 const after = me->GetHealth() > damage ? me->GetHealth() - damage : 0;
                float const pctAfter = me->GetMaxHealth() ? 100.0f * float(after) / float(me->GetMaxHealth()) : 0.0f;

                bool const midDetonation = me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) || _detonationCounter > 2;
                if (!midDetonation && ((pctAfter < 50.0f && _detonationCounter == 0) || (pctAfter < 25.0f && _detonationCounter == 1)))
                {
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveAurasDueToSpell(SPELL_DEADLY_BLADES);
                    events.Reset();
                    DoCast(me, SPELL_CLEAR_ALL_DEBUFFS, true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->GetMotionMaster()->MoveJump(vanessaDetonationJumpPos.GetPositionX(), vanessaDetonationJumpPos.GetPositionY(),
                        vanessaDetonationJumpPos.GetPositionZ(), std::max(1.0f, me->GetExactDist(vanessaDetonationJumpPos)), 27.96389f);
                    events.ScheduleEvent(EVENT_FACE_TO_DECK, 1200);
                    _detonationCounter++;
                    _prepareForReEngage = false;
                    TC_LOG_INFO("scripts", "[DM-VANESSA] detonation %u at %.0f%%", uint32(_detonationCounter), pctAfter);
                    return;
                }
                else if (!midDetonation && pctAfter <= 1.0f && _detonationCounter == 2)
                {
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveAurasDueToSpell(SPELL_DEADLY_BLADES);
                    events.Reset();
                    DoCast(me, SPELL_CLEAR_ALL_DEBUFFS, true);
                    events.ScheduleEvent(EVENT_TALK_SUICIDE_1, 200);
                    _detonationCounter++;
                    TC_LOG_INFO("scripts", "[DM-VANESSA] 1%%: the Powder Explosion countdown");
                    return;
                }

                if (_prepareForReEngage)
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    _prepareForReEngage = false;
                    if (_detonationCounter == 1)
                        events.CancelEvent(EVENT_VENGEANCE_OF_VANCLEEF);

                    events.ScheduleEvent(EVENT_DEFLECTION, 400);
                    events.ScheduleEvent(EVENT_BACKSLASH, 5200);
                    TC_LOG_INFO("scripts", "[DM-VANESSA] re-engaged after detonation %u", uint32(_detonationCounter));
                }
            }

            void SummonRopes()
            {
                summons.DespawnEntry(NPC_ROPE);   // CPP DATA_SUMMON_ROPES re-summons only the ropes that are gone: one set at a time
                std::list<Creature*> anchors;
                GetCreatureListWithEntryInGrid(anchors, me, NPC_VANESSAS_ROPE_ANCHOR_ENTRY, 200.0f);
                for (uint8 i = 0; i < 5; ++i)
                {
                    Creature* rope = me->SummonCreature(NPC_ROPE, RopePos[i], TEMPSUMMON_MANUAL_DESPAWN);
                    if (!rope)
                        continue;
                    rope->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);   // CPP instance_deadmines.cpp:271-274
                    // the anchor whose spawn is nearest in x (CPP pairs anchor i with rope i; RopeAnchorPos and RopePos run west to east)
                    Creature* anchor = nullptr;
                    float best = 1000.0f;
                    for (Creature* a : anchors)
                    {
                        float const dx = std::fabs(a->GetPositionX() - RopePos[i].GetPositionX());
                        if (dx < best)
                        {
                            best = dx;
                            anchor = a;
                        }
                    }
                    if (anchor)
                        anchor->CastSpell(rope, SPELL_ROPE_BEAM, true);
                }
                TC_LOG_INFO("scripts", "[DM-VANESSA] detonation %u: 5 ropes", uint32(_detonationCounter));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HealthBelowPct(50) && phase == 0)
                {
                    phase = 1;
                    // #681: no more calls below 50% (see JustEngagedWith)
                    events.CancelEvent(EVENT_ADDS_1);
                    events.CancelEvent(EVENT_ADDS_2);
                    events.CancelEvent(EVENT_ADDS_3);
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BACKSLASH:
                            DoCast(me, SPELL_DEADLY_BLADES);
                            events.ScheduleEvent(EVENT_CONTINUE, 6000);
                            break;
                        case EVENT_CONTINUE:
                            if (me->GetVictim())
                                me->GetMotionMaster()->MoveChase(me->GetVictim());
                            break;
                        case EVENT_DEFLECTION:
                            if (me->HealthAbovePct(25))
                                DoCast(me, SPELL_DEFLECTION);
                            break;
                        case EVENT_ADDS_1:
                            if (!_talkedAdds)
                            {
                                Talk(SAY_ADDS);   // CPP :319-323: once, with the first reinforcement
                                _talkedAdds = true;
                            }
                            me->SummonCreature(NPC_DEFIAS_ENFORCER, addsPos[0]);
                            events.ScheduleEvent(EVENT_ADDS_2, 5000);
                            break;
                        case EVENT_ADDS_2:
                            me->SummonCreature(NPC_DEFIAS_SHADOWGUARD, addsPos[1]);
                            events.ScheduleEvent(EVENT_ADDS_3, 5000);
                            break;
                        case EVENT_ADDS_3:
                            me->SummonCreature(NPC_DEFIAS_BLOOD_WIZARD, addsPos[2]);
                            events.ScheduleEvent(EVENT_ADDS_1, 60000);
                            break;
                        case EVENT_ENTRANCE_FIRE:
                            for (Position const& pos : VanessaEntranceFirePos)
                                me->SummonCreature(NPC_GENERAL_PURPOSE_DUMMY_JMF, pos, TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        // ---- #681 detonation (CPP :346-419) ----
                        case EVENT_FACE_TO_DECK:
                            me->SetFacingTo(2.932153f);
                            events.ScheduleEvent(EVENT_TALK_DETONATION, 200);
                            break;
                        case EVENT_TALK_DETONATION:
                            Talk(_detonationCounter == 1 ? SAY_DETONATION_1 : SAY_DETONATION_2);
                            events.ScheduleEvent(EVENT_ANNOUNCE_DETONATION, 3600);
                            break;
                        case EVENT_ANNOUNCE_DETONATION:
                            Talk(_detonationCounter == 1 ? SAY_ANNOUNCE_DETONATION_1 : SAY_ANNOUNCE_DETONATION_2);
                            events.ScheduleEvent(EVENT_SUMMON_DETONATION_FIRE, 600);
                            events.ScheduleEvent(EVENT_SUMMON_ROPES, 700);
                            break;
                        case EVENT_SUMMON_DETONATION_FIRE:
                            for (Position const& pos : VanessaNightmareFirePos)
                            {
                                Creature* fire = me->SummonCreature(NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY, pos, TEMPSUMMON_MANUAL_DESPAWN);
                                if (!fire)
                                    continue;
                                fire->RemoveAllAuras();   // the template addon's Nightmare Flames are not used here (CPP :372-373)
                                _nightmareFlames.insert(fire->GetGUID());
                                if (IsExtinguishableFire(pos))
                                    _extinguishableFlames.insert(fire->GetGUID());
                            }
                            events.ScheduleEvent(EVENT_CAST_FIERY_BLAZE, 4800);
                            events.ScheduleEvent(EVENT_EXTINGUISH_FIRES, 11000);
                            break;
                        case EVENT_SUMMON_ROPES:
                            SummonRopes();
                            break;
                        case EVENT_CAST_FIERY_BLAZE:
                        {
                            // the first set burns everywhere; the second only re-lights the deck (the rest is still burning)
                            GuidSet const& set = _detonationCounter == 1 ? _nightmareFlames : _extinguishableFlames;
                            for (ObjectGuid const& guid : set)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, guid))
                                    fire->CastSpell(fire, SPELL_FIERY_BLAZE, true);
                            TC_LOG_INFO("scripts", "[DM-VANESSA] detonation %u: Fiery Blaze on %u fires", uint32(_detonationCounter), uint32(set.size()));
                            break;
                        }
                        case EVENT_EXTINGUISH_FIRES:
                            for (ObjectGuid const& guid : _extinguishableFlames)
                                if (Creature* fire = ObjectAccessor::GetCreature(*me, guid))
                                    fire->RemoveAllAuras();
                            events.ScheduleEvent(EVENT_JUMP_ON_DECK, 3200);
                            break;
                        case EVENT_JUMP_ON_DECK:
                            me->GetMotionMaster()->MoveJump(vanessaAfterDetonationJumpPos.GetPositionX(), vanessaAfterDetonationJumpPos.GetPositionY(),
                                vanessaAfterDetonationJumpPos.GetPositionZ(), std::max(1.0f, me->GetExactDist(vanessaAfterDetonationJumpPos)), 32.75936f);
                            events.ScheduleEvent(EVENT_FACE_TO_ROPES, 1200);
                            break;
                        case EVENT_FACE_TO_ROPES:
                            me->SetFacingTo(4.642576f);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            _prepareForReEngage = true;
                            events.ScheduleEvent(EVENT_VENGEANCE_OF_VANCLEEF, 1200);
                            break;
                        case EVENT_VENGEANCE_OF_VANCLEEF:
                            DoCast(me, SPELL_VENGEANCE_OF_VANCLEEF, true);
                            events.ScheduleEvent(EVENT_VENGEANCE_OF_VANCLEEF, 2400);
                            break;
                        // ---- #681 Powder Explosion (CPP :420-443) ----
                        case EVENT_TALK_SUICIDE_1:
                            Talk(SAY_SUICIDE_1);
                            events.ScheduleEvent(EVENT_COSMETIC_BOMB_STATE, 4600);
                            break;
                        case EVENT_COSMETIC_BOMB_STATE:
                            DoCast(me, SPELL_COSMETIC_BOMB_STATE, true);
                            events.ScheduleEvent(EVENT_TALK_SUICIDE_2, 200);
                            break;
                        case EVENT_TALK_SUICIDE_2:
                            Talk(SAY_SUICIDE_2);
                            events.ScheduleEvent(EVENT_ANNOUNCE_POWDER, 2400);
                            break;
                        case EVENT_ANNOUNCE_POWDER:
                            Talk(SAY_ANNOUNCE_POWDER);
                            events.ScheduleEvent(EVENT_TALK_LAST_WORDS, 3500);
                            break;
                        case EVENT_TALK_LAST_WORDS:
                            Talk(SAY_DEATH);
                            events.ScheduleEvent(EVENT_POWDER_EXPLOSION, 1100);
                            break;
                        case EVENT_POWDER_EXPLOSION:
                            _allowDeath = true;
                            TC_LOG_INFO("scripts", "[DM-VANESSA] Powder Explosion");
                            DoCastAOE(SPELL_POWDER_EXPLOSION, true);
                            break;
                        default:
                            break;
                    }
                }

                if (!_prepareForReEngage && !me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                    DoMeleeAttackIfReady();
            }

        private:
            uint8 phase;
            uint8 _detonationCounter = 0;
            bool _prepareForReEngage = false;
            bool _allowDeath = false;
            bool _talkedAdds = false;
            GuidSet _nightmareFlames;
            GuidSet _extinguishableFlames;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_vanessa_vancleefAI>(creature);
        }
};

class npc_vanessa_vancleef_a_note_from_vanessa : public CreatureScript
{
    public:
        npc_vanessa_vancleef_a_note_from_vanessa() : CreatureScript("npc_vanessa_vancleef_a_note_from_vanessa") { }

        bool OnGossipHello(Player* /*player*/, Creature* creature) override
        {
            InstanceScript* instance = creature->GetInstanceScript();
            if (!instance)
                return true;

            if (instance->GetBossState(DATA_CAPTAIN) != DONE)
                return true;

            if (instance->GetData(DATA_VANESSA_EVENT) != NOT_STARTED)
                return true;

            creature->SummonCreature(NPC_VANESSA_SITTING, vanessaPos);
            instance->SetData(DATA_VANESSA_EVENT, IN_PROGRESS);

            creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            creature->DespawnOrUnsummon(1000);
            return true;
        }
};

class npc_vanessa_vancleef_vanessa_sitting : public CreatureScript
{
    public:
        npc_vanessa_vancleef_vanessa_sitting() : CreatureScript("npc_vanessa_vancleef_vanessa_sitting") { }

        struct npc_vanessa_vancleef_vanessa_sittingAI : public ScriptedAI
        {
            npc_vanessa_vancleef_vanessa_sittingAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->setActive(true);
                instance = me->GetInstanceScript();
            }

            void Reset() override
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                Talk(SAY_NOISE_FROM_ABOVE);   // #826: CPP boss_vanessa_van_cleef.cpp:489-491
                events.ScheduleEvent(EVENT_INTRO_1, 1000);
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO_1:
                            Talk(SAY_INTRO_1);
                            events.ScheduleEvent(EVENT_INTRO_2, 5000);
                            break;
                        case EVENT_INTRO_2:
                            Talk(SAY_INTRO_2);
                            events.ScheduleEvent(EVENT_INTRO_3, 8000);
                            break;
                        case EVENT_INTRO_3:
                            Talk(SAY_INTRO_3);
                            events.ScheduleEvent(EVENT_INTRO_4, 8000);
                            break;
                        case EVENT_INTRO_4:
                            Talk(SAY_INTRO_4);
                            events.ScheduleEvent(EVENT_INTRO_5, 5000);
                            break;
                        case EVENT_INTRO_5:
                            Talk(SAY_INTRO_5);
                            DoCastAOE(SPELL_NOXIOUS_CONCOCTION);
                            me->RemoveAura(SPELL_SITTING);
                            events.ScheduleEvent(EVENT_INTRO_6, 5000);
                            break;
                        case EVENT_INTRO_6:
                            me->GetMotionMaster()->MoveJump(centershipPos.GetPositionX(), centershipPos.GetPositionY(), centershipPos.GetPositionZ(), 5.f, 10.f);
                            // #826: CPP :535-542 - the Nightmare Elixir, then the trap
                            DoCastAOE(SPELL_NIGHTMARE_ELIXIR, true);
                            DoCastAOE(SPELL_VANESSAS_BLACKOUT, true);
                            Talk(SAY_ANNOUNCE_NIGHTMARE_ELIXIR);
                            events.ScheduleEvent(EVENT_INTRO_7, 5000);
                            break;
                        case EVENT_INTRO_7:
                            // #826: heroic Deadmines (the only difficulty with Vanessa) goes through Vanessa's Nightmare first; the
                            // instance shows the boss when James Harrington dies (instance_deadmines.cpp)
                            if (me->GetMap()->IsHeroic())
                                instance->SetData(DATA_VANESSA_EVENT, NIGHTMARE_STAGE_TRAP);
                            else
                            {
                                instance->SetData(DATA_VANESSA_EVENT, DONE);
                                if (Creature* pVanessa = me->FindNearestCreature(NPC_VANESSA_VANCLEEF, 40.0f))
                                    pVanessa->SetVisible(true);
                                instance->DoStartCriteria(CRITERIA_START_TYPE_EVENT, ACHIEVEMENT_EVENT);
                            }
                            me->DespawnOrUnsummon(1000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_vanessa_vancleef_vanessa_sittingAI>(creature);
        }
};

class npc_vanessa_vancleef_defias_enforcer : public CreatureScript
{
        public:
            npc_vanessa_vancleef_defias_enforcer() : CreatureScript("npc_vanessa_vancleef_defias_enforcer") { }

            struct npc_vanessa_vancleef_defias_enforcerAI : public ScriptedAI
            {
                npc_vanessa_vancleef_defias_enforcerAI(Creature* creature) : ScriptedAI(creature)
                {
                    me->setActive(true);
                }

                void Reset() override { }

                void UpdateAI(uint32 diff) override
                {
                    if (!UpdateVictim())
                        return;

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    DoMeleeAttackIfReady();
                }
            };

            CreatureAI* GetAI(Creature* creature) const override
            {
                return GetInstanceAI<npc_vanessa_vancleef_defias_enforcerAI>(creature);
            }
};

// #681: the Rope (49550, vehicle 1399, spellclick 46598 Ride Vehicle). CPP npc_vanessa_rope (boss_vanessa_van_cleef.cpp:623-697):
// 0.8 s after a player boards, it swings him off the ship and back (RopePath1-5, 22 yd/s), invisible, and despawns 8.9 s later -
// the passenger lands on the deck. Needs ScriptName 'npc_dm_vanessa_rope' (2026_10_23_dm_nightmare_gauntlet.sql).
struct npc_dm_vanessa_rope : public ScriptedAI
{
    npc_dm_vanessa_rope(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;
    bool swinging = false;

    void Reset() override { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!passenger || !apply || passenger->GetTypeId() != TYPEID_PLAYER || swinging)
            return;

        swinging = true;
        me->RemoveAurasDueToSpell(SPELL_CLICK_ME);
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        events.ScheduleEvent(EVENT_START_SWING, 800);
        TC_LOG_INFO("scripts", "[DM-VANESSA] rope %u boarded by %s", uint32(me->GetGUID().GetCounter()), passenger->GetName().c_str());
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);
        if (events.ExecuteEvent() != EVENT_START_SWING)
            return;

        Position const* paths[5] = { RopePath1, RopePath2, RopePath3, RopePath4, RopePath5 };
        for (uint8 i = 0; i < 5; ++i)
        {
            if (RopePos[i].GetExactDist(me) > 1.0f)
                continue;

            me->DespawnOrUnsummon(8900);
            me->SetSpeed(MOVE_RUN, 22.0f / 7.0f);   // 22 yd/s (CPP: "synch with sniff behaviour"); this core's SetSpeed takes a rate
            me->SetDisplayId(MODEL_ID_INVISIBLE);
            me->GetMotionMaster()->MoveSmoothPath(0, paths[i], 5, false, true);
            break;
        }
    }
};

// #826: the Steam Valve (49457). CPP npc_deadmines_steam_valve (deadmines.cpp:231-258): a click on a selectable valve counts for the
// instance (four free the trapped players). Only the valves the trap summons are selectable; the test DB's own spawns of 49457
// (spawnMask 6) keep their UNIT_FLAG_NOT_SELECTABLE. Needs ScriptName 'npc_dm_steam_valve' (2026_10_23_dm_nightmare_gauntlet.sql).
struct npc_dm_steam_valve : public ScriptedAI
{
    npc_dm_steam_valve(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override { }
    void UpdateAI(uint32 /*diff*/) override { }

    void OnSpellClick(Unit* clicker, bool& /*result*/) override
    {
        if (!me->IsSummon() || me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            return;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        TC_LOG_INFO("scripts", "[DM-NIGHTMARE] steam valve %u clicked by %s", uint32(me->GetGUID().GetCounter()), clicker ? clicker->GetName().c_str() : "?");
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_ACTIVATED_VALVE, 1);
    }
};

class spell_vanessa_vancleef_backslash_targeting : public SpellScriptLoader
{
    public:
        spell_vanessa_vancleef_backslash_targeting() : SpellScriptLoader("spell_vanessa_vancleef_backslash_targeting") { }

        class spell_vanessa_vancleef_backslash_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_vanessa_vancleef_backslash_targeting_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_BACKSLASH, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_vanessa_vancleef_backslash_targeting_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_vanessa_vancleef_backslash_targeting_SpellScript();
        }
};

void AddSC_boss_vanessa_vancleef()
{
    new boss_vanessa_vancleef();
    new npc_vanessa_vancleef_a_note_from_vanessa();
    new npc_vanessa_vancleef_vanessa_sitting();
    new spell_vanessa_vancleef_backslash_targeting();
    new creature_script<npc_dm_vanessa_rope>("npc_dm_vanessa_rope");   // #681
    new creature_script<npc_dm_steam_valve>("npc_dm_steam_valve");     // #826
}
