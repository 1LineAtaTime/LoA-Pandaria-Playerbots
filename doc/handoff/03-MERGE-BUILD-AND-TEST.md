# Merge, build and test

## Complete public checkout

This public branch is already the reconstructed and reconciled full source tree.
Do not copy a source overlay or apply the old review patch on top of it. Start at
UPGRADE.md and CONTRIBUTION.md in the repository root. Historical package paths
below describe the original export; the146-file public inventory now lives at
database/contribution-migrations.csv.

## Windows build

Keep the upstream toolchain requirements. The tested development environment
used Visual Studio 2022, x64 RelWithDebInfo, all scripts/Playerbots/modules enabled,
with CMake/OpenSSL/Boost and a supported MySQL/MariaDB client library installed.
Populate private connection files yourself; the distributed templates deliberately
use `DB_USER` and `CHANGE_ME`.

```text
cmake --preset vs2022-x64 -DBUILD_DATABASE_TESTS=ON
cmake --build --preset vs2022-x64-relwithdebinfo --parallel 4
ctest --preset vs2022-x64-relwithdebinfo
```

Stage resolved programs/DLLs/default templates with
`contrib/windows/Stage-Runtime.ps1`. Keep PDBs paired with their executable when
investigating crashes. Never use a PDB from a different build as matching proof.

## Linux and other platforms

The feature sources are provided, not a certified Linux binary package. Use the
upstream platform build procedure with the same feature flags and compatible
libraries. Windows PowerShell/console helpers are platform-specific. Validate
compiler/API differences, SQL behavior and lifecycle behavior on Linux before
claiming platform acceptance. Do not ship Windows DLLs for Linux.

## Automated checks

```text
python -m unittest discover -s contrib/headless-client -p "test_qa*.py"
python -m unittest discover -s contrib/quest-qa -p test_runtime_diagnostics.py
```

The portable headless entry point accepts an explicit config/suite/report folder
and a world PID for owned account cleanup:

```text
python contrib/headless-client/qa_run.py --config /path/to/worldserver.conf --suite contrib/headless-client/qa-scenarios.json --report-dir /path/to/reports --world-pid WORLD_PID
```

On Windows the simplified Start-Server/Run-QA wrappers reuse one visible auth/world
pair. **Their deployment guards are environment-specific**: tool locations,
`exports/ACTIVE-RUNTIME-MANIFEST.json`, schema names and ports must be adapted by
the maintainer. They are not a universal fresh-install launcher as exported.
The recipient supplies the runtime manifest and private config, not someone
else's runtime/account database. Older multi-realm/observer workflows are not
the recommended default; this handoff includes their source history/reference
tools for review, not instructions to start multiple servers.

QA can run without Codex or an AI service. It still needs its configured database,
Python and the game server. It creates temporary ordinary accounts/characters,
retains evidence and cleans up only exact journal-owned identities. Each
scenario uses its own account/character, respecting the 11-character account cap.
No actual QA passwords are included; test credentials are generated at runtime.

## Reports and failure handling

The one-shot runner writes readable results, structured JSON, a repair queue and
grouped runtime diagnostics. Unsupported cases remain untested; empty/no-case,
blocked and incomplete runs must not turn into a complete-game pass. Current
warnings/errors are retained without lowering logger thresholds. Duplicate
appender copies are deduplicated while repeated event counts are preserved.

Logout records the server result. Only explicit combat refusal is retried for a
bounded 45 seconds; falling/duel/freeze/acknowledgement failures stay visible.
Cleanup waits up to 90 seconds for the native 60-second disconnect grace before
declaring the owned character stuck online. Ownership/dependency checks protect
other accounts. An interrupted or failed run's journal must be inspected before
retrying cleanup; never delete all accounts matching a broad prefix.
