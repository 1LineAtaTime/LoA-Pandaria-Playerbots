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
#include "Group.h"
#include "MoveSpline.h"
#include "Vehicle.h"

/*######
## Scholomancer (26924 Horde, 27162 Alliance): Darkmaster Gandling, Koltira and Thassarian
######*/

// Sourced: the Shadow Prison spells and NPC from the 5.4.8 DBCs and both TDBs: 86797 summons the Shadow Prison 46558 (an invisible
// vehicle, Vehicle 1204, one seat, no exit; 15 s), 86826 is its Ride Vehicle, 86780 "Imprisons the target. Surrounded by a shadowy
// prison." (stun, 15 s). The lines from broadcast_text 45042-45056 (in that order), Gandling's three groups from creature_text, and the
// spots where Koltira and Thassarian end up from the sniffs (TDB 442/1210 guids 328338/328339, also TCPP). Wowhead comments (27162):
// "When you get Gandling to about 50% hp, he will lift you in the air, and Koltira and Thassarian will come and finish him off, after
// he teleports away, Koltira and Thassarian will have the following conversation: ..." (the whole conversation, Koltira whispering
// the agreement to a Horde player); "After Gandling disappears, Thassarian whispered me to not tell anyone of their 'secret
// agreement'" (Alliance).
// Ours: the scene runs on Gandling (shared, so every player near him sees it); the timings, the heights, where the death knights come
// from, the laughs given to Koltira, credit at the moment of the prison, the teleport visuals, and Gandling hidden (not despawned)
// until the death knights leave, so the scene keeps its director.
enum Scholomancer
{
    QUEST_SCHOLOMANCER_HORDE        = 26924,
    QUEST_SCHOLOMANCER              = 27162,

    NPC_THASSARIAN                  = 44453,
    NPC_KOLTIRA_DEATHVIEWER         = 44452,
    NPC_DARK_MASTER_GANDLING        = 44323,
    NPC_GANDLING_SHADOW_PRISON      = 46558,

    SPELL_SHADOWBOLT                = 79932,
    SPELL_SHADOW_ERUPTION           = 79932,
    SPELL_SHADOW_PRISON             = 86780,
    SPELL_TELEPORT_VISUAL_EVIL      = 52233,
    SPELL_DK_TELEPORT_VISUAL        = 51347,

    // Darkmaster Gandling 44323
    SAY_GANDLING_AGGRO              = 0,    // Prepare to die, $r!
    SAY_GANDLING_FORFEIT            = 1,    // Your life is forfeit.
    SAY_GANDLING_ENOUGH             = 2,    // Enough of this.  The Cult of the Damned doesn't need Andorhal...

    // Koltira Deathweaver 44452 and Thassarian 44453
    SAY_DK_QUEST_ACCEPT             = 0,    // Gandling is strong, but do not worry...
    SAY_DK_BATTLE_CRY               = 1,    // For the Horde! / For the Alliance!

    SAY_KOLTIRA_HEARD_CORRECTLY     = 2,
    EMOTE_KOLTIRA_LAUGH             = 3,
    SAY_KOLTIRA_NOT_TODAY           = 4,
    SAY_KOLTIRA_GO_EASY             = 5,
    EMOTE_KOLTIRA_LAUGH_2           = 6,
    WHISPER_KOLTIRA_AGREEMENT       = 7,

    SAY_THASSARIAN_RUMORS           = 2,
    SAY_THASSARIAN_COME_TO_THAT     = 3,
    SAY_THASSARIAN_SAME             = 4,
    SAY_THASSARIAN_BROTHERS         = 5,
    SAY_THASSARIAN_EVENTUALLY       = 6,
    SAY_THASSARIAN_LOOK_FORWARD     = 7,
    WHISPER_THASSARIAN_AGREEMENT    = 8,

    EVENT_SHADOW_BOLT               = 1,
    EVENT_SHADOW_ERUPTION,

    // The scene, in order
    SCENE_PRISON                    = 1,
    SCENE_STRIKE,
    SCENE_ENOUGH,
    SCENE_TELEPORT,
    SCENE_VANISH,
    SCENE_RUMORS,
    SCENE_HEARD_CORRECTLY,
    SCENE_LAUGH,
    SCENE_COME_TO_THAT,
    SCENE_NOT_TODAY,
    SCENE_SAME,
    SCENE_BROTHERS,
    SCENE_EVENTUALLY,
    SCENE_GO_EASY,
    SCENE_LAUGH_2,
    SCENE_LOOK_FORWARD,
    SCENE_WHISPER,
    SCENE_DK_TELEPORT,
    SCENE_END,

    POINT_DK_STRIKE                 = 1,
    POINT_DK_TALK                   = 2,
};

// Where the death knights stand to talk (sniffed: TDB 442/1210 guids 328338 and 328339, facing each other)
Position const KoltiraTalkSpot     = { 1288.65f, -1536.47f, 58.7686f, 1.18098f };
Position const ThassarianTalkSpot  = { 1291.0f,  -1530.75f, 58.7618f, 4.32258f };

uint8 const GandlingPrisonPct = 50;         // Wowhead comment: "about 50% hp"

static bool IsShadowPrisonPassenger(Unit const* unit)
{
    Unit const* base = unit ? unit->GetVehicleBase() : nullptr;
    return base && base->GetEntry() == NPC_GANDLING_SHADOW_PRISON;
}

// Dark Master Gandling 44323
// Can't be killed: at half health he imprisons everyone fighting him (credit for them and their group), Koltira and Thassarian come to
// finish him, he leaves, they talk and leave, and only then does he despawn (respawn from his spawntimesecs) for the next player.
struct npc_dark_master_gandling_quest : public ScriptedAI
{
    npc_dark_master_gandling_quest(Creature* creature) : ScriptedAI(creature), _summons(creature) { }

