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
SDName: Western_Plaguelands
SD%Complete:
SDComment:
SDCategory: Western Plaguelands
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "TemporarySummon.h"

enum eQuests
{
    QUEST_SCHOLOMANCER_HORDE    = 26924,
    QUEST_SCHOLOMANCER          = 27162,
};

enum eCreatures
{
    NPC_THASSARIAN           = 44453,
    NPC_KOLTIRA_DEATHVIEWER  = 44452,
    NPC_DARK_MASTER_GANDLING = 44323,
};

enum eSpells
{
    SPELL_SHADOWBOLT      = 79932,
    SPELL_SHADOW_ERUPTION = 79932,
    SPELL_SHADOW_PRISON   = 86780,
};

enum eEvents
{
    EVENT_SHADOW_BOLT = 1,
    EVENT_SHADOW_ERUPTION,
};

enum eTalks
{
    SAY_INTRO,
    SAY_SPECIAL_1,
    SAY_SPECIAL_2,
    SAY_SPECIAL_3,
    SAY_SPECIAL_4,
};

// Dark Master Gandling 44323
struct npc_dark_master_gandling_quest : public ScriptedAI
{
    npc_dark_master_gandling_quest(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;
    bool eventDone;

    void Reset() override
    {
        SetCombatMovement(false);
        events.Reset();
        eventDone = false;
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_PACIFIED);
    }

    void JustEngagedWith(Unit* who) override
    {
        Talk(SAY_INTRO);
        events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(1.5 * IN_MILLISECONDS, 3 * IN_MILLISECONDS));
        events.ScheduleEvent(EVENT_SHADOW_ERUPTION, 12 * IN_MILLISECONDS);
    }

    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (eventDone)
            damage = 0;

        if (me->GetHealth() <= damage && !eventDone)
        {
            damage = 0;
            eventDone = true;
            Talk(SAY_SPECIAL_1);
            me->InterruptNonMeleeSpells(true);
            events.Reset();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_PACIFIED);
            DoCast(attacker, SPELL_SHADOW_PRISON);

            me->m_Events.Schedule(16000, [this]()
            {
                Talk(SAY_SPECIAL_2);
                HandleCompleteQuest();
                me->CombatStop();
                me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
            });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHADOW_BOLT:
                    if (Unit* target = me->GetVictim())
                        DoCast(target, SPELL_SHADOWBOLT);

                    events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(3.1 * IN_MILLISECONDS, 3.5 * IN_MILLISECONDS));
                    break;
                case EVENT_SHADOW_ERUPTION:
                    if (Unit* target = me->GetVictim())
                        DoCast(target, SPELL_SHADOW_ERUPTION);

                    events.ScheduleEvent(EVENT_SHADOW_ERUPTION, 14 * IN_MILLISECONDS);
                    break;
            }
        }
    }

    private:
        // Spell not found
        void HandleCompleteQuest()
        {
            std::list<Player*> pList;
            GetPlayerListInGrid(pList, me, 100.0f);
            // The Horde (26924, from Koltira) and the Alliance (27162, from Thassarian) Scholomancer share the objective 44323
            pList.remove_if([=](Player* target)
            {
                return target && target->GetQuestStatus(QUEST_SCHOLOMANCER) != QUEST_STATUS_INCOMPLETE
                    && target->GetQuestStatus(QUEST_SCHOLOMANCER_HORDE) != QUEST_STATUS_INCOMPLETE;
            });

            for (auto&& itr : pList)
                itr->KilledMonsterCredit(me->GetEntry());
        }
};

// Thassarian 44453
class npc_thassarian_quest : public CreatureScript
{
    public:
        npc_thassarian_quest() : CreatureScript("npc_thassarian_quest") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_SCHOLOMANCER)
                creature->AI()->Talk(SAY_INTRO);

            return true;
        }

        struct npc_thassarian_questAI : public ScriptedAI
        {
            npc_thassarian_questAI(Creature* creature) : ScriptedAI(creature) { }

            void UpdateAI(uint32 /*diff*/) override { }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thassarian_questAI(creature);
        }
};

