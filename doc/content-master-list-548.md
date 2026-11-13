# 5.4.8 content master list and comparison

Next layer: the [quest dependency audit](quest-dependency-audit-548/README.md)
connects prerequisites, objectives, givers/turn-ins and rewards, including server
spell overrides. Its findings are attributed per quest and grouped for repair;
neither catalogue presence nor graph consistency is gameplay certification.

Start with the [catalogue index](content-catalogue-548/README.md). It links 36 CSV
lists covering 414,598 rows, including overlapping views of the same definitions.
These are not 414,598 distinct playable features. Files are UTF-8 CSVs usable in
Excel or scripts. They contain public world/client definitions only; no accounts,
characters, passwords, balances or player mail were exported.

## What counts as the reference

There is no single independently verified, complete 18414 world specification
established by this review. This catalogue combines several sources and labels
what each can tell us:

1. Local client DBC/DB2 data: IDs, names, encounter journal and supporting records.
   The project targets 5.4.8 build 18414, matching the upstream
   [supported client version](https://github.com/ProjectSkyfire/SkyFire_548/blob/main/README.md).
2. The original [LoA v1.0.4 database release](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases/tag/v1.0.4),
   commit e135742e1891d3e653d27966da2dc70e793f203e, as a community peer reference.
   Its world.sql archive member SHA-256 was verified identical to the local
   reference file. A community database can itself be wrong or incomplete.
3. The current mop_loa_world definitions and current startup registration errors.
4. Explicitly scoped existing gameplay QA results for the five starter quests.

For file layouts, the generator follows the core's DBCfmt/DB2fmt and loader, plus
[WoWDBDefs](https://github.com/wowdev/WoWDBDefs). The 5.4.8 layouts for
[JournalEncounter](https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/JournalEncounter.dbd),
[JournalEncounterCreature](https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/JournalEncounterCreature.dbd),
[JournalInstance](https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/JournalInstance.dbd) and
[QuestV2](https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/QuestV2.dbd)
are explicitly documented there. Modern retail or MoP Classic tables are not
silently substituted for original 5.4.8 data.

## Important build provenance

All 58 local WDB2 headers record 18273, below the configured 18414 target. DBC headers contain no
build identifier. Unchanged client tables may legitimately retain an earlier
header, so this does not prove that the assets are wrong. It also does not prove
that every file includes all 18414 changes or hotfixes. Verify a pristine client
extraction/hotfix provenance before treating this as the final 18414 reference.

The [453-file provenance manifest](content-catalogue-548/client-table-provenance.json)
records hashes and header information: 428 headers parsed and 25 zero-length files
marked for review. Some zero-length files are optional/internal or superseded
variants; they are not automatically broken required assets. This is a file
snapshot, not a dump of final in-memory stores after database overrides/hotfixes.

## Main comparisons

| Content | Catalogue scope | What the comparison says |
|---|---|---|
| Quests | 18,161 IDs from client, LoA reference and current server | 18,145 server definitions; 16 reference/client-only IDs need availability review. 117 server-only IDs also need classification. Titles for some client-only IDs are unknown. |
| Items | 88,306 IDs including server additions | 79,923 present on both sides; 8,282 client base IDs lack server templates; 101 server-only additions. |
| Item details | 72,187 Item-sparse IDs | All 72,187 have server templates. The 8,282 unmatched base IDs have no sparse-detail rows here; do not call them 8,282 missing usable items. |
| Creature templates | 57,638 peer/current IDs | 57,634 current templates; 4 peer-only IDs. Includes helpers, phase variants, retired/test NPCs and custom additions. |
| Gameobject templates | 41,373 peer/current IDs | 7,006 peer-only definitions, many placeholders. No currently placed gameobject refers to a missing template. This says nothing about unplaced objects needed by a quest or summon. |
| Spells | 94,842 client records | Names and native-binding diagnostics; most spells do not require a custom C++ handler. Effects/cooldowns/procs are not certified by this list. |
| Dungeon encounters | 699 client records | 487 have server credit mappings; 212 need review. These are encounter/difficulty records, not 699 unique bosses. Some credit is script-controlled. |
| Encounter journal | 83 instances,430 encounters,507 actor records |A useful client-facing boss index, including historical content. It is not all possible outdoor encounters. |
| Server boss candidates | 1,425 templates |Union of world-boss rank, instance-binding flag, encounter-credit references and boss_* script names. Includes candidates/helpers; not a verified unique boss list. |

Other lists cover maps/areas, achievements/criteria, battlegrounds/arenas,
scenarios/steps, classes/races/specs/talents, factions/currencies, professions and
recipe candidates, archaeology, taxi nodes/routes, mount-item links and battle
pet species. Supporting server tables cover vendors/trainers, loot, gossip,
conditions, transports, events, instance definitions, BattlePay and related data.
For those supporting systems the export is a table/count inventory, not every
individual loot/vendor row or a verified retail drop/stock specification.

Playable versus test/retired/seasonal/class-restricted content still needs explicit
classification. For example, the 26 race rows include non-playable races, and the
37 specializations include pet specializations. Mount-item links exclude mounts
learned without an item; profession ability candidates are not automatically
obtainable recipes. None of those raw counts is a playable-completeness percentage.

## Reading a row

The paired lists keep reference_name and server_name separately and flag label
differences. Differences may be cosmetic, localization, custom content or genuine
bad data; no name or ID was automatically changed by this comparison.

- present_in_both: the ID has definitions on both sides, not proof it works.
- reference_only_review: investigate build, availability, prerequisites and actual
  references before importing anything.
- server_only_review: may be intentional BattlePay tokens/custom content or an
  incompatible record. Do not automatically delete it.
- not_evaluated: no gameplay acceptance is claimed.
- tested_delivery_scenario / tested_acceptance_only: narrow existing QA coverage,
  not every branch, class, faction or difficulty.

A native binding marked reported_unregistered comes from the current startup
log. no_native_binding does not mean broken: default/data-driven AI may suffice.
not_flagged_in_log is not an exhaustive positive proof of script correctness.
Template bindings also do not summarize every per-spawn override or triggered scene.

## Boss identity traps

JournalEncounter IDs, DungeonEncounter IDs, creature template IDs and creature
display IDs are different namespaces. Never join them just because the numbers
match. Journal actor display IDs are explicitly not NPC entry IDs.

Journal actor names are candidate mappings only. Two reviewed aliases are retained
as candidates: Dragha Shadowburner -> Drahga Shadowburner and Durumu -> Durumu the
Forgotten. There are multiple phase/template candidates for Durumu; neither name
difference proves a missing boss or selects a uniquely correct actor.

## Concrete current reference problems

The [reference integrity findings](content-catalogue-548/reference_integrity_findings.csv)
and [summary](content-catalogue-548/reference-integrity-summary.json) identify:

- Seven item objectives with no server item template. Six have only a base client
  item row and no sparse details; 113135 is absent from both local item tables.
- Two creature-objective targets and one gameobject-objective target missing their
  templates. These are review targets, not instructions to fabricate dummy actors.
- Zero placed creature and zero placed gameobject references lacking templates.

Separate from this catalogue, the [native-handler inventory](remaining-native-script-bindings-2026-11-08.json)
and [QA framework](automated-gameplay-qa.md) track unavailable custom code and
actual behavior. World spawn definitions, phase visibility, obtainable loot,
quest chain logic, boss mechanics, PvP rules, professions, pet battles and geometry/
navmesh completeness all need additional validation. ID/name existence cannot
certify those systems.

## Regenerating and validation

Read-only generation from the repository root:

```powershell
python contrib/content-audit/test_catalogue.py
python contrib/content-audit/catalogue.py --qa-report out/legendary-native-20261108/starter-qa/qa-report.json
```

Use --config, --client, --data, --reference-sql and --output to select inputs.
The default comparison is pinned to the local v1.0.4 world.sql hash, not an
unrecorded latest download. summary.json records source/revision hashes and
category limits. catalogue-files.json records normalized output hashes. The
client-table manifest includes all 453 table files, even ones not decoded into
individual catalogue rows. Metadata reads are independent snapshots; regenerate
if another migration changes definitions concurrently.

Ten parser/safety tests cover WDBC/WDB2 indexing, string bounds, truncation,
duplicate IDs, SQL reference parsing and refusal of database mutations. Generated
CSV row counts and hashes are checked before committing. No game/database change,
compile or restart is part of generating the catalogue.

## Using this to drive repairs

```mermaid
flowchart LR
  A[Client IDs and pinned peer data] --> B[Catalogue differences]
  B --> C[Build and availability review]
  C --> D[Independent expected outcomes]
  D --> E[Normal-client scenarios]
  E --> F[Repair and regression results]
```

Prioritize active quest-chain references and spawned NPCs with missing handlers;
then cover instances and professions. Keep uncertain/retired/test rows in review
until their intended 5.4.8 availability is established. This avoids inflating the
fix count by adding unused placeholders or declaring missing behavior solved
because a database row exists.
