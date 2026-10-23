# Headless client

`headless.py` is a minimal, scriptable World of Warcraft 5.4.8 (build 18414) client written in
plain Python 3 (standard library only). It logs in over real TCP sockets exactly like the game
client: SRP6 against the auth server, the world-server handshake, character selection and
`CMSG_PLAYER_LOGIN`. The server therefore treats it as an ordinary player session, which makes
it useful for automated tests of anything that needs a real player: grouping with bots, LFG and
battleground queues, duels, trades, quests, transports and so on.

It has no graphics and no game data. It only speaks the protocol.

## Requirements

- Python 3.8 or newer.
- A running authserver and worldserver built from this repository.
- A game account with at least one character (or use `--cmd create` to make one).
- For the GM verbs (see below): SOAP enabled in `worldserver.conf` (`SOAP.Enabled = 1`,
  `SOAP.IP`, `SOAP.Port`) and an account with GM level 3 (administrator) for SOAP.

## Pointing it at a server

Every connection setting can be given on the command line or through an environment variable.
There is no built-in account or password.

| Option | Environment variable | Default |
|---|---|---|
| `--host` | `MOP_HOST` | `127.0.0.1` |
| `--auth-port` | `MOP_AUTH_PORT` | `3724` |
| `--world-port` | `MOP_WORLD_PORT` | `8085` |
| `--realm` | | `1` |
| `--account` | `MOP_ACCOUNT` | none (required) |
| `--password` | `MOP_PASSWORD` | none (required) |
| `--char` | `MOP_CHAR_GUID` | `auto` (first character on the account) |
| `--soap-host` | `MOP_SOAP_HOST` | `127.0.0.1` |
| `--soap-port` | `MOP_SOAP_PORT` | `7878` |
| `--soap-user` | `MOP_SOAP_USER` | none (needed only for SOAP verbs) |
| `--soap-pass` | `MOP_SOAP_PASSWORD` | none (needed only for SOAP verbs) |
| `--server-log` | `MOP_SERVER_LOG` | `Server.log` (read by `wait-for-server-pos`) |

Precedence is: command line, then environment variable, then an optional local JSON file, then
the default above. The JSON file is `headless.local.json` next to the script, or the path in
`MOP_HEADLESS_DEFAULTS`. It holds the same keys in lower case (`account`, `password`, `host`,
`auth_port`, `world_port`, `soap_host`, `soap_port`, `soap_user`, `soap_pass`, `server_log`).
It is ignored by git; keep credentials out of version control.

## Usage

```
# auth handshake only
python headless.py --account TESTACC --password PASS --cmd test

# log in, stay in the world for 60 s
python headless.py --account TESTACC --password PASS --cmd play --hold 60

# queue for Warsong Gulch (battleground type 2), accept the invite, stay 5 minutes
python headless.py --cmd play --queue 2 --accept --hold 300

# create a character from the character-select screen
python headless.py --cmd create --create-name Testone --create-race 2 --create-class 1

# log in and run steps in order
python headless.py --cmd play --hold 120 \
    --step "invite Somebot" --step "wait-for group members=2 timeout=30" --step "say hello"

# same, steps read from a file (one verb line per line)
python headless.py --cmd play --script steps.txt
```

`--cmd` is one of `test` (auth only), `login` (auth + world handshake), `play` (log a character
in), `create` (create a character) and `botchecks` (log in and record bot interactions; with
`--duel-guid` and `--events-out`). Other useful options: `--trace` logs every packet,
`--invite NAME` invites a character before anything else, `--bf-accept` accepts Wintergrasp and
Tol Barad battlefield invites, `--max-session N` caps a scripted session at N seconds, and
`--log-movement all|units|names` logs every movement packet received for other units as
`MOVEPKT` lines (`--log-movement-out FILE` writes them to a file).

Output is line based and meant to be grepped: `LOGIN-OK`, `LOGIN-MS`, `step: ...`, one line per
observed event, and a JSON summary of battleground world states at the end of a battleground
session. Exit code 0 means the session ran to the end; non-zero means a login failure, a failed
gate (`wait-for-server-pos`) or a session timeout.

## Step verbs

A step is one line: a verb followed by its arguments. `npc` in an argument means the guid cached
by the last `npcguid` verb. The exact argument forms are documented in comments next to each
verb in `headless.py`.

| Area | Verbs |
|---|---|
| Waiting and control | `wait-for group members=N \| map=ID \| evt=KIND [timeout=S]`, `wait-for-server-pos NAME x,y,z [tol=] [timeout=]`, `wait-file PATH [timeout=S]`, `soap COMMAND...` |
| Targeting and group | `select`, `target GUID`, `invite NAME`, `accept`, `leave` / `disband`, `uninvite`, `leader`, `loot-method`, `difficulty normal\|heroic`, `raid`, `icon`, `ready-check`, `ready` |
| Chat | `say`, `party`, `raid-chat`, `whisper NAME TEXT` |
| Movement | `setpos`, `goto`, `walk [speed=] x,y,z ...`, `mover NPC\|off`, `face`, `stop`, `mount`, `taxi`, `areatrigger ID` |
| Transports | `board-walk ENTRY [seat]`, `board-natural`, `leave-transport` |
| Quests and NPCs | `quest-accept`, `quest-share`, `quest-complete`, `quest-reward`, `gossip-hello NPC`, `gossip-select ID\|text=SUBSTR [code=] [guid=] [menu=]`, `faction-select horde\|alliance`, `spellclick NPC`, `gameobject-use` / `gameobject-report-use GUID\|entry:spawnId` |
| Loot and trade | `loot`, `loot-all [seconds]`, `roll`, `master-give`, `trade-open`, `trade-item`, `trade-gold`, `trade-accept`, `trade-cancel` |
| Combat and spells | `attack NPC`, `attack-stop`, `cast SPELL [target]`, `cast-at SPELL x y z`, `cast-at-near SPELL ENTRY`, `use-item BAG SLOT`, `use-item-at BAG SLOT x y z`, `use-item-on ...`, `duel NAME`, `duel-accept [decline]` |
| Death | `die`, `release`, `reclaim`, `res-accept`, `summon-accept` |
| Dungeon finder | `lfg-join`, `lfg-roles`, `lfg-accept`, `lfg-teleport`, `lfg-leave` |
| Battlegrounds and arenas | `bg-join`, `bg-accept`, `bg-leave`, `bg-queue-leave`, `arena-join SLOT [rated]` |
| Character | `create NAME RACE CLASS [gender]` |
| GM verbs (over SOAP) | `npcguid`, `goguid`, `nearcorpse`, `kill`, `board`, `equip`, `learn`, `masterprobe`, `castself`, `cansee`, `tele-near MAP ENTRY [off=]` |

The GM verbs are sent to the worldserver as `.botscenario <this character> <verb> ...` through
SOAP, so they need the SOAP settings above. `.botscenario` is the playerbots test command set
(`modules/mod_playerbots/src/Scenario/`); it is a GM-only debugging aid. `npcguid`, `goguid` and
`nearcorpse` replies are parsed for `guid=<n>` and cached for later verbs.

## Logging out

By default the character stays in the world until the server times the socket out, so a test
driver can keep using it after the script exits. Set `HEADLESS_LOGOUT=1` to send a clean logout
when the session ends.
