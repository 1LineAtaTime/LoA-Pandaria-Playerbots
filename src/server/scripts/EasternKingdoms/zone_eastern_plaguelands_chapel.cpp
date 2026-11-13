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

/* Eastern Plaguelands, Light's Hope Chapel: Argent Call: The Trial of the Crypt (27464).

   Sourced: the Argent Scroll 61309 casts 85308, whose only effect sends event 26356 and which can be cast only at the spell
   focus in the crypt's last chamber (object 301078). Lord Raymond George 45707 presides ("Let the trial begin. Rise,
   champions!"), Argent Wardens 45698 rise from the four corners (Summon Argent Warden NW/NE/SE/SW 85263-85266 and their
   spell_target_position rows), walk in (the waypoint paths of the old static wardens, a sniff of one trial), and when they
   are down George says "Your warm-up is complete. Now, the real trial begins!" and fights; he dies saying "Well-done, heroes.
   You have passed my test of combat." His kill is the objective.
   Ours: two waves of four, the second when the first is down or 30 s after it rose; the actors are private to the reader's
   group, so several groups can run the trial at once; the trial ends when no living member of that group who still has
   the quest is within 30 yd of the focus for 5 s; the NE path (the sniff has none for that corner). */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Player.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "TemporarySummon.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

enum TrialOfTheCrypt
{
    QUEST_TRIAL_OF_THE_CRYPT        = 27464,

    NPC_LORD_RAYMOND_GEORGE         = 45707,
    NPC_ARGENT_WARDEN               = 45698,
    // The paladin followers of 27463 / 27464 and 27465 (section "caravan events + followers" owns them)
    NPC_GIDWIN_TRIAL                = 45677,
    NPC_TARENAR_TRIAL               = 45678,
    NPC_TARENAR_GLADE               = 45792,
    NPC_GIDWIN_GLADE                = 45793,

    SAY_GEORGE_BEGIN                = 0,    // 45805
    SAY_GEORGE_WARMUP_DONE          = 1,    // 45806
    SAY_GEORGE_DEFEATED             = 2,    // 45812

    // Sent to the followers of the trial's group when it begins: the scroll's buff is theirs to give (ours, question 11)
    ACTION_TRIAL_OF_THE_CRYPT_BEGINS = 27464,

    DATA_WARDEN_CORNER              = 1,
    DATA_TRIAL_READER               = 2,
    POINT_WARDEN_IN                 = 1,

};

enum TrialEvents
{
    EVENT_GEORGE_BEGIN = 1,
    EVENT_WAVE_1,
    EVENT_WAVE_2,
    EVENT_WARMUP_DONE,
    EVENT_GEORGE_ENGAGE,
    EVENT_CHECK_GROUP,
};

// The focus 301078 (guid 216180) and George's sniffed spot above it
Position const TrialCenter      = { 2323.2f, -5386.0f, 76.6f, 0.0f };
Position const GeorgePosition   = { 2323.28f, -5385.66f, 76.9932f, 4.64258f };
float const TrialRange          = 30.0f;

// Corners of 85263 NW, 85264 NE, 85265 SE, 85266 SW (spell_target_position) and the walk in. NW, SE and SW are the sniffed
// paths 307922, 307923 and 307925 of the old static wardens; NE is ours, the mirror of NW.
std::vector<Position> const WardenPaths[4] =
{
    { { 2335.45f, -5392.75f, 75.6515f }, { 2333.46f, -5393.64f, 75.6515f }, { 2331.95f, -5394.32f, 75.9572f }, { 2324.00f, -5397.89f, 75.5137f } },
    { { 2336.81f, -5404.36f, 75.6517f }, { 2334.40f, -5403.10f, 75.6515f }, { 2332.00f, -5401.90f, 75.9572f }, { 2324.40f, -5398.90f, 75.5130f } },
    { { 2309.34f, -5404.63f, 75.6515f }, { 2311.74f, -5403.52f, 75.6515f }, { 2312.69f, -5403.09f, 75.6515f }, { 2314.02f, -5402.48f, 75.9572f }, { 2322.18f, -5398.72f, 75.5123f } },
    { { 2310.78f, -5390.87f, 75.6515f }, { 2314.27f, -5392.98f, 75.6515f }, { 2316.04f, -5394.05f, 75.9572f }, { 2322.24f, -5397.78f, 75.5228f } },
};
float const WardenFacing[4] = { 3.68744f, 2.68056f, 0.438247f, 5.57476f };

