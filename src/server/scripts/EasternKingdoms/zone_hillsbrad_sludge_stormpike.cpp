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

/* Hillsbrad Foothills: the Sludge Fields
 *
 * No One Here Gets Out Alive (28192)   the camera tour of the Sludge Fields
 *
 * Sourced: the spells (5.4.8 Spell.dbc: the camera summon, its ride aura with the scene's phase, the end spell with
 * QUEST_COMPLETE), the line (broadcast_text) and the camera start (spell_target_position of 89144, as in the Cataclysm Classic
 * TDB). Ours: the camera path, every timing and the replay when the scene was interrupted. None of this was sniffed.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "CombatAI.h"
#include "Player.h"
#include "Vehicle.h"
#include "TemporarySummon.h"

namespace
{

enum HillsbradScenes
{
    // The camera's own steps; the scenes number theirs from EVENT_SCENE_FIRST on
    EVENT_CAMERA_WAIT_FOR_RIDER                 = 1,
    EVENT_SCENE_FIRST                           = 10,

    SEAT_SCENE_CAMERA                           = 0
};

// The player's own summon of an entry near the player (personal summons are private: closest-creature searches skip them)
ObjectGuid FindOwnSummon(Player* player, uint32 entry, float range = 100.0f)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, range);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return creature->GetGUID();
    return ObjectGuid::Empty;
}

// Starts a scene: the summon spell seats the player in the camera. If the camera did not come (the spell failed), the quest is
// completed at once, so that the chain can never stop here (ours).
void StartScene(Player* player, uint32 questId, uint32 cameraEntry, uint32 summonSpellId)
{
    player->CastSpell(player, summonSpellId, true);
    if (FindOwnSummon(player, cameraEntry, 50.0f).IsEmpty() && player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(questId);
}

// Starts a scene again for a player whose quest is still open (logged out, crashed or left the area mid-scene), when the player
// talks to the quest giver again. Ours: retail had no replay, the scene could not be interrupted there.
bool ReplayScene(Player* player, uint32 questId, uint32 cameraEntry, uint32 summonSpellId)
{
    if (player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE || player->GetVehicle())
        return false;

    if (!FindOwnSummon(player, cameraEntry, 200.0f).IsEmpty())
        return false;

    CloseGossipMenuFor(player);
    StartScene(player, questId, cameraEntry, summonSpellId);
    return true;
}

// The cameras of the five scenes. Summoned on the player by the scene's summon spell, whose ride aura seats the player (hidden,
// with the scene's phase). The camera waits for its rider, plays the scene, then lets the player out and casts the end spell
// on the player: it puts the player back at the quest giver and completes the quest. Getting out early ends the scene the same
// way (the end spells are named Cancel Cinematic), so the quest is never left open by a scene that went wrong.
class npc_hillsbrad_scene_camera : public VehicleAI
{
public:
    npc_hillsbrad_scene_camera(Creature* creature, uint32 questId, uint32 endSpellId, float speedRate) : VehicleAI(creature),
        _questId(questId), _endSpellId(endSpellId), _speedRate(speedRate), _boardTries(0), _started(false), _done(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_RUN, _speedRate);
        me->SetSpeed(MOVE_FLIGHT, _speedRate);

        _events.ScheduleEvent(EVENT_CAMERA_WAIT_FOR_RIDER, 500ms);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player got out early
        if (_started && !_done)
            Finish();
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE && type != POINT_MOTION_TYPE)
            return;

        if (_done)
            return;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            Arrived(pointId, player);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        if (_done)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            // Logged out: the quest giver plays the scene again (ReplayScene)
            _done = true;
            DespawnActors();
            me->DespawnOrUnsummon();
            return;
        }

        if (_started && player->GetVehicleBase() != me)
        {
            Finish();
            return;
        }

        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_CAMERA_WAIT_FOR_RIDER)
            {
                if (player->GetVehicleBase() == me)
                {
                    _started = true;
                    Start(player);
                }
                else if (++_boardTries < 10)
                    _events.ScheduleEvent(EVENT_CAMERA_WAIT_FOR_RIDER, 500ms);
                else
                {
                    // The player never got in: complete the quest anyway (ours), the chain must not stop here
                    _done = true;
                    DespawnActors();
                    if (player->GetQuestStatus(_questId) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(_questId);
                    me->DespawnOrUnsummon();
                }
                return;
            }

            SceneEvent(eventId, player);
            if (_done)
                return;
        }
    }

protected:
    virtual void Start(Player* player) = 0;
    virtual void SceneEvent(uint32 /*eventId*/, Player* /*player*/) { }
    virtual void Arrived(uint32 /*pointId*/, Player* /*player*/) { }

    // Flies the camera from where it is through the given points
    void FlyPath(uint32 pointId, std::initializer_list<Position> points)
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());
        for (Position const& point : points)
            path.push_back(point);

        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(pointId, path.data(), path.size(), false, true);
    }

    void FaceTo(float x, float y)
    {
        me->SetFacingTo(me->GetAngle(x, y));
    }

    // An actor of the scene: the player's private summon, passive and out of reach
    Creature* SummonActor(uint32 entry, Position const& pos)
    {
        Creature* actor = me->SummonCreature(entry, pos, TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, _playerGUID);
        if (!actor)
            return nullptr;

        actor->SetReactState(REACT_PASSIVE);
        actor->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        actor->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
        actor->SetHomePosition(pos);
        _actorGUIDs.push_back(actor->GetGUID());
        return actor;
    }

    Creature* GetActor(ObjectGuid const& guid) const
    {
        return ObjectAccessor::GetCreature(*me, guid);
    }

    void DespawnActors()
    {
        for (ObjectGuid const& guid : _actorGUIDs)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                actor->DespawnOrUnsummon();
        _actorGUIDs.clear();
    }

    // Lets the player out and casts the end spell once getting off is over: from PassengerBoarded the teleport would run
    // before the exit's jump, which would carry the player back to the camera (as in What Tomorrow Brings).
    void Finish()
    {
        if (_done)
            return;

        _done = true;
        _events.Reset();
        DespawnActors();

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();

            uint32 questId = _questId;
            uint32 endSpellId = _endSpellId;
            ReturnHook hook = _returnHook;
            player->m_Events.AddLambdaEventAtOffset([player, questId, endSpellId, hook]()
            {
                if (!player->IsInWorld())
                    return;

                if (!player->GetVehicle())
                    player->CastSpell(player, endSpellId, true);

                // The end spell's QUEST_COMPLETE, done directly if the spell could not hit (still on a vehicle, spell failed)
                if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(questId);

                if (hook)
                    hook(player);
            }, 300);
        }

        me->DespawnOrUnsummon(500);
    }

    typedef void (*ReturnHook)(Player*);

    EventMap _events;
    ObjectGuid _playerGUID;
    ReturnHook _returnHook = nullptr;

