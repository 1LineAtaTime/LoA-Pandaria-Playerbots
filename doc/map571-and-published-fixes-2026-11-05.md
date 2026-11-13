# Map571 investigation and published fix compatibility — November5,2026

This is ongoing repair work, not a flawless-release claim. No warning category
or severity was disabled. Accounts, passwords, wallet/catalog, personal quests
and character positions were not edited.

## Map571 status: not yet resolved

The earlier map571/37/24 MMap warning and zone0 graveyard lookup referred to
(-2908.362549,4147.828125,41.296257). Its terrain file is an888-byte empty tile;
there is no matching navigation tile. The existing24/37 tile is another place
and must not be renamed/copied to satisfy the filename. No Detour/map/MMap
change was made. Prior navigation-crash validation remains open.

Archived logs show the zone failure immediately before creation of bot308350
Orgek's self-update on two boots. That is a correlation, not proof he caused
it. His current saved position is valid in Howling Fjord, map571 at
1889.42/-6183.43/23.6708; transport/taxi/corpse records do not retain the failing
position. Other saved character, corpse, homebind, spawn and safe-location
checks have not found that point. No bot position was rewritten for this audit.

The13:57 and14:08 fresh runs loaded Orgek without repeating the571 failure.
Therefore these runs cannot validate a root-cause repair. A generic zone0
error now retains requester name/GUID, current map/XYZ, alive/teleport state,
and corpse map/XYZ. It works during login/map transfers, where the requester
may not yet be in the world, without dereferencing Map*. A future occurrence
can distinguish a real invalid location from a stale map/corpse/transfer mix.
The normal faction graveyard fallback and original error severity are intact.
No global stack tracing, warning filter or fabricated navigation tile added.

## Published fixes implemented

### SkyFire1408: script-cleared nearby target crash

[Original report and tested fix](https://github.com/ProjectSkyfire/SkyFire_548/issues/1408).
Local Spell::SelectImplicitNearbyTargets had the same unsafe sequence: validate
the initial candidate, invoke a hook that can clear it, then dereference it for
unit/gameobject/destination selection. The new post-hook null guard returns
before every dereference and before chain selection. Other object-selection
call sites were reviewed and already guard nullable results. This repairs the
core hazard for any hook; it does not assert that Rune of Fire's entire quest
workflow was playtested locally. Native compilation/CTest results follow.

### Stair of Destiny introductory ride / ordinary taxi map

Published release26 [world_04](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_04.sql)
and [world_05](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_05.sql)
apply to flight masters18930/18931, menus7938/7939. This fork reverses the
upstream option numbers: option0 casts the introductory flight34924/34907;
option1 opens the normal flight map. Blindly applying the original SQL would
change the wrong options. Its CONDITION_QUESTTAKEN9 also recognizes only
INCOMPLETE locally, so the adaptation uses QUESTSTATE47 mask10, allowing
incomplete or complete-but-unrewarded delivery quests10289/10140. Ordinary taxi
maps lose only their exact stale rewarded-quest gates.

Guarded SQL26 changes two exact conditions and removes two stale gates, while
preserving all other conditions, creature flags, menus and SmartAI. Disposable
fixtures verify row preservation and idempotence; the state mask excludes
NONE/FAILED/REWARDED. The full conditions table was backed up before applying
with worldserver stopped. Player flight-menu acceptance remains pending.

## Reviewed candidates and why published is not automatically applicable

The296 archived release26 SQL files are downloaded to ignored local research
storage and indexed with the195 current issue reports. A file's publication
does not prove its old schema/option IDs/script names match this fork.

- SF1553 Disengage: reported obsolete781→56446 link is absent; current core
  already handles781 as a backward jump. Existing Glyph of Liberation retained.
- Tarindrella92420 destination: current map1/10551.8/877.247/1309.94/3.47321
  already matches published2026_08_11_world_00. No overwrite needed.
- Huo offerings102522: local target condition already limits effect1 to Huo
  54787, and four spell_scripts stages cast109090/109095/109105/109109. The
  published native script name does not exist here; adding that binding alone
  would introduce another missing-script error. Gameplay review remains open.
- Merchant Square14098: local SmartGameObjectAI already uses two authored timed
  lists. The published native replacement has API/phase/lifetime/quest gating
  differences that need local reproduction/adaptation, not a blind rebind.

Keep the complete upstream worklist open except where local evidence supports
a specific completed repair or unaffected/already-present finding.


## Final deployment —14:18 local

Visual Studio RelWithDebInfo x64 native build through99aa5796/revision1348
completed with zero failed projects. All17 CTests passed against this build.
The nearby-target crash guard and enhanced graveyard error context are staged
as matching EXE/PDB in exports. Visible export launcher used; auth unchanged.
SQL26 is applied and included in exports/sql/world. It does not need further
schema/auth/player migrations.

The14:14 fresh run also exposed a separate map0/42/23 missing-tile warning;
this is not evidence of a571 fix or regression. Map571 itself did not recur in
the13:57,14:08 or14:14 fresh runs. Neither location issue is declared resolved.
A short successful startup is not an extended stability or gameplay test.

Conditions backup: `<local-workspace-path>` (1,449,743bytes).
Runtime/log backup: `<local-workspace-path>`.

| Artifact | SHA-256 |
| --- | --- |
|worldserver.exe|`774f9524d5ade7763687d27e074aa76e10ffb5dadfd486800433807abc71f60a`|
|worldserver.pdb|`ec3867e479804f1e48650c157d64bb176982c21e486e8afabda55f9765a63bbf`|


November6 follow-up: the separate map0/42/23 missing tile is now generated and
installed; parent/eight-neighbor load compatibility passes. Map571 remains open.
See mmap-42-23-repair-2026-11-06.md.


November 6 coverage follow-up: generated the actual 0571_37_24 navigation tile
and all 68 missing Northrend candidates with usable installed inputs. Every
addition passed original-parent/neighbor load checks. This resolves those missing
assets; it does not establish why a caller earlier requested a zone0 world point.
See mmap-coverage-repair-2026-11-06.md for the complete 495-tile batch.
