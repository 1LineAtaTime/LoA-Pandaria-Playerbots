# Realm status, corpse assertion and Gilneas battle repair

## Realm status

The original realm row had flag 1 (INVALID) despite the matching 18414 build and
8085 port. Only the invalid bit was cleared, preserving all other bits. The
previous row is in the private out/backups/realm-flag-before-20261111.json.
The local client already remembered the correct realm name. Red/invalid realm
status was repaired; client-side automatic entry is not certified by a UI test.

## Production corpse assertion and concurrent servers

The supplied production crash report shows two distinct Corpse objects entering
the same map object-store GUID, reached through the bot auto-spirit-release path.
The shared atomic allocator and persisted MAX(corpseGuid) seed already existed;
that fix does not coordinate two different worldserver processes. The local
worldserver was active against the same original schemas when production was
tested. Overlapping writers are a plausible cause, not a dump-level proof of
every collision. The assertion was retained; no corpse data was deleted.

A dedicated character-database connection now holds a MySQL/MariaDB named lock
for the process lifetime, keyed by the character schema hash. Startup acquires
it before resetting online flags or changing realm status. A second updated
worldserver is rejected even on a different host. A five-second ownership check
stops the world if that session reconnects or loses ownership. Pool shutdown
precedes lease release. Failed startup now explicitly closes database pools.

Both hosts must use this updated build; an older executable does not honor the
lock. Stop the local world before testing a copied production build against the
same character database. Separate character schemas may run independently.

The integration test contrib/database-tests/Test-WorldserverLease.py requires an
active updated server and checks clean duplicate exit, unchanged lease ownership
and unchanged realm flags. Its private config/log copies live under ignored out/.
The test passed in 3.65 seconds; the rejected process exited with code 1.

## The Battle for Gilneas City, quest 24904

The quest description explicitly directs the player to Krennan Aranas and the
"Let us take back our city!" option. Accepting the quest alone does not start the
assault. A disposable Worgen with the affected progression state received that
option, selected it through normal packets and observed the summoned Liam and
militia. The player's own quest, position and rewards were not edited.

Live player feedback also identified the closed bridge gate: the army started
but the event never opened gameobject 195578. The battle now opens nearby copies
of that door for the event duration and retries during the first route legs if
the neighbouring grid loads late. Joining an already-running battle now places
the player three yards behind Liam using the nearby-ground calculation, rather
than teleporting directly into his model.

Two missing spell behaviours were adapted from the GPL ArkCORE-NG reference:
https://github.com/Arkania/ArkCORE-NG/blob/a7304c3075bf8ee7eb5bdf45f31cda01c8626b52/src/server/scripts/EasternKingdoms/Gilneas/zone_gilneas_city2.cpp

- Artillery trajectory spells 72051 and 72013 preserve the requested impact
  destination, including elevated targets. Triggered damage remains normal
  spell 72050 or 72009 damage; no scripted kill or free quest credit was added.
- Patriots rapier 71388 selects friendly Gilnean Militia for both inspiration
  aura effects, with a paired entry condition and native friendly-unit filter.
- Quest catapults/cannons used a two-hour respawn range. Both minimum and maximum
  spawn intervals are now 30 seconds so leaving a vehicle does not hide it from
  later players. Current test-used timers were restored only for these artillery
  spawns. World spawn rows and matching character respawn rows were backed up.

The cast handler's possession guard incorrectly rejected every cast while the
player possessed another unit, even after selecting and validating the vehicle
as caster. It now rejects only the player's own spells in that state. The
controlled vehicle still passes its authored-spell check; other passengers do
not gain vehicle spells. This also affects other possession/vehicle quests.

Paired migration: sql/updates/world/2026_11_11_01_world_gilneas_battle_spells.sql.
Bindings/conditions/spawn backups and exact timer restoration SQL are in private
out/backups/gilneas-battle-spells-20261111/. Existing unrelated bindings and
player account/quest data were retained. Full battle completion and reward
turn-in remain separate from the successful battle-start check.

## Phase and terrain restoration for the original world

