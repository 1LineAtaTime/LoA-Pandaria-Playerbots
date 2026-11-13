# Existing MoP database deployment (2026-10-28)

The production MariaDB endpoint is `10.0.0.158:3306`. `exports/worldserver.conf`
uses `mop_auth`, `mop_characters`, `mop_world`, `mop_log`, and the dedicated
`mop_playerbots` schema. Credentials remain in the ignored local config.
The Forgejo push is pending at the user's request.

Before migration, full read-only dumps of the four existing schemas were saved
under ignored `out/compat-test` and imported into isolated `mop_test_data_*`
schemas. Existing production account, character, and DP data was preserved.
example account (`example-player`) had 745 DP after migration. The original production
counts were 531 accounts and 312 characters. The later two-bot test added only
`RNDBOT0` and `RNDBOT1` and 22 characters to those counts.

The auth and characters compatibility SQL and BattlePay accounting migrations
were applied to production. `mop_playerbots` contains the module's 28 tables.
Production World received 27 previously absent tables from the validated local
copy, including broadcast text and quest companion data. The quest column
migration retained legacy source fields; other World columns were backfilled
from those fields. `mop_world.quest_template` still contains 18,145 rows.
The World conversion SQL is tracked in
`sql/compat/emucoach/world_quest_preserving.sql` and
`sql/compat/emucoach/world_columns_preserving.sql`. Both are one-time migrations
for the original EmuCoach layout, not repeatable updates. The auth account
default is in `sql/updates/auth/2026_10_28_02_playerbots_account_default.sql`.
The three shipped name tables were imported from
`modules/mod_playerbots/data/sql/characters` into `mop_characters`.
The previously empty `mop_auth.build_info` table caused authserver to reject
client build 18414 before checking credentials. The shipped 5.4.8 row was
added through `sql/updates/auth/2026_10_28_03_mop_client_build.sql`. Account
506's password hash was not changed. Its username login then authenticated,
retrieved characters, and completed a character login in the user's client.

The latest Visual Studio 2022 build produced `exports/worldserver.exe` with
SHA-256 `85B765B1F8CF49D313A1121002012E4568B5CA718039102F908CF2F2E47E2C8B`.
`authserver.exe` and the map, vmap, and mmap utilities were rebuilt and copied
into the same folder. CMake CLI used the Visual Studio generator because no
Visual Studio MCP was exposed. In the loopback production smoke test, worldserver loaded 615
BattlePay products, 614 valid product items, 11 groups, and 659 entries,
initialized AI Playerbots, and reported “World initialized in 0 minutes 21
seconds.” A later two-bot production run created 22 bot characters and logged
two of them in; their online flags were reset after the smoke process stopped.
The freshly rebuilt worldserver reached full startup in 18 seconds and logged
in the same two production bots. Product 1 is an informational balance entry with an empty item row; its
warning is expected. The existing BattlePay tests passed 32 C++ accounting
checks and 42 isolated database transaction checks. No live purchase, boost,
or in-game delivery was exercised.

The staged 1.8 map files were replaced with 9,691 version 10 maps extracted
from the user's MoP 5.4.8 client. `GameObjectModels.dtree` was converted to
VMAP_4.3. Original maps and model list remain under ignored `out/compat-test`.
The production smoke test used these assets successfully.

The export config uses absolute data and log paths, disables locale loading
because this older World database lacks newer locale tables, and starts with
two bots. The original 500-bot default is not used on first launch. The
missing LFG debug key and other reported defaults were added to the local
ignored export config. Console output is fatal-only; detailed messages stay
in `exports/logs/Server.log` and `DBErrors.log`. Remaining validation is
live BattlePay delivery.
Existing invalid game object and SmartAI data warnings did not prevent
initialization.

On 2026-10-28, the user's `example-player` login reached the realm and character
list. ExampleCharacter initially reached the world but was disconnected by the core's
AntiDOS rule. This 5.4.8 client sends a burst of `CMSG_REQUEST_HOTFIX` packets
during world entry, while the old limit allowed one per second. The limit for
that opcode was raised to 100 per second; the other AntiDOS limits and kick
policy remain in place. The user confirmed ExampleCharacter stayed in the world after
the rebuilt `worldserver.exe` was deployed. The character database showed
ExampleCharacter online, and the server log recorded no further AntiDOS kick. The
account password hash was not changed.

The local export configuration now welcomes players to `example.invalid`
in both MOTD and player-start text. The AuctionHouseBot seller and buyer
controls used by this build were enabled. The configured GUID 274539 belongs
to `Auctionbot` on account 1. The source now honors that GUID, so the bot no
longer posts auctions from the account's other characters. Auctionbot's active
listing count rose from 156 to more than 5,900 while the other account 1
characters stopped receiving new bot listings. Temporary network tracing was
returned to warning level at runtime and in the export config; startup errors
remain in file logs rather than the console. Twelve missing Solocraft config
defaults were added locally. Remaining validation is live BattlePay delivery.
