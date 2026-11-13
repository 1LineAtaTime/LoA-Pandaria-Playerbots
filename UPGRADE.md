# Build, install and upgrade this contribution

Contributed by [Nicalicious](https://github.com/Nicalicious).
Start from a full checkout of this branch. No private repository, account,
password or live database dump is needed to obtain the source and migration files.

## Existing installation: preserve your players

1. Stop worldserver normally and back up **every** participating schema:
   auth, characters, world, playerbots, and any configured log/archive schema.
   Keep the old executables, paired symbols and private configurations together.
2. Record your current source/release and database update ledgers. Read
   [database/README.md](database/README.md) and
   [database/contribution-migrations.csv](database/contribution-migrations.csv).
   Run `python contrib/release/verify_distribution.py` to verify the indexed SQL.
3. Build this checkout. On Windows with Visual Studio2022 x64 and the upstream
   Boost/OpenSSL/MySQL client dependencies installed:

   ```text
   cmake --preset vs2022-x64 -DBUILD_DATABASE_TESTS=ON
   cmake --build --preset vs2022-x64-relwithdebinfo --parallel 4
   ctest --preset vs2022-x64-relwithdebinfo
   ```

   If discovery needs help, pass dependency locations as CMake cache variables
   (`BOOST_ROOT`, `OPENSSL_ROOT_DIR`, `MYSQL_INCLUDE_DIR`, `MYSQL_LIBRARY`).
   Do not check your machine-specific paths into the templates.
4. Review missing updates against the schema you actually have. Auth/character
   accounting and recovery updates, companion RBAC, curated names and Orb receipt
   tables belong to their respective schemas. Chat/name seed SQL belongs to the
   playerbots/characters schema specified in the inventory, never world by guess.
   Existing balances retain their raw DP units; do not reset or rescale them.
5. Compatibility-reference files and cleaned base dumps are **not** normal
   forward migrations. Do not import `sql/base/auth.sql` or `characters.sql`
   over existing player data. Legacy world aliases and signed-next-quest changes
   need the dependency review described in database/README.md. This cumulative
   contribution does not provide a verified universal blind apply-all procedure.
6. Use the upstream updater only for the reviewed applicable missing updates:
   `Updates.EnableDatabases = 15`, `SourceDirectory` set to this checkout and
   `MySQLExecutable` set to your compatible `mysql`/`mariadb` command-line client.
   Its ledgers prevent normal replay, but do not infer that a similarly named
   column/table or an old hand-applied migration has the correct meaning.
   Keep `DELIMITER` statements and per-file sessions intact. Stop on SQL errors;
   never use a force/ignore-errors flag or replay rollback SQL as an update.
7. Copy resolved binaries, required DLLs and the new templates into your runtime.
   The Windows helper is `contrib/windows/Stage-Runtime.ps1`. Retain matching
   PDBs privately. Use the supported client library matching the compiled import
   library; a newer server version alone does not require substituting an
   arbitrary DLL. See the database-compatibility feature notes for tested bounds.
8. Merge new config keys into your private `.conf` files. Or start with the full
   templates under `config/examples/`, rename them to `.conf`, and fill in your
   own connections. Default example schemas are `mop_auth`, `mop_characters`,
   `mop_world`, `mop_playerbots` and `mop_log`; names can be changed together.
   Set `DataDir`, `LogsDir`, realm address/port and source/client paths locally.
9. Start authserver, then worldserver. Check update and startup logs for failures
   before admitting players. Check login, a fresh starting quest, vendor/bank,
   companion summon/removal, auction query and a controlled BattlePay purchase,
   cancellation and insufficient-funds attempt. Bot creation on a new install
   takes time. Do not run two worldservers against the same realm/database set.
10. If an update fails, stop and restore the backed-up matching source/binaries
    and databases. A Git source rollback does not reverse a schema/data migration.

## Fresh install

Use the matching **LoA** world/playerbots baseline, obtainable from the upstream
[release assets](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases),
then the applicable contribution updates above. A bare SkyFire world or an
unrelated repack is not the matching database. The upstream README describes
extracting client build18414 DBC/maps/vmaps/mmaps/cameras. These copyrighted
client assets and the operator's live database are not distributed here.

The cleaned auth/characters schemas in `sql/base` contain no seeded player
accounts or saves. They are fresh-schema references. Review remaining upstream
baseline dumps before use; this contribution does not certify every historical
upstream database asset as sanitized. Create your own administrator locally;
no ready-made GM account/password is published.

The optional catalogue in `database/optional-battlepay-catalog.sql` contains only
static shop definitions. It is optional, uses non-destructive inserts and can
collide with an existing catalogue's IDs. Review/remap collisions before import.
Bot names/chat seeds and the550-target gameplay config are included; bot account
passwords and real assigned-character records are not.

## QA without Codex

Pass `--config /path/to/worldserver.conf --client /path/to/mysql` to the QA runner,
plus `--suite`, `--report-dir` and the local `--world-pid` for owned cleanup.
See `contrib/headless-client/qa_run.py` and
`doc/handoff/03-MERGE-BUILD-AND-TEST.md` for explicit config/suite/report arguments.
Tests create journal-owned temporary accounts and characters, respecting the
11-character account cap, and write reports for later repair. Diagnostic
inventory coverage is not proof of natural gameplay completion. Older local
launcher guards require adaptation; upstream direct server startup remains the
portable default.

Use the native auction/city regressions in
`doc/dalaran-auction-followup-2026-11-12.md`. Review each test's preconditions;
several historical scripts contain environment-specific defaults and are
maintainer tools, not universal unattended acceptance for a new installation.

## Linux

Use the upstream Linux compiler/dependency/build instructions with the same
modules enabled. Windows EXEs/DLLs/launch helpers do not apply. The source is
provided for Linux, but this contribution's final native validation is on
Windows; it does not claim a Linux gameplay soak has passed.
