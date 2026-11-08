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

// Hillsbrad Foothills: Melisara's fingers of the Stormpike hand, The Headland.
//  - Infiltration (28603) and Deception and Trickery (28604): Lilith goes with the player as a personal companion and disguises them as an SI:7
//    Operative, and the Stormpike Guard takes them for one of its own.
// Sourced: the spells, their summon properties and area group (5.4.8 DBC); that she comes as 28603 is accepted, that the disguise holds through
// the zone and that its vendors, Rufus and Magnus can be talked to (Wowhead comments on 28603 and 28604). Ours: the states that keep her
// (WantsLilith), the leash and the timings; nothing was sniffed.
//  - Cry of the Banshee (28538): the Banshee called by the Banshee Mirror lifts the agent and flies off with it. See npc_hillsbrad_banshee.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SmartAI.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "TemporarySummon.h"
#include "ModelIgnoreFlags.h"
#include "Vehicle.h"

enum HeadlandLilith
{
    QUEST_INFILTRATION                  = 28603,
    QUEST_DECEPTION_AND_TRICKERY        = 28604,

    NPC_LILITH                          = 49035,

    SPELL_LILITH_SUMMON_CONTROLLER      = 91391,    // on the player: the companion and the force reaction (Stormpike Guard 730 friendly)
    SPELL_SUMMON_LILITH                 = 91393,    // SummonProperties 3081: personal ally, no duration, the player is her owner
    SPELL_SI7_DISGUISE_PERIODIC         = 91396,    // on Lilith: every 5 s casts 91395
    SPELL_SI7_DISGUISE                  = 91395,    // aimed at her master: the transform

    EVENT_FIRST_DISGUISE                = 1
};

namespace
{
    // She is left behind by a mount, a flight or a teleport on the same map; creatures only update near players, so this is the player's check
    float const LilithLeash = 100.0f;

    // How long she stays, without her quests and her spell, once she is no longer wanted
    uint32 const LilithLeaveDelay = 2 * IN_MILLISECONDS;

    bool IsQuestInLog(Player const* player, uint32 questId)
    {
        QuestStatus status = player->GetQuestStatus(questId);
        return status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE;
    }

    bool HasLilithQuestInLog(Player const* player)
    {
        return IsQuestInLog(player, QUEST_INFILTRATION) || IsQuestInLog(player, QUEST_DECEPTION_AND_TRICKERY);
    }

    // While either quest is in the log, and after Infiltration is turned in with Deception and Trickery still to take: she is its giver in the field.
    // That last state does not bring her (spell_area autocast 0), so abandoning the quest there ends it (quest_hillsbrad_lilith_disguise), and so
    // do a relog and a map change, since the controller is not saved: Melisara then offers the quest.
    bool WantsLilith(Player const* player)
    {
        return HasLilithQuestInLog(player) || (player->GetQuestStatus(QUEST_INFILTRATION) == QUEST_STATUS_REWARDED
            && player->GetQuestStatus(QUEST_DECEPTION_AND_TRICKERY) == QUEST_STATUS_NONE);
    }

    // Her periodic goes first: left running it would disguise the player again during the delay
    void SendLilithAway(TempSummon* lilith, uint32 delay)
    {
        lilith->RemoveAurasDueToSpell(SPELL_SI7_DISGUISE_PERIODIC);
        lilith->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        lilith->UnSummon(delay);
    }

    // One Lilith near the player: the player's summons are searched map-wide, because a teleport on the same map leaves the old one where she was.
    void SummonLilith(Player* player)
    {
        if (!player->IsInWorld() || player->IsBeingTeleported())
            return;

        std::list<TempSummon*> liliths;
        player->GetSummons(liliths, NPC_LILITH);

        bool found = false;
        for (TempSummon* lilith : liliths)
        {
            // Already on her way out (UnSummon only queues the removal)
            if (lilith->IsDestroyedObject() || !lilith->GetUInt32Value(UNIT_FIELD_NPC_FLAGS))
                continue;

            if (!found && lilith->IsInWorld() && lilith->IsAlive() && lilith->IsInMap(player) && lilith->IsWithinDist(player, LilithLeash))
                found = true;
            else
                SendLilithAway(lilith, 0);
        }

        // Summon spells do nothing for a dead caster: the next check brings her
        if (found || !player->IsAlive() || player->IsInFlight() || player->GetVehicle())
            return;

        player->CastSpell(player, SPELL_SUMMON_LILITH, true);
    }
}

