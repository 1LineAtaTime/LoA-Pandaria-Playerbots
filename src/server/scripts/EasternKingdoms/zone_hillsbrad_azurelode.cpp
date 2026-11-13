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

/* Hillsbrad Foothills: the Azurelode Mine
 *
 * Coastal Delicacies! (28146)   the Hungry Mine Creeper eats the murlocs its player killed
 *
 * Sourced: the quest text ("Whatever you kill, it'll eat!"), Wowhead comments from 4.0.3 (the spider eats the corpses of the
 * murlocs you killed, not other players' kills, and does not need the killing blow), the spell 88898 "Devour Murloc" (5.4.8
 * Spell.dbc: an instant dummy with its own visual that can target dead units) and the credit NPC 47752 "Murloc Eaten Proxy".
 * Ours: the 30 yd search, the pause while it eats and the give-up time when a corpse cannot be reached.
 *
 * Deep Mine Rescue (28156)        Dumass follows the player out of the mine to Captain Keyton
 *
 * Sourced: the story (warcraft.wiki.gg, 4.0.3: the player cuts Dumass free by accepting, he follows the player out, attacks
 * every webbed human he comes near, which frees it, and the player kills the freed ones; at Captain Keyton he and Keyton talk
 * and the quest completes), every line and its emote (broadcast_text 48057-48075), the webbed humans (Hillsbrad Miner 2269 and
 * Foreman 2503: their respawn SmartAI casts one of the Paralytic Poisons 87969/87972/87973, a stun that 5.4.8
 * SpellInterrupts.dbc breaks on any damage, so his first hit frees one).
 * Ours: no source scripts this quest (neither the retail TDB nor TCPP). Dumass is a summon so that the webbed quest giver stays
 * for other players; the 8 yd at which he goes for a webbed human, which lines he says when (48059 "I'M LEAVING! BYE!" is not
 * placed by any source; it is his line when the escort fails), the chatter timer, the 12 yd arrival range at Keyton, the scene
 * timings, the 60 yd leash and failing the quest when he dies, the player dies or the player leaves him behind.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "PetDefines.h"
#include "ScriptedGossip.h"
#include "TemporarySummon.h"
#include "Group.h"

namespace
{

enum CoastalDelicacies
{
    QUEST_COASTAL_DELICACIES        = 28146,
    NPC_MURLOC_EATEN_PROXY          = 47752,
    SPELL_DEVOUR_MURLOC             = 88898,
    SPELL_HUNGRY_MINE_CREEPER       = 88762,    // the spell_area aura that summons it

    POINT_MURLOC_CORPSE             = 1
};

// The Torn Fin murlocs of the Western Strand
uint32 const TornFinMurlocs[] = { 2374, 2375, 2376, 2377 };
float const CorpseSearchRange = 30.0f;
float const EatRange = 5.0f;

// Hungry Mine Creeper 47662, summoned by the spell_area aura 88762 while Coastal Delicacies! is in the log.
// A corpse to eat comes before fighting: it stops attacking, walks over and eats, then follows its player and fights again.
// The state lives outside Reset(): the core resets an owned creature on every evade, which used to cancel the walk.
struct npc_hillsbrad_hungry_mine_creeper : public ScriptedAI
{
    npc_hillsbrad_hungry_mine_creeper(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_DEFENSIVE);
    }

    void AttackStart(Unit* who) override
    {
        if (_state == STATE_IDLE)
            ScriptedAI::AttackStart(who);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_state == STATE_IDLE)
            ScriptedAI::EnterEvadeMode(why);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_MURLOC_CORPSE && _state == STATE_WALKING)
            _timer = 200;
    }

    void UpdateAI(uint32 diff) override
    {
        // Goes home when the quest is turned in or abandoned: the aura's removal does not unsummon it, and spell_area only
        // re-checks the quest on login and area changes
        Player* player = me->GetCharmerOrOwnerPlayerOrPlayerItself();
        QuestStatus status = player ? player->GetQuestStatus(QUEST_COASTAL_DELICACIES) : QUEST_STATUS_NONE;
        if (status != QUEST_STATUS_INCOMPLETE && status != QUEST_STATUS_COMPLETE)
        {
            if (player)
                player->RemoveAurasDueToSpell(SPELL_HUNGRY_MINE_CREEPER);
            me->DespawnOrUnsummon();
            return;
        }

        switch (_state)
        {
            case STATE_IDLE:
                if (_timer <= diff)
                {
                    _timer = 1000;
                    if (GoToCorpse())
                        return;
                }
                else
                    _timer -= diff;
                break;
            case STATE_WALKING:
                if (_timer <= diff)
                    Eat();
                else
                    _timer -= diff;
                return;
            case STATE_EATING:
                if (_timer <= diff)
                {
                    _state = STATE_IDLE;
                    _timer = 500;
                    if (Player* owner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                    {
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
                    }
                }
                else
                    _timer -= diff;
                return;
        }

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    enum State
    {
        STATE_IDLE,
        STATE_WALKING,
        STATE_EATING
    };

    Player* GetOwner() const
    {
        Player* owner = me->GetCharmerOrOwnerPlayerOrPlayerItself();
        return owner && owner->GetQuestStatus(QUEST_COASTAL_DELICACIES) == QUEST_STATUS_INCOMPLETE ? owner : nullptr;
    }

    // Walks to the nearest corpse of a murloc the owner (or the owner's group) tapped that it has not eaten yet
    bool GoToCorpse()
    {
        Player* owner = GetOwner();
        if (!owner)
            return false;

        std::list<Creature*> murlocs;
        for (uint32 entry : TornFinMurlocs)
            me->GetCreatureListWithEntryInGridAppend(murlocs, entry, CorpseSearchRange);

        Creature* corpse = nullptr;
        for (Creature* murloc : murlocs)
        {
            // A respawned murloc keeps its guid: forget that it was eaten
            if (murloc->IsAlive())
            {
                _eaten.erase(murloc->GetGUID());
                continue;
            }

            if (_eaten.count(murloc->GetGUID()) || !murloc->IsTappedBy(owner))
                continue;

            if (!corpse || me->GetExactDist2dSq(murloc) < me->GetExactDist2dSq(corpse))
                corpse = murloc;
        }

        if (!corpse)
            return false;

        _corpseGUID = corpse->GetGUID();
        _state = STATE_WALKING;
        _timer = 10000;  // gives up if the corpse cannot be reached

        me->AttackStop();
        float x, y, z;
        corpse->GetContactPoint(me, x, y, z);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(POINT_MURLOC_CORPSE, x, y, z);
        return true;
    }

    void Eat()
    {
        Creature* corpse = ObjectAccessor::GetCreature(*me, _corpseGUID);
        _corpseGUID.Clear();
        _state = STATE_EATING;

        Player* owner = GetOwner();
        if (!owner || !corpse || corpse->IsAlive() || !me->IsWithinDist(corpse, EatRange))
        {
            // Unreachable or gone: don't try that one again
            if (corpse)
                _eaten.insert(corpse->GetGUID());
            _timer = 0;
            return;
        }

        _eaten.insert(corpse->GetGUID());
        me->StopMoving();
        me->SetFacingToObject(corpse);
        me->CastSpell(corpse, SPELL_DEVOUR_MURLOC, true);
        owner->KilledMonsterCredit(NPC_MURLOC_EATEN_PROXY);
        _timer = 2500;
    }

    State _state = STATE_IDLE;
    uint32 _timer = 1000;
    ObjectGuid _corpseGUID;
    std::set<ObjectGuid> _eaten;
};

enum DeepMineRescue
{
    QUEST_DEEP_MINE_RESCUE          = 28156,
    NPC_DUMASS                      = 47681,
    NPC_CAPTAIN_KEYTON              = 47554,
    NPC_HILLSBRAD_MINER             = 2269,
    NPC_HILLSBRAD_FOREMAN           = 2503,
    SPELL_PARALYTIC_POISON          = 87969,    // the web on the quest giver (creature_addon of his spawn)

    // Dumass (creature_text 47681)
    TALK_DUMASS_THANKS              = 0,        // 48057, on accept
    TALK_DUMASS_ESCORT_EMOTE        = 1,        // 48058, boss whisper to the player
    TALK_DUMASS_CHATTER             = 2,        // 48064, 48066, 48069, 48071-48075 while following
    TALK_DUMASS_WEBBED_HUMAN        = 3,        // 48063, 48065, 48067, 48070 when he goes for a webbed human
    TALK_DUMASS_BACK                = 4,        // 48060, at Keyton
    TALK_DUMASS_TARREN_MILL         = 5,        // 48062, leaving Keyton
    TALK_DUMASS_LEAVING             = 6,        // 48059, the escort failed

    // Captain Keyton (creature_text 47554)
    TALK_KEYTON_SCURRY_ALONG        = 0,        // 48061

    EVENT_DUMASS_CHECK              = 1,
    EVENT_DUMASS_CHATTER,
    EVENT_DUMASS_KEYTON,
    EVENT_DUMASS_TARREN_MILL,
    EVENT_DUMASS_LEAVE,
    EVENT_DUMASS_DESPAWN,

    POINT_DUMASS_LEAVE              = 1,

    DATA_ESCORT_OVER                = 1         // GetData: 1 once the escort has arrived, failed or left
};

uint32 const ParalyticPoisons[] = { 87969, 87972, 87973 };
float const WebbedHumanRange = 8.0f;
float const KeytonArrivalRange = 12.0f;
float const DumassLeashRange = 60.0f;

// Whether the player already has a Dumass escorting them (one whose escort is over, about to despawn, does not count)
bool HasOwnDumass(Player* player)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, NPC_DUMASS, 250.0f);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID()
            && creature->IsAIEnabled && !creature->AI()->GetData(DATA_ESCORT_OVER))
            return true;
    return false;
}

// Fails the quest for the escorting player and for the group members near this Dumass who have it open (shared to them)
// without a Dumass of their own: one failed escort must not end the others'
void FailDeepMineRescue(Player* player, Creature* dumass)
{
    if (player->GetQuestStatus(QUEST_DEEP_MINE_RESCUE) == QUEST_STATUS_INCOMPLETE)
        player->FailQuest(QUEST_DEEP_MINE_RESCUE);

    if (Group* group = player->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member != player && member->IsInMap(dumass) && member->IsAtGroupRewardDistance(dumass)
                    && member->GetQuestStatus(QUEST_DEEP_MINE_RESCUE) == QUEST_STATUS_INCOMPLETE && !HasOwnDumass(member))
                    member->FailQuest(QUEST_DEEP_MINE_RESCUE);
}

// 47681 - Dumass, the spawn in the web deep in the mine (quest giver of Deep Mine Rescue)
// Accepting cuts him free: a Dumass of the same entry is summoned next to him and follows the player (npc_hillsbrad_dumass_escort),
// while this one stays webbed so that other players can still take the quest. A public summon, not a personal one: a creature
// cannot put a private object on its threat list (ThreatManager::AddThreat), so the freed humans could never fight him back.
struct npc_hillsbrad_dumass_webbed : public ScriptedAI
{
    npc_hillsbrad_dumass_webbed(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_DEEP_MINE_RESCUE)
            SummonEscort(player);
    }

    // A player whose escort was lost while the quest stayed open (logout, server restart) gets a new one by talking to him again
    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_DEEP_MINE_RESCUE) != QUEST_STATUS_INCOMPLETE || HasOwnDumass(player))
            return false;

        CloseGossipMenuFor(player);
        SummonEscort(player);
        return true;
    }

    void UpdateAI(uint32 /*diff*/) override { }

