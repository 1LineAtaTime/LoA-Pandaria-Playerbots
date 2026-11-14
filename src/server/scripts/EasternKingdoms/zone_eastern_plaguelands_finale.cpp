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
SDName: Eastern Plaguelands, the Plaguewood and the finale
SDComment: Quest support: 27525 Guardians of Stratholme, 27530 The Corpsebeasts, 27526 Gidwin's Fate Revealed
SDCategory: Eastern Plaguelands
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellInfo.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "Timer.h"
#include "Group.h"
#include "WorldSession.h"

/*######
## Guardians of Stratholme (27525)
######*/

enum GuardiansOfStratholme
{
    NPC_OMASUM_BLIGHTHOOF                       = 45867,
    SPELL_DEATH_GRIP_JUMP                       = 49575     // the death knights' own pull: the target jumps to a destination
};

// A target this close is already at the caster's feet (melee)
float const DeathGripMinDistance = 5.0f;

// 79894 - Death Grip (Omasum Blighthoof 45867, "Cast Death Grip on Close", 10-30 yd)
// Its only effect is a dummy on the target. As a player death knight's grip (49576 -> 49560 -> 49575), the target is pulled to the
// caster: here it jumps to a spot 2 yd in front of Omasum, who faces his target (the spell needs facing). Only for Omasum: the
// spell is also cast by Mogu Infiltrator 66206 (every 2-4 s in combat, at any range) and Tharil'zun 486, for whom it stays the
// dummy it was. Karthis Darkrune 45868 does not have it. Not for a target already in melee range, on a vehicle or a taxi, or
// deflecting all spells (Deterrence), as the player death knight's grip (spell_dk_death_grip_effect)
class spell_epl_lieutenant_death_grip : public SpellScript
{
    PrepareSpellScript(spell_epl_lieutenant_death_grip);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster->GetEntry() != NPC_OMASUM_BLIGHTHOOF || !target || target == caster || target->GetVehicle() || target->IsInFlight()
            || caster->IsWithinMeleeRange(target) || caster->GetExactDist2d(target) < DeathGripMinDistance
            || target->GetTotalAuraModifier(SPELL_AURA_DEFLECT_SPELLS) >= 100)
            return;

        Position const dest = caster->GetNearPosition(2.0f, 0.0f);
        target->CastSpell(dest, SPELL_DEATH_GRIP_JUMP, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_epl_lieutenant_death_grip::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## Scourged Mass (27528)
######*/

enum ScourgedMass
{
    NPC_OVERSTUFFED_GOLEM                       = 45851,
    SPELL_BURN_CORPSE                           = 85555
};

// Burn Corpse's reach (SpellRange 7 yd)
float const BurnCorpseRange = 7.0f;

// 85555 - Burn Corpse (Kevin Frost's Crusader's Torch 61369)
// "Kill Overstuffed Golems, then use the torch on their corpses": effect 0 puts the fire on the nearest Overstuffed Golem within
// 7 yd that is dead and not burning yet (conditions 13/1/85555), effect 1 is the credit (KILL_CREDIT2 to the caster, which this
// core gives to the caster's group as well: Player::RewardPlayerAndGroupAtEvent, the DBC's group form of the credit). When effect
// 0 finds no corpse the cast goes on without a target (Spell::SelectImplicitNearbyTargets returns quietly), so the credit is
// given only when effect 0 has its corpse, and the torch is refused beforehand with no such corpse in reach. Once per corpse: a
// burning corpse is not a target (the burn aura is death-persistent), and it despawns 5 s later (its SmartAI)
class spell_epl_burn_corpse : public SpellScript
{
    PrepareSpellScript(spell_epl_burn_corpse);

    static bool IsBurnable(Unit* caster, Creature* golem)
    {
        return !golem->IsAlive() && !golem->HasAura(SPELL_BURN_CORPSE) && caster->IsWithinDistInMap(golem, BurnCorpseRange);
    }

    SpellCastResult CheckCorpse()
    {
        Unit* caster = GetCaster();
        std::list<Creature*> golems;
        caster->GetCreatureListWithEntryInGrid(golems, NPC_OVERSTUFFED_GOLEM, BurnCorpseRange + 10.0f);
        for (Creature* golem : golems)
            if (IsBurnable(caster, golem))
                return SPELL_CAST_OK;
        return SPELL_FAILED_NO_VALID_TARGETS;
    }

    void NoteCorpse(WorldObject*& target)
    {
        _corpse = target && target->ToCreature() && target->GetEntry() == NPC_OVERSTUFFED_GOLEM;
    }

    void HandleCredit(SpellEffIndex effIndex)
    {
        if (!_corpse)
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_burn_corpse::CheckCorpse);
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_epl_burn_corpse::NoteCorpse, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_epl_burn_corpse::HandleCredit, EFFECT_1, SPELL_EFFECT_KILL_CREDIT2);
    }

    bool _corpse = false;
};

/*######
## The Corpsebeasts (27530)
######*/

enum TheCorpsebeasts
{
    NPC_ARGENT_LIGHTWELL                        = 45921,

    SPELL_LIGHT_BARRAGE                         = 85593,    // cast every second by each lightwell's glow on the hulks within 15 yd
    SPELL_HOLY_OVERLOAD                         = 85594,    // credits the players within 30 yd, ends the lightwells within 15 yd

    LIGHTWELLS_TO_OVERLOAD                      = 3
};

// A lightwell's barrage counts as long as its last hit is this recent: each lightwell hits once a second (85591's period), and the
// barrage's own aura lasts 1.5 s
uint32 const LightBarrageWindowMs = 1500;
// "Must be 10 yards apart" (the quest objectives)
float const LightwellSpacing = 10.0f;