    void Reset() override
    {
        SetCombatMovement(false);
        events.Reset();
        _scene.Reset();
        _sceneStarted = false;
        _witnesses.clear();
        _prisoners.clear();
        _summons.DespawnAll();
        _koltiraGUID.Clear();
        _thassarianGUID.Clear();
        _strikes = 0;
        me->SetVisible(true);
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_PACIFIED | UNIT_FLAG_NOT_SELECTABLE);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_GANDLING_AGGRO);
        events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(1.5 * IN_MILLISECONDS, 3 * IN_MILLISECONDS));
        events.ScheduleEvent(EVENT_SHADOW_ERUPTION, 12 * IN_MILLISECONDS);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_sceneStarted)
            return;
        ScriptedAI::EnterEvadeMode(why);
    }

    void AttackStart(Unit* who) override
    {
        if (_sceneStarted)
            return;
        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_sceneStarted)
            return;

        // Back after his respawn, he leaves alone the players who already have their credit and are still standing there
        if (Player* player = who->ToPlayer())
            for (uint32 questId : { uint32(QUEST_SCHOLOMANCER_HORDE), uint32(QUEST_SCHOLOMANCER) })
            {
                QuestStatus status = player->GetQuestStatus(questId);
                if (status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_REWARDED)
                    return;
            }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    // Only players take him to the prison; the soldiers fighting around him can't (they would start the scene for nobody)
    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (_sceneStarted)
        {
            damage = 0;
            return;
        }

        if (!attacker || !attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            if (me->HealthBelowPctDamaged(GandlingPrisonPct + 1, damage))
                damage = 0;
            return;
        }

        if (me->HealthBelowPctDamaged(GandlingPrisonPct, damage))
        {
            if (damage >= me->GetHealth())
                damage = me->GetHealth() - 1;

            // Not from inside the damage code: the prison stops combat and seats players
            _sceneStarted = true;
            _scene.ScheduleEvent(SCENE_PRISON, 0);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_sceneStarted)
        {
            _scene.Update(diff);
            while (uint32 eventId = _scene.ExecuteEvent())
                RunScene(eventId);
            return;
        }

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
    void RunScene(uint32 eventId)
    {
        Creature* koltira = ObjectAccessor::GetCreature(*me, _koltiraGUID);
        Creature* thassarian = ObjectAccessor::GetCreature(*me, _thassarianGUID);

        switch (eventId)
        {
            case SCENE_PRISON:
                Imprison();
                _scene.ScheduleEvent(SCENE_STRIKE, 5s);
                break;
            case SCENE_STRIKE:
                for (Creature* dk : { koltira, thassarian })
                {
                    if (!dk)
                        continue;
                    dk->SetFacingToObject(me);
                    dk->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                }
                if (++_strikes < 4)
                    _scene.ScheduleEvent(SCENE_STRIKE, 2s);
                else
                    _scene.ScheduleEvent(SCENE_ENOUGH, 1s);
                break;
            case SCENE_ENOUGH:
                Talk(SAY_GANDLING_ENOUGH);
                _scene.ScheduleEvent(SCENE_TELEPORT, 3s);
                break;
            case SCENE_TELEPORT:
                me->CastSpell(me, SPELL_TELEPORT_VISUAL_EVIL, true);
                ReleasePrisoners();
                _scene.ScheduleEvent(SCENE_VANISH, 1s);
                break;
            case SCENE_VANISH:
                me->SetVisible(false);
                if (koltira)
                {
                    koltira->SetWalk(true);
                    koltira->GetMotionMaster()->MovePoint(POINT_DK_TALK, KoltiraTalkSpot);
                }
                if (thassarian)
                {
                    thassarian->SetWalk(true);
                    thassarian->GetMotionMaster()->MovePoint(POINT_DK_TALK, ThassarianTalkSpot);
                }
                _scene.ScheduleEvent(SCENE_RUMORS, 6s);
                break;
            case SCENE_RUMORS:
                FaceEachOther(koltira, thassarian);
                Say(thassarian, SAY_THASSARIAN_RUMORS);
                _scene.ScheduleEvent(SCENE_HEARD_CORRECTLY, 5s);
                break;
            case SCENE_HEARD_CORRECTLY:
                Say(koltira, SAY_KOLTIRA_HEARD_CORRECTLY);
                _scene.ScheduleEvent(SCENE_LAUGH, 5s);
                break;
            case SCENE_LAUGH:
                Say(koltira, EMOTE_KOLTIRA_LAUGH);
                if (koltira)
                    koltira->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                _scene.ScheduleEvent(SCENE_COME_TO_THAT, 3s);
                break;
            case SCENE_COME_TO_THAT:
                Say(thassarian, SAY_THASSARIAN_COME_TO_THAT);
                _scene.ScheduleEvent(SCENE_NOT_TODAY, 4s);
                break;
            case SCENE_NOT_TODAY:
                Say(koltira, SAY_KOLTIRA_NOT_TODAY);
                _scene.ScheduleEvent(SCENE_SAME, 7s);
                break;
            case SCENE_SAME:
                Say(thassarian, SAY_THASSARIAN_SAME);
                _scene.ScheduleEvent(SCENE_BROTHERS, 4s);
                break;
            case SCENE_BROTHERS:
                Say(thassarian, SAY_THASSARIAN_BROTHERS);
                _scene.ScheduleEvent(SCENE_EVENTUALLY, 7s);
                break;
            case SCENE_EVENTUALLY:
                Say(thassarian, SAY_THASSARIAN_EVENTUALLY);
                _scene.ScheduleEvent(SCENE_GO_EASY, 7s);
                break;
            case SCENE_GO_EASY:
                Say(koltira, SAY_KOLTIRA_GO_EASY);
                _scene.ScheduleEvent(SCENE_LAUGH_2, 4s);
                break;
            case SCENE_LAUGH_2:
                Say(koltira, EMOTE_KOLTIRA_LAUGH_2);
                if (koltira)
                    koltira->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                _scene.ScheduleEvent(SCENE_LOOK_FORWARD, 3s);
                break;
            case SCENE_LOOK_FORWARD:
                Say(thassarian, SAY_THASSARIAN_LOOK_FORWARD);
                _scene.ScheduleEvent(SCENE_WHISPER, 4s);
                break;
            case SCENE_WHISPER:
                // Each side's commander swears his own players to silence
                for (ObjectGuid const& guid : _witnesses)
                {
                    Player* player = ObjectAccessor::GetPlayer(*me, guid);
                    if (!player || !player->IsWithinDistInMap(me, 100.0f))
                        continue;
                    if (player->GetTeam() == HORDE)
                    {
                        if (koltira)
                            koltira->AI()->Talk(WHISPER_KOLTIRA_AGREEMENT, player);
                    }
                    else if (thassarian)
                        thassarian->AI()->Talk(WHISPER_THASSARIAN_AGREEMENT, player);
                }
                _scene.ScheduleEvent(SCENE_DK_TELEPORT, 4s);
                break;
            case SCENE_DK_TELEPORT:
                for (Creature* dk : { koltira, thassarian })
                {
                    if (!dk)
                        continue;
                    dk->CastSpell(dk, SPELL_DK_TELEPORT_VISUAL, true);
                    dk->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                }
                _scene.ScheduleEvent(SCENE_END, 2s);
                break;
            case SCENE_END:
                _summons.DespawnAll();
                me->DespawnOrUnsummon();    // back after spawntimesecs; Reset() on respawn makes him whole and visible again
                break;
            default:
                break;
        }
    }

    // Everyone fighting him (pets count for their owner) goes into a Shadow Prison; they and their group get the credit
    void Imprison()
    {
        me->InterruptNonMeleeSpells(true);
        events.Reset();

        std::set<ObjectGuid> fighters;
        for (ThreatReference const* ref : me->GetThreatManager().GetUnsortedThreatList())
            if (Unit* victim = ref->GetVictim())
                if (Player* player = victim->GetCharmerOrOwnerPlayerOrPlayerItself())
                    if (player->IsAlive() && player->IsInWorld() && player->IsWithinDistInMap(me, 60.0f))
                        fighters.insert(player->GetGUID());

        // Leave the fight before the flags: combat would drop IMMUNE_TO_PC again
        me->CombatStop(true);
        me->GetThreatManager().ClearAllThreat();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_PACIFIED | UNIT_FLAG_NOT_SELECTABLE);

        Talk(SAY_GANDLING_FORFEIT);

        for (ObjectGuid const& guid : fighters)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, guid);
            if (!player)
                continue;

            // Witnesses: whoever this kill credits (the fighter and the group members in reward range) with either quest open
            if (Group* group = player->GetGroup())
            {
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                    if (Player* member = itr->GetSource())
                        if (member->IsAtGroupRewardDistance(me) && HasScholomancer(member))
                            _witnesses.insert(member->GetGUID());
            }
            else if (HasScholomancer(player))
                _witnesses.insert(player->GetGUID());

            player->RewardPlayerAndGroupAtEvent(NPC_DARK_MASTER_GANDLING, me);

            if (player->IsOnVehicle() || player->IsInFlight())
                continue;

            player->RemoveAurasByType(SPELL_AURA_MOUNTED);
            Position pos = player->GetPosition();
            pos.m_positionZ += 6.0f;                // the sniffed prison hung 7 yd over the square
            if (TempSummon* prison = me->SummonCreature(NPC_GANDLING_SHADOW_PRISON, pos, TEMPSUMMON_TIMED_DESPAWN, 25s))
            {
                player->EnterVehicle(prison, 0, true);     // the prison puts its Shadow Prison aura on the player once seated
                _prisoners.insert(player->GetGUID());
            }
        }

        _koltiraGUID = SummonDeathKnight(NPC_KOLTIRA_DEATHVIEWER, KoltiraTalkSpot);
        _thassarianGUID = SummonDeathKnight(NPC_THASSARIAN, ThassarianTalkSpot);
    }

    // They come running from beyond their talking spots, shout, and close in on him
    ObjectGuid SummonDeathKnight(uint32 entry, Position const& talkSpot)
    {
        float angle = me->GetAngle(&talkSpot);
        float x = talkSpot.GetPositionX() + 15.0f * std::cos(angle);
        float y = talkSpot.GetPositionY() + 15.0f * std::sin(angle);
        float z = talkSpot.GetPositionZ();
        me->UpdateGroundPositionZ(x, y, z);

        TempSummon* dk = me->SummonCreature(entry, x, y, z, angle + float(M_PI), TEMPSUMMON_TIMED_DESPAWN, 150s);
        if (!dk)
            return ObjectGuid::Empty;

        dk->AI()->Talk(SAY_DK_BATTLE_CRY);
        dk->SetWalk(false);
        Position strike;
        me->GetNearPoint2D(nullptr, strike.m_positionX, strike.m_positionY, 2.5f, angle);
        strike.m_positionZ = me->GetPositionZ();
        dk->GetMotionMaster()->MovePoint(POINT_DK_STRIKE, strike);
        return dk->GetGUID();
    }

    // Let them out while the prisons are still in the world (set down under them), then remove what is left
    void ReleasePrisoners()
    {
        for (ObjectGuid const& guid : _prisoners)
            if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
            {
                player->RemoveAurasDueToSpell(SPELL_SHADOW_PRISON);
                if (IsShadowPrisonPassenger(player))
                    player->ExitVehicle();
            }
        _prisoners.clear();

        std::list<ObjectGuid> prisons(_summons.begin(), _summons.end());
        for (ObjectGuid const& guid : prisons)
            if (Creature* prison = ObjectAccessor::GetCreature(*me, guid))
                if (prison->GetEntry() == NPC_GANDLING_SHADOW_PRISON)
                    prison->DespawnOrUnsummon();
    }

    static bool HasScholomancer(Player const* player)
    {
        return player->GetQuestStatus(QUEST_SCHOLOMANCER) == QUEST_STATUS_INCOMPLETE
            || player->GetQuestStatus(QUEST_SCHOLOMANCER_HORDE) == QUEST_STATUS_INCOMPLETE;
    }

    static void Say(Creature* speaker, uint8 group)
    {
        if (speaker)
            speaker->AI()->Talk(group);
    }

    static void FaceEachOther(Creature* a, Creature* b)
    {
        if (!a || !b)
            return;
        a->SetFacingToObject(b);
        b->SetFacingToObject(a);
    }

    EventMap events;
    EventMap _scene;
    SummonList _summons;
    std::set<ObjectGuid> _witnesses;
    std::set<ObjectGuid> _prisoners;
    ObjectGuid _koltiraGUID;
    ObjectGuid _thassarianGUID;
    uint8 _strikes = 0;
    bool _sceneStarted = false;
};

