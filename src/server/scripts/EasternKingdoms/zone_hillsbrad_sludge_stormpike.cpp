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

/* Hillsbrad Foothills: the Sludge Fields and the Stormpike scenes
 *
 * No One Here Gets Out Alive (28192)   the camera tour of the Sludge Fields
 * Welcome to the Machine (28096)       the quest giver on the Skeletal Steed and his three visitors
 * A Blight Upon the Land (28237)       Lydon's sentence and Warden Stillwater's new form
 * Matters of Loyalty (28600)           Drek'Thar refuses to aid the Forsaken
 * March of the Stormpike (28506)       the camera tour of the four fingers of Hillsbrad
 * Stormpike Apocalypse (28616)         the pocket-nukes go off
 *
 * Sourced: the spells (5.4.8 Spell.dbc: the camera summons, their ride auras with the scene's phase, the end spells with
 * QUEST_COMPLETE), the actors (their summon spells and templates), every line (broadcast_text), the Sludge Fields camera start
 * (spell_target_position of 89144 in the Cataclysm Classic TDB), Stillwater's combat spells (its SmartAI in the same TDB) and
 * the order of the scenes (warcraft.wiki.gg). Ours: the camera paths, every timing, where the actors stand, the vehicle of the
 * three Stormpike cameras (no source has it) and the replay when a scene was interrupted. None of this was sniffed.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "CombatAI.h"
#include "Player.h"
#include "Vehicle.h"
#include "TemporarySummon.h"
#include "SpellScript.h"

namespace
{

enum HillsbradScenes
{
    // The camera's own steps; the scenes number theirs from EVENT_SCENE_FIRST on
    EVENT_CAMERA_WAIT_FOR_RIDER                 = 1,
    EVENT_SCENE_FIRST                           = 10
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

// A passive actor of a scene: the camera moves and speaks for it
struct npc_hillsbrad_scene_actor : public ScriptedAI
{
    npc_hillsbrad_scene_actor(Creature* creature) : ScriptedAI(creature) { }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }
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
    {
        _events.Reset();
        // Also on respawn (Creature::Respawn resets the AI): the intro's hostile faction is not kept past death or evade
        me->SetFaction(me->GetCreatureTemplate()->faction);
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
        _introStarted = false;
        _events.Reset();
        me->SetFaction(me->GetCreatureTemplate()->faction);
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

/*######
## Matters of Loyalty (28600)
######*/

enum MattersOfLoyalty
{
    QUEST_MATTERS_OF_LOYALTY                    = 28600,

    NPC_AID_OF_THE_FROSTWOLF_CAMERA             = 48986,
    NPC_DREKTHAR                                = 48969,
    NPC_DREKTHARS_WHEELCHAIR                    = 48971,
    NPC_CAPTAIN_GALVANGAR                       = 48980,

    SPELL_SUMMON_AID_OF_THE_FROSTWOLF_CAMERA    = 91356,
    SPELL_CANCEL_CINEMATIC_MATTERS_OF_LOYALTY   = 91358,
    SPELL_MIRROR_IMAGE_AURA                     = 90746,

    // Drek'Thar: the twelve lines of the scene, in order
    TALK_DREKTHAR_FIRST                         = 0,
    TALK_DREKTHAR_LAST                          = 11,

    POINT_FROSTWOLF_VANTAGE                     = 1,
    POINT_GALVANGAR_OUT_OF_THE_TUNNEL           = 2,

    EVENT_FROSTWOLF_ACTORS_BOARD                = EVENT_SCENE_FIRST,
    EVENT_FROSTWOLF_GALVANGAR_WALK,
    EVENT_FROSTWOLF_TALK,
    EVENT_FROSTWOLF_GALVANGAR_TURN_BACK,
    EVENT_FROSTWOLF_END
};

// The entrance to Alterac Valley next to Voggah (the Pvp_Alterac_Ent01 doorway), where Galvangar comes out, and where he stops
Position const GalvangarTunnelPos   = { 405.0f, -1004.0f, 111.6f, float(M_PI) };
Position const GalvangarStopPos     = { 388.5f, -1004.0f, 110.2f };
Position const FrostwolfVantagePos  = { 378.0f,  -989.0f, 117.0f };

// Pauses after each of Drek'Thar's lines (ours)
Milliseconds const DrekTharLineDelays[TALK_DREKTHAR_LAST + 1] =
{
    4s, 3s, 7s, 4s, 8s, 5s, 7s, 5s, 4s, 7s, 8s, 3s
};

// 13817 - Voggah Deathgrip
// Accepting Matters of Loyalty starts the scene (Summon Aid of the Frostwolf Camera, cast on the player).
struct npc_hillsbrad_voggah_deathgrip : public ScriptedAI
{
    npc_hillsbrad_voggah_deathgrip(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_MATTERS_OF_LOYALTY)
            StartScene(player, QUEST_MATTERS_OF_LOYALTY, NPC_AID_OF_THE_FROSTWOLF_CAMERA, SPELL_SUMMON_AID_OF_THE_FROSTWOLF_CAMERA);
    }

    bool OnGossipHello(Player* player) override
    {
        return ReplayScene(player, QUEST_MATTERS_OF_LOYALTY, NPC_AID_OF_THE_FROSTWOLF_CAMERA, SPELL_SUMMON_AID_OF_THE_FROSTWOLF_CAMERA);
    }
};

// 48986 - Aid of the Frostwolf Camera
// Summoned on the player with phase 357 (Aid of the Frostwolf Camera); the same spell leaves a Cloned Image of the player in
// front of Voggah. Summon Drek'thar/Crew, done directly: Captain Galvangar pushes Drek'Thar in his wheelchair out of the
// Alterac Valley tunnel (Galvangar carries the wheelchair, which carries Drek'Thar: vehicles 1367 and 1363), Drek'Thar refuses
// the Forsaken their aid, and Cancel Cinematic puts the player back in front of Voggah and completes the quest.
struct npc_hillsbrad_aid_of_the_frostwolf_camera : public npc_hillsbrad_scene_camera
{
    npc_hillsbrad_aid_of_the_frostwolf_camera(Creature* creature) : npc_hillsbrad_scene_camera(creature, QUEST_MATTERS_OF_LOYALTY, SPELL_CANCEL_CINEMATIC_MATTERS_OF_LOYALTY, 1.0f), _line(0) { }

