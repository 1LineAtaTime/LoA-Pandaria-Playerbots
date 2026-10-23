/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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

// Zeppelin arrival announcements (issue #498). The zeppelin master NPC yells when its zeppelin docks.
// Ported (minimal form) from alexkulya MoP 5.4.8 commit b8e79c4, src/server/scripts/World/zeppelins.cpp.
// The master/engineer "where is the zeppelin" gossip is Classic-only and is intentionally not ported.
//
// Stop node indices are TaxiPathNode.dbc NodeIndex values of the 5.4.8 DBC (nodes with Flags & 2),
// verified for this tree:
//   path 302 (The Thundercaller)    4  (Durotar),   19 (Tirisfal)
//   path 285 (The Iron Eagle)       3  (Stranglethorn), 12 (Durotar)
//   path 301 (The Purple Princess)  3  (Stranglethorn), 19 (Tirisfal)
//   path 712 (The Mighty Wind)      4  (Borean Tundra), 20 (Durotar)
//   path 737 (The Cloudkisser)      3  (Howling Fjord), 10 (Tirisfal)
// The Zephyr (190549) is driven by SmartGameObjectAI and has no announcement here.

#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "Transport.h"
#include <map>

namespace
{
    struct Announcement
    {
        uint32 npcEntry;
        uint8 textGroup;
    };

    typedef std::map<uint32, Announcement> AnnouncementMap; // taxi path NodeIndex -> announcement

    class go_zeppelin_script : public TransportScript
    {
    public:
        go_zeppelin_script(char const* name, AnnouncementMap const& announcements) : TransportScript(name), _announcements(announcements) { }

        void OnRelocate(Transport* transport, uint32 waypointId, uint32 /*mapId*/, float /*x*/, float /*y*/, float /*z*/) override
        {
            AnnouncementMap::const_iterator itr = _announcements.find(waypointId);
            if (itr == _announcements.end())
                return;

            if (Creature* announcer = transport->FindNearestCreature(itr->second.npcEntry, 500.0f))
                if (announcer->AI())
                    announcer->AI()->Talk(itr->second.textGroup);
        }

    private:
        AnnouncementMap _announcements;
    };
}

void AddSC_zeppelins()
{
    {
        AnnouncementMap m;
        m[4]  = { 9564, 0 };   // Frezza (Orgrimmar tower)
        m[19] = { 9566, 0 };   // Zapetta (Undercity)
        m[20] = { 9566, 1 };
        new go_zeppelin_script("zeppelin_the_thundercaller", m);
    }
    {
        AnnouncementMap m;
        m[3]  = { 3149, 0 };   // Nez'raz (Grom'gol)
        m[12] = { 12136, 0 };  // Snurk Bucksquick (Orgrimmar)
        new go_zeppelin_script("zeppelin_the_iron_eagle", m);
    }
    {
        AnnouncementMap m;
        m[3]  = { 12137, 0 };  // Squibby Overspeck (Grom'gol)
        m[19] = { 3150, 0 };   // Hin Denburg (Undercity)
        m[20] = { 3150, 1 };
        new go_zeppelin_script("zeppelin_the_purple_princess", m);
    }
    {
        AnnouncementMap m;
        m[4]  = { 26538, 0 };  // Nargo Screwbore (Warsong Hold)
        m[20] = { 26537, 0 };  // Greeb Ramrocket (Orgrimmar)
        new go_zeppelin_script("zeppelin_the_mighty_wind", m);
    }
    {
        AnnouncementMap m;
        m[3]  = { 26540, 0 };  // Drenk Spannerspark (Vengeance Landing)
        m[10] = { 26539, 0 };  // Meefi Farthrottle (Undercity)
        new go_zeppelin_script("zeppelin_the_cloudkisser", m);
    }
}
