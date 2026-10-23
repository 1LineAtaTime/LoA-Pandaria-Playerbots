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

enum eQuests
{
    QUEST_SCHOLOMANCER = 27162,
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
            pList.remove_if([=](Player* target) { return target && target->GetQuestStatus(QUEST_SCHOLOMANCER) != QUEST_STATUS_INCOMPLETE; });

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
    new aura_script<spell_shadow_prison_gandling>("spell_shadow_prison_gandling");
    new npc_anchorite_truuen();
}
