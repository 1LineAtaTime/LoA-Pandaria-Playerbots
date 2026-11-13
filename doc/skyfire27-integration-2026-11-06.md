# SkyFire27 component integration — 2026-11-06

## Source and method

Pinned official source: https://github.com/ProjectSkyfire/SkyFire_548/tree/df19329b209fa1d1ae80444b43f6201d5dc124f0 .
Database release: sf_db_27 (27.001), release tag f7a9880b9452e4db900dc9ff783b7b6be3aeeb86;
its release source is d3ce4e58ea929b45bfeb3b0332b0933b419ab810.

The existing LoA fork and official SF tree have no common Git ancestor. This is
therefore a component integration into the existing fork, not a claimed automatic
merge of unrelated histories. The official `skyfire` remote is fetch-only with a
disabled push URL. Work is isolated on codex/skyfire-integration; Forgejo remains
authoritative and GitHub its private push mirror. Existing custom source/history
is retained. The machine-readable manifest pins revisions and compatibility
policy. A separate native tree inventory records 2,283 differing paths; this is a
divergence map, not 2,283 alleged defects or independent patches. The inventory records all 102 archived world updates and seven current
world updates with hashes, descriptions, component groups and disposition.

## Integrated components

SQL40 adapts the three coupled August 17 combat sources: 380 class/level stat rows
and 6,312 matching NPC templates. The original upstream DROP/CREATE/DELETE of the
stat table is replaced with keyed upserts, preserving 20 extra local level 96–100
rows. Template changes are limited to mindmg/maxdmg/attackpower and guarded by
exact NPC name. Nine differently named identities are recorded for review rather
than silently repurposed. Upstream03 and 06 resolve in source order.

Native efc81bafc restores loading those existing template columns and uses authored
world melee ranges/AP. Invalid/missing ranges fall back to class/level data.
Battleground and AutoBalance expansion-aware scaling and ranged behavior retain
this fork's curve. **Per-template damage multipliers are not imported:** this
fork's runtime/event multiplier and rank policy remain in place pending a matched
reconciliation. This is a declared partial port of the combat component, not an
assertion of identical combat balance to official SF. No health/mana modifier,
AI/script binding, faction, loot, navigation or bot behavior is replaced.

Single-template reload also now includes entry at column 0, matching the bulk
loader. It previously omitted entry and shifted every read; both paths append
the same three melee fields at 87–89.

SQL41 adapts seven published service/gate sources: Nyoma/Gorbold vendor options,
stable-master flags, innkeeper bind/shop flags, Lakka's current quest gate,
monk eligibility for the affected Argent quests and Plan B's outfit conditions.
The legacy quest RequiredClasses column maps to active
quest_template_addon.AllowableClasses. Existing already-correct rows stay as-is;
this database needs 58 stable-master flag repairs and three condition repairs.
No account, character, password, mail, BattlePay catalogue/DP or AH rows change.
The database version table is not relabeled as a complete27.001 installation;
only the listed component migrations are applied.

## Checks and remaining acceptance

Combat disposable-table fixture: 6,692 desired-value checks, every unrelated
creature field preserved, all untargeted creatures and20 extra stat rows preserved,
repeat application stable. All 6,312 intended templates and 380 stat rows differ
from the starting database. The comparison uses keyed row hashes; no live world tables were modified by
fixture trials.

Services fixture: 12 numeric updates modeled independently against full row
multisets; unrelated fields/rows preserved and repeat application stable.
64 resulting service/gate checks cover final effective rows. Zero-match guards
are documented in the fixture; they are not described as newly repaired content.

Native build, complete CTest results, stopped-server backups, deployed artifact
hashes, final live data contracts and fresh startup comparison are recorded below
when deployment completes. These checks do not replace client/gameplay acceptance
or a sustained bot/crash soak.

## Remaining integration

The CSV's pending entries are real remaining review/adaptation work, not a claim
that every entry is broken. Larger dungeon datasets require matching scripts,
spawn identities/phase handling and loot references. Class datasets require their
matching registered handlers and local API adaptations. Detection-range schema
and native changes are coupled. Newer update files remain separately visible.
Older release26 compatibility ledgers also remain authoritative; do not erase
local fixes by importing a whole official world database.

