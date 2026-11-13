# Full historical patch audit — November 9, 2026

The earlier latest-batch audit was insufficient to answer whether **all** older
SkyFire repairs are integrated. A concrete older gap was found and repaired.
Do not describe this customized LoA server as containing every SF patch.

## Exhaustive source listing, bounded implementation evidence

`inventory_full_history.py` lists every SQL file and every `src/server` C++/header
file at the refreshed SF and LoA pins. It does not limit the scan to current
updates, release27, selected file names or previously adapted modules.

- SF pin `6f000f1878c547f04680cd7c269388a245132ff2`:6,544 SQL files and1,433
  C++/headers. The SQL includes1,433 archived5.4.8 files as well as older-version
  history, schema files and pending/current updates.
- LoA pin `68f80984313dc15a6144d4e91c325632810a488d`:998 SQL files and1,763
  C++/headers.
- Total10,738 file entries, each with pinned path,Git blob,LF-normalized SHA256,
  exact matching local paths and a deliberately non-deployment evidence label.
- Release25→26 alone:296 SQL files,3,478 statements. Only89 statements have an
  exact canonical-text match in tracked local world-update SQL at source5ae1a5d0ee.
  The other3,389 require baseline/adaptation/supersession reconciliation.
  **They are not3,389 proven missing fixes**: transformed SQL and already-populated
  base data will not text-match. Previous quest/service subset ledgers remain
  linked to all296 inputs in the private generated review manifest.

Tracked evidence:

- [All file entries](full-history-source-inventory-2026-11-09.csv)
- [Counts and pins](full-history-source-summary-2026-11-09.json)
- [All release26 statements, keyed by source and ordinal](release26-statement-coverage-2026-11-09.csv)

Reproduce from refreshed remote refs:

```powershell
python contrib/upstream-tests/inventory_full_history.py --output out/full-history-review
python contrib/upstream-tests/test_inventory_full_history.py
```

This uses committed Git objects, not uncommitted files. Four evidence/parser
tests pass. No DB access occurs in the inventory. Exact file presence does not
establish compilation,registration,deployment or gameplay. A same-path difference
can be an intentional custom change. An absent path can have an equivalent local
implementation. The source summary retains the pre-follow-up checkpoint pin so
it is reproducible instead of silently moving to a later checkout.

## Confirmed absent historical correction deployed

SF `2026_07_20_world_06.sql` corrects StranglethornVale teleport956 from height
10.1021 to18.2021. The SF27 full database retains18.2021 and the later SF27/current
SQL has no game_tele override. Both active worlds retained10.1021 with the exact
same ID,name,map,x,y and orientation. This is an actual missing historical data
correction, rather than a failed text-match inference.

Deployed `sql/updates/world/2026_11_09_01_world_historical_stranglethorn_tele.sql`
to `mop_loa_qa_world` and `mop_loa_world`. The migration guards the old destination
identity and coordinates, preserving renamed/renumbered/custom destinations.
Backed up each whole game_tele table first; verified18.2021 after apply and stable
whole-table checksums after a second apply. Both running world consoles executed
`reload game_tele` at11:33:44; their Server.log records the reload. No executable
or DLL change,server restart or diagnostic suppression was needed. This is a
data/reference check,not a visual in-game teleport acceptance test.

Private backups,hashes and completed manifests:
`out/full-history-20261109/deployment/{qa,played}.json` and adjacent table dumps.
Rollback restores the corresponding pre-change game_tele table and reloads it;
no auth,character,shop or bot tables were touched.

## Remaining gaps and next batches

The archived `2026_07_22_world_03.sql` binds four Westfall Transient entries
42383/42384/42386/42391 to npc_transient. All four active ScriptName fields are
empty and the current scripts have no npc_transient implementation. This is a
matched code/data integration candidate; installing only the binding would add
a broken registration. Its GUID-based roaming edits also need spawn-identity
reconciliation before application. Keep it OPEN.

The all-history inventory is a complete source listing,**not completed semantic
reconciliation of all history**. Remaining work includes mapping older base-dump
content,statement adaptations and superseding changes; matching missing native
implementations; and pairing them with this fork's schema and custom systems.
The earlier18,766-contract audit and111 release27 dispositions cover useful
subsets but cannot close these tasks. Bulk replaying the entire historical SQL
tree would re-run old schema transitions and obsolete data,including pre-MoP
updates; it is not equivalent to integrating every surviving repair.

Public correction source:
[SF historical teleport patch](https://github.com/ProjectSkyfire/SkyFire_548/blob/6f000f1878c547f04680cd7c269388a245132ff2/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_06.sql).