Bloodfang Worgen35118 and Rippers35505/35916 belong to phase masks2/4/8,
not the battle's262144. The original world had no condition rows for its late
Gilneas phase definitions; they accumulated earlier masks unconditionally.
Source25 conditions now select the stage after the Observatory, the battle and
its following milestones exclusively, with overwrite+stop flags. The battle
starter/rejoin actors remain visible in the battle phase and Lorna's quest
turn-in/next-quest handoff spans its two required stages. These are shared world
rules, not a character-specific phase override. Earlier pre-Observatory phase
rules still require their own reconciliation.

Applied the existing2026_11_09_05 terrain/cage migration so map654 selects its
correct638/655/656 terrain stage rather than overlapping sibling terrain.
Restored24 missing battle dialogue records from the archived documented LoA
world, added two lines from the existing client broadcast table and one short
authored cannon-arrival connective line. Existing dialogue rows were preserved.
Krennan's menu now distinguishes starting the battle from joining its leader.

Backups: out/backups/gilneas-phase-restoration-20261111/. Paired new migrations:
2026_11_11_02_world_gilneas_battle_phase_isolation.sql and
2026_11_11_03_world_gilneas_battle_dialogue.sql. A normal-security protocol fixture
at the first battlefield checkpoint saw zero earlier Bloodfang enemies and16
intended Forsaken enemies. Full quest completion is not inferred from visibility.

The initial artillery trace returned SPELL_FAILED_LINE_OF_SIGHT(50) against
Gorerot's rooftop spawn. An indirect-targeting exception was tried and produced
damage, but that did not prove the encounter was playable from the client.
The subsequent correction below removes that workaround and repairs the spawn.
Targeted packet traces remain opt-in; normal warnings/errors stay visible.

## Runtime folder cleanup and deployment

73 top-level obsolete entries, about 5280 MiB, were moved from exports to
out/backups/exports-cleanup-20261111-144844/, with a move inventory. These include
old SQL dumps, duplicate data sets, tools/test executables, reports, old configs,
spare DLLs and crash files. No recursive deletion of game assets was performed.
Current maps/vmaps/mmaps/dbc/db2, scripts, configs, logs and launchers remain.
Current EXE/PDB pairs remain for useful production crash reports. PE dependency
inspection and actual loaded modules require libmysql.dll and
libcrypto-1_1-x64.dll; obsolete alternate DLLs were archived. The deployment
manifest now lists the retained artifacts, rather than missing archived DLLs.

Visual Studio RelWithDebInfo/x64 built the new worldserver successfully and the
matching EXE/PDB were staged in exports. All 24 existing CTests passed. Runtime
spell validation reported no new artillery/inspiration hook errors. Both
services use the original mop_ databases. Older original-world content loader
errors remain visible; this is not a claim that every quest or error is fixed.
No pull request to the original upstream has been opened.

## Final validation results

- 24 native CTests and 37 Python QA tests passed.
- The duplicate-world test rejected a second process cleanly in 2.6 seconds,
  preserving the original lease owner and realm flags.
- The phase fixture observed no Bloodfang entries 35118/35505/35916 while
  23 intended Forsaken (38210) remained visible. Bridge gate 195578 reported
  ACTIVE (0) through the client object update.
- Ordinary logout was refused during combat in that phase fixture. Native
  cleanup subsequently removed its test character and account. The visibility
  and gate assertions do not certify a whole-quest gameplay run.
- The earlier fixture reduced rooftop Gorerot from 81,900 to 62,393 HP with
  a temporary LOS exception. That result did not validate natural aiming.
  See the courtyard correction below for the replacement test and limitations.
- The join offset is implemented, but an automated distance check against a
  moving leader/teleport acknowledgement was inconclusive. Manual retry and a
  full battle completion/turn-in remain open checks.

Private evidence: out/battle-gilneas-artillery-probe.json,
out/battle-gilneas-phase-probe.json and out/database-tests/world-lease/result.json.
All temporary battle QA accounts were removed. No universal quest coverage or
an error-free original world is claimed.

## Gorerot courtyard correction