private:
    std::vector<ObjectGuid> _actorGUIDs;
    uint32 _questId;
    uint32 _endSpellId;
    float _speedRate;
    uint32 _boardTries;
    bool _started;
    bool _done;
};

} // namespace

/*######
## No One Here Gets Out Alive (28192)
######*/

enum NoOneHereGetsOutAlive
{
    QUEST_NO_ONE_HERE_GETS_OUT_ALIVE            = 28192,

    NPC_SLUDGE_FIELDS_CAMERA                    = 47928,

    SPELL_SUMMON_SLUDGE_FIELDS_CAMERA           = 89144,
    SPELL_TELEPORT_TO_SLUDGE_FIELDS_LAB         = 89148,

    TALK_SLUDGE_CAMERA_TROUBLE                  = 0,

    POINT_SLUDGE_TOUR                           = 1,
    POINT_SLUDGE_BACK                           = 2,

    EVENT_SLUDGE_TOUR                           = EVENT_SCENE_FIRST,
    EVENT_SLUDGE_BACK,
    EVENT_SLUDGE_END
};

// 47781 - Warden Stillwater
// Accepting No One Here Gets Out Alive starts the camera tour (Summon Sludge Fields Camera, cast on the player).
struct npc_hillsbrad_warden_stillwater_sludge : public ScriptedAI
{
    npc_hillsbrad_warden_stillwater_sludge(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_NO_ONE_HERE_GETS_OUT_ALIVE)
            StartScene(player, QUEST_NO_ONE_HERE_GETS_OUT_ALIVE, NPC_SLUDGE_FIELDS_CAMERA, SPELL_SUMMON_SLUDGE_FIELDS_CAMERA);
    }

    bool OnGossipHello(Player* player) override
    {
        return ReplayScene(player, QUEST_NO_ONE_HERE_GETS_OUT_ALIVE, NPC_SLUDGE_FIELDS_CAMERA, SPELL_SUMMON_SLUDGE_FIELDS_CAMERA);
    }
};

