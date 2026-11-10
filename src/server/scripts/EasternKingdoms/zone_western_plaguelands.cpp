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
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SmartAI.h"
#include "DBCStores.h"
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

// 6235 - by Lurid's body, at the Alliance camp in western Andorhal (unused areatrigger, radius 25, around Lurid 45129, the ender)
// Ace in the Hole: "Then take a group of them into Alliance-held western Andorhal and hunt down this death knight, Thassarian."
// The hunt ends at Lurid's body (Wowhead comments: "Once you get to the end you will see the mob on the ground dead"); Thassarian
// is gone, so reaching the body gives the "Hunt Thassarian" credit (44453).
class at_wpl_lurids_body : public AreaTriggerScript
{
public:
    at_wpl_lurids_body() : AreaTriggerScript("at_wpl_lurids_body") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (player->IsAlive() && player->GetQuestStatus(QUEST_ACE_IN_THE_HOLE) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_THASSARIAN);
        return true;
    }
};

/*######
## Northridge Lumber Camp: Learning the Ropes (27000), This Means WAR (Wild Arachnid Roundup) (27001)
######*/

enum NorthridgeSpiders
{
    QUEST_LEARNING_THE_ROPES                    = 27000,
    QUEST_THIS_MEANS_WAR                        = 27001,

    NPC_NORTHRIDGE_SPIDER                       = 44284,
    NPC_HEARTHGLEN_RECRUIT                      = 44437,
    NPC_NORTHRIDGE_SPIDER_SCARE_CREDIT          = 44850,
    NPC_NORTHRIDGE_SPIDER_SCARE_KILL_CREDIT     = 44838,

    SPELL_DEADLY_POISON                         = 3583,
    SPELL_SPIDER_SLAY                           = 83621,
    SPELL_WHINNY_FEAR                           = 81442, // Fear: 4 s MOD_FEAR, the usual NPC fear visual

    DATA_SPIDER_WHINNY                          = 1,

    EVENT_SPIDER_DEADLY_POISON                  = 1,
    EVENT_SPIDER_CHECK_RECRUITS                 = 2,
    EVENT_SPIDER_FEAR_OVER                      = 3,

    POINT_SPIDER_FRIGHTENED                     = 1
};