// 45886, 45893, 45895 - Into the Corpsebeast (the lifeless hulks)
// "Place three or more Argent Lightwells around the corpsebeast ... to destroy it." Three lightwells must hit the same hulk: the
// hulk counts the distinct lightwells whose barrage hit it within the last 1.5 s, and at three it overloads (Holy Overload: the
// credit and the end of the lightwells) and goes 2 s later, as the SmartAI rows it replaces did after ten hits from any number.
// The barrage's own aura cannot be counted: the same spell from another caster replaces the old aura (not periodic, no stacking).
struct npc_epl_corpsebeast : public ScriptedAI
{
    npc_epl_corpsebeast(Creature* creature) : ScriptedAI(creature), _overloaded(false) { }

    void Reset() override
    {
        _barrages.clear();
        _overloaded = false;
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (_overloaded || !caster || spell->Id != SPELL_LIGHT_BARRAGE)
            return;

        uint32 const now = getMSTime();
        _barrages[caster->GetGUID()] = now;
        for (auto itr = _barrages.begin(); itr != _barrages.end();)
        {
            if (getMSTimeDiff(itr->second, now) > LightBarrageWindowMs)
                itr = _barrages.erase(itr);
            else
                ++itr;
        }

        if (_barrages.size() < LIGHTWELLS_TO_OVERLOAD)
            return;

        _overloaded = true;
        _barrages.clear();
        me->CastSpell(me, SPELL_HOLY_OVERLOAD, true);
        me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
    }

private:
    std::map<ObjectGuid, uint32> _barrages;
    bool _overloaded;
};

// 85590 - Summon Argent Lightwell (Argent Lightwell Charm 61375)
// "Must be 10 yards apart": refused within 10 yd of any lightwell, anyone's, with the game's own message. Measured flat from where
// the new lightwell will stand: the summon's TARGET_DEST_CASTER_FRONT without a radius puts it the caster's object size in front
// of the caster (Spell::SelectImplicitCasterDestTargets), the same point taken here. Not by a condition: a range check adds the
// object sizes (the lightwell's combat reach of 15 would turn 10 yd into about 26). The sourced marker of the rule is the
// lightwell's own 10 yd buff (Lightwell's Glow 85591, "the lightwells will give you this buff if you're within 10 yards"), which
// stretches the same way here
class spell_epl_summon_argent_lightwell : public SpellScript
{
    PrepareSpellScript(spell_epl_summon_argent_lightwell);

    SpellCastResult CheckSpacing()
    {
        Unit* caster = GetCaster();
        Position const spot = caster->GetFirstCollisionPosition(caster->GetObjectSize(), 0.0f);
        std::list<Creature*> lightwells;
        caster->GetCreatureListWithEntryInGrid(lightwells, NPC_ARGENT_LIGHTWELL, LightwellSpacing + 10.0f);
        for (Creature* lightwell : lightwells)
        {
            if (lightwell->IsAlive() && lightwell->GetExactDist2d(&spot) < LightwellSpacing)
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TOO_CLOSE_ARGENT_LIGHTWELL);
                return SPELL_FAILED_CUSTOM_ERROR;
            }
        }
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_summon_argent_lightwell::CheckSpacing);
    }
};

/*######
## Gidwin's Fate Revealed (27526)
######*/

enum GidwinsFateRevealed
{
    QUEST_GIDWINS_FATE_REVEALED                 = 27526,

    // Who joined the caravan: the criteria of Full Caravan (achievement 5442, Achievement_Criteria.dbc), and Beezil's flag quest,
    // which the caravan's script completes when he joins (his criterion is a spell the 5.4.8 client data no longer has)
    QUEST_BATTLE_OF_DARROWSHIRE                 = 27390,    // Pamela Redpath (her doll rides with the caravan: decisions R8)
    QUEST_HONOR_AND_STRENGTH                    = 27449,    // Vex'tul joins
    QUEST_INTO_THE_FLAMES                       = 27482,    // Vex'tul dies in the fire; he comes back as a ghost
    QUEST_AN_OPPORTUNE_ALLIANCE                 = 27457,    // Rimblat Earthshatter
    QUEST_BEEZILS_FLAG                          = 27563,    // Beezil Linkspanner

    NPC_GFR_GIDWIN                              = 45955,
    NPC_GFR_ANASTARI                            = 45956,
    NPC_GFR_TARENAR                             = 45957,
    NPC_GFR_ARGUS                               = 45958,
    NPC_GFR_FIONA                               = 45959,
    NPC_GFR_RIMBLAT                             = 45960,
    NPC_GFR_PAMELA                              = 45961,
    NPC_GFR_VEXTUL_DEAD                         = 45962,
    NPC_GFR_VEXTUL_LIVE                         = 45963,
    NPC_GFR_GEORGE                              = 45964,
    NPC_GFR_CAMERA                              = 45965,
    NPC_GFR_DOPPELGANGER                        = 45978,
    NPC_GFR_BEEZIL                              = 46043,
    NPC_GFR_CREDIT                              = 45940,    // GFR Credit: "Find Gidwin Goldbraids"