// Koltira Deathweaver 44452 (the Forsaken camp in Andorhal) and Thassarian 44453 (the Alliance camps)
// Both give Scholomancer with the same promise: "Gandling is strong, but do not worry. I will join you shortly after you begin to fight
// him." Gandling summons copies of both for his scene: those are actors only (no quests, no gossip, never in combat; the camp copies
// are the quest givers), voiced by Gandling's script, and gone with him.
struct npc_wpl_andorhal_death_knight : public ScriptedAI
{
    npc_wpl_andorhal_death_knight(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _actor = true;
        _directorGUID = summoner->GetGUID();
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, 0);
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS + 1, 0);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
    }

    // A scene copy is visible to clients before IsSummonedBy clears its npcflags: it never offers a quest
    Optional<QuestGiverStatus> GetDialogStatus(Player* /*player*/) override
    {
        if (_actor || me->IsSummon())
            return QuestGiverStatus::None;
        return {};
    }

    void OnQuestAccept(Player* /*player*/, Quest const* quest) override
    {
        if (!_actor &&(quest->GetQuestId() == QUEST_SCHOLOMANCER_HORDE || quest->GetQuestId() == QUEST_SCHOLOMANCER))
            Talk(SAY_DK_QUEST_ACCEPT);
    }

    void AttackStart(Unit* who) override
    {
        if (_actor || me->GetEntry() == NPC_THASSARIAN)
            return;
        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (_actor || me->GetEntry() == NPC_THASSARIAN)
            return;
        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_actor)
            return;
        ScriptedAI::EnterEvadeMode(why);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_actor)
        {
            // The camp Thassarian never fought (immune to everyone); the camp Koltira does
            if (me->GetEntry() != NPC_THASSARIAN)
                ScriptedAI::UpdateAI(diff);
            return;
        }

        // An actor without its scene (Gandling gone or reset) leaves
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 2 * IN_MILLISECONDS;

        Creature* director = ObjectAccessor::GetCreature(*me, _directorGUID);
        if (!director || !director->IsAlive() || !director->IsInWorld())
            me->DespawnOrUnsummon();
    }

