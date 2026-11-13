# Single-server gameplay QA

## Current runtime (November 10 restoration)

`Start-Server.cmd` starts or reuses the visible authserver and worldserver in
`exports`, on auth **3724** and world **8085**. They use the preserved regular
`mop_loa_auth`, `mop_loa_characters`, and `mop_loa_world` databases. Existing
player accounts remain usable. The launcher validates the schema selection,
realm port/build and recorded worldserver hash before starting anything.

`Run-QA.cmd` uses this same pair. It creates temporary normal-security accounts
for reviewed protocol gameplay scenarios, records results and cleans up its own
test accounts through the server. It never starts a game client, observer,
autoplay loop or second server pair. Repeated server starts reuse the pair;
another installation running causes a startup block rather than a duplicate.

The earlier choice of the isolated `out/quest-qa-runtime` as the default was
incorrect for the user's normal accounts. It is superseded by this restoration.
Its `mop_loa_qa_*` databases and historical evidence remain preserved offline.
No account passwords or regular account records were changed for the restoration.

## Local client

For the build 18414 client at `<local-workspace-path>`, use these settings
in `WTF\Config.wtf`:

```text
SET realmList "127.0.0.1"
SET realmName "ExampleRealm LoA"
```

The default auth port is 3724; the realm advertises world port 8085. Use your
existing regular LoA account, not a separate isolated QA account. The launchers
do not rewrite client settings or credentials.

## Running without Codex

```powershell
.\Start-Server.cmd
.\Run-QA.cmd --select quest-25152
```

Omit `--select` to run the starter suite plus the existing authored workflows
and invalid-quest regressions in `contrib/headless-client/qa-*-scenarios.json`
(the separate controlled legendary fixture is not included). Every invocation executes fresh
cases; `--force` is retained only for command-line compatibility.
Python, MariaDB and the configured database endpoint must be available.
No AI service or Codex tokens are required. A network database still requires
connectivity even when Codex is closed.

## Reports and limits

Each invocation writes `out/qa-reports/<UTC timestamp>/README.md` and `run.json`.
`out/qa-reports/latest.json` points at the latest result. Detailed evidence is in
that run's `details/qa-report.md`, `details/qa-report.json` and `details/gameplay.json`.
Startup status is in `out/qa-automation/server/latest.json`.
The readable report includes a `repair-queue.json` of failed/blocked/untested
cases. `runtime-diagnostics.json` groups current warning/error messages and
retains repetition counts, without changing logging thresholds or deleting logs.

Gameplay scenarios and read-only structural data audits are reported separately.
`COMPLETED` requires the selected batch to pass with complete diagnostics;
`NEEDS_REVIEW` includes failed, blocked or untested gameplay/data checks and
runtime diagnostic findings. `BLOCKED` means setup prevented a valid batch.
A passing structural check does not prove a quest is playable. These reviewed
cases do not certify every quest, class, profession, dungeon or raid.

Autonomous bot circling, terrain clipping and unsupported quest-item targeting
remain defects. Observer/autoplay experiments are retired from the default
workflow and are not accepted as whole-game QA evidence. Original reports and
archive tags remain available; no history was rewritten.
