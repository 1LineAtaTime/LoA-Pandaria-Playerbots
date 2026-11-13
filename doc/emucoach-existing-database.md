# Existing EmuCoach database compatibility (2026-10-28)

This is the initial schema-only test record. Later full-data results and the
current deployment state are in `doc/emucoach-followup.md`.

This records the read-only inspection of the existing MariaDB 10.11.6 server and
startup tests against **schema-only** copies on local MariaDB 10.5.29. No source
account, character, world, log, bot, or DP row was copied or changed. No new
worldserver process was connected to the source databases.

## Configuration and startup order

`StartDB()` runs before the Playerbots module loads `playerbots.conf`. Therefore
`LoginDatabaseInfo`, `CharacterDatabaseInfo`, `WorldDatabaseInfo`,
`PlayerbotsDatabaseInfo`, the database thread counts, `ThreadPool`,
`Updates.AutoSetup`, and `Updates.EnableDatabases` must be in `worldserver.conf`.
The Playerbots sample configuration now says this. The local, ignored
`exports/worldserver.conf` has all these settings, pointing at the existing
schemas and a proposed dedicated `mop_playerbots` schema. Its credentials are
not tracked. The local `exports/playerbots.conf` keeps bot behavior settings.
The separate test config redirects every database to loopback, disables core
updates, and disables Playerbots and random bot login.

The source has `mop_auth`, `mop_characters`, `mop_world`, and `mop_log`; an
`acore_playerbots` schema exists but is empty. The new module opens a separate
Playerbots pool. Its prepared statements need 28 Playerbots tables in the
disposable fixture: 13 from `playerbots_database_full.sql` plus 15 from
individual base files. The eventual dedicated schema must contain the
appropriate module data as well as those definitions. It must not receive
duplicate account or character records. The existing bot-related tables in
auth, characters and world remain in place.

## Schema findings

The fork auth baseline expects 44 tables; the source has 53 tables but lacks
ten expected tables, including `rbac_account_permissions`, `battlepay_log`,
`wow_token`, `updates` and `updates_include`. `account.lock_country` and
`account.project_member_id`, and `battlenet_accounts.project_member_id`, are
missing. The Character baseline expects 129 tables; the source has 137 but
lacks five expected tables and several optional project migration columns.
The source has existing `account.dp` and `account_boost`; no wallet balance
modification was made. Its existing World BattlePay catalog tables remain
untouched. The new durable BattlePay ledgers are absent; the existing
`2026_10_28_00` and `_01` auth/character migrations were tested only on the
disposable schemas.

World is a larger semantic mismatch. The existing `creature_text` uses `entry`
where this core reads `CreatureID`; it also lacks `SoundType`. `creature_template`
uses `faction_A` and `faction_H` where this core reads one `faction`, and lacks
`femaleName`. `creature` has `spawndist` where this core uses
`wander_distance`, and lacks `walk_mode`. `smart_scripts` lacks
`event_param5` and `target_param4`; `waypoints` lacks `orientation` and
`delay`; `creature_template_movement` and `keychain` are absent. Simply adding
defaulted columns can make empty prepared statements succeed, but it does
**not** translate existing world rows. The invalid `vw_honor` view in auth and
characters was omitted from the schema-only copies.

The `sql/compat/emucoach/*_additive_candidate.sql` files capture additive
auth/character DDL that passed on the schema-only fixtures. They are review
candidates, not a production migration. In particular, old account/Battle.net
relationships and catalog semantics need validation on representative rows
before any deployment. There is no production-ready World migration yet.

## Isolated startup result

The compiled `exports/worldserver.exe` loaded its test configuration and
connected to all four schema-only copies plus the Playerbots fixture. Its
initial Login and World prepared-statement errors were resolved in the
disposable fixtures with additive DDL. The existing BattlePay ledger migrations
were then applied to the disposable auth/character fixtures. Startup passed
database preparation and BattlePay schema checking, loaded zero RBAC
permissions, and stopped at `trinity_string`: the schema-only World copy has
zero rows. This does not establish that the server can load actual world data,
that bot accounts/characters work, or that BattlePay fulfillment works with
the existing data. The binary did not need rebuilding for the configuration
fix.

## Deployment gate

Before using the new worldserver on writable source databases, validate a
representative isolated **data** copy, including account links, characters,
bot state, world reference data, BattlePay catalog and DP invariants. A full
`mop_auth` dump was blocked by automatic approval review because it would copy
sensitive account and wallet data. It was not retried. The current source
databases were only queried read-only for schema metadata and server version.
Once the data validation and migration review are complete, the user must
approve the first writable production startup. Keep automatic updates and
random bot autologin off for that first startup, then enable a small bot cohort
after verification.