The user correctly reported that the damaged catapult could not aim at rooftop
Gorerot. The original spawn (GUID 234444) was at (-1674.29,1446.18,52.3704).
ArkCORE-NG commit a7304c3075bf8ee7eb5bdf45f31cda01c8626b52 explicitly places
Gorerot at (-1703.10,1419.87,21.9268), in the prison courtyard:
https://github.com/Arkania/ArkCORE-NG/blob/a7304c3075bf8ee7eb5bdf45f31cda01c8626b52/src/server/scripts/EasternKingdoms/Gilneas/zone_gilneas_city2.cpp

Migration 2026_11_11_04_world_gorerot_ground_spawn.sql moves the existing actor
on map 654 to that position. No character position, quest state or credit is
changed. Private spawn backup and exact rollback SQL are under
out/backups/gorerot-ground-20261111/. The earlier exception allowing spells
72051/72050/72013/72009 to ignore LOS has been removed. Liam's jail checkpoint
now continues waiting while Gorerot lives instead of skipping him after three
minutes; the existing overall battle timeout still aborts abandoned events.

Visual Studio RelWithDebInfo/x64 succeeded with zero failed projects; matching
EXE/PDB were staged in exports and the one local worldserver restarted. All 24
existing CTests passed. The protocol fixture observed Gorerot at courtyard height
21.9268 and three Fiery Boulder shots reduced his health from 81,900 to 62,696
without the removed LOS exception. Full battle completion and real-client aiming
remain separate checks from this protocol damage test.

## Leave Gorerot available for the user's quest test

The persistent spawn still had a fixed 7,200-second respawn delay. After QA
combat it was unavailable for the user's battle attempt. Migration
2026_11_11_05_world_gorerot_respawn.sql sets both delay bounds to 60 seconds.
This is a persistent-spawn retry policy, not a claim of per-battle ownership.
With worldserver stopped, backed up and cleared only Gorerot's pending respawn
(GUID 234444, map 654, instance 0) in the character database, then restarted the
same local runtime. Backup: out/backups/gorerot-respawn-20261111/before.json.
No character quest progress changed. No further damaging QA was performed.

When applying this migration elsewhere, an already saved two-hour respawn is
not shortened by changing the world spawn's default alone. Back up the matching
creature_respawn row, stop worldserver and clear only that actor's saved wait
before restarting. Verify the world's creature GUID rather than clearing all
respawn records. Longer term, per-battle summons would isolate simultaneous
attempts; the current encounter still shares its existing persistent actor.

## Gorerot combat and allied participation

The user reported Gorerot did not retaliate, Crowley stood idle, and killing
the boss did not complete quest 24904. Gorerot had neither ScriptName nor AIName,
but did have VehicleId 629. CreatureAISelector therefore selected VehicleAI,
whose AttackStart and combat updates do not fight. Migration
2026_11_11_06_world_gorerot_combat.sql binds npc_gorerot_gilneas. The native
script holds his position, retaliates in melee, uses Smash 71774 every 13 seconds
against a nearby victim and Thunderclap 8078 every 21 seconds in combat.
Ability IDs/cadences are adapted from the pinned ArkCORE reference above;
combat-only scheduling and defensive stationary behavior are local adaptations.
The vehicle kit, faction and character quest data are preserved. Template
binding backup: out/backups/gorerot-combat-20261111/template-before.json.

The jail checkpoint previously triggered speech only. It now explicitly sends
Crowley and the summoned militia against Gorerot, and starts Gorerot's retaliation.
His defeat releases the existing checkpoint hold. Liam then proceeds to
Greymane Court/Sylvanas. The finale, not Gorerot's death alone, grants credit
(entry 38854) to nearby eligible players after the scripted confrontation.

An active assault is runtime state: a worldserver restart clears it while static
battlefield NPCs remain visible. After a restart, start the assault again through
Krennan's start option, follow summoned Liam through the route and remain nearby
for the finale. Joining a running assault uses the separate join option. Killing
a static boss without a running assault does not start or complete that event.
Full natural battle completion and turn-in remain unverified.