    // All of them summon at their spell_target_position spot, personal to the player (SummonProperties 2993 / 3035, flag 0x10)
    SPELL_GFR_SUMMON_GIDWIN                     = 85649,
    SPELL_GFR_SUMMON_ANASTARI                   = 85650,
    SPELL_GFR_SUMMON_TARENAR                    = 85651,
    SPELL_GFR_SUMMON_FIONA                      = 85652,
    SPELL_GFR_SUMMON_RIMBLAT                    = 85653,
    SPELL_GFR_SUMMON_ARGUS                      = 85655,
    SPELL_GFR_SUMMON_VEXTUL_LIVE                = 85656,
    SPELL_GFR_SUMMON_GEORGE                     = 85657,
    SPELL_GFR_SUMMON_VEXTUL_DEAD                = 85658,
    SPELL_GFR_SUMMON_PAMELA                     = 85659,
    SPELL_GFR_SUMMON_CAMERA                     = 85663,
    SPELL_GFR_SUMMON_DOPPELGANGER               = 85680,
    SPELL_GFR_MIRROR_IMAGE                      = 85682,
    SPELL_GFR_SUMMON_BEEZIL                     = 85833,

    // GFR Event Camera 45965 (Vehicle 1156): one seat, 8645 (hides its passenger, who cannot step off), CameraMode 70 "Eastern
    // Plaguelands 4.x - GFR Event Camera": the eye at the camera itself, locked on the world point (3004.8, -3438.89, 152.0) with a
    // field of view of 90, about +-22 degrees vertically and +-36 horizontally at 16:9. The camera stands at the slaughterhouse's
    // entry (85663's spot, 0.4 yd above the floor) and looks down into the chamber: the table, the Baroness, George and the ghosts
    // behind her are in the picture, both entry spots (where the party walks in from) behind the viewer. No channel aims it
    // (flags 0x170: the look-at is a world point)
    GFR_CAMERA_SEAT                             = 0,

    // Gidwin Goldbraids 45955
    TALK_GFR_GIDWIN_WITCH                       = 0,    // You'll never get away with this, you ugly, stinkin' witch!
    // Baroness Anastari 45956
    TALK_GFR_ANASTARI_NOBODY                    = 0,    // Oh really?  What exactly will you do, dwarf?  There's nobody to save you here.
    TALK_GFR_ANASTARI_TWO_OF_YOU                = 1,    // Just the two of you?  You'll never stand a chance.
    // Tarenar Sunstrike 45957
    TALK_GFR_TARENAR_WRONG                      = 0,    // You're wrong, banshee.  We've come to take Gidwin back.
    TALK_GFR_TARENAR_CHARGE                     = 1,    // Charge! (yell)
    // Argus Highbeacon 45958
    TALK_GFR_ARGUS_DIFFER                       = 0,    // I beg to differ.

    EVENT_GFR_WAIT_FOR_RIDER                    = 1,
    EVENT_GFR_GIDWIN_TAUNTS,
    EVENT_GFR_ANASTARI_SCOFFS,
    EVENT_GFR_TARENAR_ENTERS,
    EVENT_GFR_TARENAR_ANSWERS,
    EVENT_GFR_ANASTARI_LAUGHS,
    EVENT_GFR_PARTY_ENTERS,
    EVENT_GFR_ARGUS_DIFFERS,
    EVENT_GFR_CHARGE,
    EVENT_GFR_ANASTARI_FALLS,
    EVENT_GFR_END,

    DATA_GFR_WATCHER                            = 1,

    ACTION_GFR_FIGHT                            = 1,
    ACTION_GFR_STAND_DOWN                       = 2,

    POINT_GFR_STOP                              = 1
};

uint32 const GfrActorEntries[] =
{
    NPC_GFR_GIDWIN, NPC_GFR_ANASTARI, NPC_GFR_TARENAR, NPC_GFR_ARGUS, NPC_GFR_FIONA, NPC_GFR_RIMBLAT, NPC_GFR_PAMELA,
    NPC_GFR_VEXTUL_DEAD, NPC_GFR_VEXTUL_LIVE, NPC_GFR_GEORGE, NPC_GFR_DOPPELGANGER, NPC_GFR_BEEZIL
};

// Where Anastari stands (85650's sniffed spot, on the chamber floor) and Gidwin's table (85649)
Position const GfrAnastariSpot = { 3006.69f, -3429.41f, 149.709f };
Position const GfrTableSpot = { 3008.63f, -3434.42f, 151.157f };
// The middle of the slaughterhouse's chamber, between the camera and the Baroness
Position const GfrChamberMiddle = { 3005.0f, -3436.0f, 151.0f };
// The camera's spot (85663), where a seated player is saved
Position const GfrSeatSpot = { 3003.78f, -3443.88f, 152.644f };

// Where those who walk in stop, on the chamber floor before the Baroness (ours: no source places them). Tarenar and the double
// ("runs up to her") stop 4-5 yd short of her, the party behind and beside them; from the camera all of them lie within +-30
// degrees of the axis, Tarenar 9 degrees off it so that she stays in view while they talk
struct GfrStop
{
    uint32 entry;
    Position pos;
};

GfrStop const GfrStops[] =
{
    { NPC_GFR_TARENAR,      { 3004.3f, -3433.8f, 149.81f } },
    { NPC_GFR_DOPPELGANGER, { 3002.6f, -3433.4f, 149.81f } },
    { NPC_GFR_FIONA,        { 3001.6f, -3430.6f, 149.81f } },
    { NPC_GFR_RIMBLAT,      { 2999.8f, -3431.5f, 149.81f } },
    { NPC_GFR_ARGUS,        { 3005.9f, -3436.3f, 149.81f } },
    { NPC_GFR_VEXTUL_LIVE,  { 3008.6f, -3431.0f, 149.81f } },
    { NPC_GFR_BEEZIL,       { 3001.5f, -3435.0f, 149.81f } }
};

// Who fights the Baroness: the TDB factions (2244 against her 2245, hostile both ways). Beezil (474), George and the ghosts (35)
// fight nobody
static bool IsGfrFighter(uint32 entry)
{
    switch (entry)
    {
        case NPC_GFR_TARENAR:
        case NPC_GFR_ARGUS:
        case NPC_GFR_FIONA:
        case NPC_GFR_RIMBLAT:
        case NPC_GFR_VEXTUL_LIVE:
        case NPC_GFR_DOPPELGANGER:
            return true;
        default:
            return false;
    }
}

