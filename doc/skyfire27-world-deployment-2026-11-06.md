# SF27 quest, loot and travel deployment — 2026-11-06

Pinned upstream: df19329b209fa1d1ae80444b43f6201d5dc124f0, official
ProjectSkyfire/SkyFire_548. SQL 42 adapts 75 statements from 30 archived source
files; per-file hashes, selected scope and exclusions are in the paired ledger.
The integration inventory is updated without marking unimported native modules
complete.

The batch restores quest dialogue/progression and race masks, keyed quest turn-in
links, Silty Oozeling/Timberling harvesting and Coldridge pouch loot, Scarlet
Halls/Monastery seasonal LFG coordinates, Dragonmaw loot ownership and published
Blood Furnace targeting flags. Legacy quest fields map to active split quest
text/addon tables. The unused lfg_entrances table is not created: the active
lfg_dungeon_template loader gets the coordinates. Broad source loot/quest-link
deletes become keyed upserts/additions, preserving unrelated custom rows.

All affected quest IDs exist locally, and all 22 authored turn-in actors/objects
have local spawns. Existing turn-in links already match; those are not counted
as new repairs. Preserve the explicit local Skull Bash trainer entry200011 /
spell106839 until the corresponding specialization-learning component is
reconciled; the published general cleanup would otherwise undo that native port.
No objective/AI/spawn/dungeon handler is replaced without its matching native
component. The code and EXE/PDB are unchanged from native efc81bafc; a data-only
migration does not require a new compile.

The user confirmed the rogue Eviscerate practice quest works in game. Remaining
Bloodfang crowding/evade/ordinary-combat acceptance stays separately tracked.
No account, character, password, DP balance, shop catalogue or auction rows are
changed. This does not claim complete SF integration, an error-free world,
full gameplay acceptance or a sanitized distributable.


Disposable full-table fixture passes all 145 new and 9166 retained contracts.
It resolves 77 initial desired-value mismatches and changes 80 rows in nine of
the twelve protected world tables. Untargeted rows/fields remain unchanged,
repeat application is stable and no overfull loot group is introduced/worsened.
The selected trainer removal is already otherwise satisfied locally; zero trainer
rows change after preserving Skull Bash. Gameplay acceptance remains pending.


## Deployment evidence

The server stopped normally with bot logout and database-pool closure; no forced
kill or Ctrl+C batch prompt. The private stopped-world backup is `out\skyfire27-world-before-20261106-141820.sql`,
62,846,394 bytes; SHA256 `9e1fb8c884c338fb2547dccd628fb512f54248a7298821d435868a8e2901e221`.
All twelve affected tables are included; no personal schemas are copied.

SQL 42 is applied. All **9311/9311 live data contracts pass**,
including the existing 9166. The native EXE/PDB still match the hashes in the
preceding SF27 integration report. Previous runtime logs and that pair are at
`<local-workspace-path>`. Worldserver was relaunched visibly with its normal export launcher;
authserver remains running.


## Dragonmaw compatibility correction

The first fresh boot exposed three orphan-owner warnings after SQL 42's published
shared-loot redirects. The private pre-migration backup proves the local owners
were 41072/41080/42107, with 163/143/114 existing loot rows respectively; donor
2102/2103 have only 32/31 rows. SQL 43 restores the three verified original owners
only when they still match SQL 42's donor values. All original loot rows and
the source donor tables are preserved. This fixes actual ownership instead of
deleting drops or filtering the warning. Apply SQL 42 then 43 as a matched pair.

A three-actor disposable fixture confirms the owners, every unrelated field and
repeat stability. Three explicit ownership/data-presence contracts are added
to the suite (148 new, 9314 combined). The reusable full fixture includes both
migrations for future installations. The original 145-check full-table fixture
and 9166 retained checks remain recorded; the correction was tested separately.


Final SQL 43 pre-correction table backup: `out\skyfire27-ownership-before-20261106-142226.sql`,
16,819,837 bytes, SHA256 `9e0a6f4fa050d74ff74afcbf769672a951246e8929455ff5bc7d1805fb2a76a9`.
Final live result: **9314/9314 contracts pass**. Worldserver PID 35500 was launched
visibly at 14:22:41; authserver PID 61228 remains unchanged. Both services listen on
8085/3724. The fresh startup has zero new exact warning/error signatures, zero
Dragonmaw orphan-owner records and zero MMap warnings. Existing unrelated
content diagnostics remain visible; logger/configuration levels are unchanged.
The corrected pair has 77 net changed rows from the pre-SQL42 baseline; the three
loot-owner redirects are restored to their verified original values.


The export package has 566 verified manifest members, including both migrations,
the paired source ledger, refreshed inventory and this report. Unchanged members
were compared with their preceding hashes before updating known files. Private
backups/logs/configurations stay outside Git. The runtime package remains a local
test deployment with personal settings; public sanitization remains pending.
