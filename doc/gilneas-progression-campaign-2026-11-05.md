# Gilneas progression and ride safety campaign — November 5, 2026

The user authorized large repair batches on this existing test deployment.
Nineteen pinned historical migrations were reviewed and assembled as SQL30,
then reconciled with the actual engine/backend in SQL31. Apply the pair together.
Crowley's incomplete boarding bridge is deferred explicitly; this is not a claim
that nineteen new quests have passed natural playthroughs. Native source is
recorded through77777246, with the ride/catapult implementation in fce8533f.

## Source and compatibility repairs

All nineteen source paths and canonical-LF hashes are pinned against b2c6b144 in
contrib/upstream-tests/batches/gilneas-20261105.json. Original migrations retain
their attribution and research. The assembler reproduces SQL30 and rejects
changed source hashes. Independent raw fixtures reported fourteen changes, four
SQL failures and one no-change result; SQL success alone was not accepted as a
gameplay test. Coverage includes Two by Sea, Duskhaven sparring, Sylvanas/Tobias,
Koroth's banner, Losing Your Tail, drowning watchmen/crash survivors, Hewell's
ride, Battle for Gilneas, Endgame departure, invasion camera and gunship deck.

- Populate both creature_text actor identities because the legacy primary key
  uses entry while the loader reads CreatureID. Preserve authored dialogue and
  test text hashes; reject conflicting nonzero ownership before deployment.
- Make only waypoints.orientation nullable/defaultNULL, matching the loader's
  optional facing and authored new76-point route. Preserve every old value and
  the original15-point38765 route. No navigation files are renamed or replaced.
- The first real engine load exposed four backend diagnostics that the original
  historical migrations did not account for. SQL31 fixes Lorna's missing C2
  backend with the reviewed quest-accept SmartAI, using Krennan's existing group1
  handoff text. Remove only obsolete survivor source0 SmartAI after its complete
  native handler owns rescue/combat. This does not remove unrelated timed lists.
- Crowley's old native handler does not exist here. Remove only the newly
  introduced boarding bridge/condition under that exact backend guard, preserving
  the pre-existing binding and timed list. Do not activate an incomplete ride,
  clear a missing binding just to silence it, or fabricate quest completion.
  Its complete33-point jumps/seats/ejection/credit sequence remains open.

## Native safety repairs

- Stout Mountain Horse now owns a coordinate snapshot instead of retaining
  pointers into a reloadable waypoint manager. Reject null/nonfinite/gapped paths
  and safely release passengers; remove recursive searching across missing IDs.
- Catapults require an explicit finite destination and valid velocities before
  jumping or granting the auxiliary no-fall aura. Delayed callbacks recheck the
  vehicle kit before passenger access. Valid downward trajectories are retained.
- A reserved-bot-only read-only worldheight scenario queries actual normal and
  phased Map heights, with coordinate/proximity/teleport guards. The console
  runner retains its existing event mode and requests return to saved position.
- Correct the invalid-speed diagnostic's floating-point slowAura format. The
  prior integer specifier caused a formatting exception and hid useful values.
  This repairs observability; the actual upstream invalid-speed producer remains
  a separate investigation. Invalid spline input is still rejected.

No Detour, MMap/VMap files, logging severities or diagnostic filters were changed.
The original navigation crash and map571 investigation remain separate open work.

## Verification and runtime

- Visual Studio worldserver build: five projects succeeded, zero failed.
- All19 native CTests and five Python SQL/assembly tests pass. Path tests include
  access after original pointers are deleted, gap/finite checks and trajectory
  boundaries. No full gameplay success is inferred from these tests.
- Final paired fixture passes85 data/text contracts, full-row preservation outside
  declared per-statement key scopes, repeat-application stability and full table/
  schema restoration from the actual pre-SQL30 backup. The restored orientation
  column is NOT NULL. All disposable schemas are removed. This uses trusted
  checked-in SQL as administrator, not an untrusted-SQL privilege sandbox.
- Live data:442/442 pass (85 Gilneas,306 training,34 chain links,17 prior ports).
  Original pre-SQL30 baseline was14/78, with64 mismatches and no SQL errors.
- Real reserved-bot Map height: normal -62.854923 versus quest phase4194304
  floor105.347 at the gunship furnace. The model8253 WMO/.vmo and finite bounds
  were checked. This establishes a real collision surface, not complete pathing.