// The scene's actors and its camera are the player's personal summons, so they are found from the player's side
static Creature* FindGfrActorOf(Player* player, uint32 entry, float range = 100.0f)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, range);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            return creature;
    return nullptr;
}

// Removes every actor and camera of the player's scenes, alive or not (a run cut short, a dead Baroness)
static void DespawnGfrActorsOf(Player* player, bool includingCamera)
{
    std::list<Creature*> creatures;
    for (uint32 entry : GfrActorEntries)
        player->GetCreatureListWithEntryInGridAppend(creatures, entry, 150.0f);
    if (includingCamera)
        player->GetCreatureListWithEntryInGridAppend(creatures, NPC_GFR_CAMERA, 150.0f);

    for (Creature* creature : creatures)
        if (creature->IsSummon() && creature->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
            creature->DespawnOrUnsummon();
}

static void RunToGfrStop(Creature* actor)
{
    if (!actor)
        return;

    for (GfrStop const& stop : GfrStops)
    {
        if (stop.entry != actor->GetEntry())
            continue;

        actor->SetWalk(false);
        actor->GetMotionMaster()->MovePoint(POINT_GFR_STOP, stop.pos);
        return;
    }
}

// The scene is shown to the player's group (one scene per group: the others watch it, see at_epl_gfr_slaughterhouse). The summon
// spells make the actors private to the player (SummonProperties 2993, flag 0x10), and a private object is shown to a group only
// when its owner is the group itself (WorldObject::CheckPrivateObjectOwnerVisibility asks Player::IsInGroup with the owner GUID,
// which compares group GUIDs), so the owner becomes the group, as SUMMON_PROP_FLAG_PERSONAL_GROUP_SPAWN would make it. The camera
// stays the player's own
static void ShareGfrActorWithGroup(Player* player, Creature* actor)
{
    if (!actor)
        return;

    if (Group* group = player->GetGroup())
    {
        actor->SetPrivateObjectOwner(group->GetGUID());
        actor->UpdateObjectVisibility();
    }
}

// A scene of the player's group running near the player: its camera's owner
static Player* FindGroupGfrScene(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != player && member->IsInMap(player) && member->IsWithinDistInMap(player, 150.0f)
            && FindGfrActorOf(member, NPC_GFR_CAMERA, 150.0f))
            return member;
    }
    return nullptr;
}

// "Find Gidwin Goldbraids": a scene that froze "ended after about 15 seconds of waiting, and the quest still completed"
// (2010-11-29 [+45]). Given once (only while the quest is in progress), also to a dead player
static void CreditGfr(Player* player)
{
    if (player->GetQuestStatus(QUEST_GIDWINS_FATE_REVEALED) == QUEST_STATUS_INCOMPLETE)
        player->KilledMonsterCredit(NPC_GFR_CREDIT);
}

// 6300 - the slaughterhouse door (AreaTrigger.dbc: 2999.26, -3465.67, 158.4, a box of 7 x 5.3 x 10 yd)
// "As soon as you enter the doorway you are thrown into a cutscene" - only with Gidwin's Fate Revealed in the log and not done;
// mounted, flying or transformed all work (comments). After an abandon, retaking the quest and going through the door replays it.
// The double is made first: it copies the player's looks, a transform included, before the camera's seat dismounts the player.
// One scene per group near the chamber (retail froze with two at once; ours): a group member who comes in while another's scene
// runs watches it in person (the actors are shown to the group) and gets the credit when it ends, as does any group member holding
// the quest who is inside the chamber then, alive or dead. A playerbot never gets a scene of its own (mod_playerbots fires a non-teleport areatrigger for every bot in the box at
// once): it watches its group's scene, or with none running it is credited at once
class at_epl_gfr_slaughterhouse : public AreaTriggerScript
{
public:
    at_epl_gfr_slaughterhouse() : AreaTriggerScript("at_epl_gfr_slaughterhouse") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (!player->IsAlive() || player->GetQuestStatus(QUEST_GIDWINS_FATE_REVEALED) != QUEST_STATUS_INCOMPLETE)
            return false;

        // Already watching, or carried by something else (a passenger's seat, a taxi)
        if (player->GetVehicle() || player->IsInFlight() || FindGfrActorOf(player, NPC_GFR_CAMERA, 150.0f))
            return false;

        // A watcher of the group's scene: credited by that scene's camera at its end
        if (Player* owner = FindGroupGfrScene(player))
        {
            if (Creature* camera = FindGfrActorOf(owner, NPC_GFR_CAMERA, 150.0f))
                camera->AI()->SetGUID(player->GetGUID(), DATA_GFR_WATCHER);
            return true;
        }

        if (player->GetSession()->IsBot())
        {
            CreditGfr(player);
            return true;
        }

        // What is left of an earlier run of this player's
        DespawnGfrActorsOf(player, false);

        // The seated player cannot be reached, but mobs that followed would wait at the door: they let go (ours)
        player->CombatStop();

