**Mists of Pandaria 5.4.8 (build 18414) with playerbots**

A Mists of Pandaria 5.4.8 server core with a full playerbots module: a world populated by
hundreds of AI-controlled player characters that quest, group, run dungeons, queue for
battlegrounds and arenas, and talk, alongside real players.

This fork builds on the Legends of Azeroth Pandaria 5.4.8 core. See [THANKS](THANKS.md) for its
authors and contributors.

## What this fork adds

**Playerbots ported to MoP 5.4.8**
+ Random bots that live in the world: roaming, questing (including the Cataclysm and Pandaria
  zone chains), hunting, gathering, travelling by flight path, boat, zeppelin and portal.
+ Groups with real players: follow, assist, loot, ready checks, master/leader commands, and
  bots that follow their group onto boats, zeppelins, elevators and other moving transports.
+ Dungeon finder with bots: bots queue, accept and run Classic through Mists of Pandaria
  5-player dungeons, with encounter strategies for the bosses that need them.
+ Battlegrounds and arenas with bots: objective play in every battleground (flags, bases,
  carts, orbs, siege vehicles), rated battlegrounds, skirmish and rated arenas.
+ Vehicles and transports: bots drive and ride vehicles where a battleground or quest needs it.
+ Class rotations for every class and specialisation of MoP, including the Monk, with PvE and
  PvP priorities, interrupts, defensives and pet control.
+ Bot chat: greetings, reactions, battleground and group chatter, and duels around the cities.

**Performance**
+ Reworked bot tick scheduling and per-bot update delays: with about 1000 bots online the world
  update averages a few milliseconds (median 2 ms in our 1000-bot runs), at no extra CPU cost.
+ `.server info` prints the update-time statistics of the last 500 world updates (mean, median,
  95th and 99th percentile, max), so the cost of a bot population can be measured directly.

**Testing and diagnostics**
+ A headless client (`contrib/headless-client/`): a scriptable, graphics-free 5.4.8 client that
  logs in as a real player, so grouping, queues, duels, quests and transports can be tested
  automatically. See its README.
+ GM test commands for bots (`.botscenario`, in `modules/mod_playerbots/src/Scenario/`) to place,
  group, queue and probe bots in reproducible scenarios.
+ Playerbots logging and diagnostics: per-feature log channels, performance monitor counters,
  and per-bot debug output that can be switched on from the config.

## Requirements
+ Platform: Linux, Windows or Mac
+ AMD64 or AArch64 CPU architecture
+ MySQL = 5.7 / 8.0-8.1
+ CMake ≥ 3.27.2
+ OpenSSL = 1.1.1 / 3.0-3.1.1 (3.2.0 not supported) (MySQL >= 8.0.33)
+ Boost ≥ 1.85.0_msvc 14.3 x64 / 1.81 above Linux
+ Windows SDK version 10.0.22621
+ MS Visual Studio (Community) ≥ 16.4 (2019) (Desktop) (Not previews)
+ GCC >= 13.0, Clang >= 12.0
+ A 5.4.8 (build 18414) game client, enUS locale, for the data extraction below.

## Latest builds (CI)
This branch has no automatic builds. Untested development builds of the `playerbot-beta`
branch (Windows x64, Linux x64, macOS arm64 and a database with every SQL update applied)
are published at most once a day:
**[latest playerbot-beta build](../../releases/tag/ci-playerbot-beta)**.
For a tested package, use the
[releases](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases).

The beta pre-release is optional. It has five assets:

+ `playerbot-beta-ci-win64.zip`, `playerbot-beta-ci-linux-x64.zip`,
  `playerbot-beta-ci-macos-arm64.zip`: the binaries.
+ `playerbot-beta-ci-database.zip`: full dumps (`auth`, `characters`, `world`, `playerbots`)
  with every update up to that commit applied, for a **new install only**.
+ `playerbot-beta-ci-sql.zip`: the `sql-sources` folder the built-in updater reads, for
  **upgrading** an install you play on.

Downloading a run's Actions artifact gives you a GitHub wrapper zip with both database files
inside; use the release assets instead.

A fresh install takes every package (server, extractors, database) from the same release.
Each package is complete on its own: never unpack one over another, or a development build
over a release.

## Getting started

### 1. Binaries
Each release has Windows x64 assets:

+ `server`: `worldserver.exe`, `authserver.exe`, the runtime DLLs and the `.conf.dist` files.
+ `extractors`: `mapextractor.exe`, `vmap4extractor.exe`, `vmap4assembler.exe`,
  `mmaps_generator.exe`.
+ `database`: `world.sql` (complete world database) and `auth.sql`, `characters.sql`,
  `playerbots.sql` (schemas with the static data they need; no accounts or characters).
+ `symbols`: `.pdb` debug symbols, optional, for readable crash reports.

