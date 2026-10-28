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
 * A Blight Upon the Land (28237)       Lydon's sentence and Warden Stillwater's new form
 *
 * Sourced: the spells (5.4.8 Spell.dbc: the camera summon, its ride aura with the scene's phase, the end spell with
 * QUEST_COMPLETE, SMASH!), every line (broadcast_text), the camera start (spell_target_position of 89144, as in the
 * Cataclysm Classic TDB), Stillwater's combat spells (his SmartAI in the same TDB) and the order of the dialogue
 * (warcraft.wiki.gg). Ours: the camera path, every timing, where Lydon and Johnny stand and the replay when the scene was
 * interrupted. None of this was sniffed.
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

/*######
## A Blight Upon the Land (28237)
######*/

enum ABlightUponTheLand
{
    QUEST_A_BLIGHT_UPON_THE_LAND                = 28237,

    NPC_MASTER_APOTHECARY_LYDON                 = 48020,
    NPC_JOHNNY_AWESOME                          = 48021,
    NPC_FLESH_SHAPER_ARNAULD                    = 48081,
    NPC_SLUDGE_GUARD                            = 48083,

    SPELL_PLAGUE_SLIME                          = 89461,
    SPELL_PLAGUE_BARREL                         = 89463,
    SPELL_SMASH                                 = 89450,

    // Warden Stillwater 48103, his new form: the Wight Scientist model at three times the size
    DISPLAY_STILLWATER_NEW_FORM                 = 30993,

    TALK_STILLWATER_LYDON_ALIVE                 = 0,
    TALK_STILLWATER_ALLEGIANCES                 = 1,
    TALK_STILLWATER_JENNY                       = 2,
    TALK_STILLWATER_THREAT                      = 3,
    TALK_STILLWATER_EVOLUTION                   = 4,
    TALK_STILLWATER_KILL_THEM_ALL               = 5,
    TALK_STILLWATER_NEW_FORM                    = 6,

    TALK_LYDON_SENTENCE                         = 10,
    TALK_LYDON_TRAITOROUS_SCUM                  = 11,
    TALK_LYDON_KNEEL                            = 12,

    // SmartAI of Arnauld and the Sludge Guards (2026_10_28_41): turn hostile and attack the closest player
    DATA_FOLLOWERS_ATTACK                       = 1,

    EVENT_BLIGHT_INTRO                          = 1,
    EVENT_BLIGHT_INTRO_LAST                     = 9,
    EVENT_BLIGHT_PLAGUE_BARREL                  = 20,
    EVENT_BLIGHT_PLAGUE_SLIME                   = 21,
    EVENT_BLIGHT_SMASH                          = 22
};

float const StillwaterIntroRange = 25.0f;
float const StillwaterIntroLeaveRange = 60.0f;

// 48080 - Warden Stillwater
// Phase 332 shows him with Flesh-Shaper Arnauld and his Sludge Guards to players on A Blight Upon the Land. When one comes
// near, Lydon and Johnny Awesome (whom Stillwater calls Jenny) stand beside the player, Lydon passes the sentence and
// Stillwater's people attack (the dialogue in the order of the wiki's notes; Lydon and Johnny are temporary copies here, as mop
// keeps the freed Lydon as a static quest giver instead of the player's follower). The blow that would kill Stillwater makes
// him yell "Stand before my new form and tremble!" and take his new form (48103's model, and its health: HealthModifier 10
// against his 3 in the Cataclysm Classic TDB); SMASH! then throws the players back and kills his followers (its instakill
// effect, given the followers by conditions). He drops his head as before (his own loot).
struct npc_hillsbrad_warden_stillwater_blight : public ScriptedAI
{
    npc_hillsbrad_warden_stillwater_blight(Creature* creature) : ScriptedAI(creature), _introStarted(false), _newForm(false), _baseMaxHealth(0) { }

    void Reset() override
        // Also on respawn (Creature::Respawn resets the AI): the intro's hostile faction is not kept past death or evade
        me->SetFaction(me->GetCreatureTemplate()->faction);
    {
        _events.Reset();
        if (_newForm)
        {
            _newForm = false;
            me->DeMorph();
            if (_baseMaxHealth)
                me->SetMaxHealth(_baseMaxHealth);
            me->SetFullHealth();
        }
    }

