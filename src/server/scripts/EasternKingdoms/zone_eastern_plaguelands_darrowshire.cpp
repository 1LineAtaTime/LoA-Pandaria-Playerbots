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
SDName: Eastern Plaguelands, Darrowshire
SDComment: "Rewriting the Battle of Darrowshire" (quests 27383-27392)
SDCategory: Eastern Plaguelands
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "GameObjectAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "Player.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Chat.h"
#include "Group.h"

/*######
## Pamela's Doll (27384): one Ghost of the Past per doll part
######*/

enum DarrowshireDoll
{
    NPC_GHOST_OF_THE_PAST           = 10940,
};

// Ghost of the Past cap (ours): a living ghost within 4.5 yd of the trap (3D: two parts lie 4.9 yd apart on two floors of one house,
// 157555 upstairs and 157567 downstairs, and each keeps its own ghost), or a living ghost already fighting the player the trap fired for
static constexpr float DOLL_GHOST_CAP_RANGE     = 4.5f;
static constexpr float DOLL_GHOST_CHASE_RANGE   = 40.0f;

// 17132 - Pamela's Doll Trap (summons a Ghost of the Past 10940 at the trap 176117, 5 min)
// The trap fires on approach (radius 4, its own cooldown and despawn) and through the doll part on every opening of the part's loot
// window (GameObject::TriggeringLinkedGameObject, which applies neither the trap's cooldown nor its despawn), so a player who opens a
// part twice would get two ghosts. Sourced: one ghost per part (OLD comment 2004-12-20 reply [+41], OLD 2010-01-06).
class spell_darrowshire_pamelas_doll_trap : public SpellScript
{
    PrepareSpellScript(spell_darrowshire_pamelas_doll_trap);

