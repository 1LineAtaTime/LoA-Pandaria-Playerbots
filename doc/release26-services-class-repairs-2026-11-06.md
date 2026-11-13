# Release-26 loot, services and class repairs — November 6, 2026

This continues the authorized large-batch SkyFire campaign on the current test
server. It draws from 77 archived patch files: 21 loot/vendor/trainer sources
and 56 class-spell/proc sources. Some intended values already matched locally.
This is not a claim that 77 previously broken features or all 296 published files
are now fully implemented. Per-source selections, original hashes, upstream
links, exact expected rows and missing-native exclusions are recorded in
release26-services-ledger-2026-11-06.json.

## Functional scope

SQL36 merges quest/ordinary loot, skinning, pickpocketing and spell-crafting loot;
11 vendors and profession trainer data. It covers Pandaria serpent eggs/herbs,
Stranglethorn and Redridge quest drops, Gnomeregan spare parts, Barrens creature
loot, source creatures' loot/skin/pickpocket links, Darkmoon souvenir items and
Lien Farner's published trainer list. All incoming items exist and none carries
the local RequiredLevel>=100 quarantine marker. All486 referenced spell IDs and
both Darkmoon item-cost IDs exist in installed DBC/DB2/server data. Vendor flags
were already present on all 11 target NPCs.

SQL37 aligns selected registered class-script bindings, proc masks, bonus
coefficients, linked cleanup and Skull Bash teaching data. This includes the
published rogue, druid, warrior and death-knight subsets whose handlers exist
locally. Missing native implementations and research/probe-only mass binding
files are excluded explicitly. Source-token/registration checks do not establish
identical implementations; the native adaptations below resolve four actual
backend differences found before activation.

## Preservation and fork adaptations

- Merge only incoming primary/unique keys. Preserve unmentioned local vendor
  items, loot rows, custom variants, trainer spells and proc/bonus/link entries.
  This does not preserve the old value of a key deliberately corrected by a patch;
  those values are recoverable from the backup.
- Fill omitted lootmode with regular mode1. This schema defaults it to an empty
  SET mask; copying a sparse upstream INSERT literally would create disabled drops.
- Keep expected row defaults explicit for complete proc/bonus rules and vendor/
  trainer/loot rows. Preserve non-client custom spell bindings when aligning
  obsolete retail bindings. The binding snapshot is pinned in the ledger so the
  assembler does not regenerate different cleanup predicates after deployment.
- The published Zanzil loot uses five root reference groups absent locally.
  Extract those groups and their transitive dependencies ONLY: 15 groups/540 rows.
  Put them in unused IDs source + 6000000 and rewrite reference edges; do not
  replace any existing local reference group. All 524 real dependency items exist
  locally and none is quarantined. Reference cycles are rejected.
- The public reference group 24161/group1 totals 110%. Scale its five positive
  weights proportionally to 100%, retaining the published relative distribution.
  Do not silently keep an overfull group or manufacture replacement items.
- Gnomeregan source loot template 30020 has no owner in this fork. Map its item 57764
  to actual chest templates 203443/203964/203965/203966/203967/203968 instead.
  SQL39 removes the newly introduced orphan under exact guards, repairs their
  six disabled mode0 rows and supplies active quest-required drops. These real
  chest templates have 70 retained spawns; no chest or quest completion is invented.

