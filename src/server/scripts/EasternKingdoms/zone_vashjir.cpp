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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "WorldSession.h"
#include "TemporarySummon.h"
#include "Transport.h"
#include "ChatTextBuilder.h"
#include "WorldPacket.h"
#include <cmath>
#include <vector>

// http://www.wowhead.com/quest=25281/pay-it-forward http://www.wowhead.com/quest=25936/pay-it-forward
enum QuestPayItForward
{
    SPELL_BLOW_BUBBLE_A  = 74151,
    SPELL_BUBBLE_SELF    = 74416,
    SPELL_BLOW_BUBBLE_H  = 77825,

    NPC_DROWNING_SOLDIER = 39663,
    NPC_DROWNING_WARRIOR = 41672
};

struct npc_drowning_soldier_and_warrior : public ScriptedAI
{
    npc_drowning_soldier_and_warrior(Creature* creature) : ScriptedAI(creature)
    {
        JustAppeared();
    }

    void JustAppeared() override
    {
        me->setRegeneratingHealth(false);
        me->SetHealth(6190);
        move_timer = 0;
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (Player* player = caster->ToPlayer())
        {
            if (me->HasAura(SPELL_BUBBLE_SELF))
                return;
            if (spell->Id == SPELL_BLOW_BUBBLE_A)
                player->KilledMonsterCredit(NPC_DROWNING_SOLDIER);
            else if (spell->Id == SPELL_BLOW_BUBBLE_H)
                player->KilledMonsterCredit(NPC_DROWNING_WARRIOR);
            me->CastSpell(me, SPELL_BUBBLE_SELF, true);
            move_timer = 3000;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!move_timer)
            return;

        move_timer -= diff;
        if (move_timer <= 0)
        {
            move_timer = 0;

            // Бежать вперёд на 15 ярдов
            const float angle = (me->GetOrientation()) - static_cast<float>(M_PI / 2);
            const float x = me->GetPositionX() - 15 * std::sin(angle);
            const float y = me->GetPositionY() + 15 * std::cos(angle);
            const float z = me->GetMap()->GetHeight(x, y, me->GetPositionZ());
            me->GetMotionMaster()->MovePoint(0, x, y, z);

            me->DespawnOrUnsummon(5000);
        }
    }
private:
    int32 move_timer;
};

// Call of Duty (14482 Alliance, 25924 Horde): the mercenary Ship to Vashj'ir (197195 / 203466) reaches its sea stop over
// Seafarer's Tomb, the crew sights a wreck, a kraken attacks and throws the riders overboard, Erunak Stonespeaker saves
// them and they wake beside him in the Briny Cutter / Immortal Coil, which completes the quest (Warcraft Wiki "Call of
// Duty (Alliance)" / "Call of Duty (Horde)", the scene before 7.0.3; the texts are the scene's broadcast texts). Kept to
// what this core has: the crew lines, the throw into the water with the drowned screen effect, the rescue Erunak (the
// creatures of the 74701 / 76127 summon spells, as a private summon) and the wake-up beside the quest ender. The
// tentacle vehicle rides of the retail scene are left out. Only riders with the quest in progress take part; the ship
// then sails on as before.
enum CallOfDutyShip
{
    MAP_VASHJIR                     = 0,        // Vashj'ir (Kelp'thar Forest) is on the Eastern Kingdoms map
    GO_SHIP_TO_VASHJIR_ALLIANCE     = 197195,
    GO_SHIP_TO_VASHJIR_HORDE        = 203466,
    SHIP_SEA_STOP_NODE_ALLIANCE     = 11,       // TaxiPathNode path 1368 node 11 (-4632.9, 3863.7), 45 s
    SHIP_SEA_STOP_NODE_HORDE        = 4,        // TaxiPathNode path 2204 node 4, the same point, 45 s

    QUEST_CALL_OF_DUTY_ALLIANCE     = 14482,
    QUEST_CALL_OF_DUTY_HORDE        = 25924,
    NPC_SUBMERGE_BUNNY              = 36901,    // the objective of 25924 (kill credit)

    NPC_ADARRAH                     = 43051,
    NPC_MACK_FEARSEN                = 43022,
    NPC_ERUNAK_ALLIANCE             = 36915,    // quest ender in the Briny Cutter
    NPC_ERUNAK_HORDE                = 41618,    // quest ender in the Immortal Coil
    NPC_ERUNAK_RESCUE_ALLIANCE      = 40053,    // Summon Erunak Rescue (74701)
    NPC_ERUNAK_RESCUE_HORDE         = 40801,    // Summon Erunak Rescue 02 (76127)

    SPELL_DROWNED_SCREEN_EFFECT     = 73734,

