# MoP Playerbots implementation status

Updated 2026-10-29. This repository targets the existing 5.4.8 client and
MariaDB schemas; production configuration and credentials stay in ignored
`exports/*.conf` files.

## Implementation map

| System | Connected source and data | Current state |
| --- | --- | --- |
| BattlePay | `src/server/game/BattlePay`, `Handlers/BattlePayHandler.cpp`, `BattlePayAccounting.h`, `BattlePayQueries.h`; `mop_world.battle_pay_*`, `mop_auth.account.dp` and purchase tables | Native shop and durable purchase/delivery flow are integrated. `account.dp` now stores whole DP, with 10000 precision only in client packets. |
| AuctionHouseBot | `src/server/game/AuctionHouseBot`, `World.cpp` update timer; `mop_characters.auctionhousebot` | Buyer and seller run periodically. New simulated auctions use faction-matched RNDBOT characters; legacy character GUID 274539 remains recognized. Restricted item names are excluded. |
| Playerbots and duels | `modules/mod_playerbots`, especially `NewRpgDuelSpot.cpp`, `PlayerbotAIConfig.cpp`, `RandomPlayerbotMgr.cpp`; `mop_playerbots` | The existing duel-spot and class AI code is connected. Production export config now targets 30 online bots and both capital gates. |
| Build and server | Visual Studio 2022 CMake build, `exports/worldserver.exe`, `exports/authserver.exe` | Both daemons start from `exports`; executable files are local artifacts, not committed. |

## Verification

- BattlePay accounting executable: 39 checks passed. Isolated MariaDB charge,
  refund, replay, and delivery tests: 44 checks passed.
- Visual Studio 2022 `RelWithDebInfo` worldserver build succeeded.
- Live example account purchase of product 4401 completed: raw DP decreased from
  745 to 730 for a 15 DP item, `battlepay_purchase` reached state 2, and the
  character delivery receipt reached state 1. Earlier quotes made with the old
  scaled-price build were cancelled without a debit.
- The store's existing product 1 is a non-purchasable `Your balance` tile. It
  displays the raw account balance through native client price precision. The
  user confirmed its crossed-out zero is gone.
- With example account at 16 DP, a 20 DP purchase now shows the client's native
  insufficient-funds error. The client's response reader expects purchase ID,
  client token, then result code; correcting that packet order cleared the
  stuck `Connecting to shop` overlay. The failed attempt made no charge or
  delivery. The user also made a successful 15 DP purchase, which delivered;
  the temporary wallet balance was then restored to 16 DP.
- The 5.4.8 client sends a zero wire price when the confirmation page is closed.
  The server now accepts that cancellation only with the outstanding server
  token, while a completed Buy still requires the exact quoted price. The user
  confirmed a new purchase can start immediately after closing confirmation;
  cancelled purchase 13 reached state 3 within three seconds and the wallet
  stayed at 16 DP.
- The legacy AuctionHouseBot owner GUID is limited to `Auctionbot` rather than
  every character on admin account 1. Buyer/seller operation and auction
  persistence were observed. Player bidding, buyout, and settlement still
  require an end-to-end client check.
- With random playerbot auction actors enabled, worldserver loaded 88 characters
  (42 Alliance, 46 Horde). The first live cycle created 40 listings with 32
  distinct RNDBOT owners. It expired 121 unbid deprecated listings from the
  legacy Auctionbot; the auction table then had zero names matching the
  restricted markers. Multiple RNDBOT characters then bid and bought out
  ExampleCharacter's Windwool Cloth auctions, and successful-sale mail rows were created.
  Client receipt after the configured mail delay remains to be checked.
- A prior two-bot smoke test succeeded. With the new export configuration,
  worldserver initialized eight bot accounts with 88 characters and 30 were
  online. Duel-spot seeding placed bots near Stormwind and Orgrimmar. An actual
  duel fight and result still need observation.
- ExampleCharacter's world-entry disconnect was traced to the hotfix-request AntiDOS
  limit and fixed; the user confirmed a stable in-world session afterward.
- Forgejo `develop` and the default `main` branch are kept at the same validated
  commit; the private GitHub push mirror is checked after each push.

## Remaining work

- Restore example account's original 730 DP when the user finishes insufficient-funds
  testing and authorizes the end of the temporary 16 DP test state.
- Observe AI duels at both capital gates, including combat, completion, and
  recovery. Tune the population or movement only from observed behavior.
- Verify one player purchase and one buyer transaction at the auction house.
