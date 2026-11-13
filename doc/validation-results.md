# Validation results — 2026-10-28

The source was configured with Visual Studio 2022 Community, MSVC v143,
Windows SDK 10.0.26100, x64 `RelWithDebInfo`, `PLAYERBOTS=ON`, `USE_MODULES=ON`,
all scripts, Eluna, authserver and extraction tools. `cmake --build` linked
`worldserver.exe` and `authserver.exe`; the final incremental worldserver build
also included the redeemed-token GUID restart safeguard. The runtime staging
script resolved non-system imports and placed the server programs, PDBs,
default configs (including `playerbots.conf.dist`) and required DLLs in
`exports/`, with a SHA-256 manifest. `worldserver.exe --version` exited 0.

| Validation | MySQL 5.5.62 | MariaDB 10.5.29 |
| --- | --- | --- |
| Base auth/character/Playerbots SQL import into isolated databases | Pass | Pass |
| MySQL 5.7.44 and MariaDB Connector/C 3.1.28 binary client probes | Pass with both connectors | Pass with both connectors |
| Actual Login, Character and Playerbots prepared statements | Pass | Pass |
| Actual core transaction/rollback/reconnect probe | Pass | Pass |
| Authserver startup and 5.4.8 SRP6 client handshake with a disposable account | Pass | Pass |
| Production BattlePay SQL harness against disposable schemas | 42/42 | 42/42 |

The Playerbots combined SQL dump has only 13 tables. For the prepared-statement
checks, the missing 15 table definitions were loaded into the disposable
fixture from the module's individual base files using
`contrib/database-tests/Prepare-PlayerbotsSchemaFixture.ps1`. All three pools
then prepared successfully on both engines. The fixture helper intentionally
does not load the large travel-node and other data tables for a new realm;
existing working Playerbots data must be preserved during deployment.

CTest passed 6/6 cases: 26 database version parser cases, native BattlePay
accounting cases, and strict config path/argument checks for both servers.
The SQL harness exercised concurrent debit, insufficient funds, external wallet
changes, exact prices, duplicate delivery, recovery, refund, boost reservation,
token credit and rollback. These are real SQL state tests but use minimal
character/mail/item fixture tables. The auth handshake connected to test-only
authservers and disposable accounts on loopback ports 33555/33556.

The following remain unverified: worldserver boot against a complete matching
world database and extracted 5.4.8 game data, live Playerbots initialization,
the store's 5.4.8 client packets and item serializer, and actual in-game
purchase/service fulfillment. No production MariaDB database, user account,
DP balance or schema was changed. BattlePay purchase-history and distribution
list packets remain empty; the durable SQL history is implemented. Stackable
donation token redemption is refused safely, as described in
`doc/battlepay-accounting.md`. Deploy the four BattlePay migrations in the
documented order after a backup and inspect existing boost-key duplicates
before starting the new worldserver.

## Live update — 2026-10-29

The four BattlePay migrations are installed on the existing MariaDB schemas.
The Visual Studio 2022 `RelWithDebInfo` worldserver was rebuilt and deployed to
`exports`; authserver already matches its Visual Studio build. Worldserver and
authserver both start from that folder. The native BattlePay accounting CTest
passes, and the isolated MariaDB SQL harness passed 44 checks in this session.

example account made three successful 15 DP purchases of product 4401. Each reached
delivered state 2 with a character delivery receipt in state 1. The shop balance
tile showed the account's whole DP balance without a crossed-out zero. With
the temporary wallet at 16 DP, a 20 DP item produced the native client
insufficient-funds error and cleared the `Connecting to shop` overlay. The
failed attempt created no debit or delivery. A temporary 15 DP follow-up
purchase left 1 DP, and the test balance was restored to 16 DP. The original
730 DP balance can be restored when insufficient-funds testing ends.

The client binary reader established that `SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE`
contains purchase ID, client token, then result code. The server previously
swapped the latter two fields; the client ignored its failure replies. The
updated server now matches that order. Player auction transactions, duel
completion, disconnect during purchase, and process-kill recovery still need
end-to-end validation. Purchase-history and distribution-list client packets
remain unimplemented.

<!-- mop-navmesh-null-edge-fix-2026-10-30 -->
## Navigation crash fix (2026-10-30)

