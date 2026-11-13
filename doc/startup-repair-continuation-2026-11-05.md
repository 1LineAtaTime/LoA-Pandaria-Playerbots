# Startup repair continuation — November 5, 2026

This continues the 07:33 batch report; it is not an error-free release. Full
file-category diagnostics remain enabled. A clean console is not proof of a
clean world database. Forgejo origin remains authoritative, GitHub its private
push mirror; this work stays on codex/city-companions pending validation.

## Native changes and evidence

- Follow movement no longer copies a zero/NaN/infinite stop-spline velocity.
  It uses the master's valid configured speed. Invalid movement still fails
  validation, with added actual velocity, motion type, speed rates, root/stun
  state and slow-aura context. Pet44109 and Iron Sentinel29984 were observed
  before this change; a short clean observation does not prove both fixed.
- Emily's12027 escort cannot be restarted by a second quest acceptance while
  already escorting. The existing active player's escort remains intact.
- Random item enchantments use normalized positive finite weights. The old
  second-pass integer division could produce an empty draw; pools above100
  also favored early entries. New distribution/edge-case tests cover both.
  The legitimate no-enchantment return0 no longer triggers a second false
  missing-enchantment0 error. Actual absent pools still produce diagnostics.
- Five missing native spell handlers restored: glyph125044 grants detection
  56814; glyph57855 grants woodland charm127757; glyph125045 grants Holy
  Nova132157; glyph126746 grants Fetch125050; item50243 has its faction-
  specific language proc (50242/50246). Glyphs grant dependent spells only and
  preserve independently learned abilities when removed. Geist58640 uses its
  existing real handler, validated against121916, with owner-safe removal.
  Installed DBC effect types and child spell IDs were checked. Gameplay
  acceptance of these glyph/language effects remains pending.
- Legacy SmartAI action154 now sets validated UnitMoveType speed multipliers
  in tenths, matching the original authored format. Nine active rows previously
  failed loading, including Forsaken Bat and Redhound transit flight speeds.
  Unknown actions remain rejected. This is not a general city-flight fix;
  cyclic/smooth legacy action153 remains unimplemented.
- A newly observed taxi failure on path2323 was a normal completed arrival:
  persisted point69 of70, map0, character0.951 yards from the final DBC point.
  ContinueTaxiFlight now consumes a verified completed leg before resuming
  the next, or performs normal final cleanup. Exact final index, matching map
  and finite distance<=60 are required. Other out-of-range/faraway/cross-map
  invalid resumes retain errors and cleanup. Regression cases include the
  observed arrival and rejected earlier/later indexes, wrong map, distance,
  NaN, infinity and empty paths. No taxi state was mass-cleared in this pass.

Reference behavior was reviewed rather than copied wholesale. Several older
warrior handlers duplicate current5.4.8 native effects and were not imported.
No inactive quest pack was enabled merely to satisfy missing script names.
No Detour source changed; its prior crash/soak requirements remain open.

## Applied data repairs

Each batch was tested against disposable fixtures and repeated for idempotence,
then applied with worldserver stopped after full affected-table backups. Tests
also preserved existing rows and selected partial/custom definitions.

| SQL suffix | Repair |
| --- | --- |
|11|51 missing LANG messages from paired SkyFire symbolic/numeric definitions; existing messages/locales win.|
|12|98 exact combat/linked TALK rows keep their owner as speaker while targeting victim/invoker; authored text groups verified.|
|13|64 original LoA gameobject loot rows in37 wholly absent groups; absent/quarantined item groups excluded and partial custom groups preserved.|
|14|Nine exact legacy actions: two combat-stop aliases and seven forward moves converted to current relative offsets.|
|15|Four literal no-script `0` sentinels normalized; one malformed Howling Gale binding removed after proving its callback empty and native NPC behavior intact.|
|16|17 exact creature/object difficulty masks repaired against installed MapDifficulty plus native scenario overrides; all identities/positions retained.|
|17|19 unusable respawn links to disabled DB copies removed; all creature spawns retained, including Hadronox's script-spawned encounter actors.|
|18|40 sourced points in five wholly missing quest POI groups; current objective identity verified and paired legacy/modern indexes synchronized.|
|19|78 valid property definitions in missing pools456/460/461; type-specific and partial custom overrides preserved.|
|20|Geist old binding replaced with its actual native handler; obsolete IC parachute duplicate retired only while the implemented IC handler remains bound.|
|23|Two obsolete duplicate class aliases retired only while current Dark Transformation63560 and Stealth1784 handlers remain bound and implement the same mechanics.|
|22|19 suffix-only IDs removed from five property pools (458/462/463/464/465); remaining rows and weights match paired SkyFire exactly. Per-pool fingerprint preserves changed custom definitions.|
|21|Five equipment sets moved from reserved ID0 to1 only when absent; two exact ID0 duplicates retired while valid equivalents remain; Flamekin Rager9779's sourced0–30 HP range restored.|

Seven equipment definitions were validated against Item.db2 build18273: all
items exist and have hand-equippable inventory types. NPC spawn equipment0
continues to mean unarmed; no spawn equipment choice was changed.

SQL11–23 are now applied. SQL20–22 were deployed at13:19 and SQL23 at13:23,
with matching native glyph builds. Final hashes and fresh-run counts follow.
No authentication/password, wallet/balance, shop catalog, mail or personal
character data was changed by these migrations.

## Private backup and test evidence

- out/startup-batch2-before-20261105-123727.sql (8,325,589 bytes):
  trinity_string, smart_scripts, gameobject_loot_template, conditions,
  spell_script_names before SQL11–15.