        player->CastSpell(player, SPELL_GFR_SUMMON_DOPPELGANGER, true);
        player->CastSpell(player, SPELL_GFR_MIRROR_IMAGE, true);
        player->CastSpell(player, SPELL_GFR_SUMMON_GIDWIN, true);
        player->CastSpell(player, SPELL_GFR_SUMMON_ANASTARI, true);
        for (uint32 entry : { NPC_GFR_DOPPELGANGER, NPC_GFR_GIDWIN, NPC_GFR_ANASTARI })
            ShareGfrActorWithGroup(player, FindGfrActorOf(player, entry));
        // The camera's summon (SummonProperties 3035, a vehicle) casts the hard-coded Ride Vehicle on it without a seat. Its only seat
        // (VehicleSeat 8645) has no CAN_ENTER_OR_EXIT, and a seat without it is taken only when named: the player is seated in it
        // by name at once (quest-chain-phasing / scripted-quest-scenes: name such a seat)
        player->CastSpell(player, SPELL_GFR_SUMMON_CAMERA, true);
        if (Creature* camera = FindGfrActorOf(player, NPC_GFR_CAMERA))
            if (player->GetVehicleBase() != camera)
                player->EnterVehicle(camera, GFR_CAMERA_SEAT);
        return true;
    }
};

// 45965 - GFR Event Camera
// The camera runs the scene once its summoner is seated. The lines and their order are sourced (the wiki's notes, comments; broadcast
// texts 46064-46069); the timings are ours, except that the quest completes about 15 s after the double reaches the Baroness (a
// scene that froze "ended after about 15 seconds of waiting, and the quest still completed"):
//  t+1 Gidwin, tied on the table, taunts her; t+5 she answers. t+8 Tarenar comes in and runs up to her with the player's double;
//  t+12 Tarenar, t+14 the Baroness. t+16 the rest of the party comes in, by who joined the caravan, and George and the ghosts
//  appear behind her; t+19 Argus. t+21 Tarenar yells "Charge!" and the fighters attack; she fights back. t+26 she dies, Gidwin
//  gets up and dances. t+28 the credit; every actor goes and the player is let out where the camera stands, inside the
//  slaughterhouse, where the party's static copies now stand (the caravan's staging: visible from 27526 complete).
// Every way out ends it once and completes the quest: the scene's end, the player's death, getting off the seat, never being seated,
// the camera unsummoned by anything else. Gidwin, the quest's ender, shows only once the quest is complete (the caravan's staging),
// so no way out may leave it in progress. A logout saves the player before the summons go: player_epl_gfr_login completes it at
// the next login. An actor that is missing only leaves its own step out. The group members who watched (came through the door
// during the scene, or are inside the chamber at its end, alive or dead) are credited at the same moment.
struct npc_epl_gfr_camera : public ScriptedAI
{
    npc_epl_gfr_camera(Creature* creature) : ScriptedAI(creature), _boardTries(0), _started(false), _done(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
        {
            _playerGUID = player->GetGUID();
            if (Group* group = player->GetGroup())
                _groupGUID = group->GetGUID();
        }

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
        // The eye is the camera: it stays where it was summoned, 0.4 yd above the floor
        me->SetCanFly(true);
        me->SetDisableGravity(true);

        _events.ScheduleEvent(EVENT_GFR_WAIT_FOR_RIDER, 500ms);
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    // Unsummoned by something other than Finish: the core unsummons a player's summons (Map::SummonCreature put the camera among
    // them) when the player dies, logs out or leaves the map. The actors go too (they also check on their own), the watchers are
    // credited, and so is the player unless logging out (saved already: player_epl_gfr_login)
    void Unsummoned() override
    {
        if (_done)
            return;
        _done = true;
        _events.Reset();
        DespawnTrackedActors();
        CreditWatchers();

        if (Player* player = ObjectAccessor::FindPlayer(_playerGUID))
            CreditLater(player);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_done)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            Finish(nullptr);
            return;
        }

        if (_started && (!player->IsAlive() || player->GetVehicleBase() != me))
        {
            Finish(player);
            return;
        }

        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GFR_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                        Start(player);
                    else if (++_boardTries < 6 && player->IsAlive())
                    {
                        // The trigger names the seat; should that not have held, it is named again
                        if (!player->GetVehicle())
                            player->EnterVehicle(me, GFR_CAMERA_SEAT);
                        _events.ScheduleEvent(EVENT_GFR_WAIT_FOR_RIDER, 500ms);
                    }
                    else
                    {
                        // Last resort, not expected: the quest still completes (its ender shows only then)
                        TC_LOG_ERROR("scripts", "npc_epl_gfr_camera: %s could not be seated in the GFR Event Camera (on a vehicle: %u, alive: %u); "
                            "Gidwin's Fate Revealed is completed without its scene", player->GetName().c_str(), player->GetVehicle() ? 1 : 0,
                            player->IsAlive() ? 1 : 0);
                        Finish(player);
                    }
                    break;
                case EVENT_GFR_GIDWIN_TAUNTS:
                    if (Creature* gidwin = Actor(NPC_GFR_GIDWIN))
                        gidwin->AI()->Talk(TALK_GFR_GIDWIN_WITCH, player);
                    break;
                case EVENT_GFR_ANASTARI_SCOFFS:
                    if (Creature* anastari = Actor(NPC_GFR_ANASTARI))
                        anastari->AI()->Talk(TALK_GFR_ANASTARI_NOBODY, player);
                    break;
                case EVENT_GFR_TARENAR_ENTERS:
                    Summon(player, SPELL_GFR_SUMMON_TARENAR, NPC_GFR_TARENAR);
                    RunToGfrStop(Actor(NPC_GFR_TARENAR));
                    RunToGfrStop(Actor(NPC_GFR_DOPPELGANGER));
                    break;
                case EVENT_GFR_TARENAR_ANSWERS:
                    if (Creature* tarenar = Actor(NPC_GFR_TARENAR))
                        tarenar->AI()->Talk(TALK_GFR_TARENAR_WRONG, player);
                    break;
                case EVENT_GFR_ANASTARI_LAUGHS:
                    if (Creature* anastari = Actor(NPC_GFR_ANASTARI))
                        anastari->AI()->Talk(TALK_GFR_ANASTARI_TWO_OF_YOU, player);
                    break;
                case EVENT_GFR_PARTY_ENTERS:
                    PartyEnters(player);
                    break;
                case EVENT_GFR_ARGUS_DIFFERS:
                    if (Creature* argus = Actor(NPC_GFR_ARGUS))
                        argus->AI()->Talk(TALK_GFR_ARGUS_DIFFER, player);
                    break;
                case EVENT_GFR_CHARGE:
                    Charge(player);
                    break;
                case EVENT_GFR_ANASTARI_FALLS:
                    AnastariFalls();
                    break;
                case EVENT_GFR_END:
                    Finish(player);
                    return;
                default:
                    break;
            }

            if (_done)
                return;
        }
    }