The binaries need the Microsoft Visual C++ 2015-2022 Redistributable (x64) and a 64-bit
MySQL 8.0 server.

On other platforms (or to build yourself), build from source with CMake.

### 2. Game data
No game data is shipped. Extract it from your own 5.4.8 client with the bundled tools, run
from the client folder:

1. `mapextractor.exe` writes `dbc/`, `maps/` and `cameras/`.
2. `vmap4extractor.exe`, then `vmap4assembler.exe Buildings vmaps`, writes `vmaps/`.
3. Create an empty `mmaps/` folder, then run `mmaps_generator.exe`. Bots need the movement
   maps: they path with them everywhere. Generation takes several hours.

Copy `dbc`, `maps`, `vmaps`, `mmaps` and `cameras` into one folder and point `DataDir` in
`worldserver.conf` at it. Bots need the enUS DBC files.

### 3. Databases
For a new install: create four empty databases (for example `auth`, `characters`, `world`,
`playerbots`), give one MySQL user access to all four, and import the four dumps
of the release's `database` zip:

```
mysql -u<user> -p auth       < auth.sql
mysql -u<user> -p characters < characters.sql
mysql -u<user> -p world      < world.sql
mysql -u<user> -p playerbots < playerbots.sql
```

Leave `Updates.EnableDatabases = 0` (the shipped default). The auth, characters and playerbots
databases fill themselves on the first start: the bot accounts, characters and bot data are
created by the worldserver.

To upgrade an install you already play on, import nothing: see
[6. Upgrading an existing install](#6-upgrading-an-existing-install-keeps-characters-accounts-and-bot-data).

### 4. Configuration
Copy the `.conf.dist` files next to the binaries and drop the `.dist` suffix:

+ `authserver.conf`: `LoginDatabaseInfo`.
+ `worldserver.conf`: `LoginDatabaseInfo`, `WorldDatabaseInfo`, `CharacterDatabaseInfo`,
  `PlayerbotsDatabaseInfo`, `DataDir`, `LogsDir`.
+ `playerbots.conf` (placed next to `worldserver.conf`): the same `PlayerbotsDatabaseInfo`, and
  the bot population, e.g. `AiPlayerbot.MinRandomBots`, `AiPlayerbot.MaxRandomBots` and
  `AiPlayerbot.RandomBotAccountCount`. `AiPlayerbot.Enabled = 0` turns the module off.

Database updates: the dumps are complete, so `Updates.EnableDatabases = 0` (the shipped
default) is fine for a new install. To upgrade an existing install, see
[6. Upgrading an existing install](#6-upgrading-an-existing-install-keeps-characters-accounts-and-bot-data).

To drive the server from scripts (the headless client's GM verbs use it), enable SOAP in
`worldserver.conf` (`SOAP.Enabled`, `SOAP.IP`, `SOAP.Port`).

### 5. Run
Start `authserver`, then `worldserver`. The first start with bots enabled creates the bot
accounts and characters and takes a while; let it finish. Create your own account from the
worldserver console (`account create <name> <password>`, then `account set gmlevel <name> 3 -1`
for a GM). The realm address is `127.0.0.1`; change it in the `realmlist` table of the auth
database if clients connect from other machines.

### 6. Upgrading an existing install (keeps characters, accounts and bot data)
1. Back up all four databases (`mysqldump`).
2. Replace the binaries. Keep your `.conf` files and copy any new keys from the new
   `.conf.dist` files.
3. Extract the SQL sources (the `Source code` zip of the release you upgrade to) and set, in
   `worldserver.conf`: `Updates.EnableDatabases = 7` (1 auth + 2 characters + 4 world; 15
   with 8 playerbots from v1.1.0-beta on), `SourceDirectory` = the extracted folder that
   contains `sql/` and `modules/` (the top folder of the `Source code` zip),
   `MySQLExecutable` = your `mysql` client (`mysql.exe` on Windows). With v1.0.x binaries,
   apply new files from `modules/mod_playerbots/data/sql/playerbots/updates` to the
   playerbots database by hand.
4. Start the worldserver once: the updater applies only the missing updates, in order, and
   records them.

   **4b.** Before playing, check the worldserver log: `>> Applying update "<file>"` for each new
   file (`>> Re-hashing update` for files already applied), `>> ... database is up-to-date!`
   or `>> Applied N queries.` for each database, and no errors. If anything fails, stop the
   server and restore the backup from step 1.
5. Never import `characters.sql`, `auth.sql` or `playerbots.sql` from a database zip over an
   existing install: they replace your saves. Replacing `world.sql` alone is safe for player
   progress but drops your own world edits.

Read the release notes too: some releases list extra upgrade steps (e.g. from v1.0.x).

## Copyright
License: GPL 2.0

Read file [COPYING](COPYING.md)

## Authors &amp; Contributors
Read file [THANKS](THANKS.md)