// Whom the trial's actors may fight: players and what players own, alive, in the crypt, and able to see the actor. The actors
// are private to the reader's group: a player outside it (or a pet that cannot see private objects) would be hit by something
// it cannot see or answer, so it is left alone, as are the Argent Sentries that faction 1990 is hostile to.
static bool IsTrialTarget(Creature const* actor, Unit* unit)
{
    if (!unit->IsAlive() || !unit->IsInMap(actor))
        return false;
    if (unit->GetTypeId() != TYPEID_PLAYER && !unit->IsControlledByPlayer())
        return false;
    if (unit->GetExactDist(&TrialCenter) > TrialRange)
        return false;
    if (!unit->CanSeeOrDetect(actor))
        return false;
    return actor->IsValidAttackTarget(unit);
}

static Unit* SelectTrialTarget(Creature* actor)
{
    std::list<Unit*> units;
    Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(actor, actor, TrialRange + 10.0f);
    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(actor, units, check);
    actor->VisitNearbyObject(TrialRange + 10.0f, searcher);

    Unit* nearest = nullptr;
    float nearestDist = 0.0f;
    for (Unit* unit : units)
    {
        if (!IsTrialTarget(actor, unit))
            continue;
        float dist = actor->GetExactDist(unit);
        if (!nearest || dist < nearestDist)
        {
            nearest = unit;
            nearestDist = dist;
        }
    }
    return nearest;
}

// The trial of this player's group, if one is running: George private to the player or to the player's group
static Creature* FindRunningTrial(Player* player)
{
    ObjectGuid groupGuid = player->GetGroup() ? player->GetGroup()->GetGUID() : ObjectGuid::Empty;
    std::list<Creature*> georges;
    player->GetCreatureListWithEntryInGrid(georges, NPC_LORD_RAYMOND_GEORGE, 100.0f);
    for (Creature* george : georges)
    {
        ObjectGuid owner = george->GetPrivateObjectOwner();
        if (george->IsAlive() && !owner.IsEmpty() && (owner == player->GetGUID() || (!groupGuid.IsEmpty() && owner == groupGuid)))
            return george;
    }
    return nullptr;
}

// 45707 - Lord Raymond George, summoned by the scroll; he runs the trial. He is summoned by the map, not by the reader: a
// player's summons are unsummoned when the player leaves the world (Unit::RemoveFromWorld), and the trial belongs to the
// group, not to the reader. The scroll hands him the reader with SetGUID, which starts the trial.
struct npc_epl_lord_raymond_george : public ScriptedAI
{
    npc_epl_lord_raymond_george(Creature* creature) : ScriptedAI(creature) { }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id != DATA_TRIAL_READER || !_readerGUID.IsEmpty())
            return;

        _readerGUID = guid;
        me->SetReactState(REACT_PASSIVE);
        // Presides hovering over the focus (AnimTier 3 from his addon), out of reach until the warm-up ends (both TDBs 768)
        me->SetDisableGravity(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        // The first line waits until the group's clients have him: a say before that reaches nobody (HaveAtClient)
        _events.ScheduleEvent(EVENT_GEORGE_BEGIN, 1500);
        _events.ScheduleEvent(EVENT_CHECK_GROUP, 1000);
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }

    void JustSummoned(Creature* summon) override
    {
        _wardens.push_back(summon->GetGUID());
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->GetEntry() != NPC_ARGENT_WARDEN)
            return;

        if (CountLiveWardens() > 0)
            return;

        if (_wave == 1)
        {
            _events.CancelEvent(EVENT_WAVE_2);
            _events.ScheduleEvent(EVENT_WAVE_2, 2000);
        }
        else if (_wave == 2 && !_warmupDone)
        {
            _warmupDone = true;
            _events.ScheduleEvent(EVENT_WARMUP_DONE, 2000);
        }
    }

    // "if you don't deal damage to the last guy you fight (Lord Raymond George), you don't get credit for the quest and have to
    // reuse the scroll" (comment 2012-05-10): the followers alone cannot win the trial. Until a player of the trial's group (or
    // a pet or totem of his own; not the quest's followers) has hurt him, he does not go below 1 health. Nothing else holds the
    // trial: an idle group still ends with the 15 min cap on George, and the wardens follow him.
    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (!_damagedByPlayer && attacker)
        {
            uint32 entry = attacker->GetEntry();
            bool follower = attacker->GetTypeId() == TYPEID_UNIT && (entry == NPC_GIDWIN_TRIAL || entry == NPC_TARENAR_TRIAL
                || entry == NPC_TARENAR_GLADE || entry == NPC_GIDWIN_GLADE);
            Player* player = attacker->GetCharmerOrOwnerPlayerOrPlayerItself();
            if (!follower && player && me->CheckPrivateObjectOwnerVisibility(player))
                _damagedByPlayer = true;
        }

        if (!_damagedByPlayer && damage >= me->GetHealth())
            damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_GEORGE_DEFEATED);
        DespawnWardens();
        me->DespawnOrUnsummon(10000);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_engaged)
            return;

        // Nobody left to fight in the crypt for now: stay on the floor where he is and look again (the trial ends through
        // the group check, not through an evade)
        if (!_EnterEvadeMode(why))
            return;
        SetGrounded();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->SetHomePosition(me->GetPosition());
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GEORGE_BEGIN:
                    Talk(SAY_GEORGE_BEGIN);
                    NotifyFollowers();
                    _events.ScheduleEvent(EVENT_WAVE_1, 3000);
                    break;
                case EVENT_WAVE_1:
                    _wave = 1;
                    SummonWave();
                    _events.ScheduleEvent(EVENT_WAVE_2, 30000);
                    break;
                case EVENT_WAVE_2:
                    if (_wave == 1)
                    {
                        _wave = 2;
                        SummonWave();
                    }
                    break;
                case EVENT_WARMUP_DONE:
                    Talk(SAY_GEORGE_WARMUP_DONE);
                    _events.ScheduleEvent(EVENT_GEORGE_ENGAGE, 3000);
                    break;
                case EVENT_GEORGE_ENGAGE:
                    _engaged = true;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    SetGrounded();
                    me->SetReactState(REACT_DEFENSIVE);
                    break;
                case EVENT_CHECK_GROUP:
                    if (!CheckGroup())
                        return;
                    _events.ScheduleEvent(EVENT_CHECK_GROUP, 1000);
                    break;
                default:
                    break;
            }
        }

        if (!_engaged)
            return;

        if (UpdateVictim())
        {
            DoMeleeAttackIfReady();
            return;
        }

        if (_targetTimer > diff)
        {
            _targetTimer -= diff;
            return;
        }
        _targetTimer = 1000;

        if (Unit* target = SelectTrialTarget(me))
        {
            me->GetThreatManager().AddThreat(target, 1.0f);
            AttackStart(target);
        }
    }

