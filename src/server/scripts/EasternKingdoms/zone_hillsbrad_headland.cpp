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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "TemporarySummon.h"

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

void AddSC_hillsbrad_headland()
{
    new creature_script<npc_hillsbrad_lilith>("npc_hillsbrad_lilith");
    new aura_script<spell_hillsbrad_lilith_summon_controller>("spell_hillsbrad_lilith_summon_controller");
    new quest_hillsbrad_lilith_disguise();
}
