# Fresh LoA restoration — November 7, 2026

## Selected baseline

The user requested returning to the customized 1LineAtaTime/LoA installation,
using a fresh game rather than continuing the clean SkyFire core migration.

The restored source is `codex/city-companions` milestone
`4aca50c794d2affbc5d6d105eff1244675b39a5c`, immediately before the cumulative
SFDB27 world conversion campaign. This is the customized LoA core, descended
from original upstream `e135742e1891d3e653d27966da2dc70e793f203e`.
It retains the previously integrated targeted fixes and native content ports;
it is not a claim to restore an untouched upstream ZIP or flawless gameplay.
The later clean-SF and cumulative-conversion work remains in its existing Git
branches and database schemas for inspection and rollback.

The world source is the complete pre-conversion backup made at 00:15 on
November 7: `out/cumulative-before-20261107-001511/mop_world.sql`, 429,781,116
bytes, SHA256 `d475d6c352437dd1d7174db43da113931d34af4942919cea37763b3471a5ebbb`.
It contains the LoA world and custom shop, rather than the subsequently converted
SF world. Its provenance is recorded in
[the cumulative deployment report](https://example.invalid/private-repository)
in the preserved development history. No old account/character dump is imported.

## Fresh database layout

| Schema | Initial contents |
| --- | --- |
| `mop_loa_auth` | Matching table definitions, allowlisted RBAC/build/realm/reference data, newly created ADMIN |
| `mop_loa_characters` | Matching definitions and bot name reference lists; no old characters, mail, items or auctions |
| `mop_loa_world` | Complete pinned pre-conversion LoA world, including 615 shop products and their product-item records |
| `mop_loa_playerbots` | Matching definitions and public navigation/name-independent strategy/reference data; fresh runtime state and caches |
| `mop_loa_log` | Empty matching archive table definitions |

The old `mop_` and `mop_sf_` schemas are preserved. New `mop_loa_server` database
credentials grant access only to the five new schemas, for the local/client
addresses used by this installation. Secrets stay in ignored runtime files.
The new ADMIN has rank 3 across realms and explicit administrator RBAC role 192.
Its private credentials are in `exports/ADMIN-CREDENTIALS.txt`; this does not
change any password or account in the previous games.

`contrib/core-rebase/restore_loa_fresh.py` documents and implements the allowlisted
fresh import. It verifies the world backup hash, refuses existing target schemas,
streams the world dump with its original CREATE DATABASE/USE headers removed,
and never imports personal state. It needs the locally retained backup and a
compatible existing schema source; it is a recovery tool, not yet a standalone
redistributable installer. An interrupted provisioning operation must be inspected
and continued deliberately, not rerun over existing databases.

## Configuration and custom features

The active root `exports/worldserver.conf` points Login, World, Character,
Archive and Playerbots database connections at the new prefix. Authserver uses
`mop_loa_auth`. `DataDir = "."` and `LogsDir = "./logs"` remain portable.
The old LoA client-data format in root `exports` is retained; the independently
generated clean-SF assets in `exports/sf-clean` are not used by these binaries.
Both launchers start direct, visible server consoles and exit their wrappers.

The previous configuration is retained, including 80 random-bot accounts,
190–210 online random bots, addon/companion settings, city activity, gold-only
companion loot and quiet greetings. Fresh bot accounts/characters are generated
on first startup. Auction fallback Account/GUID are reset to zero because those
old IDs do not exist in the fresh game; random-playerbot actors remain enabled.

Existing feature documentation and matching SQL remain tracked:

- [BattlePay accounting](battlepay-accounting.md): wallet in `auth.account.dp`,
  purchase/delivery recovery and auth/character migration paths. Fresh accounts
  start at zero DP; old wallets and transactions are not copied.
- [Auction house bots](auction-house-bots.md): randomized character owners,
  buyer/seller behavior and broken-item/required-level filters.
- [City and addon companions](city-companions.md): addon commands, permission
  1346, level/security restrictions and city activity.
- [Companion loot/status](companion-loot-and-status.md) and
  [database compatibility](database-compatibility.md).

The fresh schema definitions already include the deployed accounting/permission
migrations; do not blindly replay historical ALTER statements over them.
The checked source contains the original migrations for review and installation
against their documented starting schemas.

## Build and validation

Visual Studio rebuilt the x64 RelWithDebInfo LoA solution with Playerbots,
modules and scripts enabled. An initial solution build retained a stale project
from the later checkout; reopening the regenerated solution removed that stale
reference. The subsequent complete build succeeded with zero failed projects.
All 22 registered native CTests passed.

Executable/PDB pairs and their resolved DLLs were staged through
`contrib/windows/Stage-Runtime.ps1`. Runtime/config backups, private provisioning
credentials, staging hashes and the rewritten import remain under ignored
`out/loa-fresh-20261107`. Both servers are launched from root `exports`.

Startup, real SRP/world login, generated bot population and market/store loading
are checked separately below. The restored baseline has documented pre-existing
content diagnostics. Logging remains enabled; restoring it is not a claim that
every quest, spell or warning has been fixed. BattlePay/AH/companions remain
implemented, but the new game also needs normal client acceptance testing.

### Observed acceptance

- Authserver PID 2292 listens on 3724; its lifecycle log says ready for
  authentication against realm ExampleRealm LoA.
- Worldserver PID 42836 listens on 8085. First fresh startup took 1 minute
  44 seconds, including generating 80 accounts and 880 bot characters.
- Approximately 190 random bots are online. AuctionHouseBot resolved 880
  actors, including 423 Alliance and 405 Horde characters. It stocked 3,000
  auctions with 606 distinct character owners during the first market cycle.
- The shop loaded 615 products, 614 deliverable product-item records, 11 groups
  and 659 catalogue entries. Product 1 is the non-deliverable balance tile.
- Real SRP authentication succeeded for ADMIN. A disposable normal account
  authenticated, created a human rogue and entered map 0 through the native
  world protocol. Its account and character were subsequently removed using
  native console commands. This is login acceptance, not a quest walkthrough.
- No historical purchases were imported. ADMIN retains zero starting DP,
  administrator rank 3 and role 192.
- Initial startup/runtime diagnostics include 1,416 DB error records and
  additional bot action-bar/content records. These are retained in the diagnostic
  logs. They are not suppressed or represented as repaired by this restoration.

The source restoration is published as an explicit forward merge on the existing
repository history. The original feature development, subsequent SF experiment
and this recovery remain reachable; no reset or force push is used.