private:
    void SetGrounded()
    {
        me->SetAnimTier(AnimTier::Ground);
        me->SetDisableGravity(false);
    }

    void SummonWave()
    {
        for (uint8 corner = 0; corner < 4; ++corner)
        {
            Position pos = WardenPaths[corner].front();
            pos.SetOrientation(WardenFacing[corner]);
            if (TempSummon* warden = me->SummonCreature(NPC_ARGENT_WARDEN, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5s, 0, me->GetPrivateObjectOwner()))
                if (warden->IsAIEnabled)
                    warden->AI()->SetData(DATA_WARDEN_CORNER, corner);
        }
    }

    uint32 CountLiveWardens() const
    {
        uint32 count = 0;
        for (ObjectGuid const& guid : _wardens)
            if (Creature* warden = ObjectAccessor::GetCreature(*me, guid))
                if (warden->IsAlive())
                    ++count;
        return count;
    }

    void DespawnWardens()
    {
        for (ObjectGuid const& guid : _wardens)
            if (Creature* warden = ObjectAccessor::GetCreature(*me, guid))
                warden->DespawnOrUnsummon();
        _wardens.clear();
    }

    // A member of the trial's group who keeps it going: alive, in the crypt, sees the trial and still has the quest
    bool IsTrialist(Player* player) const
    {
        return player->IsAlive() && player->IsInMap(me) && player->GetExactDist(&TrialCenter) <= TrialRange
            && me->CheckPrivateObjectOwnerVisibility(player) && player->GetQuestStatus(QUEST_TRIAL_OF_THE_CRYPT) == QUEST_STATUS_INCOMPLETE;
    }

    // Wipe, logout, leaving the crypt, abandoning: the trial ends when no trialist is left for 5 s, and everything goes
    bool CheckGroup()
    {
        bool anyone = false;
        std::list<Player*> players;
        Trinity::AnyPlayerInObjectRangeCheck check(me, TrialRange + 10.0f);
        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
        me->VisitNearbyWorldObject(TrialRange + 10.0f, searcher);
        for (Player* player : players)
            if (IsTrialist(player))
            {
                anyone = true;
                break;
            }

        if (anyone)
        {
            _emptyChecks = 0;
            return true;
        }

        if (++_emptyChecks < 5)
            return true;

        DespawnWardens();
        me->DespawnOrUnsummon();
        return false;
    }

    // The followers of the group's players (owned guardians of the four entries) get the cue to rebuff the party
    void NotifyFollowers()
    {
        for (uint32 entry : { NPC_GIDWIN_TRIAL, NPC_TARENAR_TRIAL, NPC_TARENAR_GLADE, NPC_GIDWIN_GLADE })
        {
            std::list<Creature*> followers;
            me->GetCreatureListWithEntryInGrid(followers, entry, TrialRange + 10.0f);
            for (Creature* follower : followers)
            {
                Player* owner = follower->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (follower->IsAlive() && owner && me->CheckPrivateObjectOwnerVisibility(owner) && follower->IsAIEnabled)
                    follower->AI()->DoAction(ACTION_TRIAL_OF_THE_CRYPT_BEGINS);
            }
        }
    }

    EventMap _events;
    GuidVector _wardens;
    ObjectGuid _readerGUID;
    uint32 _targetTimer = 0;
    uint8 _wave = 0;
    uint8 _emptyChecks = 0;
    bool _warmupDone = false;
    bool _engaged = false;
    bool _damagedByPlayer = false;
};