    void HandleSummon(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();     // the trap's trigger, standing on the trap (GameObject::CastSpell)
        if (!caster)
            return;

        if (caster->FindNearestCreature(NPC_GHOST_OF_THE_PAST, DOLL_GHOST_CAP_RANGE, true))
        {
            PreventHitDefaultEffect(effIndex);
            return;
        }

        // the ghost of this player's first opening may have followed him a few yards off the part
        if (Unit* player = GetExplTargetUnit())
        {
            std::list<Creature*> ghosts;
            caster->GetCreatureListWithEntryInGrid(ghosts, NPC_GHOST_OF_THE_PAST, DOLL_GHOST_CHASE_RANGE);
            for (Creature* ghost : ghosts)
                if (ghost->IsAlive() && ghost->GetVictim() == player)
                {
                    PreventHitDefaultEffect(effIndex);
                    return;
                }
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_darrowshire_pamelas_doll_trap::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};

/*######
## The Battle of Darrowshire (27390)
######*/

// What the player sees, and where it comes from (dossiers\darrowshire.md, "Event brief (second pass)"; "classic" = the pre-4.0 script,
// research tmp\darrowshire\ext\vmangos_epl.cpp, used for positions and order only):
//  0. The Relic Bundle (item 15209 -> 18987, needs the town square focus 177528 within 20 yd) puts down a Relic Bundle 177526 for 60 s.
//     Its first update starts the battle unless one already runs within 100 yd (sourced: a second bundle does nothing while a battle
//     runs, comments 2010-11-24 reply, 2011-05-14 [+9]; the hotfix of 2010-11-30 lets the bundle start it again after a failure). The
//     battle lives on an invisible World Trigger at the well (ours: the bundle only lasts 60 s).
//  1. A Darrowshire Defender appears at the well: raid emote 45680 "Help Joseph Carlin's troops defend Darrowshire!", yell 7358 (wiki;
//     2010-12-12 [+8]; his MoP Classic pin at the well) and runs to the first defender point (classic).
//  2. Waves to the end: Marauding Corpses / Skeletons come from the north-east attacker points and march on the defender points,
//     Defenders appear at the defender points (OLD npc pages; comments 2010-11-24, 2011-06-21 [+135]). Event mobs attack anyone,
//     players without the quest too (OLD npc 10954; 2012-11-23 [+2]).
//  3. Davil Lightfire comes from the well and runs up the road (2011-06-21 [+135]): yell 7343, raid emote 45681 (wiki). Silver Hand
//     Disciples join (OLD npc page: "the second, under Davil"); Servants of Horgus come just before Horgus.
//  4. Horgus the Ravager appears beside Davil and attacks him: yell 7344, raid emote 45682; Davil answers 7346 (wiki; 2010-11-30 [+25];
//     2011-01-19). FAIL: Davil dies while Horgus lives -> raid emote 7366 "Davil Lightfire is defeated! Darrowshire is lost!".
//  5. Horgus dies: a Defender yells 7368; Davil says 7227, emote 7367 and dies shortly after (OLD npc 10944; 2011-01-19 [+7]); then
//     Captain Redpath comes from the well: yell 7355, raid emote 45683 (OLD npc 10937; wiki). Redpath Militia, Bloodletters (elite)
//     and Darrowshire Betrayers join (OLD npc pages; 2011-12-29 reply "2 of the elite ghoul things"). FAIL: Redpath killed by the
//     Scourge before Marduk -> a Defender yells 7369 "Captain Redpath is slain!".
//  6. Marduk the Black rides up (mount 10718 from the template addon), says 7471 and casts Corrupt Redpath 18666; Redpath is dazed and
//     dies on the spot; Marduk leaves (wiki; 2011-12-04, 2011-05-20 [+8] "stunned animation over his head and then he died", 2012-07-13).
//  7. Redpath the Corrupted stands up at Captain Redpath's corpse, at the battle: raid emote 45684, yell 7356 (wiki; 2011-06-21,
//     2011-01-08, 2012-11-23 [+2] (5.1); most of his MoP Classic pins on the battle line). He fights with his SmartAI.
//  8. His death ends it at once: a Defender yells 7407, raid emote 45685 "Speak with Joseph Redpath in the center of Darrowshire.", every
//     event creature despawns, also those fighting players (2011-12-04, 2011-12-29 reply).
//  9. Joseph Redpath's spirit at the well and Davil Crokford at his point (pins; wiki). Anyone with 27390 not complete who talks to
//     Joseph gets "Accept Redpath's Forgiveness", also from a Joseph someone else earned (2010-12-19 [+9] and replies to 5.3). A player
//     who has turned 27390 in and talks to him starts the reunion with Pamela (2010-12-18 [+12] and replies); else he goes by himself
//     after some minutes (reply "after 5+ minutes"; 2013-06-07 [+12]).
// Balance (ours, computed from this core's melee formula; table in the commit message and 2026_11_14_69): Horgus hits at 1.5 with
// 2.5 health, Redpath the Corrupted at 4, the Bloodletters at 2.5 (TDBs 7.5); waves are even on both sides, one Bloodletter at a
// time, and Horgus waits for a fit Davil. Davil kills Horgus alone in about 31 s and lasts about 69 s against him; a Disciple or a
// level-41 player brings it to 14-19 s. An idle player wins; failure stays for Davil or Captain Redpath dying to what attacks them.
// Ours (each one line on the test sheet): every timing below; the wave sizes and caps; the despawn 3 s after the end; the reset after
// 2 minutes with no player within 260 yd (all of Darrowshire) and at 20 minutes, with a system line to players around; Joseph's fallback at 5 minutes; Davil Crokford for 2 minutes; Pamela back
// at her spot 12 s after the reunion; the corpse of Captain Redpath taken away as the Corrupted stands up; the daze aura.

enum DarrowshireBattle
{
    QUEST_BATTLE_OF_DARROWSHIRE     = 27390,

    NPC_JOSEPH_REDPATH              = 10936,
    NPC_CAPTAIN_REDPATH             = 10937,
    NPC_REDPATH_THE_CORRUPTED       = 10938,
    NPC_MARDUK_THE_BLACK            = 10939,
    NPC_DAVIL_LIGHTFIRE             = 10944,
    NPC_DAVIL_CROKFORD              = 10945,
    NPC_HORGUS_THE_RAVAGER          = 10946,
    NPC_DARROWSHIRE_BETRAYER        = 10947,
    NPC_DARROWSHIRE_DEFENDER        = 10948,
    NPC_SILVER_HAND_DISCIPLE        = 10949,
    NPC_REDPATH_MILITIA             = 10950,
    NPC_MARAUDING_CORPSE            = 10951,
    NPC_MARAUDING_SKELETON          = 10952,
    NPC_SERVANT_OF_HORGUS           = 10953,
    NPC_BLOODLETTER                 = 10954,
    NPC_PAMELA_REDPATH              = 10926,

    SPELL_CORRUPT_REDPATH           = 18666,
    SPELL_DAZED_STUN                = 9179,     // "Stun", self, aura 12: the stunned animation (ours)

    // creature_text groups
    SAY_DEFENDER_TO_ARMS            = 0,        // 7358
    SAY_DEFENDER_HORGUS_SLAIN       = 1,        // 7368
    SAY_DEFENDER_REDPATH_SLAIN      = 2,        // 7369
    SAY_DEFENDER_VICTORY            = 3,        // 7407
    EMOTE_DEFENDER_SPEAK_WITH_JOSEPH= 4,        // 45685
    EMOTE_DEFENDER_HELP             = 70,       // 45680
    SAY_DAVIL_NO_HOPE_LOST          = 0,        // 7343
    SAY_DAVIL_CHALLENGE             = 1,        // 7346
    SAY_DAVIL_WOUNDS                = 2,        // 7227
    EMOTE_DAVIL_SUCCUMBED           = 3,        // 7367
    EMOTE_DAVIL_PROTECT             = 70,       // 45681
    EMOTE_DAVIL_DEFEATED            = 71,       // 7366
    SAY_HORGUS_APPEARS              = 0,        // 7344
    EMOTE_HORGUS_APPEARED           = 70,       // 45682
    SAY_HORGUS_TIME_OF_DEATH        = 71,       // 7345
    SAY_REDPATH_RALLY               = 0,        // 7355
    EMOTE_REDPATH_APPEARED          = 70,       // 45683
    SAY_MARDUK_LIFE_ENDS            = 0,        // 7471
    SAY_CORRUPTED_DOOMED            = 0,        // 7356
    EMOTE_CORRUPTED                 = 1,        // 45684
    SAY_MILITIA_RANDOM              = 0,        // 7347-7354
    SAY_JOSEPH_PAMELA               = 0,        // 7397
    SAY_JOSEPH_HOME                 = 1,        // 7403
    SAY_JOSEPH_LAUGH                = 70,       // 7398
    SAY_PAMELA_DADDY                = 0,        // 7399
    SAY_PAMELA_BACK                 = 1,        // 7400
    SAY_PAMELA_PLAY                 = 2,        // 7401
    SAY_PAMELA_MISSED               = 3,        // 7402

    DATA_DARROWSHIRE_BATTLE_RUNNING = 1,
    DATA_JOSEPH_REUNION             = 2,
};

enum DarrowshireBattleEvents
{
    EVENT_START = 1,
    EVENT_WAVE_ATTACKERS,
    EVENT_WAVE_DEFENDERS,
    EVENT_DAVIL,
    EVENT_WAVE_SERVANTS,
    EVENT_WAVE_DISCIPLES,
    EVENT_HORGUS,
    EVENT_DAVIL_CHALLENGE,
    EVENT_HORGUS_TIME_OF_DEATH,
    EVENT_DAVIL_WOUNDS,
    EVENT_DAVIL_DIES,
    EVENT_REDPATH,
    EVENT_WAVE_MILITIA,
    EVENT_WAVE_BLOODLETTERS,
    EVENT_WAVE_BETRAYERS,
    EVENT_MARDUK,
    EVENT_MARDUK_CAST,
    EVENT_REDPATH_DAZED,
    EVENT_REDPATH_DIES,
    EVENT_REMOVE_REDPATH_CORPSE,
    EVENT_PATROL,
    EVENT_JOSEPH,
    EVENT_CONTROLLER_GONE,
};

enum DarrowshireBattleStage
{
    STAGE_DEFENCE,          // start until Davil
    STAGE_DAVIL,            // Davil up, Horgus not yet
    STAGE_HORGUS,           // Horgus up
    STAGE_HORGUS_DEAD,      // Horgus dead, Davil dying, Redpath not yet
    STAGE_REDPATH,          // Captain Redpath up, Marduk not yet
    STAGE_MARDUK,           // Marduk came: Redpath's death is the corruption
    STAGE_CORRUPTED,        // Redpath the Corrupted up
    STAGE_OVER,             // won, failed or reset
};

// Classic positions (terrain re-read on the 5.4.8 maps by the dossier; the MoP Classic pins fall on the same line)
static Position const AttackerPoints[3] =
{
    { 1500.04f, -3662.67f, 82.83f, 3.708f },
    { 1506.17f, -3686.72f, 82.88f, 5.759f },
    { 1512.81f, -3724.64f, 87.12f, 1.642f },
};
static Position const BloodletterPoint  = { 1537.60f, -3677.10f, 88.70f, 3.149f };
static Position const DefenderPoints[3] =
{
    { 1484.68f, -3668.74f, 80.70f, 0.237f },
    { 1493.53f, -3695.01f, 80.13f, 0.264f },
    { 1505.28f, -3718.83f, 83.34f, 1.369f },
};
static Position const WellPoint         = { 1446.80f, -3694.27f, 76.60f, 0.402f };
static Position const CrokfordPoint     = { 1465.43f, -3678.48f, 78.08f, 0.040f };

// Ours: timings (classic in brackets) and limits
static constexpr uint32 BATTLE_MAX_MS           = 20 * MINUTE * IN_MILLISECONDS;    // whole battle, whatever happens
static constexpr uint32 BATTLE_NO_PLAYER_MS     = 2 * MINUTE * IN_MILLISECONDS;     // no player within range -> reset
static constexpr float  BATTLE_PLAYER_RANGE     = 260.0f;   // the farthest spawn of area 2262 Darrowshire lies 260 yd from the well;
                                                            // the entrance hill of the guides 136-159 yd
static constexpr float  BATTLE_RESET_NOTICE_RANGE = 400.0f;
static constexpr uint32 PLAYER_CHECK_MS         = 1000;
static constexpr uint32 DAVIL_DELAY_MS          = 60 * IN_MILLISECONDS;             // [120-180 s]
static constexpr uint32 HORGUS_DELAY_MS         = 45 * IN_MILLISECONDS;             // after Davil [60 s]
static constexpr uint32 DAVIL_WOUNDS_DELAY_MS   = 8 * IN_MILLISECONDS;              // after Horgus' death [3 s]
static constexpr uint32 REDPATH_DELAY_MS        = 10 * IN_MILLISECONDS;             // after Davil's death [10 s]
static constexpr uint32 MARDUK_DELAY_MIN_MS     = 120 * IN_MILLISECONDS;            // after Redpath [300-350 s]
static constexpr uint32 MARDUK_DELAY_MAX_MS     = 150 * IN_MILLISECONDS;
static constexpr uint32 MARDUK_LIFETIME_MS      = 12 * IN_MILLISECONDS;
static constexpr uint32 JOSEPH_DELAY_MS         = 5 * IN_MILLISECONDS;              // after the victory
static constexpr uint32 JOSEPH_LIFETIME_MS      = 8 * MINUTE * IN_MILLISECONDS;     // cap; the reunion starts at 5 min at the latest
static constexpr uint32 CROKFORD_LIFETIME_MS    = 2 * MINUTE * IN_MILLISECONDS;
static constexpr uint32 WAVE_LIFETIME_MS        = 2 * MINUTE * IN_MILLISECONDS;     // out of combat (classic)
static constexpr uint32 END_DESPAWN_MS          = 3 * IN_MILLISECONDS;
static constexpr uint32 MAX_ATTACKERS_ALIVE     = 8;
static constexpr uint32 MAX_DEFENDERS_ALIVE     = 8;
static constexpr uint32 HORGUS_DAVIL_MIN_PCT    = 70;                               // Horgus waits for Davil above this ...
static constexpr uint32 HORGUS_MAX_WAIT_MS      = 30 * IN_MILLISECONDS;             // ... at most this long
static constexpr float  MARDUK_RIDE_DISTANCE    = 25.0f;                            // he rides in from this far

enum DarrowshireMovePoints
{
    POINT_ADVANCE = 1,
};

static bool IsAttackerEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_MARAUDING_CORPSE:
        case NPC_MARAUDING_SKELETON:
        case NPC_SERVANT_OF_HORGUS:
        case NPC_BLOODLETTER:
        case NPC_DARROWSHIRE_BETRAYER:
            return true;
        default:
            return false;
    }
}

