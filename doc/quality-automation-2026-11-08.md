# Repeatable quality automation

The source entry point is `contrib/windows/Run-QualityChecks.ps1`. It combines
private rollback checkpoints, unit/native checks, dependency comparisons,
structural audits, repair prioritization, live client contracts and diagnostic
comparison. It never declares all content validated.

## One-command use

From the repository root:

```powershell
./contrib/windows/Run-QualityChecks.ps1 -Profile targeted
./contrib/windows/Run-QualityChecks.ps1 -Profile full
```

`targeted` selects client suites from changed paths. `full` runs all registered
suites and native CTests. Existing native binaries are reused when native source
has not changed; Python/audit/documentation edits do not force a C++ rebuild.
Native changes trigger one Visual Studio/MSBuild build through configured CMake.
If rebuilt worldserver differs from the running export, live QA stops until that
build is staged/restarted with a matching runtime manifest. This gate prevents
tests against an older executable being credited to new native source. Native
deployment remains a separate rollout step using the visible export launchers.

Use `-AuditOnly` for checks without creating accounts. Use `-Build` to force a
single build. `-RunDirectory out/...` chooses a NEW evidence directory; existing
directories are refused. CMake/CTest paths come from the configured build cache.
An OS file lock serializes automation runs and releases automatically on process
exit. The currently exercised live cleanup path is Windows; Linux live cleanup
still needs its own transport instead of the Windows console helper.

Each run writes private evidence under ignored `out/qa-automation/runs/...`:

- `run.json` and `README.md`: profile, steps, timing, outcomes and remaining debt.
- `checkpoint/`: runtime/source checkpoint and optional scoped database backups.
- `dependencies/`, `graph/`, `dependency-comparison.json`: complete dependency run.
- `data-audits.json`, `addon-auras.json`: reference checks and explicit known debt.
- `prioritized-repairs.json`: shared causes ordered for starter/progression work.
- `diagnostic-snapshots.json`: bounded warning/error snapshots and comparison.
- Per-suite client reports, action traces, cleanup evidence and harness hashes.

Successful exit means the selected regression contract passed. `content_ready`
and `all_content_validated` remain false. Missing mechanics remain untested.
Known structural problems stay visible, not reclassified as working content.

## Regression checks

The checked-in `known-data-findings.json` records exact, reviewed pre-automation
violations. Replacing one failure with a different failure cannot hide behind an
unchanged count. `required-gameplay.json` protects previously required scenarios:
removing one, returning no result, duplicating results or changing a pass into
untested fails the run. Neither baseline is automatically refreshed.

Dependency outputs must pass their hashes and row-count checks. A failed generator
cannot fall back to an older report. The pipeline also fingerprints tools and
expectation files; changing them during a run invalidates its reproducibility.
Evidence writes retry bounded Windows sharing violations and remain atomic;
persistent write failure is never treated as a pass.

Warnings/errors are parsed into ranked categories and occurrence counts. Volatile
GUIDs are grouped while spell, creature, quest, path and database error IDs remain
distinct. SQL values are redacted from diagnostic snapshots. Multiple appenders
do not double-count the same diagnostic. New signatures require review; recurring
signatures remain visible with additional occurrences. Counts are snapshots, not
time-normalized rates; log rotation can reduce counts. An absent line is not proof
of a repair. The tool does not change logging thresholds.

## Checkpoints and database rollback

Runtime EXE/PDB/DLL/configuration files are hash-verified and retained in a private
content-addressed cache. Identical artifacts are reused, avoiding repeated large
PDB copies. Git revision, working-tree patch and selected untracked source files
are also preserved. The user's untracked reference executables are excluded.
Private configuration backups contain credentials; they stay under ignored out/.
Never attach these private directories to a public release or issue.

Before a data batch, supply an affected-row backup plan:

```powershell
./contrib/windows/Run-QualityChecks.ps1 -Profile full -BackupPlan contrib/qa-automation/backup-example.json
```

Example plan:

```json
[
  {"database":"World","table":"quest_template","key":"ID","ids":[24959,25152]},
  {"database":"World","table":"quest_template_addon","key":"ID","ids":[24959,25152]}
]
```

The example protects those specific quest rows; adapt the plan to the actual
change. Identifiers and numeric selectors are validated and checked against real
metadata. SQL is not accepted as a selector. Snapshots include complete columns,
checksums and scoped restore scripts. The restore deletes only the selected key
range before inserting its snapshot, so a row absent at backup is absent again
after rollback. Dependency ordering and foreign-key constraints must be reviewed
for the actual batch. Restore SQL is prepared, never executed automatically.

For major changes, a plan entry `{"database":"World","full":true}` requests a
full schema/data/routine/event dump using configured privileges. Character and
Login are also supported. Full backups require sufficient database privileges.
Each entry is a separate single-transaction dump; the checkpoint is not an atomic
snapshot across tables/schemas/process memory. Nontransactional tables require
appropriate maintenance handling. Playerbot/log schema backup support is not yet
part of this plan format. Changed migration files require a backup plan before
the pipeline proceeds; reviewers must still verify the plan covers every change.