- Actual event26277 created14 NPCs within the expected1..25 bound. The reserved
  actor was temporarily moved; return was requested, not exact actor state reset.
  These two live tests ran on the same ride/catapult source before the final
  diagnostic-format-only rebuild. Neither forced quest completion.
- Visible worldserver51224 started20:20:24 and initialized in21 seconds, listening
  on8085. Auth61228 remains listening on3724. The fresh boot has no new exact
  error lines against the pre-batch baseline, and all four introduced migration
  diagnostics are absent. Missing native names drop953→952; existing speaker
  diagnostics remain55. Total errors1426 versus1465 are observation counts with
  different runtime exposure, not39 independent fixes. Real errors remain visible.

Final exported artifacts (identical to Visual Studio output):

- worldserver.exe SHA256: `ef920c027327fb725733385efa7bf60f0b60b255251e0254a239c62997ce16c7`
- worldserver.pdb SHA256: `24b11b86f2db129edfce670e663d349f21c6336f5c16e83ad9dcfb27b4b231ff`

Private evidence: out/gilneas-final-pair-recovery.json,
out/gilneas-final-*.json, out/gilneas-floor-live.json,
out/gilneas-cannon-regression.json and out/gilneas-final-log-comparison.json.
No private actor/account dumps, logs or configuration are distributed.

## Data footprint and recovery

Final pair's full-row footprint (old/replaced → new/replaced); every other row
was retained. Counts describe changed data, not independent quest repairs.

| Table | Rows |
| --- | ---: |
| conditions | 3 → 6 |
| creature | 57 → 57 |
| creature_addon | 0 → 0 |
| creature_template | 10 → 10 |
| creature_text | 11 → 20 |
| gameobject | 3 → 7 |
| gameobject_template | 1 → 2 |
| gossip_menu_option | 0 → 0 |
| quest_template_addon | 1 → 1 |
| smart_scripts | 66 → 53 |
| spell_script_names | 0 → 1 |
| vehicle_template_accessory | 1 → 1 |
| waypoints | 33 → 109 |

The world was stopped normally before each live migration. Complete pre-SQL30
thirteen-table backup: out/gilneas-progression-before-20261105-195748.sql,
89,028,194 bytes; SHA256
`4f6d8bd4bbcbabcef4f9f8e5b30b6be48f6d13673b1b164c9338e902045dcd03`.
The actual dump was imported into disposable schemas, upgraded with both files,
reapplied and restored with identical full-row data and original schema.

Supplemental pre-SQL31 three-table backup: out/gilneas-alignment-before-20261105-201938.sql, 24,239,892 bytes;
SHA256 `9a6e3c8a571b3ee0279c3a5d443572a795b6206dd556434d657566473ba6d955`. This preserves the intermediate state, including the four
introduced diagnostics; use the original thirteen-table dump for a full batch
reversal. Reserved actor backup and old paired EXE/PDB are retained privately.

For recovery, stop worldserver normally and preserve any newer work first.
Restore all thirteen original tables from the original dump using the configured
MariaDB client; restore the matching old EXE/PDB if reversing the native source.
Do not treat a Git revert as a database rollback, import a full foreign world DB,
or put manual restoration SQL in the automatic-update directory. Actual restore
rehearsal is repeatable with test_gilneas_migration.py --backup <original-dump>.
No runtime rollback was performed.

## Completion and remaining work

- [x] Source review, guarded adaptations, Visual Studio build and matching exports.
- [x] Offline backed-up SQL30/31 deployment,442 data checks and tested restoration.
- [x] Live floor/event mechanism checks, fresh diagnostic comparison and reports.
- [ ] Complete Crowley's missing native ride and natural quest acceptance matrix.
- [ ] Natural Worgen progression: boarding/exit, rescue credit, ambush escape,
  escort retry, group/battle ownership, gunship combat/escape and relog.
- [ ] Remaining native/SmartAI/text/loot/condition/vehicle errors,21889 semantics,
  map571/navigation, invalid-speed producer, companion/city/flight behavior and
  sanitized Windows/Linux packaging. This is not an error-free release claim.


Follow-up status: native4f904145 now implements the Crowley questgiver and
owned33-point ride with backed-up SQL32. Its natural client acceptance remains
open. The deferred-bridge discussion above records SQL30/31's historical scope;
see stability-quest-continuation-2026-11-05.md for the implemented follow-up and
vertical-knockback-repair-2026-11-06.md for the current deployed native artifacts.