static bool IsDefenderEntry(uint32 entry)
{
    return entry == NPC_DARROWSHIRE_DEFENDER || entry == NPC_SILVER_HAND_DISCIPLE || entry == NPC_REDPATH_MILITIA;
}

// The battle's director: an invisible World Trigger 12999 summoned at the well by the Relic Bundle, with this AI installed
// (WorldObject::SummonTrigger). Every actor is its summon and tracked by guid; nothing is a static spawn.
struct npc_darrowshire_battle_controller : public ScriptedAI
{
    npc_darrowshire_battle_controller(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        // Active: the grid around the well keeps updating while the only player watches from the entrance hill, beyond the
        // continents' 90 yd visibility (Map::Update visits the cells around active objects)
        me->setActive(true, ActiveFlags::Script);
        _startMs = getMSTime();
        _lastPlayerMs = _startMs;
        _lastUpdateMs = _startMs;
        _events.ScheduleEvent(EVENT_START, 1000);
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == DATA_DARROWSHIRE_BATTLE_RUNNING)
            return _stage != STAGE_OVER ? 1 : 0;
        return 0;
    }

    void Reset() override { }
    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void JustSummoned(Creature* summon) override
    {
        _summons.push_back(summon->GetGUID());
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (_stage == STAGE_OVER)
            return;

        switch (summon->GetEntry())
        {
            case NPC_DAVIL_LIGHTFIRE:
                // Davil must live until Horgus is dead (objective; broadcast 7366). His death after that is the script's own.
                if (_stage < STAGE_HORGUS_DEAD)
                {
                    summon->AI()->Talk(EMOTE_DAVIL_DEFEATED);
                    EndBattle(false);
                }
                break;
            case NPC_HORGUS_THE_RAVAGER:
                if (_stage == STAGE_HORGUS)
                    HorgusDied();
                break;
            case NPC_CAPTAIN_REDPATH:
                // Killed by the Scourge before Marduk came: failed (objective; broadcast 7369). Once Marduk is here his death is the
                // corruption, however it comes (the comments' "spontaneous deaths", dossier 27390 row 24).
                if (_stage < STAGE_MARDUK)
                {
                    if (Creature* speaker = GetDefenderSpeaker())
                        speaker->AI()->Talk(SAY_DEFENDER_REDPATH_SLAIN);
                    EndBattle(false);
                }
                else if (_stage == STAGE_MARDUK)
                    Corrupt(summon);
                break;
            case NPC_REDPATH_THE_CORRUPTED:
                if (_stage == STAGE_CORRUPTED)
                    Victory();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        uint32 now = getMSTime();

        // The creature only updates while a player is near its grid. A gap means nobody was here: the battle ends.
        bool frozen = getMSTimeDiff(_lastUpdateMs, now) > BATTLE_NO_PLAYER_MS;
        _lastUpdateMs = now;

        if (_stage == STAGE_OVER)
        {
            // only the victory's Joseph and the controller's own despawn are left
            _events.Update(diff);
            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_JOSEPH)
                    SummonJosephAndCrokford();
                else if (eventId == EVENT_CONTROLLER_GONE)
                    me->DespawnOrUnsummon();
            }
            return;
        }

        if (frozen || getMSTimeDiff(_startMs, now) > BATTLE_MAX_MS)
        {
            ResetBattle();
            return;
        }

        if (_playerCheckMs <= diff)
        {
            _playerCheckMs = PLAYER_CHECK_MS;
            _playerNear = IsPlayerNear();
        }
        else
            _playerCheckMs -= diff;
        bool playerNear = _playerNear;
        if (playerNear)
            _lastPlayerMs = now;
        else if (getMSTimeDiff(_lastPlayerMs, now) > BATTLE_NO_PLAYER_MS)
        {
            ResetBattle();
            return;
        }

        // Nothing spawns and no stage advances while no player is near
        if (!playerNear)
            return;

        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START:
                    if (Creature* defender = me->SummonCreature(NPC_DARROWSHIRE_DEFENDER, WellPoint, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, Milliseconds(WAVE_LIFETIME_MS)))
                    {
                        defender->AI()->Talk(EMOTE_DEFENDER_HELP);
                        defender->AI()->Talk(SAY_DEFENDER_TO_ARMS);
                        defender->SetWalk(false);
                        defender->SetHomePosition(DefenderPoints[0]);
                        defender->GetMotionMaster()->MovePoint(POINT_ADVANCE, DefenderPoints[0]);
                    }
                    _events.ScheduleEvent(EVENT_WAVE_ATTACKERS, 15000);
                    _events.ScheduleEvent(EVENT_WAVE_DEFENDERS, 17000);
                    _events.ScheduleEvent(EVENT_DAVIL, DAVIL_DELAY_MS);
                    _events.ScheduleEvent(EVENT_PATROL, 5000);
                    break;
                case EVENT_WAVE_ATTACKERS:
                    // one per attacker point, as many as the defenders get: the line holds and the heroes tip it (balance, ours)
                    for (Position const& point : AttackerPoints)
                        SummonAttacker(urand(0, 1) ? NPC_MARAUDING_CORPSE : NPC_MARAUDING_SKELETON, point);
                    _events.RepeatEvent(30000);
                    break;
                case EVENT_WAVE_DEFENDERS:
                    for (Position const& point : DefenderPoints)
                        SummonDefender(NPC_DARROWSHIRE_DEFENDER, point);
                    _events.RepeatEvent(30000);
                    break;
                case EVENT_DAVIL:
                    if (Creature* davil = SummonHero(NPC_DAVIL_LIGHTFIRE))
                    {
                        _davil = davil->GetGUID();
                        davil->AI()->Talk(SAY_DAVIL_NO_HOPE_LOST);
                        davil->AI()->Talk(EMOTE_DAVIL_PROTECT);
                        _stage = STAGE_DAVIL;
                        _events.ScheduleEvent(EVENT_WAVE_SERVANTS, 4000);
                        _events.ScheduleEvent(EVENT_WAVE_DISCIPLES, 6000);
                        _events.ScheduleEvent(EVENT_HORGUS, HORGUS_DELAY_MS);
                    }
                    else
                        ResetBattle();
                    break;
                case EVENT_WAVE_SERVANTS:
                    if (_stage == STAGE_DAVIL || _stage == STAGE_HORGUS)
                    {
                        for (Position const& point : AttackerPoints)
                            SummonAttacker(NPC_SERVANT_OF_HORGUS, point);
                        _events.RepeatEvent(35000);
                    }
                    break;
                case EVENT_WAVE_DISCIPLES:
                    SummonDefender(NPC_SILVER_HAND_DISCIPLE, DefenderPoints[urand(0, 2)]);
                    _events.RepeatEvent(45000);
                    break;
                case EVENT_HORGUS:
                {
                    Creature* davil = GetCreature(_davil);
                    if (!davil || !davil->IsAlive())
                        break;
                    // Horgus' fight is the test of the battle; it starts with Davil fit for it (out of combat he regenerates and the
                    // Disciples heal him): wait while he is below 70 %, at most 30 s (ours)
                    if (davil->GetHealthPct() < float(HORGUS_DAVIL_MIN_PCT) && _horgusWaitMs < HORGUS_MAX_WAIT_MS)
                    {
                        _horgusWaitMs += 5000;
                        _events.ScheduleEvent(EVENT_HORGUS, 5000);
                        break;
                    }
                    float x, y, z;
                    me->GetRandomPoint(*davil, 6.0f, x, y, z);
                    if (Creature* horgus = me->SummonCreature(NPC_HORGUS_THE_RAVAGER, x, y, z, davil->GetAngle(x, y) + float(M_PI), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(BATTLE_MAX_MS)))
                    {
                        _horgus = horgus->GetGUID();
                        _stage = STAGE_HORGUS;
                        horgus->AI()->Talk(SAY_HORGUS_APPEARS);
                        horgus->AI()->Talk(EMOTE_HORGUS_APPEARED);
                        // In combat from the first tick: his Unholy Aura row (SmartAI 10946 id 0, out of combat, 1 s after spawn) then
                        // never fires in the duel, only after an evade (SmartScript: OOC timers do not run in combat). The balance
                        // table assumes that.
                        horgus->SetInCombatWith(davil);
                        davil->SetInCombatWith(horgus);
                        horgus->GetThreatManager().AddThreat(davil, 1.0f);
                        horgus->AI()->AttackStart(davil);
                        _events.ScheduleEvent(EVENT_DAVIL_CHALLENGE, 3000);
                        _events.ScheduleEvent(EVENT_HORGUS_TIME_OF_DEATH, 15000);
                    }
                    else
                        ResetBattle();
                    break;
                }
                case EVENT_DAVIL_CHALLENGE:
                    if (Creature* davil = GetCreature(_davil))
                        if (davil->IsAlive())
                        {
                            davil->AI()->Talk(SAY_DAVIL_CHALLENGE);
                            if (Creature* horgus = GetCreature(_horgus))
                                if (horgus->IsAlive() && !davil->IsInCombatWith(horgus))
                                    davil->AI()->AttackStart(horgus);
                        }
                    break;
                case EVENT_HORGUS_TIME_OF_DEATH:
                    if (Creature* horgus = GetCreature(_horgus))
                        if (horgus->IsAlive())
                            horgus->AI()->Talk(SAY_HORGUS_TIME_OF_DEATH);
                    break;
                case EVENT_DAVIL_WOUNDS:
                    if (Creature* davil = GetCreature(_davil))
                        if (davil->IsAlive())
                        {
                            davil->AI()->Talk(SAY_DAVIL_WOUNDS);
                            davil->AttackStop();
                            davil->SetReactState(REACT_PASSIVE);
                            davil->StopMoving();
                        }
                    _events.ScheduleEvent(EVENT_DAVIL_DIES, 3000);
                    break;
                case EVENT_DAVIL_DIES:
                    if (Creature* davil = GetCreature(_davil))
                        if (davil->IsAlive())
                        {
                            davil->AI()->Talk(EMOTE_DAVIL_SUCCUMBED);
                            davil->Kill(davil);
                        }
                    _events.ScheduleEvent(EVENT_REDPATH, REDPATH_DELAY_MS);
                    break;
                case EVENT_REDPATH:
                    if (Creature* redpath = SummonHero(NPC_CAPTAIN_REDPATH))
                    {
                        _redpath = redpath->GetGUID();
                        _stage = STAGE_REDPATH;
                        redpath->AI()->Talk(SAY_REDPATH_RALLY);
                        redpath->AI()->Talk(EMOTE_REDPATH_APPEARED);
                        _events.ScheduleEvent(EVENT_WAVE_BLOODLETTERS, 4000);
                        _events.ScheduleEvent(EVENT_WAVE_MILITIA, 6000);
                        _events.ScheduleEvent(EVENT_WAVE_BETRAYERS, 20000);
                        _events.ScheduleEvent(EVENT_MARDUK, urand(MARDUK_DELAY_MIN_MS, MARDUK_DELAY_MAX_MS));
                    }
                    else
                        ResetBattle();
                    break;
                case EVENT_WAVE_MILITIA:
                {
                    bool yelled = false;
                    for (Position const& point : DefenderPoints)
                        if (Creature* militia = SummonDefender(NPC_REDPATH_MILITIA, point))
                            if (!yelled)
                            {
                                militia->AI()->Talk(SAY_MILITIA_RANDOM);
                                yelled = true;
                            }
                    _events.RepeatEvent(45000);
                    break;
                }
                case EVENT_WAVE_BLOODLETTERS:
                    SummonAttacker(NPC_BLOODLETTER, BloodletterPoint);
                    _events.RepeatEvent(60000);
                    break;
                case EVENT_WAVE_BETRAYERS:
                    SummonAttacker(NPC_DARROWSHIRE_BETRAYER, AttackerPoints[urand(0, 2)]);
                    _events.RepeatEvent(40000);
                    break;
                case EVENT_MARDUK:
                {
                    Creature* redpath = GetCreature(_redpath);
                    if (!redpath || !redpath->IsAlive())
                        break;
                    // he rides up on his horse (2011-12-04): appears 25 yd off and gallops to Redpath (distance ours)
                    float angle = frand(0.0f, 2.0f * float(M_PI));
                    float x = redpath->GetPositionX() + MARDUK_RIDE_DISTANCE * std::cos(angle);
                    float y = redpath->GetPositionY() + MARDUK_RIDE_DISTANCE * std::sin(angle);
                    float z = redpath->GetPositionZ();
                    redpath->UpdateGroundPositionZ(x, y, z);
                    if (Creature* marduk = me->SummonCreature(NPC_MARDUK_THE_BLACK, x, y, z, angle + float(M_PI), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(MARDUK_LIFETIME_MS)))
                    {
                        _marduk = marduk->GetGUID();
                        _stage = STAGE_MARDUK;
                        marduk->SetReactState(REACT_PASSIVE);
                        marduk->SetWalk(false);
                        float cx, cy, cz;
                        redpath->GetContactPoint(marduk, cx, cy, cz, 4.0f);
                        marduk->GetMotionMaster()->MovePoint(POINT_ADVANCE, cx, cy, cz);
                        marduk->AI()->Talk(SAY_MARDUK_LIFE_ENDS);
                        _events.ScheduleEvent(EVENT_MARDUK_CAST, 5000);
                        _events.ScheduleEvent(EVENT_REDPATH_DAZED, 7000);
                        _events.ScheduleEvent(EVENT_REDPATH_DIES, 9000);
                    }
                    else
                        ResetBattle();
                    break;
                }
                case EVENT_MARDUK_CAST:
                    if (Creature* marduk = GetCreature(_marduk))
                        if (Creature* redpath = GetCreature(_redpath))
                            if (redpath->IsAlive())
                            {
                                marduk->SetFacingToObject(redpath);
                                marduk->CastSpell(redpath, SPELL_CORRUPT_REDPATH, false);
                            }
                    break;
                case EVENT_REDPATH_DAZED:
                    if (Creature* redpath = GetCreature(_redpath))
                        if (redpath->IsAlive())
                        {
                            redpath->AttackStop();
                            redpath->SetReactState(REACT_PASSIVE);
                            redpath->StopMoving();
                            if (Aura* daze = redpath->AddAura(SPELL_DAZED_STUN, redpath))
                            {
                                daze->SetMaxDuration(3000);
                                daze->SetDuration(3000);
                            }
                        }
                    break;
                case EVENT_REDPATH_DIES:
                    if (Creature* redpath = GetCreature(_redpath))
                        if (redpath->IsAlive())
                            redpath->Kill(redpath);     // -> SummonedCreatureDies -> Corrupt
                    break;
                case EVENT_REMOVE_REDPATH_CORPSE:
                    if (Creature* redpath = GetCreature(_redpath))
                        redpath->DespawnOrUnsummon();
                    break;
                case EVENT_PATROL:
                    Patrol();
                    _events.RepeatEvent(5000);
                    break;
                default:
                    break;
            }
        }
    }