// 49035 - Lilith
// The player's own (Summon Lilith 91393), and the giver and ender of Deception and Trickery and a giver of Domination wherever she is: guardians and
// summons keep their template's npcflag and quest relations. She follows and never fights. The owner is set by the summon effect after
// IsSummonedBy, so the disguise, which is aimed at her master, can only be cast later.
struct npc_hillsbrad_lilith : public ScriptedAI
{
    npc_hillsbrad_lilith(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, me->GetFollowAngle());
        DoCast(me, SPELL_SI7_DISGUISE_PERIODIC, true);
        _events.ScheduleEvent(EVENT_FIRST_DISGUISE, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            // Not while leaving (SendLilithAway clears her flags). Afterwards her periodic does it, also when the player comes back from the dead.
            if (eventId == EVENT_FIRST_DISGUISE && me->GetUInt32Value(UNIT_FIELD_NPC_FLAGS))
                DoCast(me, SPELL_SI7_DISGUISE, true);
        }
    }

private:
    EventMap _events;
};

// 91391 - Lilith Summon Controller
// spell_area gives it in Hillsbrad Foothills (also its casting requirements: area group 2822 = zone 267) and takes it away when the player leaves.
// Its dummy brings Lilith here; it checks every 5 s that the quests still want her and that she is there (ours, as the Purgation Isle controllers).
// Its force reaction needs no script. When it goes, for whatever reason, everything it brought goes with it.
class spell_hillsbrad_lilith_summon_controller : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_lilith_summon_controller);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_LILITH, SPELL_SI7_DISGUISE_PERIODIC, SPELL_SI7_DISGUISE });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // The aura is not saved (SpellMgr: CHANGE_MAP interrupt, no duration): at login spell_area casts it again, with the player in the world
        if (Player* player = GetTarget()->ToPlayer())
            SummonLilith(player);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        std::list<TempSummon*> liliths;
        player->GetSummons(liliths, NPC_LILITH);
        for (TempSummon* lilith : liliths)
            if (!lilith->IsDestroyedObject() && lilith->GetUInt32Value(UNIT_FIELD_NPC_FLAGS))
                SendLilithAway(lilith, player->IsInWorld() && lilith->IsInWorld() && lilith->IsAlive() ? LilithLeaveDelay : 0);

        player->RemoveAurasDueToSpell(SPELL_SI7_DISGUISE);
        player->RemoveAurasDueToSpell(SPELL_SUMMON_LILITH);
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        // The quest script does this at once; this is for what passes it by (a GM removing a quest)
        if (!WantsLilith(player))
        {
            Remove();
            return;
        }

        SummonLilith(player);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_hillsbrad_lilith_summon_controller::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_hillsbrad_lilith_summon_controller::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_hillsbrad_lilith_summon_controller::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hillsbrad_lilith_summon_controller::OnPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 28603 - Infiltration, 28604 - Deception and Trickery
// spell_area does not follow quest changes in this core (Player::SetQuestStatus): the controller is given and taken here, at accept, abandon and
// reward. An abandon with neither quest left in the log always ends it, also in the state that would otherwise keep her (WantsLilith):
// Melisara offers both quests again.
class quest_hillsbrad_lilith_disguise : public QuestScript
{
public:
    quest_hillsbrad_lilith_disguise() : QuestScript("quest_hillsbrad_lilith_disguise") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (!player->IsInWorld())
            return;

        bool abandoned = newStatus == QUEST_STATUS_NONE;
        if (abandoned ? !HasLilithQuestInLog(player) : !WantsLilith(player))
            player->RemoveAurasDueToSpell(SPELL_LILITH_SUMMON_CONTROLLER);
        else if (!player->HasAura(SPELL_LILITH_SUMMON_CONTROLLER))
            player->UpdateZoneDependentAuras(player->GetZoneId());
    }
};

/*######
## Cry of the Banshee (28538): the Banshee carries the agent off
######*/

enum BansheeCarry
{
    NPC_BANSHEE                         = 48752,

    SPELL_BANSHEE_MIRROR                = 90976,    // Call of the Banshee, the item's spell; it triggers 90871, which summons her
    SPELL_CARRIED_BY_BANSHEE            = 90874,    // area script (10 yd): the agent casts the ride, 90873, on her

    DATA_CARRIED_AGENT                  = 1,

    TALK_BANSHEE_CARRY                  = 0,

    POINT_BANSHEE_APPROACH              = 1,
    POINT_BANSHEE_AWAY                  = 2,

