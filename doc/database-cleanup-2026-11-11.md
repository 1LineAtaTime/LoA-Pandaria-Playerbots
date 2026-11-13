# Obsolete project database cleanup, November 11

The owner requested removal of databases created during this project while
retaining the original mop_ set and unrelated applications. An explicit allowlist
was used. Unknown databases were preserved; no wildcard DROP was performed.

## Current defaults

All server .conf files in exports, including worldserver - Copy.conf, select
mop_auth, mop_characters, mop_world, mop_playerbots and mop_log as applicable.
Playerbots reads its database connection from worldserver.conf. Config backups
precede edits. No restart was needed because active connections already used
the original databases. Start-Server.cmd and Run-QA.cmd do not depend on the
retired QA schema set and do not automatically recreate those schemas.

## Archived and removed

- `codex_sf27_content_fixture_20261106`
- `codex_sf27_final_fixture_20261106`
- `codex_sf27_interaction_fixture_20261106`
- `codex_sf27_reference_20261106`
- `codex_sf27_spell_fixture_20261106`
- `codex_sf27_typed_loot_fixture_20261106`
- `codex_spawn_backup_20261107_010005`
- `codex_spawn_compat_before_20261107_0057`
- `mop_loa_auth`
- `mop_loa_characters`
- `mop_loa_log`
- `mop_loa_playerbots`
- `mop_loa_world`
- `mop_loa_qa_auth`
- `mop_loa_qa_characters`
- `mop_loa_qa_log`
- `mop_loa_qa_playerbots`
- `mop_loa_qa_world`
- `mop_sf_auth`
- `mop_sf_characters`
- `mop_sf_world`
- `mop_world_before_sf27_20261107_001511`
- `mop_world_sf27_rollup`

## Backup and verification

Every selected schema was exported with routines, events, triggers and hex
binary data to its own compressed SQL file. All gzip streams were read to EOF
to verify their CRC, checked for the completed-dump marker and assigned a SHA-256
checksum. Every backup completed before any DROP. No target schema had active
database sessions before archiving or deletion.

Private archive and inventory:
`out/backups/database-cleanup-20261111-143242/manifest.json`.
The manifest records exact files, checksums, SQL sizes, preserved schemas and
completed deletions. Auth/character archives contain private data and must not
be copied into the public handoff. To roll back, stop consumers and restore only
the chosen schema's .sql.gz with the database client. Dumps contain CREATE/USE
for their original schema; do not restore them over the active mop_ databases.

All 31 databases outside the allowlist remained present, including the original
mop_ set, its older quarantine database, FusionGen and its stage schemas,
Frostwing, generic auth/characters/world databases of uncertain ownership,
unrelated applications and MariaDB system schemas. No database users or
privileges were removed. Historical reports that refer to retired databases
remain as history; compressed archives replace the in-instance rollback copies.

Both active server ports,3724 and8085, remained listening throughout cleanup.