Windows worldserver crash: `0xC0000005`, `closestPointOnDetailEdges<true>`, `DetourNavMesh.cpp:673` during creature random movement. Fix guards against missing detail-mesh edges, uses the base polygon perimeter when available, and prevents null reads. Focused sanitizer tests passed outside the Windows build. **Full Windows rebuild and long-running server validation are still pending.** Details: `doc/navmesh-crash-2026-10-30.md`. Git commit/push might be pending if the standalone installer could not locate Git; Codex should check the working tree.

## Recovery validation — 2026-11-04

Visual Studio MCP built the x64 RelWithDebInfo worldserver with both crash
repairs. Standard CTest passed 7/7 and the Windows Detour edge regression passed
1/1. An isolated MariaDB core transaction probe passed forced reconnect,
prepared reuse, and rollback tests. Source, database flight restoration, artifact
backup/staging and runtime limitations: ../MoP_Playerbots_Crash_Fix_Handoff.md.
No production soak or complete visual flight validation is claimed.

## Startup/runtime repair validation — 2026-11-05

Visual Studio MCP built the x64 RelWithDebInfo worldserver through source
cc6689cd; matching EXE/PDB were verified and staged in exports. All 14 native
CTest cases and the diagnostics Python test passed. Real MariaDB temporary
fixtures verified exact field/row changes, repeat stability and custom-data
guards for the scoped Nov4_18/_19 and Nov5_01–_07 migrations. Complete private
table backups precede live application. No auth, wallet or character-progress
rows were directly modified. Existing corrupt taxi state is recovered on login
and persisted through the normal save path.

A fresh 191-bot run eliminated the Dalaran shopkeeper/rat greeting requests,
retired-criteria diagnostics, packet-registration collisions and three corrected
aura-hook diagnostics. Zero-duration splines, invalid speeds, clock warnings
and duplicate engagement did not recur during the short observation. This is
not full flight/instance/companion acceptance or a long-running stability claim.
Missing native scripts and other content/navigation defects remain. Exact
hashes, applied backups, known limitations and subsequent observations:
doc/startup-repair-2026-11-05.md.

## Batched startup compatibility repairs —2026-11-05 follow-up

Visual Studio MCP built source through ebe4bd54; the matching world EXE/PDB and
corrected generator are staged in exports. All15 CTests passed, including a
navigation-input test that retains corrupt-file, ordinary-water, real-terrain
and uncertain-data diagnostics. SQL08/09/10 temporary-table fixtures verified
exact removals/additions/fields, other-row preservation and repeat stability.
A real ordinary-water fixture generated with the corrected format10 liquid
reader passes the native MMap load/duplicate/unload/reload/missing-map and origin
probe; the synthetic tile was not installed. Six invalid RNDBOT saved routes
were backed up and cleared while stopped;25 valid routes were retained and the
next DBC audit found no invalid saved routes.

The fresh198-bot run has zero console-category warnings/errors and two confirmed
proto-drake accessory seats. All-category logs still have1594 database/content
errors;969 missing implementations and51 real speaker/text references remain.
No diagnostic-level suppression or wholesale binding/content deletion was used.
Exact deployed hashes, backups, intermediate test regression, scope and limits:
[startup-batch-repair-2026-11-05.md](startup-batch-repair-2026-11-05.md).


## November5 continuation —13:25 local

Native follow/taxi/enchantment/glyph/legacy SmartAI compatibility repairs and
SQL11–23 are deployed; complete table backups and disposable fixture tests
are recorded in doc/startup-repair-continuation-2026-11-05.md. Visual Studio
RelWithDebInfo x64 build passed; all16 CTests plus diagnostics Python test pass.
Matching worldserver EXE/PDB staged in exports, visible PID59372 listening8085;
unchanged auth PID61228 listening3724;198 random bots online. Fresh full logs
still contain1,494 diagnostics including958 missing native names. No new crash,
movement/taxi or glyph validation failure in this short sample; extended soak
and gameplay acceptance remain open. Do not claim zero errors. Static binding
inventory is doc/missing-native-script-inventory-2026-11-05.csv. Detour unchanged.


## November5 Jaomin repair —13:45 local