private:
    ObjectGuid _directorGUID;
    uint32 _checkTimer = 2 * IN_MILLISECONDS;
    bool _actor = false;
};

// 46558 - Shadow Prison (Gandling's): holds one player in the air until Gandling lets go
struct npc_wpl_gandling_shadow_prison : public ScriptedAI
{
    npc_wpl_gandling_shadow_prison(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->SetCanFly(true);
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply)
        {
            _boarded = true;
            passenger->AddAura(SPELL_SHADOW_PRISON, passenger);
            return;
        }

        // Released, dead, logged out or teleported: the prison goes (delayed: not from inside the vehicle code)
        passenger->RemoveAurasDueToSpell(SPELL_SHADOW_PRISON);
        me->DespawnOrUnsummon(100);
    }

    // Set the player down on the ground under the prison
    void GetPassengerExitPosition(Unit* /*passenger*/, int8 /*seatId*/, Position& pos) override
    {
        float x = me->GetPositionX(), y = me->GetPositionY(), z = me->GetPositionZ();
        me->UpdateGroundPositionZ(x, y, z);
        pos.Relocate(x, y, z, me->GetOrientation());
    }

    void UpdateAI(uint32 diff) override
    {
        if (_boarded)
            return;

        // Nobody got in
        if (_emptyTimer > diff)
        {
            _emptyTimer -= diff;
            return;
        }
        _boarded = true;
        me->DespawnOrUnsummon();
    }

private:
    uint32 _emptyTimer = 3 * IN_MILLISECONDS;
    bool _boarded = false;
};

// 86780 - Shadow Prison (Gandling's): only while held in a Shadow Prison 46558 (the aura is saved on logout; the prison is not)
class spell_shadow_prison_gandling : public AuraScript
{
    PrepareAuraScript(spell_shadow_prison_gandling);

    void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* owner = GetUnitOwner())
            if (owner->GetTypeId() == TYPEID_PLAYER && !IsShadowPrisonPassenger(owner))
                Remove();
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_shadow_prison_gandling::AfterApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

// Players held in Gandling's Shadow Prison take no damage (the Scourge around the square keeps fighting)
class unit_wpl_gandling_shadow_prison : public UnitScript
{
public:
    unit_wpl_gandling_shadow_prison() : UnitScript("unit_wpl_gandling_shadow_prison") { }

    void OnDamage(Unit* /*attacker*/, Unit* victim, uint32& damage) override
    {
        if (damage && victim && victim->GetTypeId() == TYPEID_PLAYER && IsShadowPrisonPassenger(victim))
            damage = 0;
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
    NPC_ZENKIKI_ESCORT                          = 44904,
    NPC_ZENKIKI_HAWKS                           = 44269,
    NPC_DISEASED_HAWK                           = 44481,

    SPELL_ESCORTING_ZENKIKI_HAWKS               = 83456,
    SPELL_DISEASED_SPIT                         = 81236,

    EVENT_DISEASED_SPIT                         = 1,

    SAY_ZENKIKI_POKED                           = 0
};

// 83819, 83456 - Escorting Zen'Kiki
// Given by spell_area while Zen'Kiki, the Druid 26953 or Zen'Kiki and the Cultists 26955 (83819, Zen'Kiki 44904) or A Different
// Approach 26954 (83456, Zen'Kiki 44269) is in the log. Its effect 0 summons Zen'Kiki as the player's guardian, but the summon isn't
// bound to the aura (effect 2, a dummy), so he stayed with the player after the quest was abandoned or handed in. He now goes when
// the aura does; death is left to the guardian's own rules. Both summons use quest slot 6, so a new one replaces the other.
class spell_wpl_escorting_zenkiki : public AuraScript
{
    PrepareAuraScript(spell_wpl_escorting_zenkiki);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            return;