private:
    Creature* GetCreature(ObjectGuid guid) const
    {
        return guid ? me->GetMap()->GetCreature(guid) : nullptr;
    }

    bool IsPlayerNear() const
    {
        // dead players count too: one who died in the battle and has not released is still here
        std::list<Player*> players;
        Trinity::AnyPlayerInObjectRangeCheck check(me, BATTLE_PLAYER_RANGE, false);
        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
        me->VisitNearbyWorldObject(BATTLE_PLAYER_RANGE, searcher);
        for (Player* player : players)
            if (!player->IsGameMaster() && !player->GetSession()->IsBot())   // playerbots do not keep a battle going
                return true;
        return false;
    }

    uint32 CountAlive(uint32 entry) const
    {
        uint32 count = 0;
        for (ObjectGuid const& guid : _summons)
            if (Creature* creature = GetCreature(guid))
                if (creature->IsAlive() && (entry ? creature->GetEntry() == entry : true))
                    ++count;
        return count;
    }

    uint32 CountAliveSide(bool attackers) const
    {
        uint32 count = 0;
        for (ObjectGuid const& guid : _summons)
            if (Creature* creature = GetCreature(guid))
                if (creature->IsAlive() && (attackers ? IsAttackerEntry(creature->GetEntry()) : IsDefenderEntry(creature->GetEntry())))
                    ++count;
        return count;
    }

    Creature* SummonAttacker(uint32 entry, Position const& point)
    {
        if (CountAliveSide(true) >= MAX_ATTACKERS_ALIVE)
            return nullptr;
        if ((entry == NPC_BLOODLETTER && CountAlive(NPC_BLOODLETTER) >= 1) ||
            (entry == NPC_DARROWSHIRE_BETRAYER && CountAlive(NPC_DARROWSHIRE_BETRAYER) >= 2) ||
            (entry == NPC_SERVANT_OF_HORGUS && CountAlive(NPC_SERVANT_OF_HORGUS) >= 3))
            return nullptr;

        float x, y, z;
        me->GetRandomPoint(point, 5.0f, x, y, z);
        Creature* attacker = me->SummonCreature(entry, x, y, z, point.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, Milliseconds(WAVE_LIFETIME_MS));
        if (!attacker)
            return nullptr;

        // They march on the town and come back to it after an evade (classic: the Bloodletters walk, the rest move at random)
        Position const& target = DefenderPoints[urand(0, 2)];
        attacker->SetWalk(entry == NPC_BLOODLETTER);
        attacker->SetHomePosition(target);
        attacker->GetMotionMaster()->MovePoint(POINT_ADVANCE, target);
        return attacker;
    }

    Creature* SummonDefender(uint32 entry, Position const& point)
    {
        if (CountAliveSide(false) >= MAX_DEFENDERS_ALIVE)
            return nullptr;
        if (entry == NPC_SILVER_HAND_DISCIPLE && CountAlive(NPC_SILVER_HAND_DISCIPLE) >= 2)
            return nullptr;

        float x, y, z;
        me->GetRandomPoint(point, 5.0f, x, y, z);
        Creature* defender = me->SummonCreature(entry, x, y, z, point.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, Milliseconds(WAVE_LIFETIME_MS));
        if (!defender)
            return nullptr;

        defender->SetHomePosition(point);
        defender->GetMotionMaster()->MoveRandom(6.0f);
        return defender;
    }

    // Davil and Captain Redpath come from the well and run up the road (2011-06-21 [+135])
    Creature* SummonHero(uint32 entry)
    {
        Creature* hero = me->SummonCreature(entry, WellPoint, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(BATTLE_MAX_MS));
        if (!hero)
            return nullptr;

        hero->SetWalk(false);
        hero->SetHomePosition(DefenderPoints[0]);
        hero->GetMotionMaster()->MovePoint(POINT_ADVANCE, DefenderPoints[0]);
        return hero;
    }

    // Every 5 s: an idle actor out of combat takes up its role again (after an arrival or an evade). The heroes and the Bloodletters
    // roam between the defender points and the well ("he runs off a lot", 2011-02-20; classic), the attackers push on to a defender
    // point, the defenders wander at their point.
    void Patrol()
    {
        for (ObjectGuid const& guid : _summons)
        {
            Creature* creature = GetCreature(guid);
            if (!creature || !creature->IsAlive() || creature->IsInCombat())
                continue;
            if (creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
                continue;

            uint32 entry = creature->GetEntry();
            if (entry == NPC_DAVIL_LIGHTFIRE || entry == NPC_CAPTAIN_REDPATH || entry == NPC_BLOODLETTER)
            {
                if (creature->GetReactState() == REACT_PASSIVE)
                    continue;           // dying or dazed
                uint32 pick = urand(0, 3);
                Position const& target = pick < 3 ? DefenderPoints[pick] : WellPoint;
                creature->SetHomePosition(target);
                creature->GetMotionMaster()->MovePoint(POINT_ADVANCE, target);
            }
            else if (IsAttackerEntry(entry))
            {
                Position const& target = DefenderPoints[urand(0, 2)];
                creature->SetHomePosition(target);
                creature->GetMotionMaster()->MovePoint(POINT_ADVANCE, target);
            }
            else if (IsDefenderEntry(entry))
                creature->GetMotionMaster()->MoveRandom(6.0f);
        }
    }

    // A living Defender says the battle's news; failing that any of them (a corpse still speaks), failing that one comes to the well
    Creature* GetDefenderSpeaker()
    {
        Creature* fallback = nullptr;
        for (ObjectGuid const& guid : _summons)
            if (Creature* creature = GetCreature(guid))
                if (creature->GetEntry() == NPC_DARROWSHIRE_DEFENDER)
                {
                    if (creature->IsAlive())
                        return creature;
                    fallback = creature;
                }
        if (fallback)
            return fallback;
        return me->SummonCreature(NPC_DARROWSHIRE_DEFENDER, WellPoint, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(10 * IN_MILLISECONDS));
    }

    void HorgusDied()
    {
        _stage = STAGE_HORGUS_DEAD;
        _events.CancelEvent(EVENT_HORGUS_TIME_OF_DEATH);
        if (Creature* speaker = GetDefenderSpeaker())
            speaker->AI()->Talk(SAY_DEFENDER_HORGUS_SLAIN);
        _events.ScheduleEvent(EVENT_DAVIL_WOUNDS, DAVIL_WOUNDS_DELAY_MS);
    }

    void Corrupt(Creature* redpath)
    {
        _stage = STAGE_CORRUPTED;
        _events.CancelEvent(EVENT_MARDUK_CAST);
        _events.CancelEvent(EVENT_REDPATH_DAZED);
        _events.CancelEvent(EVENT_REDPATH_DIES);

        if (Creature* corrupted = me->SummonCreature(NPC_REDPATH_THE_CORRUPTED, *redpath, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(BATTLE_MAX_MS)))
        {
            _corrupted = corrupted->GetGUID();
            corrupted->AI()->Talk(EMOTE_CORRUPTED);
            corrupted->AI()->Talk(SAY_CORRUPTED_DOOMED);
        }
        else
        {
            ResetBattle();
            return;
        }
        _events.ScheduleEvent(EVENT_REMOVE_REDPATH_CORPSE, 2000);
    }

    void Victory()
    {
        if (Creature* speaker = GetDefenderSpeaker())
        {
            speaker->AI()->Talk(SAY_DEFENDER_VICTORY);
            speaker->AI()->Talk(EMOTE_DEFENDER_SPEAK_WITH_JOSEPH);
        }
        EndBattle(true);
    }

    // Victory or failure: every actor goes within 3 s (also those fighting players), the controller after Joseph
    void EndBattle(bool victory)
    {
        _stage = STAGE_OVER;
        _events.Reset();
        DespawnActors(END_DESPAWN_MS);
        if (victory)
        {
            _events.ScheduleEvent(EVENT_JOSEPH, JOSEPH_DELAY_MS);
            _events.ScheduleEvent(EVENT_CONTROLLER_GONE, JOSEPH_DELAY_MS + 1000);
        }
        else
            _events.ScheduleEvent(EVENT_CONTROLLER_GONE, END_DESPAWN_MS + 1000);
    }

    // No player near for 2 minutes, the 20 minute cap, or an actor that could not be summoned: everything goes, and players
    // around are told so (ours: no source has a line for it)
    void ResetBattle()
    {
        std::list<Player*> players;
        Trinity::AnyPlayerInObjectRangeCheck check(me, BATTLE_RESET_NOTICE_RANGE, false);
        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
        me->VisitNearbyWorldObject(BATTLE_RESET_NOTICE_RANGE, searcher);
        for (Player* player : players)
            if (!player->GetSession()->IsBot())
                ChatHandler(player->GetSession()).SendSysMessage("The Battle of Darrowshire fades away. Place the Relic Bundle in the town square to begin it again.");

        _stage = STAGE_OVER;
        _events.Reset();
        DespawnActors(0);
        me->DespawnOrUnsummon();
    }

    void DespawnActors(uint32 delayMs)
    {
        for (ObjectGuid const& guid : _summons)
            if (Creature* creature = GetCreature(guid))
            {
                if (delayMs && creature->IsAlive())
                {
                    creature->AttackStop();
                    creature->SetReactState(REACT_PASSIVE);
                }
                creature->DespawnOrUnsummon(delayMs);
            }
        _summons.clear();
    }

    void SummonJosephAndCrokford()
    {
        // A Joseph of an earlier victory who is still waiting at the well gives way to the new one; one on his way to Pamela stays
        std::list<Creature*> josephs;
        me->GetCreatureListWithEntryInGrid(josephs, NPC_JOSEPH_REDPATH, 150.0f);
        for (Creature* joseph : josephs)
            if (joseph->IsSummon() && joseph->IsAIEnabled && !joseph->AI()->GetData(DATA_JOSEPH_REUNION))
                joseph->DespawnOrUnsummon();

        std::list<Creature*> crokfords;
        me->GetCreatureListWithEntryInGrid(crokfords, NPC_DAVIL_CROKFORD, 150.0f);
        for (Creature* crokford : crokfords)
            if (crokford->IsSummon())
                crokford->DespawnOrUnsummon();

        me->SummonCreature(NPC_JOSEPH_REDPATH, WellPoint, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(JOSEPH_LIFETIME_MS));
        me->SummonCreature(NPC_DAVIL_CROKFORD, CrokfordPoint, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(CROKFORD_LIFETIME_MS));
        _summons.clear();       // they outlive the controller on their own timers
    }

    EventMap _events;
    std::vector<ObjectGuid> _summons;
    ObjectGuid _davil;
    ObjectGuid _horgus;
    ObjectGuid _redpath;
    ObjectGuid _marduk;
    ObjectGuid _corrupted;
    DarrowshireBattleStage _stage = STAGE_DEFENCE;
    uint32 _startMs = 0;
    uint32 _lastPlayerMs = 0;
    uint32 _lastUpdateMs = 0;
    uint32 _horgusWaitMs = 0;
    uint32 _playerCheckMs = 0;
    bool _playerNear = false;
};