Native1339/source4b6b2f48 builds in Visual Studio;17/17 CTests pass. Backed-up
SQL24 and paired native EXE/PDB deployed to exports; world45872 starts in21sec
and listens8085; no Jaomin/vehicle-related script-load errors. Seven dialogue
rows preserved. ExampleCharacter's quest/faction/phase untouched. Fight/credit acceptance
is pending; see doc/jaomin-repair-2026-11-05.md. Other world errors remain open.


13:47 user acceptance: ExampleCharacter can attack Jaomin at close range and29409 completes
on defeat. Reported progression blocker fixed; extended group/vehicle/reset and
next-quest checks remain separate. No Jaomin-specific error during live test.


## November5 bridge visibility —14:01 local

User confirmed bridge Master53566 stays visible after area transition. Native
read-only snapshot proves missing type12 detection was restored by sourced,
guarded SQL25: ExampleCharacter detect4096/aura105064, NPC invis4096, both visibility checks
pass at2.42yd in area5843/phase1. Saved29409 rewarded and29410 complete verify
normal turn-in/next-quest progression. No manual aura/quest/phase/faction bypass.
Separate Northrend571 zone0/empty-tile warning remains open. See
 doc/shang-xi-visibility-2026-11-05.md for backup/hash/evidence details.


## November5 map571 / published fixes —14:18 local

Native99aa5796/revision1348 compiled in Visual Studio;17/17 CTests passed.
Published SF1408 post-hook null-target crash guard applied. Guarded SQL26
adapts published Stair of Destiny flight-menu fixes to local reversed option
IDs and active-state mask10; full conditions backup, preservation/idempotence
fixtures and mask-state checks pass. Client flight/Rune quest tests pending.
Map571 failed lookup did not recur across three fresh runs; requester/current
map/teleport/corpse context now retained even during login. Do not mark map571
resolved or claim flawless startup. Separate0/42/23 missing MMap warning seen.
No navigation files, Detour code or log severities changed. Full report:
doc/map571-and-published-fixes-2026-11-05.md.

## November 5 published-patch audit —14:42 local

All 296 pinned release26 SQL files executed independently in privilege-isolated
world-table fixtures: 175 changed fixtures, 70 had no observed change, 51 failed.
Live SELECT and zero-row UPDATE were denied to the executor. Both fixture schemas
and the temporary account were removed; no live patch was deployed by this run.
These results establish SQL compatibility observations, not behavioral acceptance
or cumulative upgrade correctness. See published-sql-compatibility-2026-11-05.csv.

Read-only catalog census covered 18,145 quests, 95,337 spell IDs, 80,022 items and
17,679 objectives. Findings: 13 missing objective references and 519 missing item
spell references (54 already quarantined at required level >=100). All need intent/
runtime review; none were blindly removed. Quest playthroughs, spell casts and
item-use behavioral tests in this census: zero. Both Python tools compile-check.
Existing native build/17-test results remain unchanged; no native rebuild/restart
was needed. Current full logs still contain real content errors. Coverage design,
existing BotScenario limitations and primary examples are documented in
automated-content-testing-2026-11-05.md.

## November 5 starting-zone numeric batch —18:25 local

User explicitly authorized current-server gameplay/data testing without another
server. SQL27 adapts seven published files, corrects their Night Elf Simple Sigil
3106/3116 error, and changes223 numeric fields across111 quests. All42 active
class-mask expectations already match. Pre-deployment306-case suite:83 passed,
223 failed; post-deployment:306/306 passed, no SQL errors. These are data contracts,
not completed quest playthroughs. Full-table fixture preservation and second-run
idempotence passed. Both Python tools compile-check.

Worldserver stopped for an18,176,469-byte complete two-table backup, then SQL27
was applied. Backup import,306 original expectations, migration and scoped reversal
were tested in a disposable schema; both full tables returned to identical
original values. No runtime rollback occurred. Backup hash/path and restoration
instructions: doc/starting-training-batch-2026-11-05.md.

Visible launcher starts worldserver59704, initialization19sec, listening8085.
Auth61228 unchanged. Current source99aa5796 EXE/PDB remain; no native rebuild was
needed. Real errors remain in full logs, including missing native bindings and
SmartAI/content defects. No log severity was changed. Do not claim zero warnings
or errors, or all published fixes deployed. Other portions of the seven SQL files
remain tracked separately from this completed numeric subset.

