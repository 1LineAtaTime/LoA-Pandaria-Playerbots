# Gameplay integration and single-server QA — November 10, 2026

## Deployed batch

Native source `94dfb31e1d` merges all **45** LoA beta commits from
`68f80984313dc15a6144d4e91c325632810a488d` through
`ef5112acc3`, with **41 paired world SQL files**. The merge was conflict-free.
The batch includes Arathi quest events, quest prerequisites/text, duplicate-spawn
removal, courier and Valorcall patrols, Eversong quest behavior, SmartAI counter
reset validation, SoloCraft XP handling, and bot quest-POI matching by objective ID.
Our BattlePay, auction, companion, MariaDB and earlier quest repairs are retained.

Public source: [LoA batch comparison](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/compare/68f80984313dc15a6144d4e91c325632810a488d...ef5112acc3).

Also adapted [SkyFire cast-failure packet correction](https://github.com/ProjectSkyfire/SkyFire_548/commit/c2c598bbe31362853a33efc8e573f5ad0e4068be):
optional arguments and presence bits now follow the client's argument order,
including the pet opcode. Adapted the Northshire worg engagement part of
[SkyFire b53c84a8](https://github.com/ProjectSkyfire/SkyFire_548/commit/b53c84a8e4df867e6114b8b00ca75ac228d5d97a):
retry the search consistently and use the actual nearby infantry range instead
of the old one-yard/home-position restriction.

SkyFire's newer unsigned/64-bit text-field fix is already equivalent in this
fork's `src/server/database/Database/Field.cpp` (strtoul/strtoull/strtoll).
It was not redundantly reimplemented. The generic idle-creature engagement
portion of b53c84a8 and the large Darkshore code/data patch53e1b75bc1 remain
OPEN for API/data adaptation; this is not a claim that every SF patch is installed.
Current inspected SF tip is4bd9295f2b. Do not bulk-replace the customized world.

## Deployment and rollback

Visual Studio MCP built the existing solution in RelWithDebInfo/x64 with zero
failed projects. Three pre-existing Windows revision-macro compiler warnings
remain; no compile errors were reported. The Stage-Runtime script resolved
non-system DLL imports. Matching world/auth EXEs and PDBs, DLLs and default
config templates are deployed in `exports`. Local connection configurations,
regular accounts/passwords, characters, shop and balances are preserved.
The two new SoloCraft config defaults were added without changing XP rates.

Exactly one visible regular server pair runs on3724/8085, with mop_loa_* schemas.
No observer, second realm, game client or watching interface was started.
The exports manifest records native/source identity and artifact hashes.

Private rollback material:

- `out/backups/gameplay-batch-20261110`: prior EXEs/PDBs/DLLs/local configs/manifest.
- `out/quest-qa/published-ef5112acc3/played/world-before.sql`: complete525,270,267-byte
  world dump, SHA25669e3c57db739c8324399b22e94c6314eded5b487720efc67a0725f673e6be3eb.
- Adjacent migration.json pins every input SQL hash,24 affected tables, applied
  files and post-apply table checksums. All41 applied; full replay checksum-stable.

The first dump failed on routine visibility for the restricted server DB user;
its partial file was retained. No SQL ran from that failed attempt. The complete
backup used the previously authorized database administrator credential.
To roll back, stop world gracefully, restore the complete pre-change world dump
with the database client and restore the paired export backup before restarting.
Do not mix an old executable with the newer paired world data. Account/character
schemas were not copied or replaced for this migration.

## Verification and automation

- All24 native CTests pass after the final build; includes database integer
  widths, BattlePay accounting, auction filter, companion policy and quest checks.
-33 headless QA/action tests and3 diagnostic-collector tests pass.
-7 read-only deployment-reference assertions pass: patrols, Anvilward attack
  target, soil binding, cave-floor height, Kovork duplicate and linked POIs.
- Baseline before deployment:15 executed cases pass,8 categories explicitly
  untested; cleanup verified. Four data-audit categories remain findings.
- Final expanded gameplay report and exact outcomes are recorded below after
  the deployed run completes. Data assertions are not gameplay certification.

`Run-QA.cmd` now also includes the existing authored live-combat workflows and
invalid-quest acceptance regressions on the same regular pair. It retains
per-case evidence, performs native cleanup of its temporary accounts, writes a
concise README and repair-queue.json, and exits. Unsupported categories remain
untested and prevent a complete/pass result, even if all executed cases pass.

Runtime warnings/errors are collected into runtime-diagnostics.json without
altering logger levels or deleting logs. Repeated messages retain occurrence
counts; copies across appenders are deduplicated. Oversized log sampling is
explicitly marked partial. Existing live diagnostics are not hidden or treated
as repaired merely because a smoke test passes.

## Outstanding scope

The post-restart diagnostic snapshot had1,181 unique exact warning/error
messages versus1,211 in the previous run's Errors file. These are not counts of
independent gameplay bugs. Dynamic taxi IDs/state differ; an instance-flag
SmartScript message also appeared. This is not a zero-error startup or a soak
certificate. Missing native bindings and content/condition mismatches remain.

The four reference-audit categories include2 missing creature references,
7 items,1 gameobject, and a Crowley SmartAI/native ownership overlap. The latter
must be classified against the existing native script, not 'fixed' by replacing
Crowley's AI blindly. Missing post-MoP IDs require version reconciliation,
not fabricated templates or automatic quest-credit grants.

Natural whole-chain progression, companion terrain/clipping, all professions,
bosses/raids, shop client UI flows and comprehensive content remain open. The
single-pair runner covers authored scenarios only. No invulnerability, objective
credit grant or general autonomous-bot pass is used to claim whole-game coverage.

## Expanded deployed-run outcome

Run `out/qa-reports/20261110T221150595979Z` completed with20 passed,1 failed,
8 untested gameplay entries;6 passed/4 failed structural audit categories.
All21 temporary accounts/characters were verified absent after native cleanup.
All15 previously passing baseline cases still pass. The added Orc/Undead authored
workflows and all3 malformed-quest acceptance cases also pass.

The added Human combat case earned3/6 server kill credits, survived using normal
out-of-combat regeneration, then exhausted the180-second budget. Its failure
is retained. It is not evidence that no kills or quest credit work.
The harness now follows moving targets in3.5-yard observed steps and records
per-attempt progress. The six-kill scenario allows a bounded600seconds to include
ordinary level1 recovery; it still requires all6 real server credits and the
persisted complete quest state. Non-combat waits remain capped180seconds.
A focused rerun is `out/qa-reports/20261110T222034451156Z`.
The boundary test was updated from181 to601 for combat, and a separate assertion
retains the180-second wait limit;34 headless tests now pass. A private py-spy
stack capture confirmed ordinary recovery waiting, not an infinite worker loop;
the profiler is installed only under ignored out/qa-tools, not a new service.

## Focused combat and cleanup follow-up

Focused rerun20261110T222034451156Z earned all6 real kills and persisted quest28766
with status1 (complete), without credit or power grants. Overall workflow still
failed at logout; do not relabel that original report as passed. The original
harness did not retain the exact logout refusal code, so combat/falling/other
refusal cannot be conclusively distinguished from that report.

The first cleanup check expired before the server's60-second disconnect grace
period. After the native session became offline, exact account/character ownership
was rechecked, the completed quest state was recorded, and native cleanup verified
both identities/dependencies absent. Evidence is cleanup-recovery.json beside that
run. No real player account was kicked, reset or removed.

The harness now retains the exact logout result, retries only explicit result1
(combat) for a bounded45seconds, leaves falling/duel/acknowledgement failures
visible, and waits up to90seconds for a failed client's native session to clear
before cleanup.37 headless tests and3 diagnostic tests pass. This hardens the
runner; the modified live-combat logout path is not yet a new passing full
workflow. A shorter ordinary Orc workflow rerun validates the helper's normal
logout/cleanup path. Broader quest and runtime diagnostic work remains open.

## Final bounded helper acceptance

`out/qa-reports/20261110T222912024034Z` passes the ordinary Orc delivery workflow,
including logout/re-entry/cleanup through the new helper. A separate one-kill
Human workflow in `out/qa-reports/combat-logout-retry-20261110` also passes real
kill credit, persisted active quest, logout/re-entry and automatic cleanup. It
uses the ordinary starter, one real kill and no state grants; it does not claim
six-kill completion. Both overall reports remain NEEDS_REVIEW/nonzero because
the same4 structural-audit categories and runtime diagnostics remain unresolved.

The six-kill focused report retains its original logout failure and linked
successful cleanup recovery. A full six-kill rerun with the final helper remains
open; six-kill credit plus persisted completion was established independently.
No test accounts from these runs remain. Regular auth41556/world59040 stay
visible on3724/8085. No client settings/credentials changed in this batch.