- out/startup-batch3-before-20261105-125829.sql (54,942,657 bytes):
  creature, gameobject, linked_respawn, quest_poi_points,
  item_enchantment_template before SQL16–19.
- Fixture scripts/results live in ignored out/test_runtime_batch2.py,
  test_invalid_script_sentinels.py, test_runtime_batch3.py,
  test_glyph_parachute_bindings.py and runtime-batch*-fixtures-20261105.json.
  SQL21 fixture verified five renumbers, two exact duplicate removals, one
  HP-range change, stable repeats and preservation of a custom ID1 override.
- The native suite expanded from15 to16 CTests with actual weighted enchant
  distribution tests. Final test/build/deployment result appears below.
- Private runtime backups retain matching EXE/PDB pairs and previous logs.
  Do not ship these backups, live credential-bearing configurations or logs.

## Remaining defects and validation

The12:58 session, before the final glyph/speed/taxi build, had1,499 deduplicated
WARN/ERROR/FATAL messages at13:05, including967 missing native names. This is an
improvement over the earlier1,594, but still substantial unfinished content work.
Do not erase bindings or disable diagnostic categories to achieve zero.

Known open categories include missing native quest/encounter/class scripts,
51 real missing speaker/text requests,37 unsupported source-type3 event rows,
unused loot profiles, absent spellclick/vehicle definitions, remaining POI and
condition/gossip/criteria incompatibilities. Invalid cyclic/smooth path actions
must gain correct runtime support before they can be enabled. Some source-type3
summons duplicate native event_scripts, while others include50% chance; a blind
conversion would change mechanics. Yorik's wrong target GUIDs must not receive
Turkey Hunter dialogue merely because the GUID points at a turkey today.

An Infested Bear47204 spawn175727 requests waypoint movement with no path/addon;
the original LoA data has the same defect. No imaginary route was substituted.
Outdoor SmartAI entries1706/1538/20075 also have legacy difficulty/charm flags
that need source/context reconciliation, not indiscriminate flag clearing.
Iron Sentinel29984 speed failures need observation with the new diagnostic.

Keep the Pandaren Jaomin Ro29409 duel, Worgen starting-chain audit, bot city
spacing, instance/teleport companionship, group support behavior, flight and
sanitized Windows/Linux packaging on the existing checklist. None is proven
complete by successful startup. Accounts and private settings must never be
included in a shareable release.

## Primary references

- [MoP spell implementation reference, pinned577c19c](https://github.com/FALL1N1/WoWCircle-5.4.7/tree/577c19c5ec9c6d8b5b88b3b926670dbe3cccd1e9/src/server/scripts/Spells)
- [Original legacy SmartAI speed semantics, pinned1c1f6b4](https://github.com/planee/Cataclysm-Original-Core/blob/1c1f6b4a6e0478fdf753fa5625bbeed7875963f2/src/server/game/AI/SmartScripts/SmartScript.cpp#L2538)
- Original LoA v1.0.4 and paired SkyFire26.002 database snapshots retained in
  ignored out/ for exact-row provenance. SkyFire LANG names additionally checked
  against official header505f6e63a20c3ea75dfebb17f58e891f4e5fb44c.


## Final deployment and validation —13:25 local

Visual Studio RelWithDebInfo x64 worldserver build completed with zero failed
projects. The first taxi patch used a standard-container `back()` unavailable
on this core's Path class; it was corrected to indexed access, rebuilt, and the
final native build passed. All16 CTests and the diagnostics Python test pass.
The executable includes native changes through a1836255 (its embedded revision
has a dirty marker from documentation edits during compilation); SQL23 is a
separate tested data-only commit. It is not an unbuilt native change.

Worldserver PID59372 started at13:24:16 via the visible export launcher, finished
world initialization in20 seconds, and listens on8085. All198 random bots are
online. Authserver PID61228 remains unchanged and listens on3724. No new crash
was observed in this short validation; this is not an extended stability soak.

Fresh full-category report at13:25:23 contains1,494 diagnostic messages, including
958 missing native names. They map to1,163 runtime binding references; see
[static script inventory](missing-native-script-inventory-2026-11-05.csv).
This count includes later command-table diagnostics after bot login, so compare
like observation windows rather than the shorter13:05 snapshot. Remaining
errors are recorded honestly, not hidden by changing severity/categories.
No new glyph validation, reserved equipment0, legacy speed-action154, invalid
Flamekin HP range or restored property-pool errors appeared in this run.
No taxi, invalid-speed, spline or missing-MMap warning appeared in this short
sample; flight/soak acceptance is still pending. The previous run also exposed
a missing graveyard zone on map571, which remains open with late stale command
metadata and the other content errors above.

| Artifact | SHA-256 |
| --- | --- |
|worldserver.exe|`c8dc9d8731ddf9bf2c558d5a4c091c066260a5e4c504e6c897ab420142b258c6`|
|worldserver.pdb|`b1c763c0a07b698ac300269e6b9a008f4b59e2657b4c9a593688d66aa88072a8`|

Matching built/export hashes verified. Prior runtime/log backup:
`<local-workspace-path>`.

Additional private database backups:
- `<local-workspace-path>` (7,311,627 bytes), before SQL20–22.
- `<local-workspace-path>` (208,434 bytes), before SQL23.

Source/SQL/report commit d81a30ae was pushed to Forgejo; both repositories are
private and all9 branch/tag refs match on GitHub. Administrative mirror settings
remain inaccessible to the scoped token, so only privacy/ref equality is claimed. Main/default promotion remains
pending broader content and gameplay validation.
