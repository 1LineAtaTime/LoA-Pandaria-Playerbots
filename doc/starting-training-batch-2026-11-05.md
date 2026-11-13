# Starting-zone training repair and test batch — November 5, 2026

## Environment and recovery policy

The user explicitly designated this entire deployment as the test environment
and authorized broad repair batches on November 5. Do not require another server
or realm to continue. Use disposable SQL fixtures where helpful, then test the
actual server. Record failures as well as passes. Use identified bot/test actors
for destructive gameplay setup; preserve existing personal characters and custom
BattlePay content unless a specific test requires an authorized change.

Git recovers source/configuration templates/migration history, not runtime
database contents. Keep credential-bearing configs and database dumps outside
Git. Before major data changes, save affected tables or databases and record
their hashes and restore procedure. Stop worldserver before applying this batch;
authserver can remain running because its database is not modified. Reopen the
visible export launcher after checks, with matching existing EXE/PDB retained.

## SQL27 scope and upstream adaptation

Migration: `sql/updates/world/2026_11_05_27_world_starting_training.sql`.
It adapts the numeric level/race/class fields of seven release26 patches pinned
to SkyFire commit `505f6e63a20c3ea75dfebb17f58e891f4e5fb44c`:

- `2026_07_25_world_04.sql`, `_05.sql`, `_07.sql`;
- `2026_07_26_world_01.sql`, `_02.sql`, `_03.sql`, `_04.sql`.

[Pinned source archive](https://github.com/ProjectSkyfire/SkyFire_548/tree/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001).

The active loader reads QuestLevel/MinLevel/AllowableRaces from quest_template
and AllowableClasses from quest_template_addon. Map the obsolete Level and
RequiredRaces names accordingly. All 42 published class-mask values already
match the active addon rows, so no class-mask write is needed. The 306 assertions
cover 111 quests and these numeric fields. The local migration changes 223
fields, all in quest_template; expected-original guards protect divergent data.
It makes training quests available at level 2 as published, rather than requiring
level 3 before the player can receive the instruction to reach level 3.

The Night Elf patch `_02` incorrectly uses Dwarf Simple Rune quest3106 for its
Simple Sigil changes. Local quest titles and race data identify Simple Sigil as
3116; adaptation uses 3116. Do not install the upstream questgiver/previous-quest
rows pointing to 3106. Dwarf3106 retains the Dwarf mask; only its separately
sourced level correction is included. Text, objective spells, quest chain links,
spawns and SmartAI replacements are outside this numeric batch.

## Automated evidence

- Before deployment, the tracked suite reports 83/306 passed and 223 failed,
  matching the 223 intended corrections. No query errors.
- The complete copied quest_template/quest_template_addon fixture (18,145 and
  14,232 rows) matches the expected changes across every field; a second
  application has no effect. Raw report: out/training-fixture-results-20261105.json.
- Reusable numeric-migration fixture tool independently passes all 223 guards,
  full-row preservation and repeat stability over all 18,145 quest rows, then
  removes its fixture. Report: out/training-fixture-reusable-20261105.json.
- Fixture comparison uses binary HEX values, preserving embedded carriage
  returns/newlines in authored text. Initial ad hoc TSV parsing failed on those
  fields before any live migration; disposable fixtures were cleaned up. No data
  repair was made to text merely to accommodate the comparison tool.
- Runtime application, backup, post-check and restart evidence follows below.

These checks validate world data and preservation, not normal gameplay.
The ordinary-player race/class acceptance matrix remains open: level1/2/3
availability, class/race exclusion, actual training spell credit, turn-in/reward,
next quest, abandon/retry and relog. Never force completion to report a pass.

## Repeatable checks

```powershell
python contrib/upstream-tests/test_numeric_world_migration.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --migration sql/updates/world/2026_11_05_27_world_starting_training.sql --report out/training-fixture.json
python contrib/upstream-tests/run_world_contracts.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --suite contrib/upstream-tests/suites/starting-training.json --report out/training-contracts.json
```

The contract runner issues generated read-only SELECTs and returns nonzero for
any mismatch/error. Keep schemas/passwords and raw private runtime reports out of
commits. The fixture tool accepts only the narrow guarded numeric UPDATE syntax,
copies whole referenced world tables and never applies changes to the source.

## Next repair and behavioral test batches

- [x] Finish actual runtime SQL27 deployment and all 306 postconditions.
- [ ] Exercise starter-quest eligibility/credit through bot scenarios and real
  client checks on this server. Mark data, synthetic scenario and natural
  progression results separately; record unsupported automation.
- [ ] Review the rest of the seven source files: training dummy credit/spell
  objectives, texts, questgivers and chain links. Retain the 3106/3116 correction.
- [ ] Implement the reviewed Gilneas door/wave/Avery progression repairs as paired
  native/data batches, preserving working local implementations.
- [ ] Continue missing native bindings and actual-speaker SmartAI/text errors;
  prioritize crashes and progression over cosmetic messages.
- [ ] Reproduce map571 zone0/nav request with its new caller diagnostics; do not
  rename transposed tiles, fabricate zone IDs or reduce log severity.
- [ ] Build bounded bot travel/combat/instance/auction/mail endurance runs and
  failure reporting on this server. Preserve dumps and exact matching symbols.

## Runtime deployment and recovery —18:24 local

Worldserver44328 exited through its supported shutdown command. Both complete
quest tables were dumped with the server stopped to
`out/starting-training-before-20261105-182416.sql` (18,176,469 bytes), SHA256
`50ebc8189880df4cbcb042345a576f678c753f6cf1d7797f7c2e500f14b1dec7`.
SQL27 raw-file SHA256 is
`5c4519436bf9c0c5709e4d7763c283e309bdf69069a70329531a77eac83e6f0a`.
The pre-deployment fixture used the same SQL with normalized line endings
(text SHA256 e0f985eb1f06aaab319d6d4bf5d3eab88a269f08c2c1d90c7e5594cc1f099101).

After application, all306 tracked data assertions pass. No auth, character,
playerbot, inventory, mail or BattlePay tables were changed. The visible export
launcher started worldserver59704 at18:24:51; authserver61228 remained running.
Native executable/PDB are unchanged because this is a data/test-tools batch.

Backup recovery was actually tested: import both tables into a disposable schema,
verify all306 original values, apply SQL27, then execute the manual rollback under
`contrib/upstream-tests/rollback/2026_11_05_27_world_starting_training.sql`.
Both entire tables match their restored originals after reversal, including every
text/unchanged field. Fixture was removed. Recovery report is
`out/training-recovery-results-20261105.json`; no rollback was run on the server.

To reverse this batch, stop worldserver, retain a fresh backup, review any later
quest edits, and apply the scoped manual rollback. Full recovery can instead
restore the recorded backup into the configured world schema; that replaces both
covered quest tables, so it also reverses subsequent edits to those tables.
Restart using the visible export launcher. Do not treat a Git revert as a
database restore or put the private backup/configuration into Git.
