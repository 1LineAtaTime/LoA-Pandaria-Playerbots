# Published-patch and content audits

The latest counted historical quest backlog has been reconciled and the compatible
bundle deployed. Read `doc/remaining-quest-compatibility-deployment-2026-11-09.md`
before replaying older ledgers. Source dispositions are not gameplay passes.
The extended assembler requires the paired signed native/schema build when
`--allow-signed-next` is used; it never upgrades the running database itself.

```powershell
python contrib/upstream-tests/assemble_historical_quest_fields.py --extended --allow-signed-next --output out/full-history-20261109/remaining/extended
python contrib/upstream-tests/finalize_historical_quest_compatibility.py
python contrib/upstream-tests/audit_remaining_quest_inputs.py
```

These commands assemble against the current data. Recreating the exact deployed
delta requires its pre-change public-table snapshot. The deployer separately
requires stopped, named local worlds, backup receipts and exact field/hash guards.
Do not execute archived SQL wholesale or overwrite custom service tables.

These tools produce compatibility/reference evidence. They do **not** deploy
patches or declare gameplay correct. Keep reports under ignored `out/`; the
active configuration and database snapshots are private runtime material.

## Published world SQL

```powershell
python contrib/upstream-tests/audit_world_patches.py `
  --config exports/worldserver.conf `
  --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe `
  --patches out/skyfire-quest-research/release26-sql `
  --report out/upstream-patch-audit.json
```

The configured administrative connection creates two randomly named disposable
schemas. Only referenced **world** tables are copied; auth/character schemas
are not copied. A random temporary SQL account can select the baseline and
modify the workspace, but cannot access live schemas. The runner proves that
both a live SELECT and a zero-row live UPDATE are denied before running any
published SQL. Credentials stay in process environments/memory. The account
and both schemas are removed in `finally`, and cleanup is recorded in the report.

Every patch gets an independent baseline restored from the affected tables.
SQL errors and before/after row counts, extended checksums and schema hashes
are recorded. A failed multi-statement patch may partially change its fixture;
that fixture is discarded before the next candidate.

Interpret statuses precisely:

- `sql_failed`: incompatible syntax/schema, denied privileged operation, or a
  missing dependency. Inspect the error and adapt the patch.
- `sql_no_observed_change`: no checksum/count/schema change. This does not
  establish that the feature is already repaired; its predicates could simply
  match nothing, or its intended script could be absent.
- `sql_changes_fixture`: SQL executed and changed fixture data/schema. This is
  **not** permission to replay it on production and not a gameplay pass.

In particular, this world schema contains legacy/modern column pairs. A patch
can successfully populate a legacy column that the runtime loader ignores.
Check actual loader contracts, native script registration, installed DBC,
quest/phase requirements and custom overrides before producing a scoped live
migration. Avoid full-table replacements that erase authored local content.

Baseline tables are copied when first referenced during the run. Run while
world data is not being administratively edited; this is not a transactional
snapshot of concurrent MyISAM world edits. The audit can be IO intensive; use
`--limit N` for a short smoke check. It never copies auth/character data.

## Read-only content census

```powershell
python contrib/upstream-tests/audit_content_catalog.py `
  --config exports/worldserver.conf `
  --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe `
  --data exports --report out/content-catalog-audit.json