    void Start(Player* /*player*/) override
    {
        if (Creature* galvangar = SummonActor(NPC_CAPTAIN_GALVANGAR, GalvangarTunnelPos))
            _galvangarGUID = galvangar->GetGUID();
        if (Creature* wheelchair = SummonActor(NPC_DREKTHARS_WHEELCHAIR, GalvangarTunnelPos))
            _wheelchairGUID = wheelchair->GetGUID();
        if (Creature* drekthar = SummonActor(NPC_DREKTHAR, GalvangarTunnelPos))
            _drektharGUID = drekthar->GetGUID();

        me->GetMotionMaster()->MovePoint(POINT_FROSTWOLF_VANTAGE, FrostwolfVantagePos, false);
        _events.ScheduleEvent(EVENT_FROSTWOLF_ACTORS_BOARD, 500ms);
    }

    void SceneEvent(uint32 eventId, Player* player) override
    {
        Creature* galvangar = GetActor(_galvangarGUID);
        Creature* drekthar = GetActor(_drektharGUID);
        if (!galvangar || !drekthar)
        {
            Finish();
            return;
        }

        switch (eventId)
        {
            case EVENT_FROSTWOLF_ACTORS_BOARD:
                if (Creature* wheelchair = GetActor(_wheelchairGUID))
                {
                    wheelchair->EnterVehicle(galvangar, 0);
                    drekthar->EnterVehicle(wheelchair, 0);
                }
                _events.ScheduleEvent(EVENT_FROSTWOLF_GALVANGAR_WALK, 2s);
                break;
            case EVENT_FROSTWOLF_GALVANGAR_WALK:
                galvangar->SetWalk(true);
                galvangar->GetMotionMaster()->MovePoint(POINT_GALVANGAR_OUT_OF_THE_TUNNEL, GalvangarStopPos);
                // About the walk out of the tunnel (16 yd)
                _events.ScheduleEvent(EVENT_FROSTWOLF_TALK, 7s);
                break;
            case EVENT_FROSTWOLF_TALK:
                if (_line == 0)
                    galvangar->SetFacingToObject(player);
                drekthar->AI()->Talk(TALK_DREKTHAR_FIRST + _line, player);
                if (_line < TALK_DREKTHAR_LAST)
                    _events.ScheduleEvent(EVENT_FROSTWOLF_TALK, DrekTharLineDelays[_line]);
                else
                    _events.ScheduleEvent(EVENT_FROSTWOLF_GALVANGAR_TURN_BACK, DrekTharLineDelays[_line]);
                ++_line;
                break;
            case EVENT_FROSTWOLF_GALVANGAR_TURN_BACK:
                galvangar->GetMotionMaster()->MovePoint(0, GalvangarTunnelPos);
                _events.ScheduleEvent(EVENT_FROSTWOLF_END, 4s);
                break;
            case EVENT_FROSTWOLF_END:
                Finish();
                break;
            default:
                break;
        }
    }

    void Arrived(uint32 pointId, Player* /*player*/) override
    {
        if (pointId == POINT_FROSTWOLF_VANTAGE)
            FaceTo(GalvangarStopPos.GetPositionX() + 4.0f, GalvangarStopPos.GetPositionY());
    }

private:
    ObjectGuid _galvangarGUID;
    ObjectGuid _wheelchairGUID;
    ObjectGuid _drektharGUID;
    uint8 _line;
};

// 48700 - Cloned Image
// Summon Cloned Image (triggered by Summon Aid of the Frostwolf Camera) stands in for the hidden player in front of Voggah.
// Mirror Image Aura makes it look like the player; it goes when the player leaves the camera.
struct npc_hillsbrad_cloned_image : public ScriptedAI
{
    npc_hillsbrad_cloned_image(Creature* creature) : ScriptedAI(creature), _checkTimer(5000) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _playerGUID = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
        // Mirror Image Aura, put on directly (its target is a nearby-entry search): the player is its caster, so it clones the player
        player->AddAura(SPELL_MIRROR_IMAGE_AURA, me);
        me->SetFacingTo(me->GetAngle(GalvangarStopPos.GetPositionX(), GalvangarStopPos.GetPositionY()));
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 1000;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->GetVehicle())
            me->DespawnOrUnsummon();
    }

private:
    ObjectGuid _playerGUID;
    uint32 _checkTimer;
};

/*######
## High Warlord Cromush: March of the Stormpike (28506), Stormpike Apocalypse (28616)
######*/

enum StormpikeScenes
{
    QUEST_MARCH_OF_THE_STORMPIKE                = 28506,
    QUEST_STORMPIKE_APOCALYPSE                  = 28616,

    NPC_HIGH_WARLORD_CROMUSH                    = 48545,
    NPC_TARREN_MILL_CAMERA                      = 48696,
    NPC_STORMPIKE_APOCALYPSE_CAMERA             = 49152,
    NPC_GOBLIN_POCKET_NUKE_GAVINS_NAZE          = 49153,
    NPC_GOBLIN_POCKET_NUKE_THE_HEADLAND         = 49168,
    NPC_GOBLIN_POCKET_NUKE_CORRAHNS_DAGGER      = 49169,
    NPC_GOBLIN_POCKET_NUKE_SOFERAS_NAZE         = 49171,
    NPC_MAGRUDER                                = 49165,
    NPC_LIEUTENANT_JAN_LAGRAVE                  = 49166,

    SPELL_SUMMON_TARREN_MILL_CAMERA             = 90713,
    SPELL_CANCEL_CINEMATIC_MARCH                = 90711,
    SPELL_SUMMON_STORMPIKE_APOCALYPSE           = 91608,
    SPELL_CANCEL_CINEMATIC_APOCALYPSE           = 91607,
    SPELL_GLOBAL_THERMONUCLEAR_WAR              = 91617,

    TALK_CROMUSH_HILLSBRAD_IS_OURS              = 10,

    // Tarren Mill Camera: the narration, in order
    TALK_MARCH_FIRST                            = 0,

    TALK_LAGRAVE_DEFUSE                         = 0,
    TALK_MAGRUDER_WHAT_IS_THIS                  = 0,
    TALK_MAGRUDER_WIRES                         = 1,
    TALK_MAGRUDER_RUN                           = 2,

    POINT_MARCH_SOFERAS_NAZE                    = 1,
    POINT_MARCH_CORRAHNS_DAGGER                 = 2,
    POINT_MARCH_THE_HEADLAND                    = 3,
    POINT_MARCH_GAVINS_NAZE                     = 4,
    POINT_MARCH_GRASP                           = 5,

