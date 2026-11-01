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
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "PetDefines.h"

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

} // namespace

void AddSC_hillsbrad_azurelode()
{
    new creature_script<npc_hillsbrad_hungry_mine_creeper>("npc_hillsbrad_hungry_mine_creeper");
}