```

Checks quest objective references (types0–5), starter/ender relations and item
spell references. Server-only `spell_dbc` IDs augment client `Spell.dbc` so
custom spells are not automatically reported missing. Item findings retain
the RequiredLevel>=100 quarantine marker. Do not delete or quarantine content
merely because a reference check reports it; verify intended use and runtime
corrections. The report explicitly records zero behavioral tests.

## Existing gameplay harness

`modules/mod_playerbots/src/Scenario/BotScenario.cpp` already exposes
`.botscenario` and `.botprobe`, including quest, cast, item, combat, travel,
instance and battleground operations. Quest scenario syntax is:

```
botscenario <botname> quest <map> <x> <y> <z> <questId> [seconds]
```

The harness can reset quest chains, clear logs, grant bags and optionally spawn
targets. Those are useful mechanism fixtures, but **are not full real-world
quest playthroughs**. Some verbs can target real players; never point a mutation
scenario at a user's character. The user designated the current server as the
test environment and authorized broad documented changes: use identifiable test
bots/accounts here, with backups and explicit setup/result records. Another
server or realm is not required. Disposable SQL fixtures test data preservation
without starting another game server.

For progression acceptance, require normal prerequisites and acceptance,
natural objectives (no forced completion), turn-in, correct rewards, next-quest
availability, phase/visibility and recovery after death/logout/abandon/retry.
Distinguish `passed`, `failed`, `unsupported` and `not tested`; a timeout can be
a bot-navigation failure rather than proof the quest script is broken.

Spell cases need expected targets, costs, cooldowns, damage/healing, aura
effects/removal, immunities, range and rank/difficulty rules. Item cases need
create/equip/use/consume/loot/mail/trade/relog assertions as applicable. Casting
every spell or creating every item once cannot validate those rules, and doing
so on the live realm can change the world or trigger exactly the crashes under
test. Real-client checks remain necessary for shop UI, protocol, rendering and
client-specific failures. The earlier BattlePay UI hang demonstrates why a
successful server transaction alone is insufficient.

## Tracked data contracts and numeric migration preservation

```powershell
python contrib/upstream-tests/run_world_contracts.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --suite contrib/upstream-tests/suites/starting-training.json --report out/training-contracts.json
python contrib/upstream-tests/test_numeric_world_migration.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --migration sql/updates/world/2026_11_05_27_world_starting_training.sql --report out/training-fixture.json
```

Contracts are tracked generated SELECTs with explicit expected rows. Any mismatch
or SQL error returns nonzero. They establish data coverage, not gameplay success.
The numeric fixture tester accepts only expected-original-guarded numeric UPDATEs,
copies whole referenced world tables, compares every binary field, tests a second
application and removes the fixture. It never applies SQL to the configured source.
On an already-upgraded baseline, zero matching old-value guards is expected;
the first SQL27 pre-deployment run matched all223 guards. Manual reversal SQL is
under rollback/, outside the automatic world-update directory. Stop worldserver
and preserve a fresh backup before deliberately reversing a deployed batch.

## Current native/content suites and a real event mechanism test

The existing-quest-ports.json and training-chain-links.json suites add17 and34
data assertions. Run them with run_world_contracts.py exactly as the306-case
starting-training suite. Worldserver carrying7f75c1fb and SQL28/29 are required
for the native mechanism path and updated data. No second server is required.

On Windows, the reserved-bot runner uses the visible server console and records
the server's actual [WORLD_EVENT] trace. It checks an existing RNDBOT identity,
temporarily moves it, invokes a loaded event, checks bounded NPC creation, and
requests return to the saved database location. The native command independently
rejects personal characters/pending teleports/distant positions. Example:

```powershell
python contrib/upstream-tests/run_console_world_event.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --pid <worldserver-pid> --bot <reserved-bot-name> --log exports/logs/Server.log --report out/world-event-live.json --event 26277 --map 0 --x 740 --y 680 --z 45 --min-created 1 --max-created 25
```

Preserve the test actor's row before mutation. The bot may continue normal AI
rotation afterward; the report says return requested, not exact state restored.
One chance-based sample proves execution/bounds, not a distribution or quest
playthrough. The native test checks all100 chance samples deterministically.
Raw account/configuration/actor/runtime reports belong outside Git/exports.


## Gilneas paired migration and recovery check

SQL30 and SQL31 form one batch. Apply both with worldserver stopped after
backing up all thirteen affected tables; SQL31 reconciles native/SmartAI
ownership and defers the incomplete Crowley boarding bridge. Do not apply SQL30
alone or enable Crowley's missing native ride merely to silence its warning.
The manifest pins nineteen historical inputs; that is not nineteen new verified
quest playthroughs. See doc/gilneas-progression-campaign-2026-11-05.md.

```powershell
python contrib/upstream-tests/test_gilneas_migration.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --backup <pre-SQL30-thirteen-table-dump.sql> --report out/gilneas-recovery.json
python contrib/upstream-tests/run_world_contracts.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --suite contrib/upstream-tests/suites/gilneas-progression.json --report out/gilneas-contracts.json
```

The recovery tester executes reviewed local SQL using the configured administrator;
it is not a privilege-isolated sandbox for untrusted patches. It creates unique
disposable schemas, rejects reserved-ID collisions, tracks actual pre/post row
identities at each statement (including intermediate backend changes), checks
preservation outside those scopes, second-run stability, all85 contracts and
complete baseline table/schema restoration, then removes only its own schemas.
On an already-upgraded database supply the original backup; reserved ship IDs
otherwise intentionally block the fixture. Dumps and raw reports stay private.


## Crowley ride follow-up

SQL32 requires the new native questgiver, horse and mounted-Crowley handlers.
It adds retry gossip to35552 and replaces mounted Crowley's nearest-horse
SmartAI with explicit per-ride ownership. Existing path35231, firebomb67063,
credit35582, quest relations and dialogue remain unchanged. It does not complete
quests or grant credit on arrival. The original SQL30/31 test remains available;
the new mode checks SQL32 and sixteen Crowley contracts against seven copied
tables, with full-row preservation, repeat stability and restoration:

```powershell
python contrib/upstream-tests/test_gilneas_migration.py --batch crowley --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --report out/crowley-fixture.json
```

For actual-backup recovery, add --backup <pre-SQL32-seven-table-dump.sql>.
Include creature_template, smart_scripts, creature_text, waypoints,
quest_objective, creature_queststarter and creature_questender in that dump.
No personal character or account is needed for fixture tests. Actual riding,
vehicle ability/credit, simultaneous riders and disconnect/retry still require
runtime/client validation; structural checks are not substitutes for them.


For a read-only runtime combat observation, `botscenario <reserved-bot> combatstate`
records one `[COMBAT_STATE]` INFO line in Server.log with map/coordinates, health,
combat flag and victim entry. It requires an online configured random-account
bot with no pending teleport and changes no player state. It does not enable
DEBUG output for the entire bot population. Console warning/error filtering is
unchanged. Use an identified/backed-up RNDBOT actor for any separate teleport or
engage setup, and record return requests and whether actual combat occurred.


`botscenario <reserved-bot> combatstate <npc-entry>` additionally snapshots the
nearest living NPC within60 yards. Empty/uninitialized splines explicitly report
initialized0, duration0 and rise0 without calling Duration(). For a valid spline,
records include actual Z, velocity, duration and height above its endpoint. The
Torn Earth regression used600 requests, including391 uninitialized cases and
actual rising/falling NPC samples; see doc/vertical-knockback-repair-2026-11-06.md.
Keep raw actor/trace/dump output private and distinguish observation from natural
combat/quest/client acceptance. The final reserved observer was not in combat;
the target world effect still occurred and was sampled.

## Release-26 quest / starter AI batch (November 6)

`assemble_release26_quest_batch.py` pins the 296 archived source hashes and
assembles only the documented 79-file subset into SQL33. It translates active
quest/text tables, dual speaker identities, source-type ownership, the Frostmane
self-link and the legacy forward-movement action. The JSON ledger records every
excluded portion; this does not assert that all SkyFire fixes are installed.
`test_release26_quest_batch.py` restores the operator's trusted eight-table backup
into a disposable schema and checks field preservation, expected values and repeat
stability. It does not mutate the configured source or test client gameplay.
See doc/release26-quests-worgen-2026-11-06.md for exact commands, backup identities,
SQL34/35 deployment, native Bloodfang repair and pending natural acceptance.

## Release-26 services and class batch (November 6)

`assemble_release26_services_batch.py` selects 77 pinned source files and adapts
incoming loot/vendor/trainer/proc keys while retaining unmentioned rows. Supply
`--baseline doc/release26-services-ledger-2026-11-06.json` to reuse the captured
pre-update binding pairs; do not infer cleanup from the already-updated database.
`extract_release26_reference_dependencies.py` verifies the public package digest,
extracts only the needed reference dependency graph, maps IDs and normalizes the
one known overfull group. It never imports a full database.
`test_release26_services_batch.py` checks these trusted migrations in disposable
world-table copies: expected values, mutation scopes, repeat stability and newly
invalid loot distributions. SQL39 adds real chest owner/drop/mask checks. See the
services report for exact reproduction, native requirements, backup and recovery.