// Koltira Deathweaver 44452 (the Forsaken camp in Andorhal)
// Gives the Horde Scholomancer with the same promise as Thassarian's: "Gandling is strong, but do not worry. I will join you shortly
// after you begin to fight him."
struct npc_koltira_deathweaver_andorhal : public ScriptedAI
{
    npc_koltira_deathweaver_andorhal(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* /*player*/, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_SCHOLOMANCER_HORDE)
            Talk(SAY_INTRO);
    }
};

// Shadow Prison 86780
class spell_shadow_prison_gandling : public AuraScript
{
    PrepareAuraScript(spell_shadow_prison_gandling);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* owner = GetOwner()->ToUnit())
            owner->SetCanFly(false);
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* owner = GetOwner()->ToUnit())
        {
            owner->SetCanFly(true);
            GetPositionWithDistInOrientation(owner, 4.5f, Position::NormalizeOrientation(owner->GetOrientation() - M_PI), x, y);
            owner->GetMotionMaster()->MoveJump({ x, y, owner->GetPositionZ() + 3.0f, owner->GetOrientation() }, 20.0f, 20.0f);
        }
    }

    private:
        float x, y;

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_shadow_prison_gandling::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_shadow_prison_gandling::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

/*######
## The Enthralled Val'kyr: When Death is Not Enough (26937), Combat Training (26938), Ace in the Hole (27089)
######*/

enum EnthralledValkyr
{
    QUEST_WHEN_DEATH_IS_NOT_ENOUGH              = 26937,
    QUEST_COMBAT_TRAINING_HORDE                 = 26938,
    QUEST_ACE_IN_THE_HOLE                       = 27089,

    NPC_ENTHRALLED_VALKYR_GUARDIAN              = 44491,
    NPC_ENTHRALLED_VALKYR_PET                   = 45080,
    NPC_ALLIANCE_LABORER                        = 45047,
    NPC_FRIGHTENED_PROVINCIAL_MINUTEMAN         = 45101,
    NPC_FORSAKEN_TROOPER_FELSTONE               = 45085,
    NPC_FT_FELSTONE_01                          = 45048,
    NPC_FT_FELSTONE_02                          = 45049,
    NPC_FT_FELSTONE_03                          = 45050,
    NPC_FT_ANDORHAL_01                          = 45058,
    NPC_FT_ANDORHAL_02                          = 45059,
    NPC_FT_ANDORHAL_03                          = 45060,
    NPC_FT_ANDORHAL_04                          = 45061,
    NPC_FELSTONE_CHARM_CREDIT                   = 45090,
    NPC_ANDORHAL_CHARM_CREDIT                   = 45103,

    SPELL_BOND_OF_THE_VALKYR_WPL                = 84204,
    SPELL_SUMMON_ENTHRALLED_VALKYR_GUARDIAN     = 84016,
    SPELL_SUMMON_ENTHRALLED_VALKYR_PET          = 84149,
    SPELL_RAISE_FORSAKEN_FELSTONE               = 84143,
    SPELL_RAISE_FORSAKEN_ANDORHAL               = 84197,
    SPELL_NOTIFY_VALKYR_FELSTONE                = 84145,
    SPELL_NOTIFY_VALKYR_ANDORHAL                = 84195,
    SPELL_GENERIC_QUEST_INVISIBILITY_4          = 82343,

    DATA_TROOPER_RAISER                         = 1,
    DATA_TROOPER_MOBILIZE                       = 2,
    DATA_TROOPER_OWNER                          = 3,

    EVENT_TROOPER_STAND                         = 1,
    EVENT_TROOPER_SALUTE                        = 2,
    EVENT_TROOPER_LEAVE                         = 3,
    EVENT_TROOPER_CHECK_OWNER                   = 4,
    EVENT_TROOPER_EXPIRE                        = 5,
    EVENT_TROOPER_DISMISS                       = 6
};