    EVENT_BANSHEE_NO_AGENT              = 1,
    EVENT_BANSHEE_GRAB                  = 2,
    EVENT_BANSHEE_NOT_SEATED            = 3,
    EVENT_BANSHEE_LIFT                  = 4,
    EVENT_BANSHEE_END                   = 5
};

namespace
{
    // The wiki's caption has her "taking an SI:7 Operative back to Tarren Mill": she heads there. All the rest of the path is ours.
    Position const BansheeTarrenMillPos = { -30.0f, -900.0f, 57.0f };

    float const BansheeApproachSpeed    = 10.0f;    // yd/s: she was summoned 10 yd behind the agent
    float const BansheeFlightSpeed      = 5.0f;     // yd/s: time for a ranged player to finish the agent off
    float const BansheeFlightDistance   = 35.0f;
    float const BansheeRises[]          = { 16.0f, 10.0f, 6.0f };

    uint32 const AgentNeverSeatedDelay  = 8 * IN_MILLISECONDS;
    uint32 const AgentDroppedDelay      = 4 * IN_MILLISECONDS;

    Position BansheeCurvePoint(Position const& a, Position const& c, Position const& b, float t)
    {
        float const u = 1.0f - t;
        return { u * u * a.GetPositionX() + 2.0f * u * t * c.GetPositionX() + t * t * b.GetPositionX(),
                 u * u * a.GetPositionY() + 2.0f * u * t * c.GetPositionY() + t * t * b.GetPositionY(),
                 u * u * a.GetPositionZ() + 2.0f * u * t * c.GetPositionZ() + t * t * b.GetPositionZ() };
    }
}

// 48741 - SI:7 Operative, 48742 - SI:7 Assassin
// Their SmartAI as before (the mirror's credit and Gaze of the Banshee 90903, stun + pacify; fighting), plus what being carried needs. From the mirror's hit until it goes the agent
// neither fights nor evades: an evade would run a passenger home, and clear the gaze that makes the mirror's credit once-only. Respawning resets it
// all (Creature::Respawn: react state, AI Reset; the corpse's removal takes the auras).
struct npc_hillsbrad_si7_agent : public SmartAI
{
    npc_hillsbrad_si7_agent(Creature* creature) : SmartAI(creature), _state(CARRY_NONE), _despawnTimer(0) { }

    void Reset() override
    {
        _state = CARRY_NONE;
        _despawnTimer = 0;
        _casterGUID.Clear();
        SmartAI::Reset();
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        SmartAI::SpellHit(caster, spell);

        if (spell->Id != SPELL_BANSHEE_MIRROR || _state != CARRY_NONE)
            return;

        _state = CARRY_HELD;
        _casterGUID = caster->GetGUID();
        _despawnTimer = AgentNeverSeatedDelay;

        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MoveIdle();
    }

    void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
    {
        if (_state == CARRY_NONE)
            SmartAI::EnterEvadeMode(why);
    }

    void AttackStart(Unit* who) override
    {
        if (_state == CARRY_NONE)
            SmartAI::AttackStart(who);
    }

    // Vehicle calls this only for leaving. Dying and despawning count as dead by now (Unit::setDeathState sets the state first), so this is
    // a living agent let go of: she was unsummoned (her owner logged out or left the map) or killed. _ExitVehicle has it falling.
    // It stays held and goes soon after it lands; meanwhile it can be killed.
    void BoardedVehicle(Vehicle* /*vehicle*/, int8 /*seatId*/, bool apply) override
    {
        if (apply || !me->IsAlive())
            return;

        _state = CARRY_DROPPED;
        _despawnTimer = AgentDroppedDelay;
    }

    void JustDied(Unit* killer) override
    {
        SmartAI::JustDied(killer);

        // Unit::setDeathState stopped the exit's fall, and only makes creatures that can fly fall again (Creature::setDeathState).
        // A plain fall, with no TransportExit: the exit's packet already took it off her, as any SMSG_MONSTER_MOVE without a transport does
        // (client CGUnit_C::OnMonsterMove, ForceSetTransport), and the client's CMovement_C::OnSpline does not play a spline with that flag.
        if (_state != CARRY_NONE)  // MoveFall does nothing on the ground
            me->GetMotionMaster()->MoveFall();
    }