// 83605 - Whinny
// The Hearthglen Mustang's only ability (its rider's first button): spiders in front of the horse run away from it. On Learning the
// Ropes each spider frightened counts; on This Means WAR the rider pushes them on to the Hearthglen Recruits at the lumber mill
class spell_wpl_whinny : public SpellScript
{
    PrepareSpellScript(spell_wpl_whinny);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Creature* spider = GetHitCreature();
        Player* player = GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (spider && player && spider->IsAlive() && spider->GetEntry() == NPC_NORTHRIDGE_SPIDER && spider->IsAIEnabled)
            spider->AI()->SetGUID(player->GetGUID(), DATA_SPIDER_WHINNY);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wpl_whinny::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 44284 - Northridge Spider
// A Whinny frightens it (fear aura, 4 s): it runs 18 yd away from the horse and stays about where it stopped. A spider that comes
// onto a Hearthglen Recruit, frightened there or chasing a player, is slain by Spider Slay; that counts for This Means WAR for
// every player on it within 40 yd of the spider (Wowhead comments: "you need to be about within 40 yards from the mill when the
// spiders are killed to receive credit, even if you were the one to lure them there"; "you don't need to be partied with other
// players"; "lure the spiders right on top of the NPCs").
struct npc_wpl_northridge_spider : public ScriptedAI
{
    npc_wpl_northridge_spider(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_SPIDER_CHECK_RECRUITS, 1s);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_SPIDER_DEADLY_POISON, 5s, 8s);
    }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id != DATA_SPIDER_WHINNY)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, guid);
        if (!player)
            return;

        // Each rider can count this spider once every 10 seconds
        if (player->GetQuestStatus(QUEST_LEARNING_THE_ROPES) == QUEST_STATUS_INCOMPLETE)
        {
            uint32 now = getMSTime();
            auto itr = _lastScare.find(guid);
            if (itr == _lastScare.end() || getMSTimeDiff(itr->second, now) >= 10 * IN_MILLISECONDS)
            {
                player->KilledMonsterCredit(NPC_NORTHRIDGE_SPIDER_SCARE_CREDIT);
                _lastScare[guid] = now;
            }
        }

        Unit* horse = player->GetVehicleBase();
        Frighten(horse ? horse : player);
    }

    void JustDied(Unit* killer) override
    {
        if (!killer || killer->GetEntry() != NPC_HEARTHGLEN_RECRUIT)
            return;

        std::list<Player*> players;
        GetPlayerListInGrid(players, me, 40.0f);
        for (Player* player : players)
            if (player->GetQuestStatus(QUEST_THIS_MEANS_WAR) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(NPC_NORTHRIDGE_SPIDER_SCARE_KILL_CREDIT);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SPIDER_CHECK_RECRUITS:
                    if (Creature* recruit = me->FindNearestCreature(NPC_HEARTHGLEN_RECRUIT, 5.0f))
                    {
                        recruit->SetFacingToObject(me);
                        recruit->CastSpell(me, SPELL_SPIDER_SLAY, true);
                    }
                    if (me->IsAlive())
                        _events.ScheduleEvent(EVENT_SPIDER_CHECK_RECRUITS, 1s);
                    break;
                case EVENT_SPIDER_DEADLY_POISON:
                    if (me->GetVictim())
                    {
                        DoCastVictim(SPELL_DEADLY_POISON);
                        _events.ScheduleEvent(EVENT_SPIDER_DEADLY_POISON, 15s, 18s);
                    }
                    else if (me->IsInCombat()) // frightened when it came up: try again once it fights
                        _events.ScheduleEvent(EVENT_SPIDER_DEADLY_POISON, 2s);
                    break;
                case EVENT_SPIDER_FEAR_OVER:
                    if (me->HasAuraType(SPELL_AURA_MOD_FEAR))
                        _events.ScheduleEvent(EVENT_SPIDER_FEAR_OVER, 500ms);
                    else
                        EndFright();
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    // A real fear aura from the horse (the spider shows it and is flagged fleeing), with a scripted run straight away from the
    // horse instead of the aura's own fleeing movement: that one runs only to 28 yd from the caster, so a spider hit from 20 yd
    // barely moved and riders couldn't herd them to the recruits for This Means WAR ("Use the whinny when behind the spiders").
    // The aura is added, not cast, and Whinny itself no longer starts combat (spell_custom_attribute), so the spider stays out of
    // combat: in combat with nobody to fight, it would evade, lose the fear and run home.
    void Frighten(Unit* horse)
    {
        me->CombatStop(true);
        if (IsEngaged())
            EngagementOver();
        me->SetReactState(REACT_PASSIVE);
        _events.CancelEvent(EVENT_SPIDER_DEADLY_POISON);

        // Idle underneath, so the end of the fear doesn't resume the random or waypoint movement towards the old spot
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MoveIdle();

        // A new aura brings the fleeing movement (Unit::SetFeared); a second Whinny only refreshes the aura
        horse->AddAura(SPELL_WHINNY_FEAR, me);

        // MovePositionToFirstCollision adds the spider's own facing to the angle
        Position pos = me->GetPosition();
        me->MovePositionToFirstCollision(pos, 18.0f, horse->GetAngle(me) - me->GetOrientation());
        me->SetWalk(false);
        // Same priority as the fleeing movement, so it replaces it; the fear aura and its fleeing flag stay
        me->GetMotionMaster()->MovePoint(POINT_SPIDER_FRIGHTENED, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), true,
            MOTION_SLOT_CONTROLLED);
        _events.RescheduleEvent(EVENT_SPIDER_FEAR_OVER, 500ms);
    }

    // The spider wanders around where it stopped (3 yd), so the next Whinny moves it on from there.
    // If someone attacked it meanwhile, it fights back instead (UpdateVictim picks the target).
    void EndFright()
    {
        _events.CancelEvent(EVENT_SPIDER_FEAR_OVER);
        me->SetReactState(REACT_AGGRESSIVE);

        if (me->IsInCombat())
            return;

        me->SetHomePosition(me->GetPosition());
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MoveRandom(3.0f);
    }

    EventMap _events;
    std::unordered_map<ObjectGuid, uint32> _lastScare;
};

/*######
## The Writhing Haunt: Combat Training (27174)
######*/

enum CombatTrainingAlliance
{
    QUEST_COMBAT_TRAINING_ALLIANCE              = 27174,

    NPC_GORY                                    = 45212,
    NPC_PROVINCIAL_MINUTEMAN_1                  = 45231,
    NPC_PROVINCIAL_MINUTEMAN_2                  = 45232,
    NPC_PROVINCIAL_MINUTEMAN_3                  = 45233,
    NPC_PROVINCIAL_MINUTEMAN_4                  = 45234,

