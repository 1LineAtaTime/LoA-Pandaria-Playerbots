# MoP Playerbots crash repair handoff — 2026-11-04

Recovered all four CODEX_HANDOFF documents, the Detour incident, validation log,
project checklist, and repository workflow. Existing changes were preserved in
commit e4258e3b. Forgejo origin remains authoritative; GitHub is its push mirror.
The local develop branch initially matched origin/develop and was two commits
ahead of origin/main. No remote changes were discarded.

## November 2 crash

The production path in the report is a deployment location, not the source tree.
Matching symbols were found in out/build/vs2022-x64/bin/RelWithDebInfo:
PDB GUID 60dedc38-69f2-45df-9261-6da3e0db98a9, age 1. The matching executable
and PDB were preserved under ignored out/crash-evidence-20261102 before building.
DbgHelp resolves RVA 0x21D24A to MySQLPreparedStatement::ClearParameters+0x7a,
and its caller 0x21C92E to MySQLPreparedStatement::~MySQLPreparedStatement+0xe.
The stack includes PlayerbotsDatabaseConnection::DoPrepareStatements and
RandomPlayerbotMgr::GetEventValue/AddRandomBots. This crash differs from the
October 30 Detour missing-edge incident.

MySQLConnection::DiscardConnection cleared m_stmts. Reconnect set m_reconnecting
and called DoPrepareStatements, which deliberately skips initial vector resizing
on reconnect. PrepareStatement consequently indexed destroyed unique_ptr slots.
Destroying a stale prepared statement reached its already-destroyed vector<bool>
parameter bitmap, consistent with count 1 and null storage in the report.
Commit 5c1cfc65 resets individual statement handles while preserving slot count.
It corrects the reconnect invariant, rather than masking a null bitmap access.
The existing interrupted-transaction probe now follows production conditional
resize behavior and executes a prepared debit after reconnect.

## Auctions and flight

Commit 4e4fa5cd adds a 65% chance to reuse the previous seller/buyer per auction
house. Otherwise a fresh random character is selected. Buyers still exclude the
seller. Existing uniform selection already allowed repeats; this makes sessions
of repeated activity more visible. Existing auctions are not rewritten.

Airborne creatures now bypass the ground navmesh, while landed flyers retain
normal ground navigation. This uses current flying/disabled-gravity flags, not
flight capability alone. All flight-capable templates have not been visually
validated; Dalaran Visitor and other scripted landing sequences still need tests.

The live five-city repairs were confirmed present. Only five flight-enabled
movement rows existed, while 1,486 additional existing templates (13,008 spawns)
were missing explicit valid flight records from the 2024 source migration.
2026_11_04_00_world_restore_flight_movement.sql restores missing records only,
joins existing templates, preserves existing records/overrides, excludes Flight=3,
and retains source Ground/Swim/Flight values. An isolated MariaDB test confirmed
existing-row preservation and repeat-run idempotence. The live movement table was
backed up to ignored out/flight-movement-backup-20261104.sql, then the restoration
was applied successfully: 1,491 flight-enabled records. Restart is needed to load
this data. No paths, spawns, maps, or MMaps were removed or regenerated.

## Validation and remaining work

Windows Visual Studio MCP is connected to the correct solution, x64 RelWithDebInfo.
Build and reconnect regression results are recorded in the follow-up below.
Do not call either crash permanently resolved without representative production
runtime testing. Preserve working FusionGEN store/cart/SOAP fixes; those were
recovered as historical context and were not altered. Created-text/sound issues,
Forsaken Bat and Dalaran special scripts remain separate open investigations.

## Completed validation — 2026-11-04

- Visual Studio MCP built worldserver x64 RelWithDebInfo successfully after
  correcting the Unit movement-flag accessor. Final gameplay source: 4355f94c.
- Standard CTest: 7/7 passed. Standalone Windows Detour CTest: 1/1 passed.
- Fresh isolated MariaDB 10.5.29, loopback port 33557: real core transaction
  probe passed interrupted transaction rollback/reconnect and prepared reuse,
  plus externally killed idle connection followed by prepared Execute/retry.
- Error paths now clear parameters BEFORE reconnect can destroy the statement,
  eliminating stale statement access if reconnection fails as well.
- Flight SQL test preserved existing rows, excluded unknown templates, and was
  idempotent. Live restoration completed with 1,491 flight-enabled records.
- Stage-Runtime completed for exports, including worldserver.exe and matching
  symbols; prior exports are in ignored out/exports-backup-20261104.
- No production executable was remotely replaced or started. Copy the staged
  executable AND matching PDB together, restart, and test flight/auction behavior
  and prolonged normal bot/NPC load. Build/tests do not establish a production
  soak or validate every scripted flying NPC. Forsaken Bat 51384 and Dalaran
  Visitor 32600 have no source-reference restoration row and remain separate
  per-spawn investigations; they were not forced into permanent flight mode.
- Existing auctions already had repeated owners (881 of 882 owners had more
  than one listing, maximum 17). The new 65% reuse rule strengthens clustering.
- Two root reference executables remain untracked intentionally. Dumps, symbols,
  databases, backups, production credentials and build outputs remain ignored.

## Publication

Recovery code and validation milestone 02079182d6e5899d6f56b4bddf569d6376d815b8
was pushed normally to Forgejo develop and fast-forwarded to main. The private
mirror check confirmed both repositories private and all eight branch/tag refs
matching. Mirror administration settings were not readable by the scoped token.
Later documentation-only commits do not change the staged gameplay binary,
which was built from 4355f94c plus the previously committed gameplay changes.