    POINT_APOCALYPSE_GAVINS_NAZE                = 1,
    POINT_APOCALYPSE_THE_HEADLAND               = 2,
    POINT_APOCALYPSE_CORRAHNS_DAGGER            = 3,
    POINT_APOCALYPSE_SOFERAS_NAZE               = 4,

    EVENT_MARCH_SECOND_LINE                     = EVENT_SCENE_FIRST,
    EVENT_MARCH_FLY_ON,
    EVENT_MARCH_END,

    EVENT_APOCALYPSE_FLY                        = EVENT_SCENE_FIRST,
    EVENT_APOCALYPSE_LAGRAVE_DEFUSE,
    EVENT_APOCALYPSE_MAGRUDER_WHAT_IS_THIS,
    EVENT_APOCALYPSE_MAGRUDER_WIRES,
    EVENT_APOCALYPSE_MAGRUDER_RUN,
    EVENT_APOCALYPSE_GAVINS_NAZE_BLOWS,
    EVENT_APOCALYPSE_TO_THE_HEADLAND,
    EVENT_APOCALYPSE_TO_CORRAHNS_DAGGER,
    EVENT_APOCALYPSE_TO_SOFERAS_NAZE,
    EVENT_APOCALYPSE_END
};

// 48545 - High Warlord Cromush
// Accepting March of the Stormpike or Stormpike Apocalypse starts its scene (Summon Tarren Mill Camera, Summon Stormpike
// Apocalypse, cast on the player).
struct npc_hillsbrad_high_warlord_cromush : public ScriptedAI
{
    npc_hillsbrad_high_warlord_cromush(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_MARCH_OF_THE_STORMPIKE)
            StartScene(player, QUEST_MARCH_OF_THE_STORMPIKE, NPC_TARREN_MILL_CAMERA, SPELL_SUMMON_TARREN_MILL_CAMERA);
        else if (quest->GetQuestId() == QUEST_STORMPIKE_APOCALYPSE)
            StartScene(player, QUEST_STORMPIKE_APOCALYPSE, NPC_STORMPIKE_APOCALYPSE_CAMERA, SPELL_SUMMON_STORMPIKE_APOCALYPSE);
    }

    bool OnGossipHello(Player* player) override
    {
        return ReplayScene(player, QUEST_MARCH_OF_THE_STORMPIKE, NPC_TARREN_MILL_CAMERA, SPELL_SUMMON_TARREN_MILL_CAMERA)
            || ReplayScene(player, QUEST_STORMPIKE_APOCALYPSE, NPC_STORMPIKE_APOCALYPSE_CAMERA, SPELL_SUMMON_STORMPIKE_APOCALYPSE);
    }
};

// Where the player set the pocket-nukes in Breaking the Hand: the four Credit bunnies (and Four Fingers Spell Focus objects)
Position const SoferasNazeNukePos       = { 235.465f, -850.531f, 146.945f };
Position const CorrahnsDaggerNukePos    = { -26.2726f, -550.632f, 151.308f };
Position const TheHeadlandNukePos       = { -104.951f, -287.163f, 143.865f };
Position const GavinsNazeNukePos        = { -104.993f, -69.8663f, 147.112f };

// 48696 - Tarren Mill Camera
// Summoned on the player with phase 355 (Tarren Mill Camera). It flies from Tarren Mill over the four fingers of Hillsbrad in
// the order of the narration, stopping at each while the two lines on it show, rises over Gavin's Naze for the last line, and
// Cancel Cinematic puts the player back in front of Cromush and completes the quest. The path is ours: 17 yd or more above the
// ground on every leg.
struct npc_hillsbrad_tarren_mill_camera : public npc_hillsbrad_scene_camera
{
    npc_hillsbrad_tarren_mill_camera(Creature* creature) : npc_hillsbrad_scene_camera(creature, QUEST_MARCH_OF_THE_STORMPIKE, SPELL_CANCEL_CINEMATIC_MARCH, 3.5f), _line(TALK_MARCH_FIRST), _stop(0) { }

    void Start(Player* /*player*/) override
    {
        FlyOn();
    }

    void SceneEvent(uint32 eventId, Player* player) override
    {
        switch (eventId)
        {
            case EVENT_MARCH_SECOND_LINE:
                Talk(_line++, player);
                _events.ScheduleEvent(EVENT_MARCH_FLY_ON, 5s);
                break;
            case EVENT_MARCH_FLY_ON:
                FlyOn();
                break;
            case EVENT_MARCH_END:
                Finish();
                break;
            default:
                break;
        }
    }

    void Arrived(uint32 pointId, Player* player) override
    {
        switch (pointId)
        {
            case POINT_MARCH_SOFERAS_NAZE:
                FaceTo(SoferasNazeNukePos.GetPositionX(), SoferasNazeNukePos.GetPositionY());
                break;
            case POINT_MARCH_CORRAHNS_DAGGER:
                FaceTo(CorrahnsDaggerNukePos.GetPositionX(), CorrahnsDaggerNukePos.GetPositionY());
                break;
            case POINT_MARCH_THE_HEADLAND:
                FaceTo(TheHeadlandNukePos.GetPositionX(), TheHeadlandNukePos.GetPositionY());
                break;
            case POINT_MARCH_GAVINS_NAZE:
                FaceTo(GavinsNazeNukePos.GetPositionX(), GavinsNazeNukePos.GetPositionY());
                break;
            case POINT_MARCH_GRASP:
                // "Together the fingers represent the grasp of Stormpike closing upon us."
                Talk(_line++, player);
                _events.ScheduleEvent(EVENT_MARCH_END, 6s);
                return;
            default:
                return;
        }

        // The finger's name, then what the Stormpike do there
        Talk(_line++, player);
        _events.ScheduleEvent(EVENT_MARCH_SECOND_LINE, 5s);
    }

private:
    void FlyOn()
    {
        switch (++_stop)
        {
            case POINT_MARCH_SOFERAS_NAZE:
                FlyPath(POINT_MARCH_SOFERAS_NAZE, { { -5.0f, -895.0f, 95.0f }, { 100.0f, -860.0f, 160.0f }, { 175.0f, -865.0f, 185.0f } });
                break;
            case POINT_MARCH_CORRAHNS_DAGGER:
                FlyPath(POINT_MARCH_CORRAHNS_DAGGER, { { 60.0f, -720.0f, 200.0f }, { -10.0f, -610.0f, 195.0f } });
                break;
            case POINT_MARCH_THE_HEADLAND:
                FlyPath(POINT_MARCH_THE_HEADLAND, { { -100.0f, -420.0f, 200.0f }, { -120.0f, -345.0f, 190.0f } });
                break;
            case POINT_MARCH_GAVINS_NAZE:
                FlyPath(POINT_MARCH_GAVINS_NAZE, { { -90.0f, -230.0f, 190.0f }, { -65.0f, -155.0f, 175.0f } });
                break;
            case POINT_MARCH_GRASP:
                FlyPath(POINT_MARCH_GRASP, { { -45.0f, -235.0f, 230.0f } });
                break;
            default:
                Finish();
                break;
        }
    }

