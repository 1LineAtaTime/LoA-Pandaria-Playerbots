# Companion loot and status follow-up

`AiPlayerbot.Companion.GoldOnlyLoot = 1` (default) applies to bot sessions
grouped with an online human player. Money looting still uses the server's normal
gold/silver/copper split. Bots skip all item and currency-token slots, pass item
rolls at creation, and are excluded from round-robin item ownership. The core
StoreLootItem path also rejects item collection, protecting alternative AI paths.
Autonomous bot-only groups retain their normal progression behavior.

`AiPlayerbot.Companion.SilentGreetings = 1` (default) suppresses hello/goodbye
from login, logout, group invitation acceptance and group departure. General quest
and other notification routing remains a separate pending change.

Companion initialization now uses BotFactory directly at the requested player
level. The random-population manager could select its own level and teleport the
bot independently. After specialization/equipment initialization, stats and health
are recalculated. On teleport acknowledgement the existing full group-status update
is requested and the roster refreshed. This addresses the transient offline/1-HP
presentation; client validation is still required to establish whether it explains
every reported disconnect.

The running server loaded 312,954 creature spawns. Rumbling Terrace (area 6387)
has Mushan, poachers, plainshawks and Galleon in the database, with phase mask 1.
Only 14 future respawns existed across all of Pandaria during the read-only audit.
The user subsequently confirmed visible hostile mobs at Shado-Pan Garrison.
No spawn, phase, respawn or movement database records were changed in this pass.
`NullGameObjectAI` identifies the object's selected AI implementation, not a load error.

Pending live checks: coin-only corpse looting, need/greed passing, player access to
ordinary round-robin loot, silent greetings/departures, and bot status after first
summon and subsequent near/far teleports. Sustained crash validation remains open.

Validation: Visual Studio x64 RelWithDebInfo built revision 2fa43dda with zero failed projects. CTest passed 9/9. The previous local server shut down gracefully before staging worldserver.exe and its matching PDB; both SHA-256 hashes match the build output. The runtime manifest was refreshed, and the visible export launcher was opened again. Live loot, roll, chat and party-status acceptance remains pending.