The next milestone should take those matched modules from the same pinned tree,
then move the pin forward and review its diff. This integration does not certify a
fully merged SF server or a sanitized Linux/Windows distributable.


## Repeating the upstream comparison

A fresh checkout can add the official source as a read-only comparison remote:

```powershell
git remote add skyfire https://github.com/ProjectSkyfire/SkyFire_548.git
git config remote.skyfire.pushurl DISABLED
git fetch --filter=blob:none --no-tags skyfire main
```

Use the manifest's pinned revisions when reproducing this batch. A later
`skyfire/main` is a new comparison target, not permission to replace the current
working tree or database. Export the three combat inputs from the pinned tree,
provide an entry/name TSV from the destination world, then run
contrib/upstream-tests/assemble_release27_combat.py. Its output and paired ledger
show any identity exclusions. The fixture tools consume a local config/client
path; credentials stay in process memory/environment and are not written to
reports. Run fixtures before any stopped-server migration and preserve the actual
database dump outside Git.


## Deployment evidence

Visual Studio completed the full affected core/bot/script build on November 6 at
13:51 local: seven projects built, zero failed, 27m39.650s. All 20 CTests passed;
both worldserver config/argument tests were rerun successfully against the final
linked executable. Native source is efc81bafc; the src tree is
`5ffc4cc0fe4dced6c3a04579c5d744be571f3e9c`. Later commits only add data/tooling/docs.

The old server completed normal bot logout and database-pool shutdown at 13:53:15.
It exceeded the initial 55-second wait, so the migration guard correctly refused
to run until process exit was observed; there was no forced kill or Ctrl+C batch
prompt. Allow more than 55 seconds for this bot population on future restarts.

Affected-table backup (world only; private, outside Git):

- `out\skyfire27-before-20261106-135406.sql`
- 20,249,983 bytes; SHA256 `6f524ac55eeaf0cf7eb2c8a83d67c1990f53b0e1ddd3a0d2d1425d072b00947e`
- Tables: creature_template, creature_classlevelstats, gossip_menu_option, conditions, quest_template_addon

Both SQL 40/41 batches applied offline. Live combined contracts: **9166/9166 passed**, including all 2,410 prior checks.

Old native pair and logs: `<local-workspace-path>`.

| Artifact | Bytes | SHA256 |
| --- | ---: | --- |
| worldserver.exe | 70,960,128 | `6823c43cdcbc194d6708bf3ca4897eba8f33545e1675c6963fb3157e06cf67f0` |
| worldserver.pdb | 663,408,640 | `a2e49db66f9c325f27f906fedbb129f35cb33c1ba84a89f5755271659b05aecf` |

Both exported hashes equal their build outputs. Worldserver was launched visibly
through Start-Worldserver.cmd at 13:54:43 (PID 62812). Authserver PID 61228 was not
changed or restarted. No logger/configuration suppression was added.


Fresh world initialization completed at 13:55:15 in 29 seconds. Ports 8085/world
and 3724/auth are listening on the expected processes. The actual CLI
`reload creature_template 6` executed and logged the correct entry, exercising
the repaired prepared-query/loader layout without a new diagnostic.

The initial fresh startup contained 1,423 existing DB/content error records;
the later observation contained 1,447 including recurring known records. The pre-restart
long-running file contained 1,503, including recurring runtime records, so that
count difference is not claimed as 80 newly repaired defects. Both the database
and server log comparisons contain **zero new exact warning/error signatures**.
The fresh Server.log contains zero MMap warnings. Errors.txt also receives the
existing DB/content channel; it is not a runtime-only error counter. Those
existing errors remain open and visible. This milestone does not claim an
error-free server, complete quest/spell gameplay or a long stability soak.


The export manifest now contains 562 members and every hash matches; untouched
members were checked against their preceding hashes before updating known
artifacts. The package includes SQL 40/41, the source/value ledgers, pinned
upstream manifest/inventory, the new native test and this report. Actual configs,
private backups and runtime personal data are not uploaded to Git. The runtime
exports directory still contains local settings and is not a sanitized public
release. Forgejo/GitHub feature mirroring was verified with 10 matching refs;
validated main was then fast-forwarded through Forgejo to bf246b71a.
The read-only mirror check verified both repositories private, both default
branches main, and all 10 refs identical. This documentation follow-up changes no
runtime source or data; native source/artifact provenance remains as above.