    SPELL_SUMMON_PROVINCIAL_MINUTEMAN_1_PET     = 84452,
    SPELL_SUMMON_PROVINCIAL_MINUTEMAN_2         = 84456,
    SPELL_SUMMON_PROVINCIAL_MINUTEMAN_3         = 84457,
    SPELL_SUMMON_PROVINCIAL_MINUTEMAN_4         = 84458,
    SPELL_GORY_CLEAVE                           = 40504,
    SPELL_GORY_SCOURGE_HOOK                     = 50335,

    GOSSIP_MENU_GORY                            = 11953,
    GOSSIP_OPTION_GORY_CHALLENGE                = 0,

    TALK_GORY_CHALLENGED                        = 0,    // You want to fight?  Alright, Gory fight.
    TALK_GORY_GIVES_UP                          = 1,    // Gory give up!  You farmers fight more good!

    FACTION_GORY_FIGHTING                       = 974,

    EVENT_GORY_CLEAVE                           = 1,
    EVENT_GORY_SCOURGE_HOOK                     = 2,
    EVENT_MINUTEMAN_ASSIST                      = 1
};

static uint32 const ProvincialMinutemen[4][2] =
{
    { NPC_PROVINCIAL_MINUTEMAN_1, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_1_PET },
    { NPC_PROVINCIAL_MINUTEMAN_2, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_2 },
    { NPC_PROVINCIAL_MINUTEMAN_3, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_3 },
    { NPC_PROVINCIAL_MINUTEMAN_4, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_4 }
};