    void MoveInLineOfSight(Unit* who) override
    {
        ScriptedAI::MoveInLineOfSight(who);

        if (_introStarted || me->IsInCombat() || !me->IsAlive())
            return;

        Player* player = who->ToPlayer();
        if (!player || !player->IsAlive() || player->IsGameMaster() || !me->IsWithinDistInMap(player, StillwaterIntroRange)
            || player->GetQuestStatus(QUEST_A_BLIGHT_UPON_THE_LAND) != QUEST_STATUS_INCOMPLETE || !me->IsWithinLOSInMap(player))
            return;

        StartIntro(player);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        _introStarted = false;
        me->SetFaction(me->GetCreatureTemplate()->faction);
        DespawnWitnesses();
        ScriptedAI::EnterEvadeMode(why);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        // Cataclysm Classic TDB SmartAI: Plague Barrel 5-8 s, then every 12-15 s; Plague Slime after 10 s, then every 18-22 s
        _events.ScheduleEvent(EVENT_BLIGHT_PLAGUE_BARREL, 5s, 8s);
        _events.ScheduleEvent(EVENT_BLIGHT_PLAGUE_SLIME, 10s);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (_newForm || damage < me->GetHealth())
            return;

        damage = me->GetHealth() - 1;
        _newForm = true;
        _baseMaxHealth = me->GetMaxHealth();

        Talk(TALK_STILLWATER_NEW_FORM);
        me->SetDisplayId(DISPLAY_STILLWATER_NEW_FORM);
        me->SetMaxHealth(_baseMaxHealth * 10 / 3);
        me->SetFullHealth();
        me->CastSpell(me, SPELL_SMASH, true);

        // Cataclysm Classic TDB SmartAI: SMASH! on close every 15-25 s
        _events.ScheduleEvent(EVENT_BLIGHT_SMASH, 15s, 25s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // The AI does not update once he is dead: the witnesses go by themselves. He respawns friendly, for the next player's
        // intro (Respawn keeps the faction).
        me->SetFaction(me->GetCreatureTemplate()->faction);
        _introStarted = false;
        _events.Reset();
        if (Creature* lydon = ObjectAccessor::GetCreature(*me, _lydonGUID))
            lydon->DespawnOrUnsummon(10000);
        if (Creature* johnny = ObjectAccessor::GetCreature(*me, _johnnyGUID))
            johnny->DespawnOrUnsummon(10000);
        _lydonGUID.Clear();
        _johnnyGUID.Clear();
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (!me->IsInCombat())
        {
            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId >= EVENT_BLIGHT_INTRO && eventId <= EVENT_BLIGHT_INTRO_LAST)
                    IntroStep(eventId);
            }
            return;
        }

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BLIGHT_PLAGUE_BARREL:
                    DoCastVictim(SPELL_PLAGUE_BARREL);
                    _events.ScheduleEvent(EVENT_BLIGHT_PLAGUE_BARREL, 12s, 15s);
                    break;
                case EVENT_BLIGHT_PLAGUE_SLIME:
                    DoCastVictim(SPELL_PLAGUE_SLIME);
                    _events.ScheduleEvent(EVENT_BLIGHT_PLAGUE_SLIME, 18s, 22s);
                    break;
                case EVENT_BLIGHT_SMASH:
                    if (me->GetVictim() && me->IsWithinDistInMap(me->GetVictim(), 10.0f))
                    {
                        DoCast(me, SPELL_SMASH);
                        _events.ScheduleEvent(EVENT_BLIGHT_SMASH, 15s, 25s);
                    }
                    else
                        _events.ScheduleEvent(EVENT_BLIGHT_SMASH, 2s);
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
    void StartIntro(Player* player)
    {
        _introStarted = true;
        _playerGUID = player->GetGUID();
        me->GetMotionMaster()->Clear();
        me->StopMoving();
        me->SetFacingToObject(player);

        // Lydon and Johnny stand to either side behind the player, facing Stillwater (ours)
        float angle = player->GetAngle(me);
        float x, y, z;
        player->GetNearPoint(player, x, y, z, 2.5f, angle + float(M_PI) * 0.7f);
        if (Creature* lydon = me->SummonCreature(NPC_MASTER_APOTHECARY_LYDON, x, y, z, angle, TEMPSUMMON_MANUAL_DESPAWN))
            SetUpWitness(lydon, _lydonGUID);
        player->GetNearPoint(player, x, y, z, 2.5f, angle - float(M_PI) * 0.7f);
        if (Creature* johnny = me->SummonCreature(NPC_JOHNNY_AWESOME, x, y, z, angle, TEMPSUMMON_MANUAL_DESPAWN))
            SetUpWitness(johnny, _johnnyGUID);

        _events.ScheduleEvent(EVENT_BLIGHT_INTRO, 1s);
    }