private:
    void Start(Player* player)
    {
        _started = true;

        for (uint32 entry : { NPC_GFR_DOPPELGANGER, NPC_GFR_GIDWIN, NPC_GFR_ANASTARI })
            Track(player, entry);

        _events.ScheduleEvent(EVENT_GFR_GIDWIN_TAUNTS, 1s);
        _events.ScheduleEvent(EVENT_GFR_ANASTARI_SCOFFS, 5s);
        _events.ScheduleEvent(EVENT_GFR_TARENAR_ENTERS, 8s);
        _events.ScheduleEvent(EVENT_GFR_TARENAR_ANSWERS, 12s);
        _events.ScheduleEvent(EVENT_GFR_ANASTARI_LAUGHS, 14s);
        _events.ScheduleEvent(EVENT_GFR_PARTY_ENTERS, 16s);
        _events.ScheduleEvent(EVENT_GFR_ARGUS_DIFFERS, 19s);
        _events.ScheduleEvent(EVENT_GFR_CHARGE, 21s);
        _events.ScheduleEvent(EVENT_GFR_ANASTARI_FALLS, 26s);
        _events.ScheduleEvent(EVENT_GFR_END, 28s);
    }

    // "Fiona, Argus and anyone else you've recruited", Lord Raymond George, Vex'tul "as a ghost" and Pamela's ghost "by the
    // baroness" (comments). Fiona, Argus (his recruitment, The Trek Continues, is on the main chain) and George always come
    void PartyEnters(Player* player)
    {
        Summon(player, SPELL_GFR_SUMMON_FIONA, NPC_GFR_FIONA);
        Summon(player, SPELL_GFR_SUMMON_ARGUS, NPC_GFR_ARGUS);
        if (player->GetQuestRewardStatus(QUEST_AN_OPPORTUNE_ALLIANCE))
            Summon(player, SPELL_GFR_SUMMON_RIMBLAT, NPC_GFR_RIMBLAT);
        if (player->GetQuestRewardStatus(QUEST_BEEZILS_FLAG))
            Summon(player, SPELL_GFR_SUMMON_BEEZIL, NPC_GFR_BEEZIL);
        if (player->GetQuestRewardStatus(QUEST_INTO_THE_FLAMES))
            Summon(player, SPELL_GFR_SUMMON_VEXTUL_DEAD, NPC_GFR_VEXTUL_DEAD);
        else if (player->GetQuestRewardStatus(QUEST_HONOR_AND_STRENGTH))
            Summon(player, SPELL_GFR_SUMMON_VEXTUL_LIVE, NPC_GFR_VEXTUL_LIVE);
        if (player->GetQuestRewardStatus(QUEST_BATTLE_OF_DARROWSHIRE))
            Summon(player, SPELL_GFR_SUMMON_PAMELA, NPC_GFR_PAMELA);
        Summon(player, SPELL_GFR_SUMMON_GEORGE, NPC_GFR_GEORGE);

        for (uint32 entry : { NPC_GFR_FIONA, NPC_GFR_ARGUS, NPC_GFR_RIMBLAT, NPC_GFR_BEEZIL, NPC_GFR_VEXTUL_LIVE })
            RunToGfrStop(Actor(entry));
    }

    // Tarenar yells; the fighters attack the Baroness and she fights back. Nobody can die of it (the actors' damage floor): she
    // falls at t+26 by the script
    void Charge(Player* player)
    {
        Creature* anastari = Actor(NPC_GFR_ANASTARI);
        if (Creature* tarenar = Actor(NPC_GFR_TARENAR))
            tarenar->AI()->Talk(TALK_GFR_TARENAR_CHARGE, player);

        if (!anastari)
            return;

        anastari->AI()->DoAction(ACTION_GFR_FIGHT);
        for (auto const& actor : _actors)
        {
            if (!IsGfrFighter(actor.first))
                continue;
            if (Creature* fighter = ObjectAccessor::GetCreature(*me, actor.second))
            {
                fighter->AI()->DoAction(ACTION_GFR_FIGHT);
                fighter->AI()->AttackStart(anastari);
            }
        }

        if (Creature* tarenar = Actor(NPC_GFR_TARENAR))
            anastari->AI()->AttackStart(tarenar);
    }

    // "Everyone attacks Anastari, and eventually she dies. Gidwin gets up from the table and starts to dance."
    void AnastariFalls()
    {
        if (Creature* anastari = Actor(NPC_GFR_ANASTARI))
        {
            Creature* tarenar = Actor(NPC_GFR_TARENAR);
            Unit* killer = tarenar ? tarenar : me;
            // Nobody's kill: no loot, and no experience (flags_extra NO_XP_AT_KILL on every actor)
            anastari->SetLootRecipient(nullptr);
            killer->Kill(anastari);
        }

        for (auto const& actor : _actors)
            if (IsGfrFighter(actor.first))
                if (Creature* fighter = ObjectAccessor::GetCreature(*me, actor.second))
                    if (fighter->IsAlive())
                        fighter->AI()->DoAction(ACTION_GFR_STAND_DOWN);

        // Gidwin's pose on the table is ours (no source shows it): lying tied, then up and dancing
        if (Creature* gidwin = Actor(NPC_GFR_GIDWIN))
        {
            gidwin->SetStandState(UNIT_STAND_STATE_STAND);
            gidwin->SetEmoteState(EMOTE_STATE_DANCE);
        }
    }

    void Summon(Player* player, uint32 spellId, uint32 entry)
    {
        player->CastSpell(player, spellId, true);
        Track(player, entry);
        ShareGfrActorWithGroup(player, Actor(entry));
    }

    void Track(Player* player, uint32 entry)
    {
        if (Creature* actor = FindGfrActorOf(player, entry))
            _actors[entry] = actor->GetGUID();
    }

    Creature* Actor(uint32 entry)
    {
        auto itr = _actors.find(entry);
        if (itr == _actors.end())
            return nullptr;

        Creature* actor = ObjectAccessor::GetCreature(*me, itr->second);
        return actor && actor->IsAlive() ? actor : nullptr;
    }

    void DespawnTrackedActors()
    {
        for (auto const& actor : _actors)
            if (Creature* creature = ObjectAccessor::GetCreature(*me, actor.second))
                creature->DespawnOrUnsummon();
        _actors.clear();
    }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id == DATA_GFR_WATCHER)
            _watchers.insert(guid);
    }

    // The group members holding the quest who watched: those who came through the door during the scene, and those inside the
    // chamber when it ends (within 20 yd of its middle, the camera's spot included, the door not), alive or dead
    void CreditWatchers()
    {
        if (_groupGUID.IsEmpty())
            return;

        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* member = itr->GetSource();
            if (!member || member->GetGUID() == _playerGUID || !member->IsInWorld() || !member->IsInGroup(_groupGUID))
                continue;

            bool const insideChamber = member->GetExactDist2d(GfrChamberMiddle.GetPositionX(), GfrChamberMiddle.GetPositionY()) <= 20.0f
                && std::fabs(member->GetPositionZ() - GfrChamberMiddle.GetPositionZ()) <= 8.0f;
            if (insideChamber || (_watchers.count(member->GetGUID()) && member->IsWithinDistInMap(me, 150.0f)))
                CreditGfr(member);
        }
    }

    // A player who may be in the middle of leaving the map (Unit::RemoveFromWorld unsummons the camera): credited from the player's
    // own event queue a moment later, once in a map again. A logout destroys the queue with the player, so nothing runs then
    static void CreditLater(Player* player)
    {
        if (player->GetSession()->PlayerLogout())
            return;

        player->m_Events.AddLambdaEventAtOffset([player]()
        {
            if (player->IsInWorld() && !player->GetSession()->PlayerLogout())
                CreditGfr(player);
        }, 500);
    }

    // The actors go before the credit: the credit shows the party's static copies in the slaughterhouse (spell_area 49417 and
    // 60922 in 2277 from 27526 complete), so nobody stands there twice. Then the player gets off where the camera stands, as in The Depravity of the
    // Forsaken; no teleport follows, so the exit is not cut off. The camera goes 500 ms later
    void Finish(Player* player)
    {
        if (_done)
            return;
        _done = true;
        _events.Reset();

        DespawnTrackedActors();

        CreditWatchers();

        // Gone from this map but still playing (a teleport): the credit follows
        if (!player)
            if (Player* away = ObjectAccessor::FindPlayer(_playerGUID))
                CreditLater(away);

        if (player)
        {
            DespawnGfrActorsOf(player, false);

            CreditGfr(player);

            if (player->GetVehicleBase() == me)
                player->ExitVehicle();
        }

        me->DespawnOrUnsummon(500);
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _groupGUID;
    GuidSet _watchers;
    std::map<uint32, ObjectGuid> _actors;
    uint8 _boardTries;
    bool _started;
    bool _done;
};