    void UpdateAI(uint32 diff) override
    {
        SmartAI::UpdateAI(diff);

        if (_state == CARRY_HELD)
        {
            if (me->GetVehicle())
            {
                _state = CARRY_SEATED;
                _despawnTimer = 0;
                return;
            }

            GiveToBanshee();
        }

        if (!_despawnTimer)
            return;

        // At once and in plain sight: SMART_ACTION_FORCE_DESPAWN would hide a living creature and keep it, killable, for 5 s more
        if (_despawnTimer <= diff)
            me->DespawnOrUnsummon();
        else
            _despawnTimer -= diff;
    }

private:
    enum CarryState
    {
        CARRY_NONE,
        CARRY_HELD,
        CARRY_SEATED,
        CARRY_DROPPED
    };

    // Tells the Banshee this mirror called which agent she came for. The mirror has a 5 s cooldown, so its caster has one Banshee waiting for an agent;
    // an earlier one may still be in the air, having left alone
    void GiveToBanshee()
    {
        if (_casterGUID.IsEmpty())
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _casterGUID);
        if (!player)
        {
            _casterGUID.Clear();
            return;
        }

        std::list<TempSummon*> banshees;
        player->GetSummons(banshees, NPC_BANSHEE);
        for (TempSummon* banshee : banshees)
        {
            if (banshee->IsDestroyedObject() || !banshee->IsInWorld() || !banshee->IsAIEnabled || !banshee->AI()->GetData(DATA_CARRIED_AGENT))
                continue;

            banshee->AI()->SetGUID(me->GetGUID(), DATA_CARRIED_AGENT);
            _casterGUID.Clear();
            return;
        }
    }

    CarryState _state;
    uint32 _despawnTimer;
    ObjectGuid _casterGUID;
};

// 48752 - Banshee
// Sourced: the spells (90871 summons her behind the agent, for 30 s; 90874 and 90873 Carried by Banshee, 5.4.8 DBC), her lines (broadcast 48962-48965)
// and the event: comments on 28538 and 48752 have the agent carried off while she speaks, killable and lootable by ranged players on the way,
// and the wiki has her taking it to Tarren Mill. Ours: Vehicle 1348 (no source gives her a vehicle, see the SQL), the approach, when she speaks, the
// flight and that both then vanish. She flies a curve up and towards Tarren Mill, out of melee reach within a few seconds; she leaves the same
// way if the agent is dead or lost. The agent's death mid-flight drops it (npc_hillsbrad_si7_agent) and she flies on. Never in combat.
struct npc_hillsbrad_banshee : public ScriptedAI
{
    npc_hillsbrad_banshee(Creature* creature) : ScriptedAI(creature), _flying(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetWalk(false);
        me->setActive(true, ActiveFlags::Script);   // the flight goes on if the player runs off

        _events.ScheduleEvent(EVENT_BANSHEE_NO_AGENT, 2s);
    }