Reference source is the publicly published release asset
[SFDB26.002 archive](https://github.com/ProjectSkyfire/SkyFire_548/releases/download/sf_db_26/SFDB_full_548_26.002_2026_008_18_Release.zip),
21,625,833 bytes, SHA256
`d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f`.
The GitHub API's published sha256 digest matched. Only scoped reference INSERTs
are extracted from its121MB SQL; no base world/auth/character import is performed.

## Native ports paired with SQL37

Pinned source revision 505f6e63a20c3ea75dfebb17f58e891f4e5fb44c; complete source URLs
and file hashes are in the ledger. Existing public-project attribution is retained.

- Last Stand: use the published MoP AuraScript health-amount calculation on 12975.
  The old dummy handler required missing 12976 and could not validate. Adapt the
  callback to this fork's float amount API; do not invent spell 12976 or unbind it
  merely to clear the warning.
- Runic Corruption: replace the old AuraScript with the published spender-hit
  SpellScript and register it accordingly. Spell47632/49143/56815 are damage
  spells, not auras. Talent gating, 45% proc and regen refresh/extension follow the
  published implementation. Remove only the obsolete retail 51462 binding.
- Skull Bash: port published dependency validation and charge→interrupt→mana
  effect order; adapt the local misspelled INTERRUPT constant identifier while
  preserving its correct 93985 spell ID. All three triggered spells exist.
- Shattering Throw: use the published before-hit removal of immunity mechanics,
  preserving the Ice Block glyph exception and adding the null-target guard.
  The old on-hit callback ran too late for an immune target.

No common navigation header, Detour code, configuration verbosity, account,
mail, inventory, wallet or BattlePay catalog migration is included.

## Verification and deployment

Initial disposable fixture: 1,562 expected data rows, 1,333 initial mismatches;
all expectations, unrelated-row/field preservation and repeat stability pass.
It caught the 110% reference group before runtime writes. Existing unrelated
invalid loot groups are recorded separately; none is worsened or newly introduced.
The final alignment adds explicit owner/drop/mask/orphan checks. These checks do
not constitute natural quest turn-ins, loot statistics, or a complete class-spell
combat playthrough.

Final Visual Studio incremental build:3 succeeded,0 failed,30.570 seconds.
The first compile exposed this fork's float AuraEffect callback type; corrected
before deployment. All 19 native CTests pass. The original 835-plus4 prior data
contracts also remain passing. The first runtime deployment passed 2,401 combined
checks and exposed the Gnomeregan orphan, which SQL39 repairs functionally.
Final contract/startup evidence follows below.

Worldserver remained available during preparation except for a normal stop while
readiness was checked; the old build was immediately reopened when native porting
became necessary. Final artifacts were staged only after the successful build.
The pre-batch world tables are backed up privately:
`out/release26-services-before-20261106-050017.sql`,44,951,812 bytes,
SHA256 `eff94a1f3ccc06f3ce3eb19b45840b28874c0b2f7ed18c5d294134fb34987322`.
It covers all14 mutated tables. A read-only gameobject_template fixture copy is
only context for chest ownership; that table is not changed by this batch.
Previous EXE/PDB/logs: out/runtime-before-release26-services-20261106-050039.
No raw backups, client assets, credentials or crash dumps are committed.

Apply the reviewed local migrations in this order:38 reference dependencies,
36 services/loot,37 class data,39 spare-parts alignment. Restart with the matching
new EXE/PDB; native SQL37 bindings require the paired native ports. Do not apply
the entire raw upstream world archive or replace personal/custom databases.

For rollback: stop worldserver normally, preserve a fresh backup of later edits,
restore the14 covered tables from the recorded pre-batch dump, and restore the
old EXE/PDB together. Whole-table restoration also reverts later edits in those
tables; inspect/select a narrower subset of the backup when appropriate. A Git
revert alone cannot restore database state.

## Reproduction and remaining work

```powershell
python contrib/upstream-tests/assemble_release26_services_batch.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --patches out/skyfire-quest-research/release26-sql --baseline doc/release26-services-ledger-2026-11-06.json --report doc/release26-services-ledger-2026-11-06.json
python contrib/upstream-tests/extract_release26_reference_dependencies.py --package out/skyfire-quest-research/SFDB_full_548_26.002_2026_008_18_Release.zip --ledger doc/release26-services-ledger-2026-11-06.json
python contrib/upstream-tests/test_release26_services_batch.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --report out/release26-services-fixture.json
```

The fixture runs the checked-in trusted SQL as administrator against disposable
world-table copies, checks exact mutation scopes and repeats, and removes its
schema in finally. It is not an untrusted-SQL security sandbox. Keep the original
pinned binding baseline when reproducing cleanup; the assembler reads current
schema defaults but does not choose obsolete bindings from the post-update DB.

Natural quest/drop/vendor/training and class-combat acceptance remains open.
Continue excluded native handlers, complex starter quest/event/phase patches,
known issue reports and sanitized release/long-load work. Do not mark all published
SkyFire content or all remaining startup faults fixed from this batch.


## Final deployment observation

The final visible worldserver is PID 39852, started 05:09:06 local; auth 61228 is
unchanged. Final 1571/1571 services/alignment checks and all 839 prior contracts
pass: 2410/2410 combined. The final fixture verifies field/row preservation and
repeat stability after the alignment. It starts from the earlier upgraded state;
the original 1333-mismatch baseline evidence is retained separately under
out/release26-services-fixture-before-alignment.json.

The spare-parts pre-alignment rows were also preserved in
out/release26-spare-parts-before-20261106-050903.sql, 396 bytes, SHA256
`8d75f2c8ddfbcd75bf4a56fa62334087844157ae8d0a323437cb73c51badfde1`.
Fresh runtime errors: 1425 records versus 1448 before this batch, with zero new exact
error signatures. The orphaned loot-template error and class hook-validation
errors are absent. Fresh Server.log contains zero MMap warnings. Existing
unrelated startup/content errors remain recorded; no logging filter changed.

| Artifact | SHA256 |
| --- | --- |
| worldserver.exe | 1d78ca3727acc462484097cf71e8e8bbf82bf7b194a7a72e0d59df703eb5ed38 |
| worldserver.pdb | 47ffa66c23b3c92161bf1daddc1a9ba6647eaa5264933e6c4361bf934f61f1de |

Built/exported artifacts match. The export manifest pins the native pair, four
reviewed migrations and this report. Migration regeneration against the pinned
binding baseline and reference package reproduced all three assembled SQL files
byte-for-byte. Across the quest and services batches, 147 distinct archived files
have contributed reviewed portions; that count is not complete gameplay coverage.

Before applying SQL38 to a different database, verify that its reference IDs are
unused or already contain this exact previously applied batch. Stop on collisions;
the current test deployment's unused namespace was explicitly checked before
installation. A portable installer preflight and sanitized clean-install checks
remain part of the release checklist.