    uint8 _line;
    uint8 _stop;
};

// Cromush's yell once the player is back in Tarren Mill (Cancel Cinematic has put the player in front of him)
static void CromushHillsbradIsOurs(Player* player)
{
    if (Creature* cromush = player->FindNearestCreature(NPC_HIGH_WARLORD_CROMUSH, 30.0f))
        if (cromush->IsAIEnabled)
            cromush->AI()->Talk(TALK_CROMUSH_HILLSBRAD_IS_OURS, player);
}

// 49152 - Stormpike Apocalypse Camera
// Summoned on the player with phase 364 (Stormpike Apocalypse Camera). The actors are those of the 5.4.8 summon spells, done
// directly: Magruder (Summon Magruder) and Lieutenant Jan Lagrave (Summon Pixin) at the pocket-nuke on Gavin's Naze (Summon
// Goblin Pocket Nuke), and one pocket-nuke on each of the other fingers. The camera flies to Gavin's Naze, where Lagrave asks
// Magruder to defuse the nuke and Magruder runs; the nuke goes off (Global Thermonuclear War), and the camera pulls back over
// the Headland, Corrahn's Dagger and Sofera's Naze as their nukes go off in turn. Cancel Cinematic puts the player back in
// front of Cromush and completes the quest, and Cromush yells "LOK'TAR! HILLSBRAD IS OURS!".
struct npc_hillsbrad_stormpike_apocalypse_camera : public npc_hillsbrad_scene_camera
{
    npc_hillsbrad_stormpike_apocalypse_camera(Creature* creature) : npc_hillsbrad_scene_camera(creature, QUEST_STORMPIKE_APOCALYPSE, SPELL_CANCEL_CINEMATIC_APOCALYPSE, 5.0f)
    {
        _returnHook = &CromushHillsbradIsOurs;
    }

    void Start(Player* /*player*/) override
    {
        _nukeGUIDs[0] = SummonNuke(NPC_GOBLIN_POCKET_NUKE_GAVINS_NAZE, GavinsNazeNukePos);
        _nukeGUIDs[1] = SummonNuke(NPC_GOBLIN_POCKET_NUKE_THE_HEADLAND, TheHeadlandNukePos);
        _nukeGUIDs[2] = SummonNuke(NPC_GOBLIN_POCKET_NUKE_CORRAHNS_DAGGER, CorrahnsDaggerNukePos);
        _nukeGUIDs[3] = SummonNuke(NPC_GOBLIN_POCKET_NUKE_SOFERAS_NAZE, SoferasNazeNukePos);

        // Bent over the nuke, on the platform where it stands (ours)
        if (Creature* magruder = SummonActor(NPC_MAGRUDER, { -102.0f, -72.5f, 147.0f }))
        {
            magruder->SetFacingTo(magruder->GetAngle(&GavinsNazeNukePos));
            _magruderGUID = magruder->GetGUID();
        }
        if (Creature* lagrave = SummonActor(NPC_LIEUTENANT_JAN_LAGRAVE, { -107.5f, -72.5f, 147.0f }))
        {
            lagrave->SetFacingTo(lagrave->GetAngle(&GavinsNazeNukePos));
            _lagraveGUID = lagrave->GetGUID();
        }

        _events.ScheduleEvent(EVENT_APOCALYPSE_FLY, 1s);
    }