// 84460 - Combat Training
// Given by spell_area at the Writhing Haunt while Combat Training is in the log: keeps Thurman's four settlers with the player (the
// first one as a pet, with a pet bar, as warcraft.wiki.gg describes: "You control them with a basic pet interface"). As with the
// val'kyr, the pet gives way to a hunter's or warlock's own pet once the player has called it back.
class spell_wpl_combat_training_minutemen : public AuraScript
{
    PrepareAuraScript(spell_wpl_combat_training_minutemen);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_PROVINCIAL_MINUTEMAN_1_PET, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_2,
            SPELL_SUMMON_PROVINCIAL_MINUTEMAN_3, SPELL_SUMMON_PROVINCIAL_MINUTEMAN_4 });
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
        for (auto const& minuteman : ProvincialMinutemen)
        {
            GetTarget()->RemoveAllMinionsByEntry(minuteman[0]);
            GetTarget()->RemoveAurasDueToSpell(minuteman[1]);
        }
    }

    static void Check(Player* player, bool force)
    {
        if (!player->IsInWorld() || !player->IsAlive() || player->IsInFlight() || player->GetVehicle())
            return;

        for (auto const& minuteman : ProvincialMinutemen)
        {
            Creature* found = nullptr;
            std::list<Creature*> minions;
            player->GetAllMinionsByEntry(minions, minuteman[0]);
            for (Creature* minion : minions)
                if (minion->IsAlive() && !minion->IsDestroyedObject())
                    found = minion;

            if (found && found->IsWithinDistInMap(player, 60.0f))
                continue;

            if (minuteman[0] == NPC_PROVINCIAL_MINUTEMAN_1 && !found && !force && player->GetPetGUID())
                continue;

            if (found)
                found->DespawnOrUnsummon();

            player->CastSpell(player, minuteman[1], true);
        }
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_wpl_combat_training_minutemen::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_wpl_combat_training_minutemen::OnPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectApply += AuraEffectApplyFn(spell_wpl_combat_training_minutemen::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_wpl_combat_training_minutemen::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

enum ZenKiki
{
    NPC_ZENKIKI_ESCORT                          = 44904
};

// 83819 - Escorting Zen'Kiki
// Given by spell_area while Zen'Kiki, the Druid 26953 or Zen'Kiki and the Cultists 26955 is in the log. Its effect 0 summons
// Zen'Kiki 44904 as the player's guardian, but the summon isn't bound to the aura (effect 2, a dummy), so he stayed with the
// player after the quest was abandoned or handed in. He now goes when the aura does; death is left to the guardian's own rules.
class spell_wpl_escorting_zenkiki : public AuraScript
{
    PrepareAuraScript(spell_wpl_escorting_zenkiki);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            return;

        GetTarget()->RemoveAllMinionsByEntry(NPC_ZENKIKI_ESCORT);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_wpl_escorting_zenkiki::AfterRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 45231-45234 - Provincial Minuteman (Combat Training)
// Follows the player and joins whatever he fights, unless set to passive from the pet bar
struct npc_wpl_provincial_minuteman : public ScriptedAI
{
    npc_wpl_provincial_minuteman(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_MINUTEMAN_ASSIST, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (_events.ExecuteEvent() == EVENT_MINUTEMAN_ASSIST)
        {
            if (!me->GetVictim() && !me->HasReactState(REACT_PASSIVE))
                if (Unit* owner = me->GetCharmerOrOwner())
                    if (owner->IsInCombat())
                        if (Unit* victim = owner->GetVictim())
                            if (me->IsValidAttackTarget(victim))
                                AttackStart(victim);
            _events.ScheduleEvent(EVENT_MINUTEMAN_ASSIST, 1s);
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// 45212 - Gory
// A friendly abomination who wants to be a farmer. A player on Combat Training can challenge him from his menu; he fights that
// player and the settlers, and near the end gives up instead of dying, which completes the quest for the players fighting him.
// He is friendly again after every fight and respawn (Hillsbrad lesson: Respawn keeps a faction set by a script).
struct npc_wpl_gory : public ScriptedAI
{
    npc_wpl_gory(Creature* creature) : ScriptedAI(creature), _fighting(false), _givingUp(false) { }

    void Reset() override
    {
        _events.Reset();
        _fighting = false;
        _givingUp = false;
        me->RestoreFaction();
        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_GORY || gossipListId != GOSSIP_OPTION_GORY_CHALLENGE)
            return false;

        CloseGossipMenuFor(player);
        if (_fighting || me->IsInCombat() || player->GetQuestStatus(QUEST_COMBAT_TRAINING_ALLIANCE) != QUEST_STATUS_INCOMPLETE)
            return true;

        _fighting = true;
        Talk(TALK_GORY_CHALLENGED, player);
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->SetFaction(FACTION_GORY_FIGHTING);
        me->SetReactState(REACT_AGGRESSIVE);
        AttackStart(player);
        return true;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_GORY_CLEAVE, 5s, 8s);
        _events.ScheduleEvent(EVENT_GORY_SCOURGE_HOOK, 3s);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (!_fighting || _givingUp)
        {
            damage = 0;
            return;
        }

        if (me->GetHealth() <= damage || me->GetHealthPct() - float(damage) * 100.0f / float(me->GetMaxHealth()) < 10.0f)
        {
            if (me->GetHealth() <= damage)
                damage = me->GetHealth() - 1;
            _givingUp = true;
            me->m_Events.Schedule(1, [this]() { GiveUp(); });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GORY_CLEAVE:
                    DoCastVictim(SPELL_GORY_CLEAVE);
                    _events.ScheduleEvent(EVENT_GORY_CLEAVE, 12s, 15s);
                    break;
                case EVENT_GORY_SCOURGE_HOOK:
                    if (Unit* victim = me->GetVictim())
                        if (me->GetDistance(victim) > 8.0f && me->GetDistance(victim) < 40.0f)
                        {
                            DoCast(victim, SPELL_GORY_SCOURGE_HOOK);
                            _events.ScheduleEvent(EVENT_GORY_SCOURGE_HOOK, 10s, 12s);
                            break;
                        }
                    _events.ScheduleEvent(EVENT_GORY_SCOURGE_HOOK, 1s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    // "When defeated, he gives up and tells the farmers they fight well" (warcraft.wiki.gg); Wowhead: "I got him on 1 health and he
    // gave up". Every player fighting him with the quest in progress gets the credit.
    void GiveUp()
    {
        if (!_fighting)
            return;
        _fighting = false;

        Talk(TALK_GORY_GIVES_UP);

        std::list<Player*> players;
        GetPlayerListInGrid(players, me, 40.0f);
        for (Player* player : players)
            if (player->IsInCombatWith(me) && player->GetQuestStatus(QUEST_COMBAT_TRAINING_ALLIANCE) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(NPC_GORY);

        me->RestoreFaction();
        me->CombatStop(true);
        EnterEvadeMode();
    }

    EventMap _events;
    bool _fighting;
    bool _givingUp;
};

/*######
## Andorhal: The Endless Flow (26922 Horde, 27161 Alliance)
######*/

enum EndlessFlow
{
    NPC_STICKBONE_BERSERKER                     = 44329,

    SPELL_HAND_GRENADE                          = 83108,    // Lang's Hand Grenades 60849 / the Horde's grenade 60678
    SPELL_SUMMON_STICKBONE_BERSERKER_MISSILE    = 82875,    // random spot within 10 yd, triggers 82861 (summon 44329, 40 s)
    SPELL_ENDLESS_FLOW_KILL_CREDIT              = 83109,    // credit 44360 to players within 30 yd, kills berserkers within 35 yd

    EVENT_ANIMUS_RAISE                          = 1
};

// The pile is on the tower's ground floor, so only players this close make it raise berserkers. Every static berserker the
// sniff caught at the two towers stood within this range of its pile.
static float const AnimusPlayerRange            = 40.0f;
// "the bones spew them at a rate of about 1 every second until there are 15 or 20 there" (Wowhead, The Endless Flow 27161)
static size_t const AnimusMaxBerserkers         = 15;
static uint32 const AnimusMissileFlightMs       = 4 * IN_MILLISECONDS; // 82875 flies at 5 yd/s over at most 10 yd

// 44360 - Scourge Bone Animus Bunny
// The burning bone pile on the ground floor of each Andorhal tower. While a player is near, it raises a Stickbone Berserker
// about every second (82875 -> 82861) until 15 of its own stand; a killed one is replaced at once ("they respawn immediately
// after you kill them"). A Hand Grenade on the pile destroys it: 83109 gives the credit to the players around and kills the
// berserkers near it ("Any skellies hitting you when you do that die with it"), the thrower gets the credit even from outside
// 83109's 30 yd, and the pile comes back after its respawn time.
struct npc_wpl_scourge_bone_animus : public ScriptedAI
{
    npc_wpl_scourge_bone_animus(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_ANIMUS_RAISE, 1s);
    }

    void JustSummoned(Creature* summon) override
    {
        if (summon->GetEntry() != NPC_STICKBONE_BERSERKER)
            return;

        _berserkers.insert(summon->GetGUID());
        if (!_missiles.empty())
            _missiles.erase(_missiles.begin());
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        _berserkers.erase(summon->GetGUID());
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _berserkers.erase(summon->GetGUID());
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (!caster || spell->Id != SPELL_HAND_GRENADE || !me->IsAlive())
            return;

        DoCast(me, SPELL_ENDLESS_FLOW_KILL_CREDIT, true);
        if (Player* player = caster->GetCharmerOrOwnerPlayerOrPlayerItself())
            player->KilledMonsterCredit(me->GetEntry());

        // Berserkers out of the blast that aren't fighting go with their pile; those in a fight stay until killed or until
        // their 40 s run out after the fight. Swapped out first: each unsummon calls SummonedCreatureDespawn, which edits the set
        GuidSet berserkers;
        berserkers.swap(_berserkers);
        _missiles.clear();

        // The fighters stay counted: the pile comes back as the same creature, and it must not raise 15 more beside them
        uint32 despawned = 0;
        GuidSet fighting;
        for (ObjectGuid const& guid : berserkers)
            if (Creature* berserker = ObjectAccessor::GetCreature(*me, guid))
                if (berserker->IsAlive())
                {
                    if (berserker->IsInCombat())
                        fighting.insert(guid);
                    else
                    {
                        berserker->DespawnOrUnsummon();
                        ++despawned;
                    }
                }
        _berserkers.insert(fighting.begin(), fighting.end());

        TC_LOG_DEBUG("scripts", "npc_wpl_scourge_bone_animus: animus %u destroyed by %s, %u of its %u living berserkers despawned",
            me->GetDBTableGUIDLow(), caster->GetName().c_str(), despawned, uint32(berserkers.size()));

        _events.Reset();
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId != EVENT_ANIMUS_RAISE)
                continue;

            Prune();
            if (_berserkers.size() + _missiles.size() < AnimusMaxBerserkers && PlayerNearby())
            {
                DoCast(me, SPELL_SUMMON_STICKBONE_BERSERKER_MISSILE, true);
                _missiles.push_back(getMSTime());
            }
            _events.ScheduleEvent(EVENT_ANIMUS_RAISE, 1s);
        }
    }

private:
    // Drops berserkers that are gone or dead and missiles that never summoned. 82861 gives its summons a 40 s timer that also
    // runs in a fight (TEMPSUMMON_TIMED_DESPAWN, set after JustSummoned); TEMPSUMMON_TIMED_OR_DEAD_DESPAWN restarts it while
    // they fight, so a berserker only goes 40 s after its last fight
    void Prune()
    {
        for (auto itr = _berserkers.begin(); itr != _berserkers.end();)
        {
            Creature* berserker = ObjectAccessor::GetCreature(*me, *itr);
            if (!berserker || !berserker->IsInWorld() || !berserker->IsAlive())
            {
                itr = _berserkers.erase(itr);
                continue;
            }

            if (TempSummon* summon = berserker->ToTempSummon())
                summon->SetTempSummonType(TEMPSUMMON_TIMED_OR_DEAD_DESPAWN);
            ++itr;
        }

        uint32 now = getMSTime();
        while (!_missiles.empty() && getMSTimeDiff(_missiles.front(), now) > AnimusMissileFlightMs)
            _missiles.erase(_missiles.begin());
    }

    bool PlayerNearby() const
    {
        std::list<Player*> players;
        GetPlayerListInGrid(players, me, AnimusPlayerRange);
        for (Player* player : players)
            if (player->IsAlive() && !player->IsGameMaster() && player->InSamePhase(me))
                return true;
        return false;
    }

    EventMap _events;
    GuidSet _berserkers;
    std::vector<uint32> _missiles;
};

/*######
## Andorhal: Araj the Summoner (26925 Horde, 27164 Alliance)
######*/

enum ArajTheSummoner
{
    SPELL_ARAJ_FROSTBOLT                        = 9672,
    SPELL_ARAJ_FROST_NOVA                       = 75062,
    SPELL_ARAJ_SUMMON_BERSERKERS                = 83701,    // dummy on himself, "server-side script" (Wowhead)
    SPELL_ARAJ_PHYLACTERY                       = 18661,    // summons Araj's Phylactery 177241 (shard 17114)

    EVENT_ARAJ_FROSTBOLT                        = 1,
    EVENT_ARAJ_SUMMON_BERSERKERS                = 2
};

// Ours: neither the DBC nor any source gives the size of a batch. Comments speak of a "batch" of skeletons that one grenade
// kills, and a retail video shows a small group, so three per cast. A batch lives 40 s and the next comes 50-65 s later, so the
// cap of six is only a safety net: it is never reached in a normal fight
static uint32 const ArajBerserkersPerCast       = 3;
static size_t const ArajMaxBerserkers           = 6;
static float const ArajFrostNovaRange           = 8.0f;

// 1852 - Araj the Summoner
// The TDB (4.4.2 and 12.1.0) SmartAI, ported as is: Frostbolt every 3.4-4.7 s standing still while the target is in range
// (SMARTCAST_COMBAT_MOVE), Frost Nova within 8 yd every 15-25 s, Summon Berserkers 83701 after 18-23 s and then every 50-65 s,
// Araj's Phylactery on death. 83701 is a bare dummy, so here it fires Summon Stickbone Berserker: Missile 82875 (random spot
// within 10 yd, triggers 82861: 44329 for 40 s), the zone's only spell that summons a berserker. "When you fight Araj he/she
// will summon skeletons periodically", "when his first batch of Berserkers were summoned, I tossed a grenade, killing them
// all" (Wowhead, Araj the Summoner 27164). The berserkers join his fight and outlive him ("it's looting before he respawns,
// with all those skeletons beating on you", 26925); they go with him only when he evades. Only his own summons reach this
// JustSummoned (82861's summoner is whoever cast 82875), so the bone piles' berserkers are never counted or despawned here.
struct npc_wpl_araj_the_summoner : public ScriptedAI
{
    npc_wpl_araj_the_summoner(Creature* creature) : ScriptedAI(creature), _combatMove(true), _frostNovaTimer(0) { }

    void Reset() override
    {
        _events.Reset();
        _missiles.clear();
        _frostNovaTimer = 0;
        _combatMove = true;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_ARAJ_FROSTBOLT, 0ms);
        _events.ScheduleEvent(EVENT_ARAJ_SUMMON_BERSERKERS, 18s, 23s);
    }

    // A new target while he stands keeps him standing until the next Frostbolt checks the range again, as in SmartAI
    void AttackStart(Unit* who) override
    {
        if (_combatMove)
            ScriptedAI::AttackStart(who);
        else
            AttackStartNoMove(who);
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (caster != me || spell->Id != SPELL_ARAJ_SUMMON_BERSERKERS)
            return;

        Prune();
        if (_berserkers.size() + _missiles.size() >= ArajMaxBerserkers)
            return;

        uint32 count = std::min<uint32>(ArajBerserkersPerCast, uint32(ArajMaxBerserkers - _berserkers.size() - _missiles.size()));
        for (uint32 i = 0; i < count; ++i)
        {
            DoCast(me, SPELL_SUMMON_STICKBONE_BERSERKER_MISSILE, true);
            _missiles.push_back(getMSTime());
        }
    }

    void JustSummoned(Creature* summon) override
    {
        if (summon->GetEntry() != NPC_STICKBONE_BERSERKER)
            return;

        if (!_missiles.empty())
            _missiles.erase(_missiles.begin());

        // A missile still flying when he evaded lands after it: that berserker would only stand idle
        if (me->IsInEvadeMode())
        {
            summon->DespawnOrUnsummon();
            return;
        }

        _berserkers.insert(summon->GetGUID());

        if (Unit* victim = me->GetVictim())
            if (summon->IsAIEnabled)
                summon->AI()->AttackStart(victim);
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        _berserkers.erase(summon->GetGUID());
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _berserkers.erase(summon->GetGUID());
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // Swapped out first: each unsummon calls SummonedCreatureDespawn, which edits the set
        GuidSet berserkers;
        berserkers.swap(_berserkers);
        for (ObjectGuid const& guid : berserkers)
            if (Creature* berserker = ObjectAccessor::GetCreature(*me, guid))
                if (berserker->IsAlive())
                    berserker->DespawnOrUnsummon();
        _missiles.clear();

        ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _events.Reset();
        me->InterruptNonMeleeSpells(false);
        DoCast(me, SPELL_ARAJ_PHYLACTERY, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (_frostNovaTimer > diff)
            _frostNovaTimer -= diff;
        else
            _frostNovaTimer = 0;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (!_frostNovaTimer && me->IsWithinDistInMap(me->GetVictim(), ArajFrostNovaRange))
        {
            DoCast(me, SPELL_ARAJ_FROST_NOVA);
            _frostNovaTimer = urand(15000, 25000);
            return;
        }

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ARAJ_FROSTBOLT:
                    UpdateCombatMove(me->GetVictim());
                    DoCastVictim(SPELL_ARAJ_FROSTBOLT);
                    _events.ScheduleEvent(EVENT_ARAJ_FROSTBOLT, 3400ms, 4700ms);
                    break;
                case EVENT_ARAJ_SUMMON_BERSERKERS:
                    DoCast(me, SPELL_ARAJ_SUMMON_BERSERKERS);
                    _events.ScheduleEvent(EVENT_ARAJ_SUMMON_BERSERKERS, 50s, 65s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    // SMARTCAST_COMBAT_MOVE as SmartAI does it: stand while the target is in Frostbolt range and sight and the mana lasts,
    // otherwise chase it
    void UpdateCombatMove(Unit* victim)
    {
        SpellInfo const* frostbolt = sSpellMgr->GetSpellInfo(SPELL_ARAJ_FROSTBOLT);
        if (!victim || !frostbolt)
            return;

        bool move = me->GetDistance(victim) > frostbolt->GetMaxRange(false) || !me->IsWithinLOSInMap(victim)
            || me->GetPower(POWER_MANA) < frostbolt->CalcPowerCost(me, frostbolt->GetSchoolMask());
        if (move == _combatMove)
            return;

        _combatMove = move;
        if (move)
            me->GetMotionMaster()->MoveChase(victim);
        else
        {
            me->StopMoving();
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
        }
    }

    // Drops berserkers that are gone or dead and missiles that never summoned
    void Prune()
    {
        for (auto itr = _berserkers.begin(); itr != _berserkers.end();)
        {
            Creature* berserker = ObjectAccessor::GetCreature(*me, *itr);
            if (!berserker || !berserker->IsInWorld() || !berserker->IsAlive())
                itr = _berserkers.erase(itr);
            else
                ++itr;
        }

        uint32 now = getMSTime();
        while (!_missiles.empty() && getMSTimeDiff(_missiles.front(), now) > AnimusMissileFlightMs)
            _missiles.erase(_missiles.begin());
    }

    EventMap _events;
    GuidSet _berserkers;
    std::vector<uint32> _missiles;
    bool _combatMove;
    uint32 _frostNovaTimer;
};

/*######
## Andorhal: Opengut Behemoth (first battle, phase 259)
######*/

enum OpengutBehemoth
{
    SPELL_OPENGUT_CLEAVE                        = 83136,    // physical damage + knockback, no source casts it
    FACTION_SCOURGE_PLAYERS_ONLY                = 974       // the sniffed faction: hostile only to players
};

static uint32 const OpengutCleaveFirstMin       = 3000;
static uint32 const OpengutCleaveFirstMax       = 5000;
static uint32 const OpengutCleaveRepeatMin      = 6000;
static uint32 const OpengutCleaveRepeatMax      = 9000;

// Only NPCs nobody plays: soldiers, never players, their pets, guardians, companions or vehicles
static bool IsOpengutCleaveNpcTarget(Unit const* target)
{
    return target && target->GetTypeId() == TYPEID_UNIT && !target->IsControlledByPlayer() && !target->IsCharmedOwnedByPlayerOrPlayer();
}

// 44562 - Opengut Behemoth; Cleave 40504, Scourge Hook 50335 and the attacks on the soldiers stay in its SmartAI
// Retail (a video the user found; Wowhead, The Battle for Andorhal 27159: "the abominations will, after a few seconds, use a
// one-shot ability on horde/alliance NPCs - but never on players"; War Machines 26923: "throwing my deathguards around like
// ragdolls"): a Behemoth fighting a soldier one-shots it. The spell is our inference: Opengut Cleave 83136 ("Inflicts physical
// damage, knocking the target back"), next to Summon Andorhal Deathguard/Defender 83121/83135 in the DBC. The template uses
// faction 1988 so it can fight the soldiers; it still starts a fight on sight only with what faction 974 is hostile to (players
// and their units), so it never wanders into quest NPCs or other Alliance and Horde NPCs. Fights with soldiers come from its
// SmartAI (the nearest within 5 yd, out of combat) or from soldiers that attack it.
struct npc_wpl_opengut_behemoth : public SmartAI
{
    npc_wpl_opengut_behemoth(Creature* creature) : SmartAI(creature), _cleaveTimer(0) { }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who)
            return;

        if (!who->IsControlledByPlayer())
        {
            FactionTemplateEntry const* sniffed = sFactionTemplateStore.LookupEntry(FACTION_SCOURGE_PLAYERS_ONLY);
            FactionTemplateEntry const* theirs = who->GetFactionTemplateEntry();
            if (!sniffed || !theirs || !sniffed->IsHostileTo(*theirs))
                return;
        }

        SmartAI::MoveInLineOfSight(who);
    }

    void Reset() override
    {
        SmartAI::Reset();
        _cleaveVictim.Clear();
        _cleaveTimer = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        SmartAI::UpdateAI(diff);

        if (!me->IsAlive() || !me->IsInCombat())
            return;

        Unit* victim = me->GetVictim();
        if (!IsOpengutCleaveNpcTarget(victim))
        {
            _cleaveVictim.Clear();
            return;
        }

        // "after a few seconds" of fighting each new soldier
        if (victim->GetGUID() != _cleaveVictim)
        {
            _cleaveVictim = victim->GetGUID();
            _cleaveTimer = urand(OpengutCleaveFirstMin, OpengutCleaveFirstMax);
            return;
        }

        if (_cleaveTimer > diff)
        {
            _cleaveTimer -= diff;
            return;
        }

        if (me->HasUnitState(UNIT_STATE_CASTING) || !me->IsWithinMeleeRange(victim))
            return;

        DoCastVictim(SPELL_OPENGUT_CLEAVE);
        _cleaveTimer = urand(OpengutCleaveRepeatMin, OpengutCleaveRepeatMax);
    }

private:
    ObjectGuid _cleaveVictim;
    uint32 _cleaveTimer;
};

// 83136 - Opengut Cleave
// The one-shot: an NPC target dies on the hit, whatever its health and its 85% sparring limit (which caps every NPC's damage).
// Any other target (none: the Behemoth casts it only on NPCs) takes the spell's normal damage and knockback.
class spell_wpl_opengut_cleave : public SpellScript
{
    PrepareSpellScript(spell_wpl_opengut_cleave);

    void HandleDamage(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive() || !IsOpengutCleaveNpcTarget(target))
            return;

        PreventHitDefaultEffect(effIndex);
        caster->Kill(target, false, GetSpellInfo());
    }

    // No knockback for a corpse
    void HandleKnockBack(SpellEffIndex effIndex)
    {
        if (Unit* target = GetHitUnit())
            if (!target->IsAlive())
                PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wpl_opengut_cleave::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(spell_wpl_opengut_cleave::HandleKnockBack, EFFECT_1, SPELL_EFFECT_KNOCK_BACK);
    }
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
    new at_wpl_lurids_body();
    new spell_script<spell_wpl_whinny>("spell_wpl_whinny");
    new creature_script<npc_wpl_northridge_spider>("npc_wpl_northridge_spider");
    new aura_script<spell_wpl_combat_training_minutemen>("spell_wpl_combat_training_minutemen");
    new aura_script<spell_wpl_escorting_zenkiki>("spell_wpl_escorting_zenkiki");
    new creature_script<npc_wpl_provincial_minuteman>("npc_wpl_provincial_minuteman");
    new creature_script<npc_wpl_gory>("npc_wpl_gory");
    new creature_script<npc_wpl_scourge_bone_animus>("npc_wpl_scourge_bone_animus");
    new creature_script<npc_wpl_araj_the_summoner>("npc_wpl_araj_the_summoner");
    new creature_script<npc_wpl_opengut_behemoth>("npc_wpl_opengut_behemoth");
    new spell_script<spell_wpl_opengut_cleave>("spell_wpl_opengut_cleave");
}