The rollback verifier uses a uniquely owned disposable InnoDB table:

```powershell
python contrib/qa-automation/verify_checkpoint_restore.py --run-dir out/qa-automation/my-restore-proof
```

It checks restoration of an original row, removal of a newly inserted scoped row,
and preservation of an unselected row, then removes the fixture. It does not prove
every production schema/foreign-key/trigger rollback. Runtime rollback still uses
`server shutdown 1`, waits for exit, restores matched EXE/PDB and runtime metadata,
then opens the visible launcher. Source rollback preserves Git history.

## Reusable gameplay workflows

`contrib/headless-client/qa_actions.py` adds a validated `workflow` scenario mode.
Use `qa-workflow-scenarios.json` as the starting example. Steps compose these
bounded actions:

| Action | Behavior |
|---|---|
| approach | Use packet-observed positions or a bounded static route; require client visibility. |
| gossip/select | Choose the correct quest-only/gossip packet, observe a fresh response, select one unambiguous text option. |
| accept/turn_in | Send normal quest packets; rewards need explicit independent expectations. |
| kill | Follow live positions, filter dead targets using live health, require server objective credit. |
| use_item/use_object/loot | Use owned inventory locations or selected objects and decoded loot windows. |
| cast | Send the normal spell packet; contract postconditions must establish the intended effect. |
| checkpoint_character | Save/re-enter normally so new inventory is available for persisted item lookup. |
| wait_progress/escort | Wait for objective credit; optionally follow the observed actor within bounded distance/time. |
| wait_visible | Observe visibility; disappearance requires a prior sighting and a complete decoder. |

Actions use no GM quest completion. Every workflow needs persisted quest/item
postconditions; sending a packet alone is not a mechanics pass. Rewarded workflows
must provide exact XP/money expectations. One quest is currently modelled per
workflow. Inventory obtained after the previous save may need an explicit
checkpoint before item lookup. The 5.4.8 health/NPC-flag layout comes from local
UpdateFields.h. Decoder errors cannot establish an NPC's disappearance.

Live action coverage is reported separately from implemented commands. The first
contracts cover two delivery quests and human combat objectives. Generic collect,
item-use, spell, escort and phase actions need authored positive gameplay contracts
before those mechanics are claimed verified. Vehicles, complete boss mechanics,
BattlePay transactions and companion/AH settlement retain their existing coverage
gaps. The older starter suite keeps its broad gap records.

## Crash/interruption recovery

Account/character ownership IDs are journalled immediately after creation, without
passwords. If a client process returns without a report, the pipeline attempts
recovery only from confirmed journals and still fails the incomplete run.

```powershell
python contrib/qa-automation/recover_accounts.py --journal-dir out/your-run/workflows --world-pid 12345
```

Verify the child test process has exited before manual recovery. The tool checks
the exact account ID/name and character identity. It refuses changed identities,
unexpected characters or an unconfirmed planned account. It never sweeps a name
prefix. A crash before ownership confirmation requires investigation. The native
cleanup verifier checks one empty throwaway account and repeat-safe recovery;
unit checks cover refusals. That does not simulate every possible process crash.

## Repair prioritization

Spawned shared dependencies in reviewed racial starter zones come first, followed
by structural missing definitions, other spawned dependencies and unspawned/
possibly scripted/custom actors. Shared quest counts help order work within tiers.
This avoids unspawned custom daily content outranking a level-one progression
blocker just because it links many quests.

Availability remains unknown until a reviewed source establishes active, retired,
test or custom status. `availability.json` requires an ID, source and reason for
explicit classifications. FLAG quests can be essential progression state and
must not be bulk-retired. No content is removed by prioritization.

## Validation

Initial integration failures are retained in private run directories. They exposed
a Windows file-sharing race, the different quest-only NPC opcode, and asynchronous
save timing. Those tester defects were corrected and covered by targeted checks.
Scoped SQL restore and verified-account recovery passed live fixture checks.
Final consolidated-run results are recorded in the completion evidence; this
document does not substitute for actual per-run reports.

### Recorded completion evidence

Implementation source: f2688160ce. The final full run passed 92 Python checks and
22 native CTests. Live cases:22 passed,1 failed; all 23 disposable accounts and
created characters were cleaned. The failed human combat contract obtained 5/6
server kill credits within 180 seconds. The pipeline correctly returned failure;
this is an unresolved bounded-route/combat scenario, not a certified server quest
defect or a clean full-run signoff. The earlier one-credit combat contract passed.
Two new delivery workflows passed with exact persisted rewards and no visibility
decoding errors. Generic item/escort/phase contracts remain unverified.

The consolidated run took 452.6 seconds and performed no native rebuild because
native source was unchanged. It retained 20 runtime artifacts by content hash and
backed up two rows each from quest_template and quest_template_addon. Its diagnostic
comparison found no new signatures. The actual SQL restore fixture and repeat-safe
account recovery fixture both passed; no fixture table/account was retained.

[Machine-readable results](quality-automation-results-2026-11-08.json) preserve
counts and report hashes without publishing private dumps/configurations/logs.
The runtime remains the existing 068fac87 native build; these are tooling changes.