    void SceneEvent(uint32 eventId, Player* player) override
    {
        Creature* magruder = GetActor(_magruderGUID);
        Creature* lagrave = GetActor(_lagraveGUID);

        switch (eventId)
        {
            case EVENT_APOCALYPSE_FLY:
                FlyPath(POINT_APOCALYPSE_GAVINS_NAZE,
                {
                    { -5.0f, -890.0f, 100.0f },
                    { -60.0f, -600.0f, 200.0f },
                    { -100.0f, -330.0f, 195.0f },
                    { -95.0f, -140.0f, 175.0f },
                    { -93.0f, -92.0f, 158.0f }
                });
                break;
            case EVENT_APOCALYPSE_LAGRAVE_DEFUSE:
                if (lagrave)
                    lagrave->AI()->Talk(TALK_LAGRAVE_DEFUSE, player);
                _events.ScheduleEvent(EVENT_APOCALYPSE_MAGRUDER_WHAT_IS_THIS, 4s);
                break;
            case EVENT_APOCALYPSE_MAGRUDER_WHAT_IS_THIS:
                if (magruder)
                    magruder->AI()->Talk(TALK_MAGRUDER_WHAT_IS_THIS, player);
                _events.ScheduleEvent(EVENT_APOCALYPSE_MAGRUDER_WIRES, 4s);
                break;
            case EVENT_APOCALYPSE_MAGRUDER_WIRES:
                if (magruder)
                    magruder->AI()->Talk(TALK_MAGRUDER_WIRES, player);
                _events.ScheduleEvent(EVENT_APOCALYPSE_MAGRUDER_RUN, 6s);
                break;
            case EVENT_APOCALYPSE_MAGRUDER_RUN:
                if (magruder)
                {
                    magruder->AI()->Talk(TALK_MAGRUDER_RUN, player);
                    magruder->SetWalk(false);
                    magruder->GetMotionMaster()->MovePoint(0, -95.0f, -45.0f, 142.0f);
                }
                if (lagrave)
                {
                    lagrave->SetWalk(false);
                    lagrave->GetMotionMaster()->MovePoint(0, -110.0f, -45.0f, 145.0f);
                }
                _events.ScheduleEvent(EVENT_APOCALYPSE_GAVINS_NAZE_BLOWS, 2s);
                break;
            case EVENT_APOCALYPSE_GAVINS_NAZE_BLOWS:
                Detonate(0);
                _events.ScheduleEvent(EVENT_APOCALYPSE_TO_THE_HEADLAND, 3s);
                break;
            case EVENT_APOCALYPSE_TO_THE_HEADLAND:
                FlyPath(POINT_APOCALYPSE_THE_HEADLAND, { { -85.0f, -170.0f, 215.0f }, { -75.0f, -245.0f, 225.0f } });
                break;
            case EVENT_APOCALYPSE_TO_CORRAHNS_DAGGER:
                FlyPath(POINT_APOCALYPSE_CORRAHNS_DAGGER, { { -40.0f, -480.0f, 230.0f } });
                break;
            case EVENT_APOCALYPSE_TO_SOFERAS_NAZE:
                FlyPath(POINT_APOCALYPSE_SOFERAS_NAZE, { { 60.0f, -700.0f, 235.0f }, { 150.0f, -790.0f, 225.0f } });
                break;
            case EVENT_APOCALYPSE_END:
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
            case POINT_APOCALYPSE_GAVINS_NAZE:
                FaceTo(GavinsNazeNukePos.GetPositionX(), GavinsNazeNukePos.GetPositionY());
                _events.ScheduleEvent(EVENT_APOCALYPSE_LAGRAVE_DEFUSE, 1s);
                break;
            case POINT_APOCALYPSE_THE_HEADLAND:
                FaceTo(TheHeadlandNukePos.GetPositionX(), TheHeadlandNukePos.GetPositionY());
                Detonate(1);
                // The next leg starts from UpdateAI, not from inside the movement generator that just finished
                _events.ScheduleEvent(EVENT_APOCALYPSE_TO_CORRAHNS_DAGGER, 100ms);
                break;
            case POINT_APOCALYPSE_CORRAHNS_DAGGER:
                FaceTo(CorrahnsDaggerNukePos.GetPositionX(), CorrahnsDaggerNukePos.GetPositionY());
                Detonate(2);
                _events.ScheduleEvent(EVENT_APOCALYPSE_TO_SOFERAS_NAZE, 100ms);
                break;
            case POINT_APOCALYPSE_SOFERAS_NAZE:
                FaceTo(SoferasNazeNukePos.GetPositionX(), SoferasNazeNukePos.GetPositionY());
                Detonate(3);
                _events.ScheduleEvent(EVENT_APOCALYPSE_END, 5s);
                break;
            default:
                break;
        }
    }

private:
    ObjectGuid SummonNuke(uint32 entry, Position const& pos)
    {
        if (Creature* nuke = SummonActor(entry, pos))
            return nuke->GetGUID();
        return ObjectGuid::Empty;
    }

    // Global Thermonuclear War: its conditions keep it on the nukes and the two engineers
    void Detonate(uint8 index)
    {
        if (Creature* nuke = GetActor(_nukeGUIDs[index]))
            nuke->CastSpell(nuke, SPELL_GLOBAL_THERMONUCLEAR_WAR, true);
    }

    ObjectGuid _nukeGUIDs[4];
    ObjectGuid _magruderGUID;
    ObjectGuid _lagraveGUID;
};

/*######
## Welcome to the Machine (28096)
######*/

// The player sits on the Skeletal Steed at Southpoint Gate as the quest giver: Dumass walks in, then Kingslayer Orkus flies in on
// his frost wyrm, then Johnny Awesome rides in, each only once the one before has been handed his quest and has gone, while
// High Executor Darthalia comments. Each is talked into his quest through two or three gossip windows, says his parting line and
// leaves the way he was sent: Dumass, sent southeast to the Azurelode Mine, runs north; Orkus flies off southeast over the gate;
// Johnny rides northeast through the gate towards the Sludge Fields. Getting off the steed sends them all away (the 2011-09-09
// hotfix on warcraft.wiki.gg).
//
// Sourced (5.4.8 data): the spells - 88542 Summon Skeletal Steed, whose effect value names the ride spell 88543 Ride Skeletal
// Steed (phase 317 "Welcome to the Machine", quest invisibility 4 on the rider), 88476 Quest Giver (makes its target cast 88563
// Summon High Executor Darthalia), 88574 Cancel Questgiver; the spell_target_position spots of the summons; every line
// (broadcast_text 47658-47689); the gossip windows and their options (gossip_menu 12272-12279 with their npc_text, the options
// of the retail TDB); the mounts; the order and the directions (warcraft.wiki.gg, the options' own text). Ours: who casts the
// Quest Giver aura and when (the player, once seated) and that it ends with the scene, every timing, the spot in front of the
// steed, Orkus' start in the air and every approach and departure path, and that the visitors are summoned from here as private
// summons of the player (the spells' summon properties are personal) rather than by casting their spells.

enum WelcomeToTheMachine
{
    QUEST_WELCOME_TO_THE_MACHINE                = 28096,

    NPC_MACHINE_JOHNNY_AWESOME                  = 47442,
    NPC_MACHINE_KINGSLAYER_ORKUS                = 47443,
    NPC_MACHINE_DUMASS                          = 47444,
    NPC_MACHINE_DARTHALIA                       = 47499,

    SPELL_MACHINE_QUEST_GIVER                   = 88476,    // forces its target to cast 88563 Summon High Executor Darthalia
    SPELL_MACHINE_CANCEL_QUESTGIVER             = 88574,    // removes 88476 from its caster

    // The last gossip window of each visitor: its option hands him his quest (12273 -> 12272, 12276 -> 12275 -> 12274,
    // 12279 -> 12278 -> 12277; the windows before it only lead on, through gossip_menu_option.ActionMenuID)
    GOSSIP_MENU_MACHINE_DUMASS_LAST             = 12272,
    GOSSIP_MENU_MACHINE_ORKUS_LAST              = 12274,
    GOSSIP_MENU_MACHINE_JOHNNY_LAST             = 12277,

    // High Executor Darthalia (47499)
    TALK_MACHINE_DARTHALIA_STAND_TALL           = 0,    // 47658
    TALK_MACHINE_DARTHALIA_ONE_COMING           = 1,    // 47659
    TALK_MACHINE_DARTHALIA_NEW_FORSAKEN         = 2,    // 47666
    TALK_MACHINE_DARTHALIA_THIS_ORC             = 3,    // 47667
    TALK_MACHINE_DARTHALIA_NEVER_RETURNS        = 4,    // 47680
    TALK_MACHINE_DARTHALIA_ON_YOUR_OWN          = 5,    // 47681