// 45955 Gidwin, 45956 Baroness Anastari, 45957 Tarenar, 45958 Argus, 45959 Fiona, 45960 Rimblat, 45961 Pamela, 45962 / 45963
// Vex'tul, 45964 Lord Raymond George, 45978 Your Doppelganger, 46043 Beezil (Gidwin's Fate Revealed)
// Actors of one player's scene: the camera moves and voices them. Until the charge they are passive and immune; then the fighters
// and the Baroness fight each other and nobody else (she is hostile to players: she never takes one as a target). No damage
// kills an actor. Without their player or the player's camera they leave.
struct npc_epl_gfr_actor : public ScriptedAI
{
    npc_epl_gfr_actor(Creature* creature) : ScriptedAI(creature), _fighting(false), _checkTimer(1 * IN_MILLISECONDS) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _playerGUID = summoner->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        // "Gidwin tied to a table" (the wiki's notes): the pose is ours, lying on it
        if (me->GetEntry() == NPC_GFR_GIDWIN)
            me->SetStandState(UNIT_STAND_STATE_SLEEP);
    }

    void AttackStart(Unit* who) override
    {
        if (!_fighting || !who || !IsOpponent(who))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }

    // No way home: the fight is over or has no target left, so the actor stands where it is
    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        StandDown();
    }

    // Players and what they control do no damage to an actor: a group member sees the scene and could hit the Baroness after the
    // charge (she has to lose IMMUNE_TO_PC for the fighters, see ACTION_GFR_FIGHT)
    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (attacker && attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            damage = 0;
            return;
        }

        if (damage >= me->GetHealth())
            damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_GFR_STOP && !_fighting)
            me->SetFacingTo(me->GetAngle(GfrAnastariSpot.GetPositionX(), GfrAnastariSpot.GetPositionY()));
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_GFR_FIGHT:
                // Both flags go for the fight. IMMUNE_TO_NPC is checked between creatures (Unit::IsValidAttackTarget, threat), and a
                // creature also refuses an IMMUNE_TO_PC target: Creature::_IsTargetAcceptable (Creature.cpp:2386-2399) asks
                // Unit::isTargetableForAttack, which rejects IMMUNE_TO_PC for every attacker (Unit.cpp:12041-12043; the one
                // exception is a player-controlled creature target), so the fighters' victim choice would drop a flagged Baroness
                _fighting = true;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                me->SetReactState(REACT_AGGRESSIVE);
                break;
            case ACTION_GFR_STAND_DOWN:
                StandDown();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer <= diff)
        {
            _checkTimer = 1 * IN_MILLISECONDS;
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player || !player->IsInWorld() || !FindGfrActorOf(player, NPC_GFR_CAMERA))
            {
                me->DespawnOrUnsummon();
                return;
            }
        }
        else
            _checkTimer -= diff;

        if (!_fighting || !UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    // The Baroness fights the fighters of the same scene, the fighters fight her
    bool IsOpponent(Unit* who) const
    {
        Creature* creature = who->ToCreature();
        if (!creature || !creature->IsSummon() || !me->IsSummon()
            || creature->ToTempSummon()->GetSummonerGUID() != me->ToTempSummon()->GetSummonerGUID())
            return false;

        if (me->GetEntry() == NPC_GFR_ANASTARI)
            return IsGfrFighter(creature->GetEntry());
        return creature->GetEntry() == NPC_GFR_ANASTARI;
    }

    void StandDown()
    {
        if (!_fighting)
            return;

        _fighting = false;
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        if (me->IsAlive())
            me->SetFacingTo(me->GetAngle(GfrTableSpot.GetPositionX(), GfrTableSpot.GetPositionY()));
    }

    ObjectGuid _playerGUID;
    bool _fighting;
    uint32 _checkTimer;
};