static CreatureAI* GetDarrowshireBattleAI(Creature* creature)
{
    return new npc_darrowshire_battle_controller(creature);
}

// 177526 - Relic Bundle (put down by 18987, 60 s): starts the battle unless one runs within 100 yd
struct go_darrowshire_relic_bundle : public GameObjectAI
{
    go_darrowshire_relic_bundle(GameObject* go) : GameObjectAI(go) { }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (_checked)
            return;
        _checked = true;

        std::list<Creature*> triggers;
        me->GetCreatureListWithEntryInGrid(triggers, WORLD_TRIGGER, 100.0f);
        for (Creature* trigger : triggers)
            if (trigger->IsAlive() && trigger->IsAIEnabled && trigger->AI()->GetData(DATA_DARROWSHIRE_BATTLE_RUNNING))
                return;

        // The controller's own checks end the battle by 20 minutes; its summon timer is the backstop
        me->SummonTrigger(WellPoint.GetPositionX(), WellPoint.GetPositionY(), WellPoint.GetPositionZ(), WellPoint.GetOrientation(),
            BATTLE_MAX_MS + MINUTE * IN_MILLISECONDS, GetDarrowshireBattleAI);
    }

private:
    bool _checked = false;
};

/*######
## Joseph Redpath 10936: forgiveness (27390's credit) and the reunion with Pamela
######*/