    // The visitors: group 0 is the parting line, the arrival lines start at group 1
    TALK_MACHINE_VISITOR_PARTING                = 0,
    TALK_MACHINE_VISITOR_ARRIVAL                = 1,

    // The steed
    EVENT_MACHINE_CHECK_RIDER                   = 1,
    EVENT_MACHINE_DARTHALIA                     = 2,
    EVENT_MACHINE_NEXT_VISITOR                  = 3,
    EVENT_MACHINE_WATCH_VISITOR                 = 4,
    EVENT_MACHINE_DISMOUNT                      = 5,

    // The visitors
    EVENT_MACHINE_VISITOR_ARRIVED               = 1,
    EVENT_MACHINE_VISITOR_TALK                  = 2,
    EVENT_MACHINE_VISITOR_STUCK                 = 3,
    EVENT_MACHINE_VISITOR_LEAVE                 = 4,
    EVENT_MACHINE_VISITOR_LEAVE_NEXT            = 5,
    EVENT_MACHINE_VISITOR_GONE                  = 6,

    POINT_MACHINE_ARRIVE                        = 1,
    POINT_MACHINE_LEAVE                         = 2,

    MACHINE_VISITOR_COUNT                       = 3
};

struct MachineVisitor
{
    uint32 Entry;
    Position Spawn;             // spell_target_position of its summon spell (Orkus: ours, see below)
    uint8 ArrivalLines;
    int8 DarthaliaBefore;       // her line as he comes
    int8 DarthaliaAfter;        // and once he is gone (-1: none)
};

MachineVisitor const MachineVisitors[MACHINE_VISITOR_COUNT] =
{
    { NPC_MACHINE_DUMASS,           { -580.382f, 453.204f, 81.575f, 4.7473f },   1, TALK_MACHINE_DARTHALIA_ONE_COMING,  TALK_MACHINE_DARTHALIA_NEW_FORSAKEN },
    // Orkus flies in: he starts in the air about 100 yd behind his summon spot (88560: -586.156 472.934 82.487), out of
    // sight but inside the cells that are updated around the player, and passes over that spot
    { NPC_MACHINE_KINGSLAYER_ORKUS, { -605.0f, 520.0f, 115.0f, 4.94f },          3, TALK_MACHINE_DARTHALIA_THIS_ORC,    TALK_MACHINE_DARTHALIA_NEVER_RETURNS },
    { NPC_MACHINE_JOHNNY_AWESOME,   { -582.636f, 470.513f, 82.4887f, 4.76475f }, 1, TALK_MACHINE_DARTHALIA_ON_YOUR_OWN, -1 }
};

float const MachineSteedFacing = 2.47837f;     // the steed at the gate (47445)
Position const MachineDarthaliaSpot = { -572.135f, 428.285f, 80.3627f, 2.25147f };   // 88563 (where the real Darthalia stands)
Position const MachineVisitorSpot   = { -578.5f, 427.5f, 80.2f };                    // in front of the steed

// Orkus' approach: down over his summon spot to the spot in front of the steed (ground clearance checked against the
// 5.4.8 terrain and vmaps: 30 yd at the start, nothing in the way)
Position const MachineOrkusArrival[] =
{
    { -586.2f, 472.9f, 100.0f },
    { -581.0f, 446.0f, 88.0f },
    { -578.5f, 427.5f, 80.2f }
};

// The departures, each the way the visitor was sent. The camp's wall runs north-south at y 392-404 with the gate at
// x -572 to -596, so everyone going into Hillsbrad passes through the gate.
// Dumass: sent southeast, he runs north along the camp (the joke of "NORTH! GOT IT!")
Position const MachineDumassDeparture[] =
{
    { -535.0f, 412.0f, 85.0f },
    { -515.0f, 414.0f, 87.0f }
};
// Orkus: southeast, "more south than east, probably off the coast": he takes off and flies out over the gate
Position const MachineOrkusDeparture[] =
{
    { -586.0f, 412.0f, 90.0f },
    { -590.0f, 395.0f, 99.0f },
    { -630.0f, 375.0f, 110.0f },
    { -660.0f, 362.0f, 118.0f }
};
// Johnny: northeast to the Sludge Fields, through the gate
Position const MachineJohnnyDeparture[] =
{
    { -584.0f, 398.0f, 77.7f },
    { -555.0f, 362.0f, 81.0f }
};

// 47444 Dumass, 47443 Kingslayer Orkus, 47442 Johnny Awesome - the player's private summons, called by the steed below
struct npc_hillsbrad_machine_visitor : public ScriptedAI
{
    npc_hillsbrad_machine_visitor(Creature* creature) : ScriptedAI(creature), _arrivalLine(0), _leavePoint(0), _arrived(false),
        _leaving(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (!summoner->IsPlayer())
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = summoner->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        // Not to be talked to before he stands in front of the steed
        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);

        switch (me->GetEntry())
        {
            case NPC_MACHINE_KINGSLAYER_ORKUS:
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                FlyPath(POINT_MACHINE_ARRIVE, std::begin(MachineOrkusArrival), std::end(MachineOrkusArrival));
                break;
            case NPC_MACHINE_JOHNNY_AWESOME:
                me->SetWalk(false);
                me->GetMotionMaster()->MovePoint(POINT_MACHINE_ARRIVE, MachineVisitorSpot);
                break;
            default:
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(POINT_MACHINE_ARRIVE, MachineVisitorSpot);
                break;
        }

        // Whatever kept him from coming, he must be there to be talked to (ours: the quest stays completable)
        _events.ScheduleEvent(EVENT_MACHINE_VISITOR_STUCK, 40s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        // The next move is started from the update: a move started here is undone when the finished one is finalized
        if (pointId == POINT_MACHINE_ARRIVE && !_arrived)
            _events.ScheduleEvent(EVENT_MACHINE_VISITOR_ARRIVED, 100ms);
        else if (pointId == POINT_MACHINE_LEAVE && _leaving)
            _events.ScheduleEvent(EVENT_MACHINE_VISITOR_LEAVE_NEXT, 100ms);
    }