// The val'kyr a quest of this chain needs: the guardian while When Death is Not Enough is in the log, the pet (with Call to Arms on
// its bar) during Combat Training and Ace in the Hole
static uint32 GetWantedValkyrEntry(Player* player)
{
    auto inLog = [player](uint32 questId)
    {
        QuestStatus status = player->GetQuestStatus(questId);
        return status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE;
    };

    if (inLog(QUEST_COMBAT_TRAINING_HORDE) || inLog(QUEST_ACE_IN_THE_HOLE))
        return NPC_ENTHRALLED_VALKYR_PET;
    if (inLog(QUEST_WHEN_DEATH_IS_NOT_ENOUGH))
        return NPC_ENTHRALLED_VALKYR_GUARDIAN;
    return 0;
}

// The player's living val'kyr of the given entry, wherever she is on the map (a hearth or a spirit healer leaves her behind)
static Creature* FindValkyrOf(Player* player, uint32 entry)
{
    std::list<Creature*> valkyrs;
    player->GetAllMinionsByEntry(valkyrs, entry);
    for (Creature* valkyr : valkyrs)
        if (valkyr->IsAlive() && !valkyr->IsDestroyedObject())
            return valkyr;
    return nullptr;
}

// 84204 - Bond of the Val'kyr
// Given by spell_area in Western Plaguelands while one of the three quests is in the log. Summons the val'kyr that quest needs, swaps
// her for the other one when the chain moves on and brings her back after a login, a death or a teleport. Both summons use summon
// slot 6, so a new one replaces the old one.
class spell_wpl_bond_of_the_valkyr : public AuraScript
{
    PrepareAuraScript(spell_wpl_bond_of_the_valkyr);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_ENTHRALLED_VALKYR_GUARDIAN, SPELL_SUMMON_ENTHRALLED_VALKYR_PET });
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
            if (player->IsInWorld())
                Check(player, true);
    }

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
        if (Player* player = GetTarget()->ToPlayer())
            Check(player, false);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAllMinionsByEntry(NPC_ENTHRALLED_VALKYR_GUARDIAN);
        GetTarget()->RemoveAllMinionsByEntry(NPC_ENTHRALLED_VALKYR_PET);
    }

    // The pet replaces a hunter's or warlock's own pet; once the player has called his own pet back, she is only brought back when
    // he has none, so the two are not swapped every few seconds
    static void Check(Player* player, bool force)
    {
        if (!player->IsInWorld() || !player->IsAlive() || player->IsInFlight() || player->GetVehicle())
            return;

        uint32 wanted = GetWantedValkyrEntry(player);
        uint32 other = wanted == NPC_ENTHRALLED_VALKYR_PET ? NPC_ENTHRALLED_VALKYR_GUARDIAN : NPC_ENTHRALLED_VALKYR_PET;
        player->RemoveAllMinionsByEntry(other);
        if (!wanted)
        {
            player->RemoveAllMinionsByEntry(NPC_ENTHRALLED_VALKYR_PET);
            return;
        }

        Creature* valkyr = FindValkyrOf(player, wanted);
        if (valkyr && valkyr->IsWithinDistInMap(player, 60.0f))
            return;

        if (wanted == NPC_ENTHRALLED_VALKYR_PET && !valkyr && !force && player->GetPetGUID())
            return;

        if (valkyr)
            valkyr->DespawnOrUnsummon();

        player->CastSpell(player, wanted == NPC_ENTHRALLED_VALKYR_PET ? SPELL_SUMMON_ENTHRALLED_VALKYR_PET : SPELL_SUMMON_ENTHRALLED_VALKYR_GUARDIAN, true);
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_wpl_bond_of_the_valkyr::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_wpl_bond_of_the_valkyr::OnPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectApply += AuraEffectApplyFn(spell_wpl_bond_of_the_valkyr::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_wpl_bond_of_the_valkyr::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 44491, 45080 - Enthralled Val'kyr
// Follows the player and never fights; she only raises the dead (Notify Val'kyr) and, as the pet, casts Call to Arms
struct npc_wpl_enthralled_valkyr : public ScriptedAI
{
    npc_wpl_enthralled_valkyr(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        ScriptedAI::InitializeAI();
        me->SetReactState(REACT_PASSIVE);
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }
};

// 84145, 84195 - Notify Val'kyr
// Cast by an Alliance Laborer (Felstone Field) or a Frightened Provincial Minuteman (Andorhal) on its killer as it dies (SmartAI).
// If the killer is on the matching quest and his val'kyr is near, she raises the body (Raise Forsaken, whose triggered Kill Credit
// goes to her master) and a Forsaken Trooper rises where it fell, in the model that matches the body (as the Forsaken Trooper Master
// Script does in Silverpine).
class spell_wpl_notify_valkyr : public SpellScript
{
    PrepareSpellScript(spell_wpl_notify_valkyr);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_RAISE_FORSAKEN_FELSTONE, SPELL_RAISE_FORSAKEN_ANDORHAL });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Creature* body = GetCaster()->ToCreature();
        if (!body)
            return;

        // A trooper the player mobilized kills for him; otherwise whoever tapped the body
        Unit* killer = GetHitUnit();
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player && killer && killer->GetTypeId() == TYPEID_UNIT && killer->IsAIEnabled)
            player = ObjectAccessor::GetPlayer(*body, killer->GetAI()->GetGUID(DATA_TROOPER_OWNER));
        if (!player)
            player = body->GetLootRecipient();
        if (!player)
            return;

        bool felstone = GetSpellInfo()->Id == SPELL_NOTIFY_VALKYR_FELSTONE;
        uint32 questId = felstone ? QUEST_WHEN_DEATH_IS_NOT_ENOUGH : QUEST_ACE_IN_THE_HOLE;
        if (player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
            return;

        Creature* valkyr = FindValkyrOf(player, felstone ? NPC_ENTHRALLED_VALKYR_GUARDIAN : NPC_ENTHRALLED_VALKYR_PET);
        if (!valkyr || !valkyr->IsWithinDistInMap(body, 60.0f))
            return;

        valkyr->CastSpell(body, felstone ? SPELL_RAISE_FORSAKEN_FELSTONE : SPELL_RAISE_FORSAKEN_ANDORHAL, true);

        static uint32 const FelstoneTroopers[] = { NPC_FT_FELSTONE_01, NPC_FT_FELSTONE_02, NPC_FT_FELSTONE_03 };
        static uint32 const AndorhalTroopers[] = { NPC_FT_ANDORHAL_01, NPC_FT_ANDORHAL_02, NPC_FT_ANDORHAL_03, NPC_FT_ANDORHAL_04 };
        uint32 trooperEntry = felstone ? FelstoneTroopers[urand(0, 2)] : AndorhalTroopers[urand(0, 3)];
        if (CreatureTemplate const* bodyTemplate = body->GetCreatureTemplate())
        {
            uint32 count = felstone ? 3 : 4;
            for (uint32 i = 0; i < count; ++i)
                if (CreatureModel const* model = bodyTemplate->GetModelByIdx(i))
                    if (model->CreatureDisplayID == body->GetNativeDisplayId())
                        trooperEntry = felstone ? FelstoneTroopers[i] : AndorhalTroopers[i];
        }

        if (TempSummon* trooper = body->SummonCreature(trooperEntry, body->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 10min))
            if (trooper->IsAIEnabled)
                trooper->AI()->SetGUID(player->GetGUID(), DATA_TROOPER_RAISER);

        // Long enough to loot; the trooper has risen beside it
        body->DespawnOrUnsummon(60 * IN_MILLISECONDS);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wpl_notify_valkyr::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 84155 - Call to Arms
// The val'kyr pet's only ability: a Forsaken Trooper the player targets joins him (see npc_wpl_forsaken_trooper)
class spell_wpl_call_to_arms : public SpellScript
{
    PrepareSpellScript(spell_wpl_call_to_arms);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Creature* trooper = GetHitCreature();
        Player* player = GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (trooper && player && trooper->IsAlive() && trooper->IsAIEnabled)
            trooper->AI()->SetGUID(player->GetGUID(), DATA_TROOPER_MOBILIZE);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wpl_call_to_arms::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 45085 - Forsaken Trooper (Felstone Field, seen with Detect: Quest Invis Zone 4)
// 45048-45050 - Forsaken Trooper (risen on Felstone Field), 45058-45061 - Forsaken Trooper (risen in Andorhal)
// A trooper the val'kyr raised gets up and salutes its master. On Felstone it then leaves; the Felstone troopers the player
// mobilizes in Combat Training are the ones already roaming the farm. In Andorhal the risen trooper waits for its master's Call to
// Arms and disappears after two minutes without it. A mobilized trooper gives the quest credit, follows its master and fights with
// him until the quest leaves his log, he goes away, or ten minutes pass.
struct npc_wpl_forsaken_trooper : public ScriptedAI
{
    npc_wpl_forsaken_trooper(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        if (!_ownerGUID)
            _events.Reset();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_PASSIVE);
        me->SetStandState(UNIT_STAND_STATE_DEAD);
        _events.ScheduleEvent(EVENT_TROOPER_STAND, 2s);
        if (!IsFelstoneRisen())
            _events.ScheduleEvent(EVENT_TROOPER_EXPIRE, 2min);
    }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id == DATA_TROOPER_RAISER)
            _raiserGUID = guid;
        else if (id == DATA_TROOPER_MOBILIZE)
            if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                Mobilize(player);
    }

    ObjectGuid GetGUID(int32 id) const override
    {
        return id == DATA_TROOPER_OWNER ? _ownerGUID : ObjectGuid::Empty;
    }

    // A static Felstone trooper keeps its AI through the respawn: it comes back unclaimed
    void JustDied(Unit* /*killer*/) override
    {
        _ownerGUID.Clear();
        _raiserGUID.Clear();
        _events.Reset();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        Player* owner = GetOwner();
        if (!owner)
        {
            ScriptedAI::EnterEvadeMode(why);
            return;
        }

        _EnterEvadeMode(why);
        me->GetMotionMaster()->MoveFollow(owner, 2.0f + frand(0.0f, 2.0f), frand(float(M_PI) * 0.5f, float(M_PI) * 1.5f));
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TROOPER_STAND:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    if (Player* raiser = ObjectAccessor::GetPlayer(*me, _raiserGUID))
                        me->SetFacingToObject(raiser);
                    _events.ScheduleEvent(EVENT_TROOPER_SALUTE, 1500ms);
                    break;
                case EVENT_TROOPER_SALUTE:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    if (IsFelstoneRisen())
                        _events.ScheduleEvent(EVENT_TROOPER_LEAVE, 3s);
                    else
                        me->SetReactState(REACT_DEFENSIVE);
                    break;
                case EVENT_TROOPER_LEAVE:
                {
                    Position pos = me->GetPosition();
                    me->MovePosition(pos, 15.0f, frand(0.0f, 2.0f * float(M_PI)));
                    me->GetMotionMaster()->MovePoint(0, pos);
                    me->DespawnOrUnsummon(8 * IN_MILLISECONDS);
                    break;
                }
                case EVENT_TROOPER_EXPIRE:
                    if (!_ownerGUID)
                        me->DespawnOrUnsummon();
                    break;
                case EVENT_TROOPER_DISMISS:
                    Release();
                    return;
                case EVENT_TROOPER_CHECK_OWNER:
                {
                    Player* owner = GetOwner();
                    if (!owner || !owner->IsAlive() || !owner->IsWithinDistInMap(me, 80.0f) || !IsInLog(owner))
                    {
                        Release();
                        return;
                    }

                    if (!me->GetVictim() && owner->IsInCombat())
                        if (Unit* victim = owner->GetVictim())
                            if (me->IsValidAttackTarget(victim))
                                AttackStart(victim);

                    _events.ScheduleEvent(EVENT_TROOPER_CHECK_OWNER, 2s);
                    break;
                }
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    bool IsFelstoneRisen() const
    {
        return me->GetEntry() == NPC_FT_FELSTONE_01 || me->GetEntry() == NPC_FT_FELSTONE_02 || me->GetEntry() == NPC_FT_FELSTONE_03;
    }

    bool IsFelstone() const { return me->GetEntry() == NPC_FORSAKEN_TROOPER_FELSTONE; }

    uint32 GetQuestId() const { return IsFelstone() ? QUEST_COMBAT_TRAINING_HORDE : QUEST_ACE_IN_THE_HOLE; }

    bool IsInLog(Player* player) const
    {
        QuestStatus status = player->GetQuestStatus(GetQuestId());
        return status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE;
    }

    Player* GetOwner() const { return _ownerGUID ? ObjectAccessor::GetPlayer(*me, _ownerGUID) : nullptr; }

    void Mobilize(Player* player)
    {
        if (_ownerGUID || IsFelstoneRisen() || me->IsInCombat())
            return;

        // A risen Andorhal trooper answers only the player whose val'kyr raised it, once it is on its feet
        if (!IsFelstone() && (player->GetGUID() != _raiserGUID || me->GetStandState() != UNIT_STAND_STATE_STAND))
            return;

        // Combat Training: only the pet of a player who has the quest can call the Felstone troopers

        if (player->GetQuestStatus(GetQuestId()) != QUEST_STATUS_INCOMPLETE)
            return;

        _ownerGUID = player->GetGUID();
        player->KilledMonsterCredit(IsFelstone() ? NPC_FELSTONE_CHARM_CREDIT : NPC_ANDORHAL_CHARM_CREDIT);

        // An invisible unit sees only those that detect its invisibility, so a Felstone trooper could attack nothing while it kept
        // 82343; its respawn gives it back
        if (IsFelstone())
            me->RemoveAurasDueToSpell(SPELL_GENERIC_QUEST_INVISIBILITY_4);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetReactState(REACT_DEFENSIVE);
        me->SetHomePosition(me->GetPosition());
        me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(player, 2.0f + frand(0.0f, 2.0f), frand(float(M_PI) * 0.5f, float(M_PI) * 1.5f));

        _events.CancelEvent(EVENT_TROOPER_EXPIRE);
        _events.ScheduleEvent(EVENT_TROOPER_CHECK_OWNER, 2s);
        _events.ScheduleEvent(EVENT_TROOPER_DISMISS, 10min);
    }

    // A static Felstone trooper goes away and comes back at its spawn after its respawn time, as one that was never called
    void Release()
    {
        _ownerGUID.Clear();
        _events.Reset();
        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _raiserGUID;
    ObjectGuid _ownerGUID;
};

/*######
## npc_anchorite_truuen
######*/

enum Truuen
{
    NPC_GHOST_UTHER             = 17233,
    NPC_THEL_DANIS              = 1854,
    NPC_GHOUL                   = 1791,     // ambush

    QUEST_TOMB_LIGHTBRINGER     = 9446,

    SAY_WP_0                    = 0,        // Beware! We are attacked!
    SAY_WP_1                    = 1,        // It must be the purity of the Mark of the Lightbringer that is drawing forth the Scourge ...
    SAY_WP_2                    = 2,        // This land truly needs to be cleansed by the Light! ...
    SAY_WP_3                    = 0,        // Thel'danis: Be welcome, friends!
    SAY_WP_4                    = 0,        // Uther: Thank you for coming here in remembrance of me. ...
    SAY_WP_5                    = 1,        // Uther: Please, rise my friend. Keep the Blessing ...
    SAY_WP_6                    = 2         // Uther: Thank you my friend for making this possible. ...
};

class npc_anchorite_truuen : public CreatureScript
{
public:
    npc_anchorite_truuen() : CreatureScript("npc_anchorite_truuen") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_TOMB_LIGHTBRINGER)
        {
            if (npc_anchorite_truuenAI* pEscortAI = CAST_AI(npc_anchorite_truuen::npc_anchorite_truuenAI, creature->AI()))
                pEscortAI->Start(true, true, player->GetGUID());
        }
        return false;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_anchorite_truuenAI(creature);
    }

    struct npc_anchorite_truuenAI : public npc_escortAI
    {
        npc_anchorite_truuenAI(Creature* creature) : npc_escortAI(creature) { }

        ObjectGuid UghostGUID;

        void Reset() override { }

        void JustSummoned(Creature* summoned) override
        {
            if (summoned->GetEntry() == NPC_GHOUL)
                summoned->AI()->AttackStart(me);
        }

        void SummonGhoul(float offset)
        {
            me->SummonCreature(NPC_GHOUL, me->GetPositionX() + offset, me->GetPositionY() + offset, me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 90s);
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();

            switch (waypointId)
            {
                case 8:
                    Talk(SAY_WP_0);
                    SummonGhoul(7.0f);
                    SummonGhoul(5.0f);
                    break;
                case 9:
                    Talk(SAY_WP_1);
                    break;
                case 14:
                    SummonGhoul(7.0f);
                    SummonGhoul(5.0f);
                    SummonGhoul(10.0f);
                    SummonGhoul(8.0f);
                    break;
                case 15:
                    Talk(SAY_WP_2);
                    break;
                case 21:
                    if (Creature* theldanis = me->FindNearestCreature(NPC_THEL_DANIS, 150.0f))
                        theldanis->AI()->Talk(SAY_WP_3);
                    break;
                case 23:
                    if (Creature* ughost = me->SummonCreature(NPC_GHOST_UTHER, 971.86f, -1825.42f, 81.99f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30s))
                    {
                        UghostGUID = ughost->GetGUID();
                        ughost->SetDisableGravity(true);
                        ughost->AI()->Talk(SAY_WP_4, me);
                    }
                    break;
                case 24:
                    if (Creature* ughost = ObjectAccessor::GetCreature(*me, UghostGUID))
                        ughost->AI()->Talk(SAY_WP_5, me);
                    break;
                case 25:
                    if (Creature* ughost = ObjectAccessor::GetCreature(*me, UghostGUID))
                        ughost->AI()->Talk(SAY_WP_6, me);
                    break;
                case 26:
                    if (player)
                        player->GroupEventHappens(QUEST_TOMB_LIGHTBRINGER, me);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_TOMB_LIGHTBRINGER);
        }
    };
};

void AddSC_western_plaguelands()
{
    new creature_script<npc_dark_master_gandling_quest>("npc_dark_master_gandling_quest");
    new npc_thassarian_quest();
    new creature_script<npc_koltira_deathweaver_andorhal>("npc_koltira_deathweaver_andorhal");
    new aura_script<spell_shadow_prison_gandling>("spell_shadow_prison_gandling");
    new npc_anchorite_truuen();
    new aura_script<spell_wpl_bond_of_the_valkyr>("spell_wpl_bond_of_the_valkyr");
    new creature_script<npc_wpl_enthralled_valkyr>("npc_wpl_enthralled_valkyr");
    new spell_script<spell_wpl_notify_valkyr>("spell_wpl_notify_valkyr");
    new spell_script<spell_wpl_call_to_arms>("spell_wpl_call_to_arms");
    new creature_script<npc_wpl_forsaken_trooper>("npc_wpl_forsaken_trooper");
}