## November 5 native/content campaign —18:59 local

Native7f75c1fb builds in Visual Studio;18/18 CTests pass. Backed-up SQL28/29
activates complete Hot Rod/pillory ports and repairs nine training-chain fields;
five-table preservation, repeat stability and exact rollback pass. Actual23.7MB
backup restore/upgrade/reversal also passes full row-hash comparison. Data checks:
357/357 (306 prior training,34 chains,17 port checks). Test reports remain ignored.

Matching EXE/PDB staged; visible world28648 starts18:54:53, initialization19sec,
listening8085; unchanged auth61228 on3724. Actual reserved-bot Map event26277
creates17 of25 chance-based NPCs; return to saved location requested. No forced
quest completion. Real-client Goblin/Worgen progression and longer soak remain
open. Full logs still contain953 missing native names plus other real defects.
Generic source_type3 errors are replaced by implemented cannon behavior, an
explicit preserved legacy21888 backend and one actionable unsupported21889 group
(six rows). No log severity or Detour/navigation data was changed.

Report, hashes, recovery and scope: doc/native-content-campaign-2026-11-05.md.


## November 5 Gilneas progression and ride safety —20:20 local

Paired SQL30/31 is deployed with full backups. All442 data contracts pass:
85 Gilneas,306 training,34 chain links,17 prior ports. Actual89MB pre-batch dump
restoration, preservation outside per-statement key scopes, repeat stability and
all85 fixture contracts pass; disposable schemas removed. Native19/19 CTests
and five Python SQL/assembly tests pass after the final Visual Studio rebuild
(five projects succeeded, zero failed). Native repairs include owned ride paths,
catapult input/callback safety and a corrected floating-point speed diagnostic.

Matching worldserver EXE/PDB is staged in exports. Visible world51224 initialized
in21 seconds and listens8085; auth61228 unchanged on3724. Actual phase4194304
gunship floor105.347 and cannon-event14-NPC mechanism tests pass on the same
functional source (final rebuild additionally corrects the diagnostic format).
These are not quest playthroughs. Fresh engine load resolves four newly exposed
historical-migration backend conflicts, with no new exact error lines against
the pre-batch baseline. Missing native names953→952. Existing errors remain
visible; no logging filters or navigation data were changed.

Crowley's boarding bridge is deferred because its complete native ride is absent;
its old missing-name diagnostic remains open. Natural Worgen progression and
other outstanding content/stability work remain on the checklist. Details,
artifact hashes, exact footprint and recovery: gilneas-progression-campaign-2026-11-05.md.


## November 5 stability and quest continuation

Source4f904145 and backed-up SQL32 implement Sacrifices14212's missing native
questgiver, owned33-point ride and mounted actor. Krennan/Josiah delayed actor
access and missing-player/vehicle paths are hardened. All19 native tests, five
Python tests,458 live data contracts and actual26.7MB backup recovery pass;
full-row preservation/repeat stability verified and temporary schemas removed.
Initial visible native load49008 resolves two bindings (952→950), with no new
exact startup errors. Natural riding/credit/concurrent-player acceptance is open.

Later runtime invalid-velocity messages on Iron Sentinel29984 remain real defects.
A provenance diagnostic is being built to identify the explicit zero-speed
producer; configured movement speeds are positive. It is not a movement fix,
crash proof, arbitrary speed replacement or a logging-severity change. Details
and recovery: stability-quest-continuation-2026-11-05.md.


## Vertical-only knockback and observer recovery follow-up

Native00348248 plus c2a21704 is staged and running as visible world43228;
paired hashes are in vertical-knockback-repair-2026-11-06.md. Torn Earth55715
requests horizontal0/upward12 on Iron Sentinel29984. A physical two-point
vertical arc now rises/lands using gravity-based time; ordinary invalid zero
travel still fails. The new sampler's first-trial crash is documented and fixed:
Duration() required an initialized spline. The full guarded repeat completed600
snapshots, including391 uninitialized states, two real launches and12 active
vertical samples, max3.728394 yards, landing and zero velocity errors. Server
survived; reserved actor returned with a live location confirmation. All19 native
CTests and five Python tests pass. These observations do not certify every quest,
client animation or long-duration load. No world migration or logging filter change.