        GetTarget()->RemoveAllMinionsByEntry(GetId() == SPELL_ESCORTING_ZENKIKI_HAWKS ? NPC_ZENKIKI_HAWKS : NPC_ZENKIKI_ESCORT);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_wpl_escorting_zenkiki::AfterRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 44269 - Zen'Kiki (A Different Approach, summoned by Escorting Zen'Kiki 83456)
// He carries the spellclick flag for his Wrath pull, so without a script the core gave him NullCreatureAI, which never runs the
// follow-the-owner step of a summon: he stayed where he was summoned. He follows the player to the hill and takes no part in
// fights himself, so a hawk only comes down when the player clicks him.
struct npc_wpl_zenkiki_hawk_helper : public ScriptedAI
{
    npc_wpl_zenkiki_hawk_helper(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        me->SetReactState(REACT_PASSIVE);
        if (summoner->GetTypeId() == TYPEID_PLAYER)
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(summoner, PET_FOLLOW_DIST, me->GetFollowAngle());
        }
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 44481 - Diseased Hawk (A Different Approach 26954)
// The hawks circle the hill immune to players and only fight one when Zen'Kiki's Wrath pulls them down. This core lets an
// immune-to-PC creature start a fight anyway (the check in Unit::IsValidAttackTarget is commented out) and drops the flag once it
// engages, and SmartAI::MoveInLineOfSight attacks on sight in any react state but passive, so with SmartAI the hawks dived on
// everyone reaching the hilltop. While the hawk carries the flag it now ignores players by sight, by assistance and by any other
// AttackStart; spell_wpl_zenkiki_wrath clears the flag before it sends the hawk at the clicker, and the core puts the flag back
// when the hawk leaves combat. A hawk is also defensive and calls no help: the core's assistance (AssistDelayEvent) starts
// combat before AttackStart, which drops the flag, and only aggressive creatures answer it. Diseased Spit 4-9 s into the
// fight and then every 35-40 s, as the SmartAI row did.
struct npc_wpl_diseased_hawk : public ScriptedAI
{
    npc_wpl_diseased_hawk(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        me->SetReactState(REACT_DEFENSIVE);
        if (!me->IsInCombat())
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (me->IsImmuneToPC())
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void AttackStart(Unit* who) override
    {
        if (!who || (me->IsImmuneToPC() && who->GetCharmerOrOwnerPlayerOrPlayerItself()))
            return;

        ScriptedAI::AttackStart(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        ScriptedAI::JustEngagedWith(who);
        me->SetNoCallAssistance(true);
        _events.ScheduleEvent(EVENT_DISEASED_SPIT, urand(4 * IN_MILLISECONDS, 9 * IN_MILLISECONDS));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        // AttackStop clears it (Feign Death, Vanish), and the next Attack would call the other hawks
        me->SetNoCallAssistance(true);

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (_events.ExecuteEvent() == EVENT_DISEASED_SPIT)
        {
            DoCastVictim(SPELL_DISEASED_SPIT);
            _events.ScheduleEvent(EVENT_DISEASED_SPIT, urand(35 * IN_MILLISECONDS, 40 * IN_MILLISECONDS));
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// 83541 - Wrath (Zen'Kiki 44269's spellclick on A Different Approach 26954)
// The Diseased Hawks circle the hill immune to players (sniffed unit_flags 0x100) and only come down when Zen'Kiki's Wrath hits one
// (Wowhead: "you can't tame or attack them unless he pulls them down"). The hit hawk turns on the player who clicked him; one that
// is already fighting keeps its fight. Zen'Kiki answers each poke with one of his lines.
class spell_wpl_zenkiki_wrath : public SpellScript
{
    PrepareSpellScript(spell_wpl_zenkiki_wrath);

    Player* GetClicker()
    {
        if (Unit* original = GetOriginalCaster())
            if (Player* player = original->ToPlayer())
                return player;
        return GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    void HandleCast()
    {
        if (Creature* zenkiki = GetCaster()->ToCreature())
            if (zenkiki->IsAIEnabled)
                zenkiki->AI()->Talk(SAY_ZENKIKI_POKED, GetClicker());
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Creature* hawk = GetHitCreature();
        Player* player = GetClicker();
        // An evading hawk is out of combat but refuses to attack: it would keep the flag off for good
        if (!hawk || !player || !player->IsAlive() || hawk->GetEntry() != NPC_DISEASED_HAWK || !hawk->IsAlive() || hawk->IsInCombat()
            || hawk->IsInEvadeMode() || !hawk->IsAIEnabled)
            return;

        // One hawk per pull, tapped by the player who clicked Zen'Kiki
        hawk->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
        hawk->SetNoCallAssistance(true);
        hawk->SetNoSearchAssistance(true);
        hawk->SetLootRecipient(player);
        hawk->AI()->AttackStart(player);

        if (!hawk->IsInCombat())
            hawk->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_wpl_zenkiki_wrath::HandleCast);
        OnEffectHitTarget += SpellEffectFn(spell_wpl_zenkiki_wrath::HandleHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
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
## The Farmers' Militia (27084): Ebon Gargoyles
######*/

enum EbonGargoyle
{
    NPC_EBON_GARGOYLE_PROVINCIAL_MINUTEMAN      = 44944,
    NPC_EBON_GARGOYLE                           = 44955,

    SPELL_FORCECAST_RIDE_GARGOYLE               = 84007, // FORCE_CAST Ride Gargoyle 84006 on the summoner

    EVENT_GARGOYLE_PICK_UP                      = 1,
    EVENT_GARGOYLE_BOARD_FALLBACK               = 2,
    EVENT_GARGOYLE_TAKE_OFF                     = 3,
    EVENT_GARGOYLE_DROP                         = 4,
    EVENT_GARGOYLE_CHECK_LANDED                 = 5,
    EVENT_GARGOYLE_DROP_CORPSE                  = 6,
    EVENT_GARGOYLE_DESPAWN                      = 7,
    EVENT_GARGOYLE_HARD_DESPAWN                 = 8,

    POINT_GARGOYLE_LIFT                         = 1,
    POINT_GARGOYLE_AWAY                         = 2
};

// The gargoyle's single seat (Vehicle 1068, VehicleSeat 8436) has neither CAN_ENTER_OR_EXIT nor a usable-by-override flag, so a ride
// that names no seat (Ride Gargoyle's base points are 0) finds no free seat in Vehicle::AddPassenger: it has to name seat 0.
static constexpr int8 EBON_GARGOYLE_SEAT = 0;

// The free Ebon Gargoyle that this minuteman summoned, if any
static Creature* FindOwnEbonGargoyle(Unit* minuteman)
{
    std::list<Creature*> gargoyles;
    minuteman->GetCreatureListWithEntryInGrid(gargoyles, NPC_EBON_GARGOYLE, 50.0f);

    Creature* found = nullptr;
    for (Creature* gargoyle : gargoyles)
    {
        if (!gargoyle->IsAlive() || !gargoyle->IsSummon() || gargoyle->ToTempSummon()->GetSummonerGUID() != minuteman->GetGUID())
            continue;

        Vehicle* kit = gargoyle->GetVehicleKit();
        if (!kit || !kit->HasEmptySeat(EBON_GARGOYLE_SEAT))
            continue;

        if (!found || minuteman->GetExactDist2dSq(gargoyle) < minuteman->GetExactDist2dSq(found))
            found = gargoyle;
    }
    return found;
}

// 44955 - Ebon Gargoyle
// The collar (Release Ebon Gargoyle 84009) makes every Provincial Minuteman near the clicked point summon a gargoyle in front of
// himself (Summon Ebon Gargoyle 83994; the quest credit is the collar's own second effect, 84289 on the player). The gargoyle
// makes its minuteman board (Forcecast Ride Gargoyle 84007 -> Ride Gargoyle 84006), climbs, lets go and flies off: "the sight of
// their fellow men falling from the skies" (reward text). Creatures take no fall damage, so the gargoyle kills the minuteman once
// he has landed; a creature killed in the air stays hanging there (Unit::setDeathState stops its fall spline). The flight, the
// drop and the kill are ours; the spells are 5.4.8's. The summon's duration is -1 (a uint32 timer of about 49 days), so the
// gargoyle despawns itself on every path, with a hard cap.
struct npc_wpl_ebon_gargoyle : public ScriptedAI
{
    npc_wpl_ebon_gargoyle(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _summonerGUID = summoner->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        // It keeps its template faction 68 (the summon has no SummonProperties in Spell::EffectSummonType's default branch), hostile
        // to the Alliance soldiers around it and to Alliance players: nobody may fight it. Its boarding spells are positive, so
        // they start no combat with the minuteman (Spell::DoSpellHitOnUnit).
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        _events.ScheduleEvent(EVENT_GARGOYLE_PICK_UP, 500ms);
        _events.ScheduleEvent(EVENT_GARGOYLE_HARD_DESPAWN, 25s);
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply)
        {
            if (_passengerGUID || _flyingAway)
                return;

            _passengerGUID = passenger->GetGUID();
            // He may have been neutralized by another gargoyle (two collar hits in quick succession); this one restores him then.
            if (passenger->GetTypeId() == TYPEID_UNIT)
                _neutralizedGUID = passenger->GetGUID();
            _events.CancelEvent(EVENT_GARGOYLE_BOARD_FALLBACK);
            _events.ScheduleEvent(EVENT_GARGOYLE_TAKE_OFF, 1s);
            return;
        }

        if (passenger->GetGUID() != _passengerGUID || _dropping)
            return;

        // Killed aboard (by a player or a soldier), or taken off by something else. Unit::setDeathState stops the fall spline that
        // the exit gave the body, so let it fall once the death code is done; then leave.
        if (!passenger->IsAlive() && passenger->GetTypeId() == TYPEID_UNIT)
        {
            _corpseGUID = passenger->GetGUID();
            _events.ScheduleEvent(EVENT_GARGOYLE_DROP_CORPSE, 300ms);
        }
        _passengerGUID.Clear();
        FlyAway();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case POINT_GARGOYLE_LIFT:
                _events.RescheduleEvent(EVENT_GARGOYLE_DROP, 300ms);
                break;
            case POINT_GARGOYLE_AWAY:
                _awayReached = true;
                TryDespawn();
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
                case EVENT_GARGOYLE_PICK_UP:
                {
                    // Already aboard (another gargoyle's forcecast can seat him here when the collar hit him twice at once)
                    if (_passengerGUID)
                        break;
                    Creature* minuteman = GetFreeSummoner();
                    if (!minuteman)
                    {
                        FlyAway();
                        break;
                    }
                    // The collar's area effect is negative, so it put him in combat with the player (Spell::DoSpellHitOnUnit) and
                    // he is chasing: while seated, the chase stops the seat spline every update, and after the drop it replaces
                    // his fall. Take him out of the fight; he is killed on landing, and Creature::Respawn restores react state
                    // and movement.
                    minuteman->CombatStop(true);
                    minuteman->SetReactState(REACT_PASSIVE);
                    minuteman->GetMotionMaster()->Clear();
                    minuteman->GetMotionMaster()->MoveIdle();
                    _neutralizedGUID = minuteman->GetGUID();
                    DoCast(minuteman, SPELL_FORCECAST_RIDE_GARGOYLE, true);
                    _events.ScheduleEvent(EVENT_GARGOYLE_BOARD_FALLBACK, 1500ms);
                    break;
                }
                case EVENT_GARGOYLE_BOARD_FALLBACK:
                {
                    // The ride spell did not seat him: seat him directly, once; still nobody aboard a second later, leave empty
                    if (_passengerGUID)
                        break;
                    Creature* minuteman = GetFreeSummoner();
                    if (_fallbackTried || !minuteman || !me->IsWithinDistInMap(minuteman, 30.0f))
                    {
                        FlyAway();
                        break;
                    }
                    _fallbackTried = true;
                    minuteman->EnterVehicle(me, EBON_GARGOYLE_SEAT);
                    if (!_passengerGUID)
                        _events.ScheduleEvent(EVENT_GARGOYLE_BOARD_FALLBACK, 1s);
                    break;
                }
                case EVENT_GARGOYLE_TAKE_OFF:
                {
                    EnableFlight();
                    float o = me->GetOrientation();
                    float x = me->GetPositionX() + 12.0f * std::cos(o);
                    float y = me->GetPositionY() + 12.0f * std::sin(o);
                    float z = me->GetPositionZ() + 5.0f;
                    me->UpdateGroundPositionZ(x, y, z);
                    me->GetMotionMaster()->MovePoint(POINT_GARGOYLE_LIFT, x, y, z + 22.0f, false);
                    // In case the point is never reached
                    _events.ScheduleEvent(EVENT_GARGOYLE_DROP, 8s);
                    break;
                }
                case EVENT_GARGOYLE_DROP:
                    DropPassenger();
                    FlyAway();
                    break;
                case EVENT_GARGOYLE_CHECK_LANDED:
                {
                    Creature* minuteman = ObjectAccessor::GetCreature(*me, _passengerGUID);
                    if (!minuteman || !minuteman->IsAlive() || minuteman->GetVehicle())
                    {
                        _passengerGUID.Clear();
                        TryDespawn();
                        break;
                    }

                    float ground = minuteman->GetMapHeight(minuteman->GetPositionX(), minuteman->GetPositionY(), minuteman->GetPositionZ());
                    bool landed = minuteman->movespline->Finalized() || (ground > INVALID_HEIGHT && minuteman->GetPositionZ() - ground < 1.0f);
                    // About 1.5 s of fall from the drop height; kill him after 4 s whatever the spline says
                    if (landed || ++_landingChecks >= 16)
                    {
                        me->Kill(minuteman, false);
                        _passengerGUID.Clear();
                        // Killed before he was down (the cap): the death code stopped his fall, so let the body down
                        if (!landed)
                        {
                            _corpseGUID = minuteman->GetGUID();
                            _events.ScheduleEvent(EVENT_GARGOYLE_DROP_CORPSE, 300ms);
                        }
                        TryDespawn();
                        break;
                    }
                    _events.ScheduleEvent(EVENT_GARGOYLE_CHECK_LANDED, 250ms);
                    break;
                }
                case EVENT_GARGOYLE_DROP_CORPSE:
                    if (Creature* corpse = ObjectAccessor::GetCreature(*me, _corpseGUID))
                        if (!corpse->IsAlive() && !corpse->GetVehicle())
                            corpse->GetMotionMaster()->MoveFall();
                    _corpseGUID.Clear();
                    TryDespawn();
                    break;
                case EVENT_GARGOYLE_DESPAWN:
                    // The away point was not reached in time
                    _awayReached = true;
                    TryDespawn();
                    break;
                case EVENT_GARGOYLE_HARD_DESPAWN:
                    // Whatever happened. A minuteman still aboard or still falling is left alive and given his own behaviour back
                    // (a passenger is set down by the vehicle's uninstall; let him fall first so nothing replaces the fall).
                    if (Creature* minuteman = ObjectAccessor::GetCreature(*me, _neutralizedGUID))
                    {
                        if (minuteman->IsAlive())
                        {
                            if (minuteman->GetVehicleBase() == me)
                                minuteman->ExitVehicle();
                            if (!minuteman->GetVehicle())
                            {
                                minuteman->InitializeReactState();
                                minuteman->GetMotionMaster()->InitDefault();
                                minuteman->GetMotionMaster()->MoveFall();
                            }
                        }
                    }
                    me->DespawnOrUnsummon();
                    return;
                default:
                    break;
            }
        }
    }

private:
    // The minuteman who summoned this gargoyle, if he can still be picked up
    Creature* GetFreeSummoner() const
    {
        Creature* minuteman = ObjectAccessor::GetCreature(*me, _summonerGUID);
        if (!minuteman || !minuteman->IsAlive() || !minuteman->IsInWorld() || minuteman->GetEntry() != NPC_EBON_GARGOYLE_PROVINCIAL_MINUTEMAN)
            return nullptr;
        // Already carried off by another gargoyle (two collar uses on him at once)
        if (minuteman->GetVehicle())
            return nullptr;
        return minuteman;
    }

    void EnableFlight()
    {
        me->SetCanFly(true);
        me->SetFlying(true); // DisableGravity and the flying animation tier
    }

    // Let the passenger go and watch for his landing
    void DropPassenger()
    {
        Unit* passenger = ObjectAccessor::GetUnit(*me, _passengerGUID);
        if (!passenger || passenger->GetVehicleBase() != me)
        {
            _passengerGUID.Clear();
            return;
        }

        _dropping = true;
        passenger->ExitVehicle();
        // Unit::_ExitVehicle launches a fall but leaves the movement generators alone; the fall in the controlled slot keeps
        // anything else from replacing it until he is down
        passenger->GetMotionMaster()->MoveFall();
        _landingChecks = 0;
        _events.ScheduleEvent(EVENT_GARGOYLE_CHECK_LANDED, 500ms);
    }

    // Climb away; despawn once there and once a dropped minuteman has been dealt with
    void FlyAway()
    {
        _events.CancelEvent(EVENT_GARGOYLE_PICK_UP);
        _events.CancelEvent(EVENT_GARGOYLE_BOARD_FALLBACK);
        _events.CancelEvent(EVENT_GARGOYLE_TAKE_OFF);
        _events.CancelEvent(EVENT_GARGOYLE_DROP);

        // Never leave with someone aboard
        if (_passengerGUID && !_dropping)
            DropPassenger();

        // Taken out of the fight but never carried: give him his own behaviour back
        if (!_passengerGUID)
        {
            if (Creature* minuteman = ObjectAccessor::GetCreature(*me, _neutralizedGUID))
                if (minuteman->IsAlive() && !minuteman->GetVehicle())
                {
                    minuteman->InitializeReactState();
                    minuteman->GetMotionMaster()->InitDefault();
                    minuteman->GetMotionMaster()->MoveFall(); // in case he was taken off in the air; nothing if he stands
                }
            _neutralizedGUID.Clear();
        }

        if (_flyingAway)
            return;
        _flyingAway = true;

        EnableFlight();
        float o = me->GetOrientation();
        float x = me->GetPositionX() + 30.0f * std::cos(o);
        float y = me->GetPositionY() + 30.0f * std::sin(o);
        float z = me->GetPositionZ() + 30.0f;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(POINT_GARGOYLE_AWAY, x, y, z, false);
        _events.RescheduleEvent(EVENT_GARGOYLE_DESPAWN, 8s);
    }

    void TryDespawn()
    {
        if (!_awayReached || _passengerGUID || _corpseGUID)
            return;
        me->DespawnOrUnsummon(100);
    }

    EventMap _events;
    ObjectGuid _summonerGUID;
    ObjectGuid _passengerGUID;  // aboard, or dropped and not yet landed
    ObjectGuid _corpseGUID;     // killed aboard, to be let down
    ObjectGuid _neutralizedGUID; // the minuteman taken out of his fight for the pick-up
    uint32 _landingChecks = 0;
    bool _fallbackTried = false;
    bool _dropping = false;
    bool _flyingAway = false;
    bool _awayReached = false;
};

// 84006 - Ride Gargoyle (aura 236 on TARGET_UNIT_NEARBY_ENTRY, cast by a minuteman through Forcecast Ride Gargoyle 84007)
// The nearest Ebon Gargoyle (conditions) may be another minuteman's when several are summoned at once: take the caster's own.
// Without a free one of his own the ride is dropped: boarding seat 0 of another gargoyle would push its minuteman off, and a ride
// aura left on a gargoyle he never boarded would take him off his own one when it is removed (HandleAuraControlVehicle).
class spell_wpl_ride_gargoyle : public SpellScript
{
    PrepareSpellScript(spell_wpl_ride_gargoyle);

    void SelectGargoyle(WorldObject*& target)
    {
        if (Unit* caster = GetCaster())
            if (Creature* gargoyle = FindOwnEbonGargoyle(caster))
                target = gargoyle;
    }

    void CheckGargoyle(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Unit* gargoyle = GetHitUnit();
        if (caster && gargoyle && gargoyle->GetEntry() == NPC_EBON_GARGOYLE && gargoyle->IsSummon()
            && gargoyle->ToTempSummon()->GetSummonerGUID() == caster->GetGUID() && !caster->GetVehicle()
            && gargoyle->GetVehicleKit() && gargoyle->GetVehicleKit()->HasEmptySeat(EBON_GARGOYLE_SEAT))
            return;

        PreventHitDefaultEffect(effIndex);
        PreventHitAura();
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_wpl_ride_gargoyle::SelectGargoyle, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_wpl_ride_gargoyle::CheckGargoyle, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 84006 - Ride Gargoyle: name seat 0 (see EBON_GARGOYLE_SEAT); spell_wpl_ride_gargoyle lets the ride reach only a free seat
class spell_wpl_ride_gargoyle_seat : public AuraScript
{
    PrepareAuraScript(spell_wpl_ride_gargoyle_seat);

    void CalcSeat(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* gargoyle = GetUnitOwner())
            if (gargoyle->GetEntry() == NPC_EBON_GARGOYLE)
                amount = float(EBON_GARGOYLE_SEAT + 1);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_wpl_ride_gargoyle_seat::CalcSeat, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE);
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
    new creature_script<npc_wpl_andorhal_death_knight>("npc_thassarian_quest");
    new creature_script<npc_wpl_andorhal_death_knight>("npc_koltira_deathweaver_andorhal");
    new creature_script<npc_wpl_gandling_shadow_prison>("npc_wpl_gandling_shadow_prison");
    new unit_wpl_gandling_shadow_prison();
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
    new spell_script<spell_wpl_zenkiki_wrath>("spell_wpl_zenkiki_wrath");
    new creature_script<npc_wpl_zenkiki_hawk_helper>("npc_wpl_zenkiki_hawk_helper");
    new creature_script<npc_wpl_diseased_hawk>("npc_wpl_diseased_hawk");
    new creature_script<npc_wpl_provincial_minuteman>("npc_wpl_provincial_minuteman");
    new creature_script<npc_wpl_gory>("npc_wpl_gory");
    new creature_script<npc_wpl_scourge_bone_animus>("npc_wpl_scourge_bone_animus");
    new creature_script<npc_wpl_araj_the_summoner>("npc_wpl_araj_the_summoner");
    new creature_script<npc_wpl_opengut_behemoth>("npc_wpl_opengut_behemoth");
    new spell_script<spell_wpl_opengut_cleave>("spell_wpl_opengut_cleave");
    new creature_script<npc_wpl_ebon_gargoyle>("npc_wpl_ebon_gargoyle");
    new spell_script<spell_wpl_ride_gargoyle>("spell_wpl_ride_gargoyle");
    new aura_script<spell_wpl_ride_gargoyle_seat>("spell_wpl_ride_gargoyle_seat");
}