enum JosephRedpathEvents
{
    EVENT_REUNION_FALLBACK = 1,
    EVENT_PAMELA_CALLS,
    EVENT_PAMELA_PLAY,
    EVENT_JOSEPH_LAUGH,
    EVENT_PAMELA_MISSED,
    EVENT_PAMELA_VANISHES,
    EVENT_JOSEPH_HOME,
    EVENT_REUNION_TIMEOUT,
};

enum JosephRedpathPoints
{
    POINT_JOSEPH_1 = 1,
    POINT_JOSEPH_2,
    POINT_JOSEPH_3,
    POINT_JOSEPH_PAMELA,
};

// Classic walk from the square to Pamela's house; Pamela steps out to meet him
static Position const JosephPath[3] =
{
    { 1431.50f, -3684.23f, 75.73f, 0.0f },
    { 1434.22f, -3668.76f, 76.67f, 0.0f },
    { 1438.53f, -3632.73f, 78.27f, 0.0f },
};
static Position const PamelaStep        = { 1450.73f, -3599.97f, 85.62f, 0.0f };
static constexpr uint32 REUNION_FALLBACK_MS = 5 * MINUTE * IN_MILLISECONDS;
static constexpr uint32 PAMELA_RESPAWN_S    = 12;
static constexpr float  JOSEPH_GROUP_CREDIT_RANGE = 100.0f;