// A player who logged out during the scene was saved before the scene's summons went, with the quest still in progress, seated on
// the camera (the seat's attachment offset is 0, so the saved position is the camera's spot). Logging in alive on that spot with the
// quest in progress completes it (ours: 2.5 yd, z within 3 yd). Nobody stands there by chance: it hangs 0.4 yd above the entry floor
// of the slaughterhouse; a ghost is left out (a player who died in the scene was credited by the scene's end)
class player_epl_gfr_login : public PlayerScript
{
public:
    player_epl_gfr_login() : PlayerScript("player_epl_gfr_login") { }

    void OnLogin(Player* player) override
    {
        if (player->GetMapId() != 0 || !player->IsAlive() || player->GetQuestStatus(QUEST_GIDWINS_FATE_REVEALED) != QUEST_STATUS_INCOMPLETE)
            return;

        if (player->GetExactDist2d(GfrSeatSpot.GetPositionX(), GfrSeatSpot.GetPositionY()) > 2.5f
            || std::fabs(player->GetPositionZ() - GfrSeatSpot.GetPositionZ()) > 3.0f)
            return;

        CreditGfr(player);
    }
};

// 85682 - GFR: Mirror Image Aura
// The player's own doppelganger takes the player's looks (effect 0, CLONE_CASTER) and weapons: effects 1 and 2 are script effects
// whose values are Copy Weapon 41055 and Copy Off-hand Weapon 45206, which the player casts on the double (spell_gen_clone_weapon
// puts the copy auras on it). The condition names only the entry; a group member's doppelganger stands on the same spot and is
// visible to the group, so the target is the caster's own. (85683, the double's "Script Effect Player Cast Mirror Image", is not
// used: the player casts this spell itself, as in The Reckoning.)
class spell_epl_gfr_mirror_image : public SpellScript
{
    PrepareSpellScript(spell_epl_gfr_mirror_image);

    void SelectOwnDouble(WorldObject*& target)
    {
        if (Player* player = GetCaster()->ToPlayer())
            if (Creature* own = FindGfrActorOf(player, NPC_GFR_DOPPELGANGER, 40.0f))
                target = own;
    }

    void HandleCopyWeapon(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_epl_gfr_mirror_image::SelectOwnDouble, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_epl_gfr_mirror_image::SelectOwnDouble, EFFECT_1, TARGET_UNIT_NEARBY_ENTRY);
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_epl_gfr_mirror_image::SelectOwnDouble, EFFECT_2, TARGET_UNIT_NEARBY_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_epl_gfr_mirror_image::HandleCopyWeapon, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        OnEffectHitTarget += SpellEffectFn(spell_epl_gfr_mirror_image::HandleCopyWeapon, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_eastern_plaguelands_finale()
{
    new spell_script<spell_epl_lieutenant_death_grip>("spell_epl_lieutenant_death_grip");
    new spell_script<spell_epl_burn_corpse>("spell_epl_burn_corpse");
    new creature_script<npc_epl_corpsebeast>("npc_epl_corpsebeast");
    new spell_script<spell_epl_summon_argent_lightwell>("spell_epl_summon_argent_lightwell");
    new at_epl_gfr_slaughterhouse();
    new creature_script<npc_epl_gfr_camera>("npc_epl_gfr_camera");
    new creature_script<npc_epl_gfr_actor>("npc_epl_gfr_actor");
    new spell_script<spell_epl_gfr_mirror_image>("spell_epl_gfr_mirror_image");
    new player_epl_gfr_login();
}