    // Not the follow-on-spawn of CreatureAI::JustAppeared, nor a return to an owner
    void JustAppeared() override { }
    void EnterEvadeMode(EvadeReason /*why*/ = EVADE_REASON_OTHER) override { }
    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }

    ObjectGuid GetGUID(int32 id = 0) const override
    {
        return id == DATA_CARRIED_AGENT ? _agentGUID : ObjectGuid::Empty;
    }

    // Still waiting to be told her agent
    uint32 GetData(uint32 id = 0) const override
    {
        return id == DATA_CARRIED_AGENT && !_flying && _agentGUID.IsEmpty() ? 1 : 0;
    }

    void SetGUID(ObjectGuid guid, int32 id = 0) override
    {
        if (id != DATA_CARRIED_AGENT || _flying)
            return;

        _agentGUID = guid;
        _events.CancelEvent(EVENT_BANSHEE_NO_AGENT);

        Creature* agent = ObjectAccessor::GetCreature(*me, _agentGUID);
        if (!agent || !agent->IsAlive())
        {
            FlyOff();
            return;
        }

        // Up to the agent's shoulder, from where she is
        float const angle = agent->GetAngle(me);
        Position const path[2] =
        {
            me->GetPosition(),
            { agent->GetPositionX() + 2.0f * std::cos(angle), agent->GetPositionY() + 2.0f * std::sin(angle), agent->GetPositionZ() + 1.0f }
        };
        SetFlightSpeed(BansheeApproachSpeed);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_BANSHEE_APPROACH, path, 2, false, true);

        _events.ScheduleEvent(EVENT_BANSHEE_GRAB, 3s);  // should the spline not report
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_BANSHEE_APPROACH && !_flying)
        {
            _events.CancelEvent(EVENT_BANSHEE_GRAB);
            _events.ScheduleEvent(EVENT_BANSHEE_GRAB, 0ms);
        }
        else if (pointId == POINT_BANSHEE_AWAY)
        {
            _events.CancelEvent(EVENT_BANSHEE_END);
            _events.ScheduleEvent(EVENT_BANSHEE_END, 0ms);
        }
    }

    void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool apply) override
    {
        if (apply)
        {
            _events.CancelEvent(EVENT_BANSHEE_NOT_SEATED);
            Talk(TALK_BANSHEE_CARRY);
            _events.ScheduleEvent(EVENT_BANSHEE_LIFT, 800ms);
        }
        else if (!_flying)  // Dead or despawned before she took off (in the air the flight just goes on)
        {
            _events.CancelEvent(EVENT_BANSHEE_LIFT);
            _events.ScheduleEvent(EVENT_BANSHEE_LIFT, 0ms);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BANSHEE_NO_AGENT:
                case EVENT_BANSHEE_NOT_SEATED:
                case EVENT_BANSHEE_LIFT:
                    FlyOff();
                    break;
                case EVENT_BANSHEE_GRAB:
                {
                    Creature* agent = ObjectAccessor::GetCreature(*me, _agentGUID);
                    if (!agent || !agent->IsAlive())
                    {
                        FlyOff();
                        break;
                    }

                    me->CastSpell(me, SPELL_CARRIED_BY_BANSHEE, true);
                    if (!agent->GetVehicle())   // The ride is instant: seated by now if it worked
                        _events.ScheduleEvent(EVENT_BANSHEE_NOT_SEATED, 500ms);
                    break;
                }
                case EVENT_BANSHEE_END:
                    if (Creature* agent = ObjectAccessor::GetCreature(*me, _agentGUID))
                        if (agent->GetVehicleBase() == me)
                            agent->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                    return;
                default:
                    break;
            }
        }
    }