struct npc_darrowshire_joseph_redpath : public ScriptedAI
{
    npc_darrowshire_joseph_redpath(Creature* creature) : ScriptedAI(creature)
    {
        _events.ScheduleEvent(EVENT_REUNION_FALLBACK, REUNION_FALLBACK_MS);
    }

    void Reset() override { }

    uint32 GetData(uint32 type) const override
    {
        return type == DATA_JOSEPH_REUNION && _reunion ? 1 : 0;
    }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_BATTLE_OF_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
        {
            player->KilledMonsterCredit(NPC_JOSEPH_REDPATH, me->GetGUID());
            // Playerbots open a gossip only when the menu has options (mod_playerbots GossipHelloAction) and 3861 has none:
            // the player's group members nearby who still need it take the forgiveness with him (ours)
            if (Group* group = player->GetGroup())
                for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
                    if (Player* member = itr->GetSource())
                        if (member != player && member->IsInMap(me) && member->IsWithinDistInMap(me, JOSEPH_GROUP_CREDIT_RANGE) &&
                            member->GetQuestStatus(QUEST_BATTLE_OF_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
                            member->KilledMonsterCredit(NPC_JOSEPH_REDPATH, me->GetGUID());
        }
        else if (player->IsQuestRewarded(QUEST_BATTLE_OF_DARROWSHIRE))
            StartReunion();
        return false;   // the menu (3861: text 4778) shows as always
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_JOSEPH_1:
                me->GetMotionMaster()->MovePoint(POINT_JOSEPH_2, JosephPath[1]);
                break;
            case POINT_JOSEPH_2:
                Talk(SAY_JOSEPH_PAMELA);
                me->GetMotionMaster()->MovePoint(POINT_JOSEPH_3, JosephPath[2]);
                _events.ScheduleEvent(EVENT_PAMELA_CALLS, 3000);
                break;
            case POINT_JOSEPH_3:
                if (Creature* pamela = GetPamela())
                {
                    pamela->AI()->Talk(SAY_PAMELA_BACK);
                    float x, y, z;
                    pamela->GetContactPoint(me, x, y, z, 1.0f);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(POINT_JOSEPH_PAMELA, x, y, z);
                }
                else
                    _events.ScheduleEvent(EVENT_JOSEPH_HOME, 1000);
                break;
            case POINT_JOSEPH_PAMELA:
                if (Creature* pamela = GetPamela())
                {
                    me->SetFacingToObject(pamela);
                    pamela->SetFacingToObject(me);
                }
                _events.ScheduleEvent(EVENT_PAMELA_PLAY, 2000);
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
                case EVENT_REUNION_FALLBACK:
                    StartReunion();
                    break;
                case EVENT_PAMELA_CALLS:
                    if (Creature* pamela = GetPamela())
                    {
                        pamela->AI()->Talk(SAY_PAMELA_DADDY);
                        pamela->GetMotionMaster()->MovePoint(0, PamelaStep);
                    }
                    break;
                case EVENT_PAMELA_PLAY:
                    if (Creature* pamela = GetPamela())
                        pamela->AI()->Talk(SAY_PAMELA_PLAY);
                    _events.ScheduleEvent(EVENT_JOSEPH_LAUGH, 5000);
                    break;
                case EVENT_JOSEPH_LAUGH:
                    Talk(SAY_JOSEPH_LAUGH);
                    _events.ScheduleEvent(EVENT_PAMELA_MISSED, 3000);
                    break;
                case EVENT_PAMELA_MISSED:
                    if (Creature* pamela = GetPamela())
                        pamela->AI()->Talk(SAY_PAMELA_MISSED);
                    _events.ScheduleEvent(EVENT_PAMELA_VANISHES, 4000);
                    break;
                case EVENT_PAMELA_VANISHES:
                    PamelaVanishes();
                    _events.ScheduleEvent(EVENT_JOSEPH_HOME, 1000);
                    break;
                case EVENT_JOSEPH_HOME:
                    _events.CancelEvent(EVENT_REUNION_TIMEOUT);
                    Talk(SAY_JOSEPH_HOME);
                    me->DespawnOrUnsummon(6000);
                    break;
                case EVENT_REUNION_TIMEOUT:
                    // stuck on the way: Pamela goes back to her spot, Joseph goes
                    PamelaVanishes();
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    void StartReunion()
    {
        if (_reunion)
            return;
        _reunion = true;
        _events.CancelEvent(EVENT_REUNION_FALLBACK);
        _events.ScheduleEvent(EVENT_REUNION_TIMEOUT, 2 * MINUTE * IN_MILLISECONDS);
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(POINT_JOSEPH_1, JosephPath[0]);
    }

    Creature* GetPamela() const
    {
        return me->FindNearestCreature(NPC_PAMELA_REDPATH, 150.0f, true);
    }

    // She vanishes and is back at her spot in 12 s: she ends 27390 and gives 27391 for everyone (her own respawn time is 300 s)
    void PamelaVanishes()
    {
        Creature* pamela = GetPamela();
        if (!pamela || pamela->IsSummon())
            return;
        uint32 respawnDelay = pamela->GetRespawnDelay();
        pamela->SetRespawnDelay(PAMELA_RESPAWN_S);
        pamela->ForcedDespawn();
        pamela->SetRespawnDelay(respawnDelay);
    }

    EventMap _events;
    bool _reunion = false;
};

void AddSC_eastern_plaguelands_darrowshire()
{
    RegisterSpellScript(spell_darrowshire_pamelas_doll_trap);
    RegisterGameObjectAI(go_darrowshire_relic_bundle);
    RegisterCreatureAI(npc_darrowshire_joseph_redpath);
}