## QA-command crash and recovery

During the isolated combat check, the temporary player-owned summon did not
provide valid retaliation evidence. A follow-up `.npc add` in a private phase
crashed worldserver before combat began. The matching c7aea802e4 crash stack is
Creature::Create (Creature.cpp:991) -> LoadCreatureFromDB -> HandleNpcAddCommand.
The immediate failure dereferenced a null phase-group vector. CreatureData's
constructor initialized only dbData; newly saved/reloaded spawns could retain
uninitialized phaseid/phaseGroup (and other fields) in memory.

The repair initializes every scalar spawn-data field and returns a visible error
if a nonzero phase group cannot be resolved. Valid DB-loaded phase assignments
are unchanged. The disposable map-0 spawn 8467275 was backed up and removed;
the original Gilneas boss was not removed. Matching crash EXE/PDB/dump/text and
owned QA cleanup evidence are preserved privately under
out/backups/crash-c7aea802e4-20261111/. The server was restored while compiling
the correction. No successful combat check or full quest run is claimed from
either of those failed fixtures. Catapult exit trouble was reported around the
crash; a separate seat/exit defect is not established by that observation.

A second c7aea802e4 crash occurred during automatic bot login after restoration,
without a new QA spawn command. Its stack was Unit::GetAuraEffect ->
GetAuraEffectOfRankedSpell -> SkyFire27_priest Power Word: Shield apply handler
(spell_priest_sf27.cpp:866) -> Player::_LoadAuras. The persisted shield's caster
was absent (null), but Body and Soul handling dereferenced GetCaster(). The
repair preserves the shield and weakened-soul behavior, and evaluates the
caster-dependent talent only when the caster resolves. No saved auras or bot
characters are deleted as a workaround. The second dump/text is preserved beside
the first, with their matching EXE/PDB.

Read-only client-data inspection found Damaged Catapult vehicle 634 uses seat
7537 (flags 0x73108c0b), whose CAN_ENTER_OR_EXIT flag is enabled. Emberstone
Cannon vehicle 470 seat 5206 also permits exit. No broad bypass of vehicle-seat
restrictions was added; exit around the observed crash remains inconclusive.

QA safety for the active user test: do not damage the shared map-654 boss to
validate spells. The recovery regression checks creation/loading of a disposable
actor in a private phase and removes it without combat. A successful spawn
regression does not certify Gorerot's combat, Crowley's movement, or quest credit.

### Rebuilt recovery validation

Visual Studio RelWithDebInfo/x64 completed with zero failed projects. Matching
EXE/PDB and SHA-256 manifest are staged in exports. The source includes
ac2a69dfee (spawn initialization/phase check) and 10d98858e0 (shield caster guard),
alongside c7aea802e4's combat/allied-engagement changes. The ignored generated
scripts project used eight compiler workers to reduce the dependent rebuild;
source CMake settings were not changed.

A disposable normal-world protocol client with GM command permissions loaded
spell 17 from its own saved aura rows, with an intentionally absent caster,
then created/reloaded an isolated Gorerot in a private map-0 phase. Both paths
survived. It removed the actor without attacking it, logged out, and native
cleanup removed its character/account/dependencies. A database check found no
Gorerot spawns outside map 654 remaining. Private result:
out/gorerot-spawn-regression.json. All 204 queued bots completed login and the
same single local worldserver remained listening after the check. This is a
bounded regression result, not a long-term crash-free guarantee.

Gorerot's combat abilities, Crowley's real movement and the complete battle
finale/turn-in remain unverified by these non-combat regression checks. After
this restart the player must start a fresh assault at Krennan, follow summoned
Liam to the jail, defeat Gorerot, and continue with the army to Sylvanas. Do not
infer completion from killing the static boss outside an active assault.

## Confirmed battle completion and post-battle quest handoff

The migration-07 boundary described here is superseded by migrations 08/09 below.