private:
    void SetFlightSpeed(float speed)
    {
        me->SetSpeed(MOVE_RUN, speed / baseMoveSpeed[MOVE_RUN]);
        me->SetSpeed(MOVE_FLIGHT, speed / baseMoveSpeed[MOVE_FLIGHT]);
    }

    bool LineIsClear(Position const& from, Position const& to) const
    {
        return me->GetMap()->isInLineOfSight(from.GetPositionX(), from.GetPositionY(), from.GetPositionZ(),
            to.GetPositionX(), to.GetPositionY(), to.GetPositionZ(), me->GetPhaseMask(), VMAP::ModelIgnoreFlags::Nothing);
    }

    // The curve from here over the control point to the end clears the terrain, the buildings and the trees (they are in the vmaps)
    bool CurveIsClear(Position const& here, Position const& control, Position const& end) const
    {
        Position from = here;
        for (uint32 i = 1; i <= 8; ++i)
        {
            float const t = float(i) / 8.0f;
            Position const to = BansheeCurvePoint(here, control, end, t);
            if (!LineIsClear(from, to))
                return false;

            // Near the start she may still be low over the ground she took the agent from
            float const ground = me->GetMap()->GetHeight(me->GetPhaseMask(), to.GetPositionX(), to.GetPositionY(), to.GetPositionZ() + 40.0f, true, 120.0f);
            if (ground > INVALID_HEIGHT && to.GetPositionZ() < ground + (t < 0.5f ? 2.0f : 6.0f))
                return false;

            from = to;
        }
        return true;
    }

    // Spawns are spread over the Headland, so the path is relative and checked here: towards Tarren Mill if that is clear, else the nearest clear
    // heading, higher if that helps; else straight up.
    void FlyOff()
    {
        if (_flying)
            return;

        _flying = true;
        _events.CancelEvent(EVENT_BANSHEE_NO_AGENT);
        _events.CancelEvent(EVENT_BANSHEE_GRAB);
        _events.CancelEvent(EVENT_BANSHEE_NOT_SEATED);
        _events.CancelEvent(EVENT_BANSHEE_LIFT);

        Position const here = me->GetPosition();

        float rise = 0.0f;
        for (float candidate : BansheeRises)
        {
            if (LineIsClear(here, { here.GetPositionX(), here.GetPositionY(), here.GetPositionZ() + candidate }))
            {
                rise = candidate;
                break;
            }
        }

        Position const control = { here.GetPositionX(), here.GetPositionY(), here.GetPositionZ() + rise };
        Position end = { here.GetPositionX(), here.GetPositionY(), here.GetPositionZ() + rise };
        bool found = false;

        float const heading = me->GetAngle(BansheeTarrenMillPos.GetPositionX(), BansheeTarrenMillPos.GetPositionY());
        float const offsets[] = { 0.0f, 0.79f, -0.79f, 1.57f, -1.57f, 2.36f, -2.36f, 3.14f };
        for (float offset : offsets)
        {
            float const x = here.GetPositionX() + BansheeFlightDistance * std::cos(heading + offset);
            float const y = here.GetPositionY() + BansheeFlightDistance * std::sin(heading + offset);
            for (float extra : { 6.0f, 12.0f, 18.0f, 24.0f })
            {
                Position const candidate = { x, y, control.GetPositionZ() + extra };
                if (CurveIsClear(here, control, candidate))
                {
                    end = candidate;
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }

        // Evenly spaced points: a smooth spline's speed jumps at uneven ones
        Position last = here;
        float length = 0.0f;
        std::vector<Position> dense;
        dense.push_back(here);
        for (uint32 i = 1; i <= 64; ++i)
        {
            dense.push_back(BansheeCurvePoint(here, control, end, float(i) / 64.0f));
            length += last.GetExactDist(dense.back());
            last = dense.back();
        }

        if (length < 1.0f)  // Under a roof with nowhere to go
        {
            _events.ScheduleEvent(EVENT_BANSHEE_END, 2s);
            return;
        }

        size_t const count = std::max<size_t>(2, size_t(std::lround(length / 3.0f)));
        float const spacing = length / count;
        std::vector<Position> path;
        path.push_back(here);
        float walked = 0.0f;
        float next = spacing;
        for (size_t i = 0; i + 1 < dense.size(); ++i)
        {
            float const segment = dense[i].GetExactDist(dense[i + 1]);
            while (segment > 0.0f && walked + segment >= next && path.size() < count)
            {
                float const f = (next - walked) / segment;
                path.emplace_back(dense[i].GetPositionX() + (dense[i + 1].GetPositionX() - dense[i].GetPositionX()) * f,
                    dense[i].GetPositionY() + (dense[i + 1].GetPositionY() - dense[i].GetPositionY()) * f,
                    dense[i].GetPositionZ() + (dense[i + 1].GetPositionZ() - dense[i].GetPositionZ()) * f);
                next += spacing;
            }
            walked += segment;
        }
        path.push_back(end);

        SetFlightSpeed(BansheeFlightSpeed);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_BANSHEE_AWAY, path.data(), path.size(), false, true);

        // Should the spline not report; well inside her 30 s
        _events.ScheduleEvent(EVENT_BANSHEE_END, Milliseconds(uint32(length / BansheeFlightSpeed * IN_MILLISECONDS) + 2 * IN_MILLISECONDS));
    }

    EventMap _events;
    ObjectGuid _agentGUID;
    bool _flying;
};

// 90874 - Carried by Banshee
// Its area would take any unit in 10 yd (the DBC's aura state is not even checked for a vehicle as caster, SpellInfo::CheckTarget): she takes
// the agent she came for, and two Banshees side by side don't swap. The script effect's value is the ride, which the agent casts on her.
class spell_hillsbrad_carried_by_banshee : public SpellScript
{
    PrepareSpellScript(spell_hillsbrad_carried_by_banshee);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Creature* caster = GetCaster()->ToCreature();
        ObjectGuid const agent = caster && caster->IsAIEnabled ? caster->AI()->GetGUID(DATA_CARRIED_AGENT) : ObjectGuid::Empty;
        targets.remove_if([agent](WorldObject* target) { return target->GetGUID() != agent; });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* agent = GetHitUnit())
            agent->CastSpell(GetCaster(), uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hillsbrad_carried_by_banshee::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_hillsbrad_carried_by_banshee::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_hillsbrad_headland()
{
    new creature_script<npc_hillsbrad_lilith>("npc_hillsbrad_lilith");
    new aura_script<spell_hillsbrad_lilith_summon_controller>("spell_hillsbrad_lilith_summon_controller");
    new quest_hillsbrad_lilith_disguise();
    new creature_script<npc_hillsbrad_si7_agent>("npc_hillsbrad_si7_agent");
    new creature_script<npc_hillsbrad_banshee>("npc_hillsbrad_banshee");
    new spell_script<spell_hillsbrad_carried_by_banshee>("spell_hillsbrad_carried_by_banshee");
}