// 45698 - Argent Warden, summoned by George at a corner; walks in and fights the trial's group
struct npc_epl_argent_warden : public ScriptedAI
{
    npc_epl_argent_warden(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _georgeGUID = summoner->GetGUID();
        // Defensive: never recruited by another trial's wardens (Creature::CanAssistTo wants aggressive helpers), and the
        // script picks its own targets
        me->SetReactState(REACT_DEFENSIVE);
    }

    void SetData(uint32 type, uint32 corner) override
    {
        if (type != DATA_WARDEN_CORNER || corner >= 4)
            return;

        std::vector<Position> const& path = WardenPaths[corner];
        me->GetMotionMaster()->MoveSmoothPath(POINT_WARDEN_IN, path.data(), path.size(), true);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == POINT_WARDEN_IN)
            InRoom();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        // Attacked on the way in: it is in the room now
        _inRoom = true;
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_EnterEvadeMode(why))
            return;

        // No going back to the corner: wait where it stands for the next target of the trial
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        InRoom();
    }

    void UpdateAI(uint32 diff) override
    {
        // Gone with the trial, in combat or not: George ended it (or was removed) without despawning this one
        if (_checkTimer > diff)
            _checkTimer -= diff;
        else
        {
            _checkTimer = 1000;
            Creature* george = ObjectAccessor::GetCreature(*me, _georgeGUID);
            if (!george || !george->IsAlive())
            {
                me->DespawnOrUnsummon();
                return;
            }
            _lookForTarget = true;
        }

        if (UpdateVictim())
        {
            DoMeleeAttackIfReady();
            return;
        }

        if (!_lookForTarget || !_inRoom)
            return;
        _lookForTarget = false;

        if (Unit* target = SelectTrialTarget(me))
        {
            me->GetThreatManager().AddThreat(target, 1.0f);
            AttackStart(target);
        }
    }

private:
    void InRoom()
    {
        _inRoom = true;
        _lookForTarget = true;
        me->SetHomePosition(me->GetPosition());
    }

    ObjectGuid _georgeGUID;
    uint32 _checkTimer = 1000;
    bool _inRoom = false;
    bool _lookForTarget = false;
};

// 85308 - Trial of the Crypt (Argent Scroll 61309): its only effect sends event 26356, which nothing handles; this starts
// the trial for the reader's group
class spell_epl_trial_of_the_crypt : public SpellScript
{
    PrepareSpellScript(spell_epl_trial_of_the_crypt);

    SpellCastResult CheckCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return SPELL_FAILED_BAD_TARGETS;

        // The provided scroll stays until the turn-in: no second trial once George is down, none while the group's runs
        if (player->GetQuestStatus(QUEST_TRIAL_OF_THE_CRYPT) != QUEST_STATUS_INCOMPLETE || FindRunningTrial(player))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleSendEvent(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Player* player = GetHitUnit() ? GetHitUnit()->ToPlayer() : nullptr;
        if (!player || FindRunningTrial(player))
            return;

        // Private to the group (or the reader alone), summoned by the map so that it outlives the reader's logout or teleport
        ObjectGuid owner = player->GetGroup() ? player->GetGroup()->GetGUID() : player->GetGUID();
        TempSummon* george = player->GetMap()->SummonCreature(NPC_LORD_RAYMOND_GEORGE, GeorgePosition, nullptr, 15min, nullptr, 0, 0, owner);
        if (!george)
            return;

        // The reader's phases, as a player summoner would have given them (the wardens take George's)
        for (uint32 phaseId : player->GetPhases())
            george->SetPhased(phaseId, false, true);
        george->SetPhaseMask(player->GetPhaseMask(), true);

        if (george->IsAIEnabled)
            george->AI()->SetGUID(player->GetGUID(), DATA_TRIAL_READER);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_trial_of_the_crypt::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_epl_trial_of_the_crypt::HandleSendEvent, EFFECT_0, SPELL_EFFECT_SEND_EVENT);
    }
};

void AddSC_eastern_plaguelands_chapel()
{
    new creature_script<npc_epl_lord_raymond_george>("npc_epl_lord_raymond_george");
    new creature_script<npc_epl_argent_warden>("npc_epl_argent_warden");
    new spell_script<spell_epl_trial_of_the_crypt>("spell_epl_trial_of_the_crypt");
}
