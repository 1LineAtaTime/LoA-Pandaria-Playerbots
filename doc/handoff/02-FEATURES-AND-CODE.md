# Feature and source map

Paths below are relative to ``. The CSV in `changes/` is the full
file inventory; this guide highlights the implementation boundaries.

## BattlePay

Primary code: `src/server/game/BattlePay/`, the BattlePay network handler,
`src/server/database/Database/Implementation/LoginDatabase.cpp` and
`CharacterDatabase.cpp`. Read `doc/battlepay-accounting.md` for detailed invariants.

- The authenticated account's `account.dp` is the wallet. One stored unit is one
  displayed DP. Catalog prices are whole DP; only the client wire format scales
  by 10000. Do not rescale existing balances or trust an echoed client price.
- Exact integer pricing/discounts, sufficient-funds debit guards, checked affected
  rows and schema/engine checks fail closed on invalid or unavailable data.
- Durable quote/purchase snapshots, character delivery receipts and completion
  receipts support retry/recovery without blindly charging or mailing twice.
  Auth and character databases are separate transactional boundaries, so receipt
  fencing matters; this is not a single cross-database transaction claim.
- Pending confirmations can be cancelled without blocking the shop for five
  minutes. Insufficient-funds handling was repaired to show the native client
  failure rather than leaving the connecting overlay stuck. The balance tile's
  crossed-out zero and unwanted balance chat text were also addressed.
- Boost entitlement reservation/application has separate recovery receipts.
  Physical nonstackable donation tokens have durable credit tracking.
  Stackable token redemption is intentionally refused; see known limitations.

Required schema work is listed in `database/README.md`. The optional catalogue
contains products, item mappings, categories, entries and localization only.
It contains no DP balances or transaction/delivery history.

## Auction house market

Code: `src/server/game/AuctionHouseBot/`. Main additions include
`AuctionHouseBotItemFilter.h`, owner selection in buyer/seller code and filters
shared by forced includes. See `doc/auction-house-bots.md`.

- `AuctionHouseBot.UseRandomPlayerbots` uses existing random-bot character GUIDs
  as visible sellers/bidders, respecting the auction house's faction.
- A bot may own/bid on multiple items; this is not a one-identity-per-item model.
- The underlying market simulator creates stock and evaluates bids. It is not a
  claim that each visible bot independently walked to an auctioneer and sold
  loot from its bags. City errands and market accounting are separate systems.
- Whole-word TEST/DEPRECATED/UNUSED/DEBUG/PLACEHOLDER/DNT/DNU/QA/NYI and other
  explicit do-not-use name markers are filtered. Legitimate substrings survive.
- Required level 100+ can mark unsuitable items. The recommended max required
  level is 99; item level remains a separate control, allowing real MoP gear.
- Legacy unbid bot listings can expire through normal auction processing.
  Listings with real bids must not be indiscriminately removed.

No live auctions, auction mail or bot account population is exported. Bot
identities are generated/configured by the recipient, not seeded from this server.

## Player companions, addon and city activity

Code: `modules/mod_playerbots/src/Manager/{CompanionAccess.h,CompanionPolicy.h,
PlayersbotsAddon.cpp,PlayerbotMgr.cpp,PlayerbotSecurity.cpp}`, `cs_playerbots.cpp`,
AI and strategy/action files. Configuration: `modules/mod_playerbots/config/`.

- Server-side companion minimum level, security rank, GM bypass and maximum
  owned/pending companions; RBAC remains an additional access check.
- `.bot` dispatch supports the Playersbots addon's role/group/control commands.
  Pending login, invitation, master assignment and control paths enforce policy.
  The summon addon patch changes that button to `.bot summon`, not privileged
  unrestricted GM summoning. The optional PowerShell patcher is included; the
  third-party addon/client and its saved settings are not redistributed.
- Grouped human companions can use gold-only corpse looting, pass rolls and
  suppress greetings through configuration. Do not confuse this policy with
  autonomous random bots' own questing/inventory requirements.
- Follow/travel and login fixes address companion persistence and initialization
  problems. They do not certify every flying/instance/transport path.
- CityLife adds service-NPC visits, pauses, target reservations and target
  abandonment for ungrouped bots. Presence depends on available bots and valid
  hubs/navigation. Crowding and terrain behavior remain open visual concerns.
- Existing class combat/healing/buff strategies are retained. Complete
  party-wide healing/buff and dungeon/raid behavior are not blanket-certified.

## Quest, spell and world repairs

Data: `sql/updates/world/`. Native changes are primarily in
`src/server/scripts/` plus core quest, gossip, phase, movement and spell handlers.

| Area | Examples of paired repairs |
|---|---|
| Wandering Isle | Jaomin Ro quest 29409 competing AI/faction ownership; bridge questgiver phase/aura boundaries |
| Undead starting area | Initial questgiver availability, recurring Rigor Mortis area application, questgiver interaction/phase problems |
| Gilneas | Starting quest/rogue training credit and spawn duplication; Crowley horse/actor ownership; terrain phase selection and cage transition; rescued-child click bindings; manor ride/retry; quest prerequisite masks |
| Worgen character systems | Controlled recovery of invalid pre-release cage positions; Two Forms reward chain; flooded-phase liquid/breathing lookup; consumable and collection-query checks |
| General content | Imported/adapted SkyFire quest, class spell, encounter, loot, service, taxi and world content batches; script binding/registration reconciliation |
| Core | Character creation roster behavior, account-achievement restore/mail duplication, creature AI/combat policies, spell cast-failure packet argument layout, object reference and movement input guards |

The Three children/horse/phase tests include controlled fixtures; they are not
natural full-chain proof for all classes. Two Forms is learned through the
quest/reward chain (24593 -> 72829 -> 72857 -> 68996), not granted to every new
Worgen at creation. Do not fix a particular saved player by granting the racial
spell and call the whole quest line repaired.

## Database/build/navigation compatibility

Code: `src/server/database/Database/`, `cmake/macros/FindMySQL.cmake`,
`cmake/compiler/msvc/settings.cmake`, `cmake/genrev.cmake`, Windows staging/tools
and connector/transaction tests. Version handling distinguishes MariaDB from
MySQL rather than applying a simplistic single version comparison. Checked
transactions and connection behavior retain explicit failure handling.

`Stage-Runtime.ps1` follows non-system DLL dependencies; connector DLLs must match
the compiled library and platform. No dependency DLLs are distributed here.
Historical probe coverage includes MySQL 5.5.62 and MariaDB 10.5.29; that is not
a guarantee for every newer server/client version.

Navigation includes the scoped Detour null-detail-edge fallback and input/grid
checks. Read `doc/navmesh-crash-2026-10-30.md` before changing it. Fixing a native
null dereference does not prove that extracted MMaps are complete or correct.
No map/DBC assets are included; use matching extraction tools and client data.