    // Group members see each other's private summons; a visitor only talks to the player on the steed
    bool OnGossipHello(Player* player) override
    {
        if (player->GetGUID() == _playerGUID)
            return false;

        CloseGossipMenuFor(player);
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 /*gossipListId*/) override
    {
        if (player->GetGUID() != _playerGUID)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        // The menu id comes from the client: only the window the player really has open counts
        if (_leaving || menuId != LastMenu() || player->PlayerTalkClass->GetGossipMenu().GetMenuId() != menuId)
            return false;   // the windows before the last one lead on by themselves

        CloseGossipMenuFor(player);
        _leaving = true;
        _events.Reset();
        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
        Talk(TALK_MACHINE_VISITOR_PARTING, player);
        player->KilledMonsterCredit(me->GetEntry());
        _events.ScheduleEvent(EVENT_MACHINE_VISITOR_LEAVE, 3s);
        // Gone at the latest after this, wherever his way ended
        _events.ScheduleEvent(EVENT_MACHINE_VISITOR_GONE, 30s);
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            me->DespawnOrUnsummon();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MACHINE_VISITOR_STUCK:
                    if (!_arrived)
                    {
                        me->GetMotionMaster()->Clear();
                        me->NearTeleportTo(MachineVisitorSpot.GetPositionX(), MachineVisitorSpot.GetPositionY(),
                            MachineVisitorSpot.GetPositionZ(), me->GetOrientation());
                        Arrive(player);
                    }
                    break;
                case EVENT_MACHINE_VISITOR_ARRIVED:
                    if (!_arrived)
                        Arrive(player);
                    break;
                case EVENT_MACHINE_VISITOR_TALK:
                    Talk(TALK_MACHINE_VISITOR_ARRIVAL + _arrivalLine, player);
                    if (++_arrivalLine < ArrivalLines())
                        _events.ScheduleEvent(EVENT_MACHINE_VISITOR_TALK, 5s);
                    break;
                case EVENT_MACHINE_VISITOR_LEAVE:
                    if (me->GetEntry() == NPC_MACHINE_KINGSLAYER_ORKUS)
                    {
                        me->SetCanFly(true);
                        me->SetDisableGravity(true);
                        FlyPath(POINT_MACHINE_LEAVE, std::begin(MachineOrkusDeparture), std::end(MachineOrkusDeparture));
                        _leavePoint = uint8(std::size(MachineOrkusDeparture));
                    }
                    else
                    {
                        me->SetWalk(false);
                        MoveOn();
                    }
                    break;
                case EVENT_MACHINE_VISITOR_LEAVE_NEXT:
                    if (_leavePoint >= DeparturePoints())
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    MoveOn();
                    break;
                case EVENT_MACHINE_VISITOR_GONE:
                    me->DespawnOrUnsummon();
                    return;
                default:
                    break;
            }
        }
    }

private:
    void Arrive(Player* player)
    {
        _arrived = true;
        if (me->GetEntry() == NPC_MACHINE_KINGSLAYER_ORKUS)
        {
            // Lands
            me->SetDisableGravity(false);
            me->SetCanFly(false);
        }
        me->SetFacingToObject(player);
        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        _events.ScheduleEvent(EVENT_MACHINE_VISITOR_TALK, 500ms);
    }

    // The next point of a ground departure
    void MoveOn()
    {
        Position const* path = me->GetEntry() == NPC_MACHINE_JOHNNY_AWESOME ? MachineJohnnyDeparture : MachineDumassDeparture;
        me->GetMotionMaster()->MovePoint(POINT_MACHINE_LEAVE, path[_leavePoint++]);
    }

    void FlyPath(uint32 pointId, Position const* begin, Position const* end)
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());   // replaced by the current position when the spline starts
        path.insert(path.end(), begin, end);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(pointId, path.data(), path.size(), false, true);
    }

    uint8 DeparturePoints() const
    {
        switch (me->GetEntry())
        {
            case NPC_MACHINE_KINGSLAYER_ORKUS: return uint8(std::size(MachineOrkusDeparture));
            case NPC_MACHINE_JOHNNY_AWESOME:   return uint8(std::size(MachineJohnnyDeparture));
            default:                           return uint8(std::size(MachineDumassDeparture));
        }
    }

    uint32 LastMenu() const
    {
        switch (me->GetEntry())
        {
            case NPC_MACHINE_KINGSLAYER_ORKUS: return GOSSIP_MENU_MACHINE_ORKUS_LAST;
            case NPC_MACHINE_JOHNNY_AWESOME:   return GOSSIP_MENU_MACHINE_JOHNNY_LAST;
            default:                           return GOSSIP_MENU_MACHINE_DUMASS_LAST;
        }
    }

    uint8 ArrivalLines() const
    {
        for (MachineVisitor const& visitor : MachineVisitors)
            if (visitor.Entry == me->GetEntry())
                return visitor.ArrivalLines;
        return 0;
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    uint8 _arrivalLine;
    uint8 _leavePoint;
    bool _arrived;
    bool _leaving;
};