private:
    void SummonEscort(Player* player)
    {
        if (HasOwnDumass(player))
            return;

        Position pos = me->GetNearPosition(2.0f, me->GetRelativeAngle(player));
        pos.SetOrientation(me->GetAngle(player));
        player->SummonCreature(NPC_DUMASS, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10s);
    }
};

// 47681 - Dumass, summoned by his player on accepting Deep Mine Rescue
// Follows the player and goes for every webbed human that comes within 8 yd; the first hit breaks the human's Paralytic Poison and
// it fights back. Within 12 yd of Captain Keyton the quest completes for the player (and the group), then the two talk and he
// leaves for "the Barrens". He fails the quest when he dies, the player dies or the player is 60 yd away; he leaves quietly when
// the player is gone (logout) or the quest is no longer open (abandoned, or completed by a group member's Dumass).
struct npc_hillsbrad_dumass_escort : public ScriptedAI
{
    npc_hillsbrad_dumass_escort(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();

        // The quest giver's web, stun and quest giver flag are not his
        me->RemoveAurasDueToSpell(SPELL_PARALYTIC_POISON);
        me->RemoveUnitFlag(UnitFlags(UNIT_FLAG_STUNNED | UNIT_FLAG_IMMUNE_TO_NPC));
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        me->SetReactState(REACT_DEFENSIVE);

        Talk(TALK_DUMASS_THANKS, player);
        Talk(TALK_DUMASS_ESCORT_EMOTE, player);
        Follow(player);

        _events.ScheduleEvent(EVENT_DUMASS_CHECK, 1s);
        _events.ScheduleEvent(EVENT_DUMASS_CHATTER, 15s, 25s);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_EnterEvadeMode(why))
            return;

