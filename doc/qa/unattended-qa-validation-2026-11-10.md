# Standalone native QA validation — November 10, 2026

The existing automation pipeline now has an `adventure` profile. The Windows
entry point is `Run-Autonomous-QA.cmd`; instructions are in
[unattended-gameplay-qa.md](unattended-gameplay-qa.md). It uses local Python,
the existing native Playerbots AI, the owned QA runtime, and existing account
leases. No Codex session, API key or browser is required during execution.

## Executed evidence

| Case | Result | Local evidence |
| --- | --- | --- |
| Initial unattended240-second native episode | EXECUTED, ordinary reconnect PASS; completed while no active agent monitoring was required | out/qa-automation/adventure/20261110T050722579205Z |
| Finalized240-second native episode | EXECUTED; naturally rewarded25127, equipped53362 slot7, exact equipment/quests/spellbook retained on reconnect | out/qa-automation/adventure/20261110T083806689367Z |
| Diagnostic gate on that same episode | Campaign NEEDS_REVIEW, correctly detecting two newly accessed missing configuration keys despite successful gameplay | Same run's run.json and repair-queue.json |
| Invalid existing-character selection999999 | BLOCKED report with actionable reason; no provisioning or character mutation | out/qa-automation/adventure/20261110T083759044294Z |
| Launcher cold QA-world startup, then invalid selection | Reused auth61648, started QA world15772; negative case recorded and launcher result persisted | out/qa-automation/adventure/20261110T084516382377Z and launcher/20261110T0844531328719Z |
| Final launcher30-second smoke after the two config additions | Episode EXECUTED, reconnect PASS, startup/launcher reports saved; campaign NEEDS_REVIEW correctly retains23 deferred command-metadata errors | out/qa-automation/adventure/20261110T084612736299Z and launcher/20261110T0846100406901Z |

The first development trial preceded the final source archiving safeguards; use
the finalized083806 run for exact worker/supervisor source evidence. It retains
all three framework source copies, matching fingerprints, source/native binary
references and immutable episode JSONL. The preliminary trial is preserved.

Two findings were genuine missing QA settings, not content failures:
`AiPlayerbot.RpgTownErrandFlightMinDistance=800` and
`AiPlayerbot.RpgTownErrandMaxBotsPerNpc=3`. Both values already exist as defaults
in the native code and distributed configuration. They were added to the tracked
QA profile and owned QA world configuration, with a private pre-change backup.
No warning threshold or validation guard was weakened. The QA world alone was
restarted; existing startup content debt remains visible.

The final short run had neither of those two missing-setting warnings. Its23
new diagnostic signatures are deferred command-table validation errors first
triggered by command loading (obsolete/mismatched `quest broken`, `guard`,
`reload locales_*`, `spectate`, and related entries). These remain in the repair
queue with exact signatures in run.json. No blanket SQL deletion or diagnostic
suppression was applied. This is a successful detection/reporting test, not a
claim that those content/configuration issues are fixed.

## Checks and boundaries

- Seven supervision tests pass: actual progression deltas, conservative travel/
  combat inactivity, recovery classification, reward/equipment resumption,
  reconnect/failure disposition, exact QA process selection, and rejecting an
  auth configuration pointing at a foreign database.
- Eight pool/reset/lease tests,33 headless protocol tests and26 existing pipeline
  tests pass:74 Python checks in this pass. The earlier native gameplay repair
  had24 passing CTests and a successful Visual Studio build; no C++ changed for
  the standalone runner, so it was not rebuilt for these Python/launcher changes.
- The live run exercised normal progression, saved observations, equipment,
  spells and reconnect. Watchdog stall/recovery classifications are unit tested;
  a suspected stall is not automatically a proven quest or core defect.
- Process matching was tested while exports world46212/auth49180 ran separately.
  Neither was shut down or reconfigured. QA auth61648 was reused; QA world39352
  was gracefully stopped after no QA characters were online, then the launcher
  started15772 visibly. Auth cold startup was not required for this validation.
- QA native source remains8e4b42d17c with matching EXE/PDB; executable SHA256
  fb319652eb7983bc422c39b43e3d836202aa0b2136e897d51d000b52bd6ca4ee.
  No new database schemas/server installations or competing gameplay AI were made.
- Ordinary played exports and production data were untouched. Runtime reports,
  credentials and private backups remain ignored local artifacts.

The persistent pilot is level4 with six actually rewarded quests; M1 is verified.
Level15, legitimate dungeon completion, level90 and exhaustive content coverage
remain unfinished. Reports explicitly retain that scope. The standalone runner
does not silently fix code, apply SQL, grant progress or suppress diagnostics.