// 47488 - Skeletal Steed, summoned by Summon Skeletal Steed (88542) when the player clicks the steed at the gate
// Its one seat (8976) can control it, so seating the player charms the steed; a plain ScriptedAI is switched off for PossessedAI
// then and the scene would never start. VehicleAI stays on, as the scene cameras do.
struct npc_hillsbrad_machine_skeletal_steed : public VehicleAI
{
    npc_hillsbrad_machine_skeletal_steed(Creature* creature) : VehicleAI(creature), _visitor(0), _boarded(false), _ended(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_WELCOME_TO_THE_MACHINE) != QUEST_STATUS_INCOMPLETE || player->GetVehicle())
        {
            me->DespawnOrUnsummon();
            return;
        }

        // Summon Skeletal Steed is a vehicle summon: right after this the core makes the player cast the ride spell its effect
        // value names, Ride Skeletal Steed (88543), which seats the player and puts the player in phase 317 and quest
        // invisibility 4 (hidden from other players). The player's own summons share the phase; the rest of the world, the
        // real Darthalia included, stays in sight through the default phase.
        _playerGUID = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        // Facing the road, as the steed at the gate does (the spell's destination has no orientation)
        me->SetFacingTo(MachineSteedFacing);
        _events.ScheduleEvent(EVENT_MACHINE_CHECK_RIDER, 2s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (passenger->GetGUID() != _playerGUID)
            return;

        if (!apply)
        {
            End();
            return;
        }

        if (!_boarded)
        {
            _boarded = true;
            // "Stand there and wait": the controlling seat would let the player ride off, away from the visitors
            me->SetControlled(true, UNIT_STATE_ROOT);
            me->SetFacingTo(MachineSteedFacing);
            _events.ScheduleEvent(EVENT_MACHINE_DARTHALIA, 1s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        if (_ended)
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player || !player->IsInWorld() || player->GetQuestStatus(QUEST_WELCOME_TO_THE_MACHINE) == QUEST_STATUS_NONE)
            {
                End();
                return;
            }

            switch (eventId)
            {
                case EVENT_MACHINE_CHECK_RIDER:
                    if (!_boarded)
                    {
                        End();
                        return;
                    }
                    break;
                case EVENT_MACHINE_DARTHALIA:
                    // The Quest Giver aura brings the player's own Darthalia (88563, onto the real one's spot); removing it
                    // sends her away (spell_hillsbrad_machine_quest_giver)
                    player->CastSpell(player, SPELL_MACHINE_QUEST_GIVER, true);
                    _darthaliaGUID = FindOwnSummon(player, NPC_MACHINE_DARTHALIA, 30.0f);
                    if (_darthaliaGUID.IsEmpty())
                        if (TempSummon* darthalia = player->SummonCreature(NPC_MACHINE_DARTHALIA, MachineDarthaliaSpot,
                            TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, player->GetGUID()))
                            _darthaliaGUID = darthalia->GetGUID();
                    DarthaliaTalk(TALK_MACHINE_DARTHALIA_STAND_TALL, player);
                    _events.ScheduleEvent(EVENT_MACHINE_NEXT_VISITOR, 6s);
                    break;
                case EVENT_MACHINE_NEXT_VISITOR:
                    // A visitor already dealt with (before a logout) is skipped
                    while (_visitor < MACHINE_VISITOR_COUNT
                        && player->GetReqKillOrCastCurrentCount(QUEST_WELCOME_TO_THE_MACHINE, MachineVisitors[_visitor].Entry))
                        ++_visitor;

                    if (_visitor >= MACHINE_VISITOR_COUNT)
                    {
                        _events.ScheduleEvent(EVENT_MACHINE_DISMOUNT, 3s);
                        break;
                    }

                    DarthaliaTalk(MachineVisitors[_visitor].DarthaliaBefore, player);
                    // He comes, talks and leaves by himself (npc_hillsbrad_machine_visitor)
                    if (TempSummon* visitor = player->SummonCreature(MachineVisitors[_visitor].Entry, MachineVisitors[_visitor].Spawn,
                        TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, player->GetGUID()))
                        _visitorGUID = visitor->GetGUID();
                    _events.ScheduleEvent(EVENT_MACHINE_WATCH_VISITOR, 1s);
                    break;
                case EVENT_MACHINE_WATCH_VISITOR:
                {
                    Creature* visitor = ObjectAccessor::GetCreature(*me, _visitorGUID);
                    if (!visitor || !visitor->IsAlive())
                    {
                        // He has his quest and is gone
                        if (MachineVisitors[_visitor].DarthaliaAfter >= 0)
                            DarthaliaTalk(uint8(MachineVisitors[_visitor].DarthaliaAfter), player);
                        _visitorGUID.Clear();
                        ++_visitor;
                        _events.ScheduleEvent(EVENT_MACHINE_NEXT_VISITOR, 6s);
                        break;
                    }
                    _events.ScheduleEvent(EVENT_MACHINE_WATCH_VISITOR, 1s);
                    break;
                }
                case EVENT_MACHINE_DISMOUNT:
                    // All three have their quests: the player gets off, which ends the scene
                    End();
                    return;
                default:
                    break;
            }
        }
    }

private:
    void DarthaliaTalk(uint8 textId, Player* player)
    {
        if (Creature* darthalia = ObjectAccessor::GetCreature(*me, _darthaliaGUID))
            if (darthalia->IsAIEnabled)
                darthalia->AI()->Talk(textId, player);
    }

    void End()
    {
        if (_ended)
            return;

        _ended = true;
        _events.Reset();

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            if (player->IsInWorld() && !player->GetSession()->PlayerLogout())
                player->CastSpell(player, SPELL_MACHINE_CANCEL_QUESTGIVER, true);
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();
        }

        for (ObjectGuid const& guid : { _visitorGUID, _darthaliaGUID })
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                actor->DespawnOrUnsummon();

        me->DespawnOrUnsummon(500);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _darthaliaGUID;
    ObjectGuid _visitorGUID;
    uint8 _visitor;
    bool _boarded;
    bool _ended;
};

// 88476 - Quest Giver: the player's own Darthalia, whom the aura brought (its effect 1 has the player cast 88563), leaves with it
class spell_hillsbrad_machine_quest_giver : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_machine_quest_giver);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        if (Creature* darthalia = ObjectAccessor::GetCreature(*player, FindOwnSummon(player, NPC_MACHINE_DARTHALIA)))
            darthalia->DespawnOrUnsummon();
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hillsbrad_machine_quest_giver::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_hillsbrad_sludge_stormpike()
{
    new creature_script<npc_hillsbrad_machine_skeletal_steed>("npc_hillsbrad_machine_skeletal_steed");
    new creature_script<npc_hillsbrad_machine_visitor>("npc_hillsbrad_machine_visitor");
    new aura_script<spell_hillsbrad_machine_quest_giver>("spell_hillsbrad_machine_quest_giver");
    new creature_script<npc_hillsbrad_scene_actor>("npc_hillsbrad_scene_actor");
    new creature_script<npc_hillsbrad_warden_stillwater_sludge>("npc_hillsbrad_warden_stillwater_sludge");
    new creature_script<npc_hillsbrad_sludge_fields_camera>("npc_hillsbrad_sludge_fields_camera");
    new creature_script<npc_hillsbrad_warden_stillwater_blight>("npc_hillsbrad_warden_stillwater_blight");
    new creature_script<npc_hillsbrad_voggah_deathgrip>("npc_hillsbrad_voggah_deathgrip");
    new creature_script<npc_hillsbrad_aid_of_the_frostwolf_camera>("npc_hillsbrad_aid_of_the_frostwolf_camera");
    new creature_script<npc_hillsbrad_cloned_image>("npc_hillsbrad_cloned_image");
    new creature_script<npc_hillsbrad_high_warlord_cromush>("npc_hillsbrad_high_warlord_cromush");
    new creature_script<npc_hillsbrad_tarren_mill_camera>("npc_hillsbrad_tarren_mill_camera");
    new creature_script<npc_hillsbrad_stormpike_apocalypse_camera>("npc_hillsbrad_stormpike_apocalypse_camera");
}