        Reset();
        if (_done)
            return;

        // Back to his player, never to the spot he was summoned on
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            Follow(player);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _events.Reset();
        if (_done)
            return;

        _done = true;
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            FailDeepMineRescue(player, me);
    }

    uint32 GetData(uint32 type) const override
    {
        return type == DATA_ESCORT_OVER && _done ? 1 : 0;
    }

    // He never takes a human below half its health: the player must deal the half that loot, XP and kill credit require
    // (Creature::IsDamageEnoughForLootingAndReward only counts damage from players and their pets)
    void DamageDealt(Unit* victim, uint32& damage, DamageEffectType /*damageType*/) override
    {
        if (victim->GetEntry() != NPC_HILLSBRAD_MINER && victim->GetEntry() != NPC_HILLSBRAD_FOREMAN)
            return;

        uint64 floor = victim->GetMaxHealth() / 2 + 1;
        uint64 health = victim->GetHealth();
        damage = health > floor ? uint32(std::min<uint64>(damage, health - floor)) : 0;
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_DUMASS_LEAVE)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DUMASS_CHECK:
                    Check();
                    break;
                case EVENT_DUMASS_CHATTER:
                    if (!me->IsInCombat())
                        Talk(TALK_DUMASS_CHATTER);
                    _events.ScheduleEvent(EVENT_DUMASS_CHATTER, 20s, 35s);
                    break;
                case EVENT_DUMASS_KEYTON:
                    if (Creature* keyton = ObjectAccessor::GetCreature(*me, _keytonGUID))
                        if (keyton->IsAIEnabled)
                            keyton->AI()->Talk(TALK_KEYTON_SCURRY_ALONG, ObjectAccessor::GetPlayer(*me, _playerGUID));
                    _events.ScheduleEvent(EVENT_DUMASS_TARREN_MILL, 10s);
                    break;
                case EVENT_DUMASS_TARREN_MILL:
                    Talk(TALK_DUMASS_TARREN_MILL);
                    _events.ScheduleEvent(EVENT_DUMASS_LEAVE, 4s);
                    break;
                case EVENT_DUMASS_LEAVE:
                {
                    // Runs off, away from Keyton, and is gone
                    Position pos = me->GetPosition();
                    if (Creature* keyton = ObjectAccessor::GetCreature(*me, _keytonGUID))
                        me->MovePositionToFirstCollision(pos, 20.0f, me->GetRelativeAngle(keyton) + float(M_PI));
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(POINT_DUMASS_LEAVE, pos);
                    _events.ScheduleEvent(EVENT_DUMASS_DESPAWN, 8s);
                    break;
                }
                case EVENT_DUMASS_DESPAWN:
                    me->DespawnOrUnsummon();
                    return;
                default:
                    break;
            }
        }

        if (_done)
            return;

        if (UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    void Follow(Player* player)
    {
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }

    void Check()
    {
        if (_done)
            return;

        _events.ScheduleEvent(EVENT_DUMASS_CHECK, 1s);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInMap(me) || player->GetQuestStatus(QUEST_DEEP_MINE_RESCUE) != QUEST_STATUS_INCOMPLETE)
        {
            _done = true;
            me->DespawnOrUnsummon();
            return;
        }

        if (!player->IsAlive() || !me->IsWithinDistInMap(player, DumassLeashRange))
        {
            _done = true;
            _events.Reset();
            me->CombatStop(true);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
            Talk(TALK_DUMASS_LEAVING);
            FailDeepMineRescue(player, me);
            me->DespawnOrUnsummon(3000);
            return;
        }

        if (me->IsInCombat())
            return;

        // An evade out here must not walk him back into the mine, and a chase must not leash to the summon spot
        me->SetHomePosition(me->GetPosition());

        if (Creature* keyton = me->FindNearestCreature(NPC_CAPTAIN_KEYTON, KeytonArrivalRange))
        {
            Arrive(player, keyton);
            return;
        }

        if (Creature* human = FindWebbedHuman())
        {
            Talk(TALK_DUMASS_WEBBED_HUMAN);
            // The humans' template flags keep them out of fights with NPCs; his attack is what frees them
            human->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
            AttackStart(human);
            return;
        }

        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
            Follow(player);
    }

    Creature* FindWebbedHuman() const
    {
        std::list<Creature*> humans;
        me->GetCreatureListWithEntryInGrid(humans, NPC_HILLSBRAD_MINER, WebbedHumanRange);
        me->GetCreatureListWithEntryInGridAppend(humans, NPC_HILLSBRAD_FOREMAN, WebbedHumanRange);

        Creature* nearest = nullptr;
        for (Creature* human : humans)
        {
            if (!human->IsAlive() || human->IsInCombat() || !me->IsWithinLOSInMap(human))
                continue;

            bool webbed = false;
            for (uint32 spellId : ParalyticPoisons)
                if (human->HasAura(spellId))
                    webbed = true;

            if (webbed && (!nearest || me->GetExactDist2dSq(human) < me->GetExactDist2dSq(nearest)))
                nearest = human;
        }
        return nearest;
    }

    void Arrive(Player* player, Creature* keyton)
    {
        _done = true;
        _keytonGUID = keyton->GetGUID();
        _events.Reset();

        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->StopMoving();
        me->SetFacingToObject(keyton);

        // Completed on arrival, so that a player who walks on during the talk keeps the credit
        player->GroupEventHappens(QUEST_DEEP_MINE_RESCUE, me);

        Talk(TALK_DUMASS_BACK);
        _events.ScheduleEvent(EVENT_DUMASS_KEYTON, 4s);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _keytonGUID;
    bool _done = false;     // arrived at Keyton, failed or left: the escort logic is over
};

class npc_hillsbrad_dumass : public CreatureScript
{
public:
    npc_hillsbrad_dumass() : CreatureScript("npc_hillsbrad_dumass") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        if (creature->IsSummon())
            return new npc_hillsbrad_dumass_escort(creature);
        return new npc_hillsbrad_dumass_webbed(creature);
    }
};

} // namespace

void AddSC_hillsbrad_azurelode()
{
    new creature_script<npc_hillsbrad_hungry_mine_creeper>("npc_hillsbrad_hungry_mine_creeper");
    new npc_hillsbrad_dumass();
}