// 47928 - Sludge Fields Camera
// Summoned in front of Stillwater (spell_target_position of 89144) with the player in it and phase 326 "The Sludge Fields"
// (Sludge Field Camera). "There's trouble at the Sludge Fields...", then the camera flies once around the fields, over the
// infected laborers and the guards' barricades, and comes back to Stillwater; Teleport to Sludge Fields Lab sets the player
// down in front of him and completes the quest. The path is ours: each leg keeps 9 yd or more above the ground and the silos.
struct npc_hillsbrad_sludge_fields_camera : public npc_hillsbrad_scene_camera
{
    npc_hillsbrad_sludge_fields_camera(Creature* creature) : npc_hillsbrad_scene_camera(creature, QUEST_NO_ONE_HERE_GETS_OUT_ALIVE, SPELL_TELEPORT_TO_SLUDGE_FIELDS_LAB, 1.5f) { }

    void Start(Player* player) override
    {
        Talk(TALK_SLUDGE_CAMERA_TROUBLE, player);
        _events.ScheduleEvent(EVENT_SLUDGE_TOUR, 3s);
    }

    void SceneEvent(uint32 eventId, Player* /*player*/) override
    {
        switch (eventId)
        {
            case EVENT_SLUDGE_TOUR:
                FlyPath(POINT_SLUDGE_TOUR,
                {
                    { -495.0f,   60.0f, 72.0f },
                    { -475.0f,   10.0f, 88.0f },
                    { -465.0f,  -50.0f, 85.0f },
                    { -450.0f, -100.0f, 96.0f },
                    { -500.0f, -140.0f, 90.0f },
                    { -560.0f,  -80.0f, 85.0f },
                    { -560.0f,   20.0f, 80.0f }
                });
                break;
            case EVENT_SLUDGE_BACK:
                FlyPath(POINT_SLUDGE_BACK,
                {
                    { -520.0f, 60.0f, 72.0f },
                    { -503.0f, 68.0f, 64.0f }
                });
                break;
            case EVENT_SLUDGE_END:
                Finish();
                break;
            default:
                break;
        }
    }

    void Arrived(uint32 pointId, Player* /*player*/) override
    {
        switch (pointId)
        {
            case POINT_SLUDGE_TOUR:
                _events.ScheduleEvent(EVENT_SLUDGE_BACK, 500ms);
                break;
            case POINT_SLUDGE_BACK:
                // Stillwater's spot
                FaceTo(-500.719f, 90.8021f);
                _events.ScheduleEvent(EVENT_SLUDGE_END, 3s);
                break;
            default:
                break;
        }
    }
};

void AddSC_hillsbrad_sludge_stormpike()
{
    new creature_script<npc_hillsbrad_warden_stillwater_sludge>("npc_hillsbrad_warden_stillwater_sludge");
    new creature_script<npc_hillsbrad_sludge_fields_camera>("npc_hillsbrad_sludge_fields_camera");
}