    TEXT_ADARRAH_SHIPWRECK          = 39672,    // "Captain! Shipwreck off the starboard bow! There appear to be survivors!"
    TEXT_MACK_WHAT_THE              = 39687,    // "What the... ?"
    TEXT_ERUNAK_TO_THE_DEPTHS       = 40551,    // "To the depths with you, beast!"
    TEXT_ERUNAK_HOLD_ON             = 40556,    // "Hold on, $r!"
    TEXT_ERUNAK_RELAX               = 39348,    // "Relax, friend. You have not passed away... yet. ..."
};

class transport_ship_to_vashjir : public TransportScript
{
public:
    transport_ship_to_vashjir() : TransportScript("transport_ship_to_vashjir") { }

    void OnRelocate(Transport* transport, uint32 waypointId, uint32 /*mapId*/, float /*x*/, float /*y*/, float /*z*/) override
    {
        bool const alliance = transport->GetEntry() == GO_SHIP_TO_VASHJIR_ALLIANCE;
        if (!alliance && transport->GetEntry() != GO_SHIP_TO_VASHJIR_HORDE)
            return;
        if (waypointId != uint32(alliance ? SHIP_SEA_STOP_NODE_ALLIANCE : SHIP_SEA_STOP_NODE_HORDE))
            return;

        uint32 const questId = alliance ? QUEST_CALL_OF_DUTY_ALLIANCE : QUEST_CALL_OF_DUTY_HORDE;
        std::vector<Player*> riders;
        for (WorldObject* passenger : transport->GetPassengers())
            if (Player* player = passenger->ToPlayer())
                if (player->IsAlive() && player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                    riders.push_back(player);
        if (riders.empty())
            return;

        // the crew sights the wreck (passengers of the phased ship: only its phase sees and hears them)
        for (WorldObject* crew : transport->GetStaticPassengers())
            if (Creature* creature = crew->ToCreature())
            {
                if (creature->GetEntry() == NPC_ADARRAH)
                    creature->Yell(TEXT_ADARRAH_SHIPWRECK);
                else if (creature->GetEntry() == NPC_MACK_FEARSEN)
                    creature->m_Events.Schedule(3000, [creature]() { creature->Say(TEXT_MACK_WHAT_THE); });
            }

        ObjectGuid const shipGuid = transport->GetGUID();
        for (Player* player : riders)
            StartScene(player, shipGuid, alliance);
    }

private:
    // each step runs in the rider's own update and stops (dropping the screen effect) when the rider left, died or no
    // longer has the quest running
    static bool StillInScene(Player* player, bool alliance)
    {
        if (player->IsInWorld() && player->IsAlive() &&
            player->GetQuestStatus(alliance ? QUEST_CALL_OF_DUTY_ALLIANCE : QUEST_CALL_OF_DUTY_HORDE) == QUEST_STATUS_INCOMPLETE)
            return true;

        player->RemoveAurasDueToSpell(SPELL_DROWNED_SCREEN_EFFECT);
        return false;
    }

    // Erunak's lines are part of the rider's own scene, so they go to the rider directly (a monster say, $r filled in
    // with the rider's race; the client leaves $r as it is). Broadcast as a say they reached the rider only while the
    // rider was registered in a grid cell within say range of Erunak, and both lines follow a near teleport, which
    // moves the rider to its new cell only when the client's teleport ack is handled; the rescue Erunak also stood on
    // the sea floor, far below the rider (GetNearPosition). Many rides lost them; the 300 yd yells always arrived.
    static void SayToRider(Creature* speaker, uint32 textId, Player* rider)
    {
        BroadcastText const* bct = sObjectMgr->GetBroadcastText(textId);
        if (!bct || !rider->GetSession())
            return;

        LocaleConstant const locale = rider->GetSession()->GetSessionDbLocaleIndex();
        std::string text = bct->GetText(locale, speaker->GetGender());
        if (ChrRacesEntry const* race = sChrRacesStore.LookupEntry(rider->GetRace()))
            if (race->name && *race->name)
            {
                std::string const raceName = race->name;
                for (std::string::size_type at = text.find("$r"); at != std::string::npos; at = text.find("$r", at + raceName.size()))
                    text.replace(at, 2, raceName);
            }

        WorldPacket data;
        Trinity::CustomChatTextBuilder(speaker, CHAT_MSG_MONSTER_SAY, text, Language(bct->LanguageID), rider)(data, locale);
        rider->SendDirectMessage(&data);
    }

    // "Relax, friend...": once the wake-up teleport has landed (the rider is beside Erunak), at most 3 s later. The line
    // goes to the rider directly, so the speaker's position does not matter: Erunak beside the rider when the grid search
    // finds him, otherwise the rescue Erunak (the same character; the search missed him in most v40 rides right after
    // the teleport and the quest credit), so the line is never dropped.
    static void WakeLine(Player* player, bool alliance, ObjectGuid rescueGuid, uint8 tries)
    {
        if (!player->IsInWorld())
            return;

        if (player->IsBeingTeleported() && tries)
        {
            player->m_Events.Schedule(500, [player, alliance, rescueGuid, tries]() { WakeLine(player, alliance, rescueGuid, tries - 1); });
            return;
        }

        Creature* erunak = player->FindNearestCreature(alliance ? NPC_ERUNAK_ALLIANCE : NPC_ERUNAK_HORDE, 30.0f);
        if (erunak)
            erunak->SetFacingToObject(player);
        else if (!rescueGuid.IsEmpty())
            erunak = ObjectAccessor::GetCreature(*player, rescueGuid);
        if (erunak)
            SayToRider(erunak, TEXT_ERUNAK_RELAX, player);
    }

    static void StartScene(Player* player, ObjectGuid shipGuid, bool alliance)
    {
        // the kraken throws the rider overboard, into the water beside the hull, where it starts to drown
        player->m_Events.Schedule(6000, [player, shipGuid, alliance]()
        {
            if (!StillInScene(player, alliance) || !player->GetTransport() || player->GetTransport()->GetGUID() != shipGuid)
                return;

            // Vashj'ir is on Eastern Kingdoms: the Horde ship's sea stop is a teleport frame, so it has already moved on to
            // Kalimdor when this runs and the rider's current map is the wrong one
            player->TeleportTo(MAP_VASHJIR, -4620.0f, 3850.0f, -15.0f, 5.95f);   // leaves the ship
            player->CastSpell(player, SPELL_DROWNED_SCREEN_EFFECT, true);

            // Erunak comes to the rescue (a private summon: seen by this rider only)
            player->m_Events.Schedule(3000, [player, alliance]()
            {
                if (!StillInScene(player, alliance))
                    return;

                // 4 yd in front of the rider at the rider's depth (GetNearPosition would put him on the sea floor), and
                // passive: he is there to save the rider, not to chase the naga
                float const o = player->GetOrientation();
                Position const pos(player->GetPositionX() + 4.0f * std::cos(o), player->GetPositionY() + 4.0f * std::sin(o),
                    player->GetPositionZ(), o + float(M_PI));   // the ctor normalizes the facing
                ObjectGuid rescueGuid;
                // he stays until after "Relax, friend..." (at most +18.5 s), which he speaks if Erunak is not found
                if (TempSummon* erunak = player->SummonCreature(alliance ? NPC_ERUNAK_RESCUE_ALLIANCE : NPC_ERUNAK_RESCUE_HORDE, pos,
                    TEMPSUMMON_TIMED_DESPAWN, Milliseconds(12000), 0, player->GetGUID()))
                {
                    rescueGuid = erunak->GetGUID();
                    erunak->SetReactState(REACT_PASSIVE);
                    erunak->SetFacingToObject(player);
                    erunak->Yell(TEXT_ERUNAK_TO_THE_DEPTHS);
                    ObjectGuid const playerGuid = player->GetGUID();
                    erunak->m_Events.Schedule(2500, [erunak, playerGuid]()
                    {
                        if (Player* target = ObjectAccessor::GetPlayer(*erunak, playerGuid))
                            SayToRider(erunak, TEXT_ERUNAK_HOLD_ON, target);   // "Hold on, <race>!"
                    });
                }

                // the rider wakes beside Erunak in the Briny Cutter / the Immortal Coil
                player->m_Events.Schedule(5000, [player, alliance, rescueGuid]()
                {
                    if (!StillInScene(player, alliance))
                        return;

                    player->RemoveAurasDueToSpell(SPELL_DROWNED_SCREEN_EFFECT);
                    if (alliance)   // the Vashj'ir Rescue Bunny's spot, beside Erunak (36915)
                        player->TeleportTo(MAP_VASHJIR, -4458.93f, 3801.23f, -81.92f, 1.0f);
                    else            // beside Erunak (41618)
                        player->TeleportTo(MAP_VASHJIR, -4609.82f, 3981.23f, -70.54f, 2.0f);

                    player->m_Events.Schedule(1500, [player, alliance, rescueGuid]()
                    {
                        if (!StillInScene(player, alliance))
                            return;

                        // the quest objective: the event (14482) / the Submerge Bunny credit (25924), then the event
                        if (!alliance)
                            player->KilledMonsterCredit(NPC_SUBMERGE_BUNNY);
                        player->AreaExploredOrEventHappens(alliance ? QUEST_CALL_OF_DUTY_ALLIANCE : QUEST_CALL_OF_DUTY_HORDE);

                        WakeLine(player, alliance, rescueGuid, 6);
                    });
                });
            });
        });
    }
};

void AddSC_vashjir()
{
    new creature_script<npc_drowning_soldier_and_warrior>("npc_drowning_soldier_and_warrior");
    new transport_ship_to_vashjir();
}
