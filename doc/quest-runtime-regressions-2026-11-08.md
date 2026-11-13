# Shared escort repair and regression gates â€” November 8, 2026

## Native repairs

The legacy escort death guard required both ESCORTING and JUMPING simultaneously.
These are alternative route states. A walking/jumping escort that died therefore
usually left its owner's quest incomplete instead of failed. Accept either active
route state; keep existing owner/quest checks and incomplete-only party handling.
The correction is corroborated in the public LoA fork at
[pinned source](https://github.com/quietpatcher/Legends-of-Azeroth-Pandaria-5.4.8/blob/c9b628a0cfff97172ab800511dd1ebc780371dad/src/server/game/AI/ScriptedAI/ScriptedEscortAI.cpp).

Also guard MovementInform against a past-the-end waypoint iterator after a late
or duplicate final-point notification. Valid out-of-order waypoints continue to
produce their existing diagnostic; no warning/error threshold was changed.

These are shared framework repairs, not restoration of the 246 missing-definition/
actor-handler repair groups. Native escort death/party failure and duplicate final
waypoint behavior still need direct in-world acceptance. Compilation and existing
regressions alone do not prove a complete escort journey.

### Aura loader repair discovered during live diagnostics

Both creature addon loaders allocated an aura vector for every input token, then
advanced an 8-bit write index only for valid spells. Rejected/empty tokens left
zero-filled holes which Creature::LoadCreaturesAddon later treated as spell 0.
Leading/repeated spaces in the legacy tokenizer also became empty tokens and
were diagnosed as spell 0. Template 45576 has ` 29266`: the valid 29266 aura must stay.

Both loaders now ignore empty formatting tokens, clear/reserve the destination
and append only validated spell IDs. This also removes the 8-bit index wrap risk
for large lists (none currently have 256 entries). Genuine bad IDs and other
validation errors still log. No database aura was removed or logging suppressed.

The new independent read-only addon_auras.py auditor scans 6,350 populated template
rows and 12,472 populated spawn-addon rows against client plus spell_dbc IDs.
Seven rows have spacing differences. One real missing reference remains:
creature_template_addon 36805 -> spell 58284. It requires source/version review;
this patch does not fabricate a spell or claim the aura works. Six new auditor
tests pass, bringing Python coverage to 51 tests.

## Additional error checking

Preferred single command:

```powershell
./contrib/windows/Run-QuestDependencyChecks.ps1
```

This creates a unique evidence directory and fails immediately if generation
fails, so an old report cannot accidentally be treated as a new pass. Existing
run directories are refused and never deleted/overwritten. The lower-level
commands below are useful when deliberately retaining named run artifacts.

`contrib/content-audit/compare_quest_audits.py` validates tracked CSV fingerprints
and row counts before comparing individual findings. New finding identities and
removed quest definitions fail the check, even when counts are unchanged. Missing
old findings are labelled absent, not automatically fixed: deleting a binding or
rotating a diagnostic log can change the evidence. No gameplay pass is inferred.

```powershell
python contrib/content-audit/quest_dependencies.py --output out/quest-repairs-20261108/dependencies --artifacts out/quest-repairs-20261108/graph
python contrib/content-audit/compare_quest_audits.py doc/quest-dependency-audit-548 out/quest-repairs-20261108/dependencies --report out/quest-repairs-20261108/dependency-regressions.json
```

New `invalid_accept` client scenarios send missing quest IDs 0, 999999999 and
4294967295 through the real accept opcode on disposable ordinary accounts. They
require no saved quest/reward and successful logout, character re-entry and second
logout. The pre-existing null-template guard must survive all three requests.
These exercise error handling, not quest progression, and do not add quest passes
to the coverage inventory. The gameplay DB reader also preserves trailing empty
columns, matching the catalogue-reader correction.

```powershell
./contrib/windows/Run-GameplayQA.ps1 -Suite contrib/headless-client/qa-invalid-quest-scenarios.json -ReportDirectory out/quest-repairs-20261108/invalid-requests
```

## Source selection

Fetched the community fork into read-only remote-tracking ref quietpatcher/custom,
pinned c9b628a0cfff97172ab800511dd1ebc780371dad. No merge or reset was performed.
Its published bot walkers/player-path tests are candidates for future selective
ports. The fork's QuestHandler still dereferences quest before checking null and
its escort/SmartAI diffs would replace local summon/evade fixes. Therefore a whole
tree replacement would regress verified repairs. BattlePay, AH, companions,
Detour, database connectors and production configuration were not replaced.

## Validation and deployment

- Visual Studio RelWithDebInfo x64: zero failed projects.
- Native CTest:22/22 pass.
- Python:51/51 pass (11 catalogue,12 graph,7 comparator,15 gameplay-framework,6 aura-auditor).
- Pre-deployment dependency comparison: zero new findings or removed quests.
- Live gameplay and final artifact evidence: recorded in the completion update.

Before the aura-loader follow-up, all 20 supported live cases passed (15 starter,
3 invalid requests,2 targeted gossip-item cases), with all 20 temporary accounts
and their characters removed. Eight broad mechanics remain untested; all three
known structural audit categories still fail. The wrapper's nonzero exit is
therefore retained rather than presenting the run as globally clean.

A 1213 deadlock was observed on an aura-effect save during the invalid-request
run. The transaction layer has bounded deadlock retry and that case completed,
but save lock-order/contention remains a diagnostic investigation item. Changed
taxi GUID/path messages are also retained. No blanket zero-error claim is made.

No world/schema migrations are included in this batch. Disposable QA account and
character mutations are logged and cleaned by the existing campaign; final cleanup
must be verified before calling the run complete.

Prior executable/PDB, configurations and runtime manifests are backed up under
ignored `out/quest-repairs-20261108/runtime-before`; diagnostics under logs-before.
To roll back: issue `server shutdown 1`, wait for worldserver to exit, restore the
previous worldserver.exe/worldserver.pdb and saved ACTIVE runtime manifests, then
open exports/Start-Worldserver.cmd. Auth need not restart if unchanged. No SQL
restore is required. Source rollback is a scoped Git revert, retaining history.

## Final completion evidence

Deployed world native source 068fac87ab2f146c1c831b78d5a3ea2678ec8409, including
escort fixes from 8d0b6e7b8d. Worldserver EXE/PDB in exports match Visual Studio
outputs by SHA-256; original runtime DLLs and authserver EXE/PDB were retained.
Auth source remains b7a409b3. All three active configuration hashes match the
pre-batch backups. World PID 46584 and auth PID 44944 were listening on 8085/3724.
The world launcher opened its visible console after graceful server shutdown.

On the FINAL aura/escort build:

- 22 native CTests and51 Python tests pass.
- 15 starter cases,3 invalid quest requests and2 gossip-item cases pass (20 total).
- All 20 final-run disposable accounts and any created characters were removed.
- Starter and gossip comparators report no regressions against the previous native
 milestone; the dependency comparator reports no new findings or lost quests.
- Spell-zero aura diagnostics fell from 2 prior records to 0 at the final observation.
 Real startup/data/script diagnostics remain visible. The addon auditor still
 correctly fails on the one missing 58284 reference.

The 20 pre-aura cases also passed. Live suites keep nonzero overall exits for the
three known structural failure categories; the starter suite keeps eight explicit
untested mechanic groups. Direct escort failure/final-waypoint playthroughs remain
untested, and the 246 quest repair groups were not silently cleared. Compiler
macro-redefinition warnings were present; zero failed projects is not a claim of
a warning-free compiler build.

[Machine-readable evidence](quest-runtime-regressions-2026-11-08.json) records
artifact/config hashes, report hashes, counts, remaining issues and rollback
baseline. Full local client traces and source fingerprints remain under ignored
out/quest-repairs-20261108. The single-command gate and its existing-directory
rejection were both exercised successfully; no stale report was accepted.