    void SetUpWitness(Creature* witness, ObjectGuid& guid)
    {
        witness->SetReactState(REACT_PASSIVE);
        witness->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        witness->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
        witness->SetFacingToObject(me);
        guid = witness->GetGUID();
    }

    void DespawnWitnesses()
    {
        if (Creature* lydon = ObjectAccessor::GetCreature(*me, _lydonGUID))
            lydon->DespawnOrUnsummon();
        if (Creature* johnny = ObjectAccessor::GetCreature(*me, _johnnyGUID))
            johnny->DespawnOrUnsummon();
        _lydonGUID.Clear();
        _johnnyGUID.Clear();
    }

    void AbortIntro()
    {
        _events.Reset();
        _introStarted = false;
        DespawnWitnesses();
        me->GetMotionMaster()->MoveTargetedHome();
    }

    void IntroStep(uint32 eventId)
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        Creature* lydon = ObjectAccessor::GetCreature(*me, _lydonGUID);
        if (!player || !player->IsAlive() || !me->IsWithinDistInMap(player, StillwaterIntroLeaveRange) || !lydon
            || player->GetQuestStatus(QUEST_A_BLIGHT_UPON_THE_LAND) != QUEST_STATUS_INCOMPLETE)
        {
            AbortIntro();
            return;
        }

        Milliseconds next = 0ms;
        switch (eventId)
        {
            case 1:
                Talk(TALK_STILLWATER_LYDON_ALIVE);
                next = 3s;
                break;
            case 2:
                me->SetFacingToObject(player);
                Talk(TALK_STILLWATER_ALLEGIANCES, player);
                next = 4s;
                break;
            case 3:
                if (Creature* johnny = ObjectAccessor::GetCreature(*me, _johnnyGUID))
                    me->SetFacingToObject(johnny);
                Talk(TALK_STILLWATER_JENNY);
                next = 5s;
                break;
            case 4:
                lydon->AI()->Talk(TALK_LYDON_SENTENCE);
                next = 8s;
                break;
            case 5:
                lydon->AI()->Talk(TALK_LYDON_TRAITOROUS_SCUM);
                next = 4s;
                break;
            case 6:
                lydon->AI()->Talk(TALK_LYDON_KNEEL);
                next = 3s;
                break;
            case 7:
                me->SetFacingToObject(lydon);
                Talk(TALK_STILLWATER_THREAT);
                next = 3s;
                break;
            case 8:
                Talk(TALK_STILLWATER_EVOLUTION);
                next = 6s;
                break;
            case EVENT_BLIGHT_INTRO_LAST:
            {
                Talk(TALK_STILLWATER_KILL_THEM_ALL);
                me->SetFaction(14);

                std::list<Creature*> followers;
                me->GetCreatureListWithEntryInGrid(followers, NPC_SLUDGE_GUARD, 40.0f);
                me->GetCreatureListWithEntryInGrid(followers, NPC_FLESH_SHAPER_ARNAULD, 40.0f);
                for (Creature* follower : followers)
                    if (follower->IsAlive() && follower->IsAIEnabled && !follower->IsInCombat())
                        follower->AI()->SetData(DATA_FOLLOWERS_ATTACK, 1);

                AttackStart(player);
                break;
            }
            default:
                break;
        }

        if (next > 0ms)
            _events.ScheduleEvent(eventId + 1, next);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _lydonGUID;
    ObjectGuid _johnnyGUID;
    bool _introStarted;
    bool _newForm;
    uint32 _baseMaxHealth;
};

void AddSC_hillsbrad_sludge_stormpike()
{
    new creature_script<npc_hillsbrad_warden_stillwater_sludge>("npc_hillsbrad_warden_stillwater_sludge");
    new creature_script<npc_hillsbrad_sludge_fields_camera>("npc_hillsbrad_sludge_fields_camera");
    new creature_script<npc_hillsbrad_warden_stillwater_blight>("npc_hillsbrad_warden_stillwater_blight");
}