The user completed The Battle for Gilneas City and turned it in to Lorna. This
confirms a real-client battle/credit/turn-in run; it does not independently prove
every combat ability. Lorna then disappeared during conversation. Read-only
quest history also showed The Hunt for Sylvanas (24902) rewarded, but no later
milestone. Our stage-10/11 phase boundary used reward 24902, hiding the city-phase
Lorna 38611 and Genn 38539 before Vengeance or Survival (24903), Slowing the
Inevitable (24920), and Knee-Deep (24678) could be handed off.

Migration 2026_11_11_07_world_gilneas_post_battle_handoff.sql changes that boundary
to reward 24678. The database's actual starter/ender relations and Previous/
RewardNextQuest chain independently place those quests before Laid to Rest
(24602) and Patriarch's Blessing (24679), whose Krennan 38144 uses stage 11.
Krennan now shares stages 10 and 11 so Knee-Deep can be turned in and Laid to
Rest offered without the quest giver disappearing. Lorna/Genn's existing spawn
masks stay unchanged. No player progress, quest objectives or rewards are edited.

Backup: out/backups/gilneas-post-battle-20261111/before.json. Structural checks
verified complementary condition boundaries and all three handoff actors visible
in stage 10; the worldserver was restarted without recompiling. The actual next
quest conversations remain for user confirmation. The earlier migration 02 is
preserved as history and migration 07 supersedes its premature 24902 boundary.

## Slowing the Inevitable: bombing phase and personal riding bat

The user reached quest 24920 but could not see Captured Riding Bat 38615. It and
all 41 Forsaken Catapults (38287) / 371 Forsaken Invaders (38363) are in phase
524288. Migration 07 held the city phase until Knee-Deep reward, which was too
late. Migration 08 now selects the bombing phase when 24920 is active, complete,
or rewarded, until 24679 is rewarded. Before acceptance, Lorna and Genn remain
in the city phase. Genn shares both phases for the offer/turn-in/next quest.
Krennan also remains visible after blessing reward to offer Keel Harbor.
These use supported quest condition types 9,28,8 with complementary AND/OR groups;
no character-specific override or quest-status mutation is applied.

The original world also lacked the personal ride configuration: summoned bat
38540 had no vehicle kit or AI; both entries lacked flight scripts/path data.
Migration 09 restores VehicleId 641, bombing spell 72246 and return spell 72849,
uses SmartAI to start after the player boards, and restores the archived LoA
35-node route. The 8-node return route reuses its landing segment. The parked
bat now summons the personal vehicle rather than also offering a conflicting
46598 seat on the parked actor. Boarding requires the active bombing quest.
This is a local SmartAI adaptation using the archived route and the existing
1020 passenger-boarding timing rule, not an unmodified published ride script.
Quest IDs in the path actions are deliberately zero: the flight grants no
completion or kill credit. Bombing objectives remain six catapults and forty
invaders. Passengers are released after the landing/despawn; abandoned personal
vehicles are cleaned up. Return path ID 3854001 was checked unused before import.

Backup: out/backups/captured-bat-20261111/before.json, including scoped template,
condition, click, spawn mask, SmartAI and waypoint rows/columns. No new schemas
or compiler changes were needed. Source route extraction evidence is private
out/captured-bat-archive.json; no runtime user data is shipped with the migration.

Five read-only phase/actor checks passed (before acceptance, active, complete,
rewarded, blessing rewarded). A disposable normal-account Worgen fixture with
seeded quest prerequisites saw the parked bat, boarded its personal vehicle,
traveled along the rising route, used the return button and was released.
Its quest remained incomplete; no bombs were fired at shared targets. Private
reports: out/captured-bat-phase-checks.json and out/captured-bat-ride-probe.json.
These establish visibility/boarding/flight/return, not a natural full bombing
objective/reward run or every possible return position.

The ride fixture's subsequent voluntary logout was not acknowledged. Its finally
cleanup completed native character/account/dependency removal. Thus its ride
assertions passed, but the overall protocol lifecycle did not; the private report
now distinguishes these results explicitly. No cause is inferred from the
missing logout acknowledgement (for example, landing movement acknowledgements
would require separate observation). No fixture identity was left behind.
