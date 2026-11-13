# Gilneas finale vehicles and missing scene

Quests: 24681, They Have Allies, But So Do We; 26706, Endgame.

The original world database had VehicleId=0 for the controllable glaive
41599, boarding hippogryph43751 and escape wyvern43713. Generic click
rows on38150 invoked conflicting summon chains; each click could leave another
non-vehicle creature. The entire static gunship scene was also absent.

The paired native/SQL repair restores vehicle447 (glaive),641 (hippogryph)
and983 (escape). Parked38150 uses native gossip: an incomplete24681 is
required; an existing vehicle blocks another summon, a per-player three-second
gate bounds repeated requests, and an owned nearby empty vehicle is reused.
Unboarded summons expire after five seconds; exit removes the abandoned vehicle.
Generic spell-click summon rows on38150/37927/43747 are removed so a direct
spell-click packet cannot bypass the native checks. Glaives have player-controlled
70735/70732 weapons; they are not autonomous following pets.

Endgame requires rewarded24681 before boarding. Existing prematurely accepted
quests are preserved. Launcher and questgiver phase visibility bridges the two
stages. Tobias retains his timed signal; the flight and escape retain native AI
while charmed. No user quest is completed, reset or rewarded by this migration.

Migration `2026_11_12_00_world_gilneas_finale_vehicles_scene.sql` restores53
scene creatures and six objects from the matched archived LoA scene, including
the static ship, ropes, furnace explosives and Korm. Its upstream provenance is
ShatterCore, documented in `2026_10_09_14_world_gilneas_endgame_gunship.sql`.
New GUIDs are allocated above each table's maximum, with entry/map/position
existence guards. Existing text wins. Lorna's seven original lines come from
the repository's `sql/old/Merged to World DB 30_12_2023/2022_12_01_creature_text.sql`.
The legacy `entry` column and compatibility `CreatureID` are both populated.

Apply while worldserver is stopped, with the matching native build. Several
world tables use MyISAM: START TRANSACTION is not an atomic rollback guarantee.
Back up affected tables first. The local private backup is
`out/backups/gilneas-vehicles-20261112/world-before.sql.gz`; binaries/PDB are
retained there too. Do not distribute that database dump.

The screenshot's taxi error is a separate bot lifecycle defect: DBC path2301
ends its map0 spline at node6 and continues on map1 at node7. A real client
sends CMSG_MOVE_SPLINE_DONE; the bot's continuation returned early whenever the
flight generator was present. The bot now performs the existing session's map
handoff when the spline has actually finalized at that boundary, retaining taxi
state through the teleport acknowledgement. Invalid saved routes still produce
diagnostics; no error severity was lowered.

Validation at implementation: Visual Studio RelWithDebInfo/x64 build succeeded.
Database checks found53 scene actors, six objects, Lorna groups0–7, and all
three vehicle kits. Live gameplay checks are recorded below after deployment.

Live evidence:40 requests (20 gossip plus20 direct spell-click) produced one
owned controllable glaive. It remained boarded beyond the failed-boarding
cleanup deadline; explicit exit removed it. Actual ability70735 dealt142
damage, and70732 killed an isolated quest infantryman (222 HP) and delivered
one37916 objective credit. These are copied NPC templates in a private test
phase; this is not proof of all52 natural objective kills and turn-in.

The timed signal allowed hippogryph boarding. Its native flight reached the
ship, ejected the rider at(-1465.15,3256.89,191.68), and exposed restored Lorna.
The final fixture did not receive a voluntary-logout acknowledgement; native
cleanup subsequently deleted its owned account, character and dependent rows.
Count the vehicle gameplay checks separately from that unresolved protocol
lifecycle result. No user account or quest was reset.

Reproducible disposable checks are in
`contrib/headless-client/gilneas_vehicle_regression.py` and
`gilneas_torch_regression.py`. They require the local test server/database and
create temporary GM fixtures, keep passwords in memory, and remove only their
owned identities. They seed prerequisite state and do not certify a natural
new-character quest chain. The report distinguishes gameplay from logout status.

The torch item probe now passes; see runtime-log-repairs-2026-11-12.md.
Existing loot/pool data diagnostics remain unresolved. This is not a zero-error
or all-quests-fixed claim, and the cross-map taxi change still needs a dedicated
live route regression.

The follow-up furnace check exposed a second escape defect:81779 delivered a
43729 objective packet without an observed usable wyvern or harbor arrival.
LaunchWyverns now explicitly validates/summons43713 and boards the player,
using the same known native vehicle path as the hippogryph. It no longer credits
an unrelated already-occupied vehicle. Empty failed-board rides expire after
five seconds; finale credit is given only on successful native escape arrival.
The follow-up furnace recovery regression passed: a seeded enlarged-health
fixture observed43713 boarding, returned to harbor(-1336.1,2097.88,13.87),
received one43729 objective credit and persisted completed status1 for26706.
It used the native timed explosion, not a GM quest-completion or kill command.
This verifies the recovery/escape path, not ordinary-level Korm combat or the
entire top-deck-to-turn-in chain. Its owned account/character/dependencies were
removed. Runner: `contrib/headless-client/gilneas_escape_regression.py`.

Companion migration `2026_11_12_01_world_endgame_ship_template.sql` supplies the
custom800100 ship template additively for a maintainer who lacks the older
gunship migration. Apply both November12 migrations before starting the paired
build. An existing authored800100 template is preserved.
