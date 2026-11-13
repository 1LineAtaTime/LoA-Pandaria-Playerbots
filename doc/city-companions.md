# City activity and Playersbots addon

The Playersbots addon inspected from the local 5.4.8 client sends ordinary chat
commands. No reference executable was needed. The server now recognizes its
group add, addRoleBotsToGroup, addPvPBotsToGroup, control attack/follow/aggressive,
and altchar login/logout/list commands through `.bot` with permission 1346.
PvP additions currently select the same role-appropriate companions as PvE;
they do not apply a separate PvP equipment preset.

`AiPlayerbot.Companion.MinLevel = 60` applies server-side to command acquisition,
delayed login, invitations, AI commands and assignment of a human master.
`MinSecurity = 0` permits normal accounts; 1/2/3 require moderator/GM/admin rank.
`GMBypass = 1` exempts ranks 2 and 3 from the level/rank threshold. Existing
command RBAC still applies. `MaxBots = 39` bounds owned plus pending logins;
the existing global/per-class limits still apply. Autonomous bots remain eligible.
Role buttons request healer/tank/damage classes and specializations, initialized
at the master's level. The existing group path promotes full parties to raids.

The addon summon button originally invoked the privileged GM `.summon` command.
Only that button was changed to `.bot summon`; no GM permission is granted.
The original local Lua file is saved under ignored `out/Playersbots-Mini_frame.original.lua`.
`contrib/windows/Patch-PlayersbotsAddon.ps1` applies this narrow change on other
clients with a backup. The addon's developer/farming tools remain administrator tools.

CityLife prefers nearby friendly capital travel hubs for ungrouped bots, lets idle
capital bots spend more time visiting service NPCs, selects auctioneer/banker/vendor/
innkeeper stops without immediate repeats, limits concurrent target reservations
to three, varies pauses and abandons targets after 90 seconds. Movement continues
through existing navigation. It does not instantly seed every empty city, simulate
actual bank purchases, or change auction accounting. Presence depends on nearby
free bots and valid travel hubs, service-NPC data and paths.

## Acceptance checks

- Normal level 59: addon and direct bot commands refused; normal invite refused.
- Normal level 60: add healer, tank and damage; inspect level/spec and group membership.
- Request a raid, rapidly repeat requests: total owned + pending must respect limit.
- Try another player's bot/character: no ability to command or alter their spec.
- Change group leader to an ineligible player: no companion master assignment.
- Attack a selected hostile target, hold, follow, passive and summon using addon.
- Walk around capital banks, auctioneers and vendors through several visits;
  verify separation, pauses, movement, and recovery from unreachable stops.
- Repeat with CityLife disabled and verify normal roaming and duel/BG behavior.

Build and policy tests do not establish successful client or city visual validation.

## Build validation — 2026-11-04

Visual Studio MCP built x64 RelWithDebInfo gameplay revision df5768fc with zero failed projects. CTest passed 8/8, including 16 companion-policy cases. Stage-Runtime copied the package to exports; worldserver.exe and worldserver.pdb SHA-256 hashes match the build output, and --version reports df5768fc. Previous worldserver EXE/PDB are preserved in ignored out/exports-before-city-companions. No production realm was started or restarted, and no database migration was applied. Client controls and visible city activity remain pending live acceptance. Forgejo and GitHub were verified private with nine matching refs; production main remains at the prior validated milestone.
