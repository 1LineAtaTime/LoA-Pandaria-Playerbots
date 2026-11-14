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

The real-player verbs, which need no GM help, are described in "Playing without GM help" below.
`kill` with no entry, `accept` with no arguments, `use-item BAG SLOT` and `spellclick npc|GUID`
keep their old meanings.

The GM verbs are sent to the worldserver as `.botscenario <this character> <verb> ...` through
SOAP, so they need the SOAP settings above. `.botscenario` is the playerbots test command set
(`modules/mod_playerbots/src/Scenario/`); it is a GM-only debugging aid. `npcguid`, `goguid` and
`nearcorpse` replies are parsed for `guid=<n>` and cached for later verbs.

## Playing without GM help

A second verb set plays the character like a normal player. These verbs never call a GM verb or SOAP,
and never write to a database. Everything the client knows comes from packets the real 5.4.8 client
receives:

- objects from `SMSG_UPDATE_OBJECT`: type, entry, position (moving units follow their
  `SMSG_MONSTER_MOVE` spline), health, level, unit/NPC/dynamic flags. `SMSG_DESTROY_OBJECT` and
  out-of-range blocks remove them;
- the character's own state: level, health, quest log (`PLAYER_FIELD_QUEST_LOG`), and inventory
  (`PLAYER_FIELD_INV_SLOTS` plus the bag slots, so an item entry resolves to its bag and slot);
- known spells (`SMSG_INITIAL_SPELLS`, `SMSG_LEARNED_SPELL`), the pet, charm or vehicle action bar
  (`SMSG_PET_SPELLS_MESSAGE`), and quest, cast and loot replies.

Everything it does goes out as packets a real client sends: movement opcodes at the character's run
speed, `CMSG_QUESTGIVER_*`, gossip, `CMSG_ATTACKSWING`, `CMSG_CAST_SPELL`, `CMSG_USE_ITEM`,
`CMSG_GAMEOBJ_USE`, `CMSG_SPELLCLICK`, `CMSG_PET_ACTION`, loot, release spirit and reclaim corpse.

Movement follows navmesh paths from `navpath/nav.py` (see `navpath/README.md`). If that module is
missing, or it finds no path, the client walks a straight line and logs `NAV-FALLBACK`. Positions
are sent 0.5 yd above the mesh surface. When the server moves the character (teleport, knockback,
vehicle exit), the client computes a new path from the new position.

Every verb logs `OK <verb> ...` when it succeeds. When a verb stops making progress, it logs one
line and the run exits with code 3:

```
STUCK verb=<verb> args=<a,b,...> reason=<why> pos=<map x y z> quest=<id:state(counts)>
```

Causes include no position progress, no path, no visible target, no damage dealt, a failed cast or
use, or a quest that does not reach the expected state. Quest events are logged as
`[QUEST] id=<q> event=accepted|progress i=<a>/<b>|complete|rewarded|failed|removed`. Kills are
logged as `[KILL]`, deaths as `[DEATH]`, and vehicle changes as `[VEHICLE]`.

`ENTRY` is a creature or gameobject template entry. A verb picks the nearest visible object of that
entry. `spellclick` and `use-item on=` prefer an object they have not used yet (`fresh=0` turns
this off). Shared options: `timeout=S` (whole verb), `stuck=S` (no-progress window, default 30),
`quest=Q` (logged with STUCK; ends `kill` early), `range=R` (how close to get), `near=x,y,z`
(where to look when the target is out of view, about 100 yd), `deaths=N` (a `kill` or `collect`
that dies more than N times logs STUCK, default 3) and `dry=N` (`collect from=` logs STUCK after N
kills without the item count changing, default 10). An object that leaves view is remembered at
its last position, so a later verb for the same entry walks back there.

| Verb | What it does |
|---|---|
| `goto-path x,y,z [map=M] [range=R] [swim=1]` | Walk the navmesh path there. `swim=1` sends the swimming movement flag. |
| `goto-npc ENTRY [range=R] [near=x,y,z] [wait=S]` / `goto-go ENTRY [...]` | Walk to the nearest creature / gameobject of that entry and re-target it if it moves. When nothing is in view, walk to `near=`, or to the nearest spawn from the optional read-only DB lookup (below). |
| `kill ENTRY [count=N] [quest=Q] [loot=1]` | Fight the nearest live, attackable unit of ENTRY that no one else has tapped. Approach, face, auto-attack, pet attack, then a starter class rotation built from the known spells. Fights back when attacked, loots, and after a death releases, walks back and reclaims the corpse. Stops after N kills, or when the quest (or this entry's objective) is complete. |
| `collect ITEM count=N from=ENTRY` / `collect ITEM count=N go=ENTRY` | Kill and loot, or use and loot gameobjects (never the same node twice within 90 s), until the bags hold N. |
| `accept ENTRY QUEST [go=1]` / `accept-go ENTRY QUEST` | Walk to the quest giver, `CMSG_QUESTGIVER_HELLO` (or `CMSG_GAMEOBJ_USE` for a gameobject giver), query and accept. Waits until the quest is in the log. |
| `turnin ENTRY QUEST [reward=i] [go=1]` / `turnin-go ENTRY QUEST` | Complete the quest, hand in the requested items, choose reward `i` (0-based) and wait for the reward packet. |
| `choose-reward [i]` | Answer a pending reward offer (for example an auto-complete quest). |
| `talk ENTRY [option=text_substring]` | Gossip hello, then optionally select the option whose text contains the substring (`_` stands for a space). |
| `use-go ENTRY [loot=1]` | Walk to the gameobject, use it (`USE` + `REPORT_USE`) and loot any window that opens. |
| `use-item ITEM [on=ENTRY\|on=go:ENTRY\|at=x,y,z\|self]` | Find the item in the bags and use it, on a unit or gameobject (walks into `range=`, default 5), at a ground point, or on yourself. `use-item BAG SLOT` keeps its old meaning. |
| `equip ITEM` | `CMSG_AUTOEQUIP_ITEM` from the bag slot that holds the item (a right-click equip). |
| `pet-attack [target=ENTRY]`, `pet-follow`, `pet-stay` | Pet or charm commands through `CMSG_PET_ACTION`. With no `target=`, `pet-attack` attacks what is attacking you. |
| `vehicle-exit` | `CMSG_REQUEST_VEHICLE_EXIT`, then wait until you are off the vehicle. When the server gives this client control of a vehicle (`SMSG_CLIENT_CONTROL_UPDATE`), `goto-path` and the other walking verbs drive the vehicle. |
| `relog` | Log out, then log the same character back in within the same run. |
| `sell VENDOR items=a,b` / `sell VENDOR all [except=a,b]` | Walk to the vendor, talk to it and sell those item entries (`CMSG_SELL_ITEM`). The client cannot tell junk from quest items, so list what to keep. |
| `destroy ITEM [count=N]` | Destroy every stack of the item (`CMSG_DESTROY_ITEM`). |
| `spirit-healer [ENTRY]` | As a ghost (it releases first if needed), walk to the spirit healer (default 6491), talk to it and accept the resurrection (`CMSG_SPIRIT_HEALER_ACTIVATE`). |
| `cast-on SPELL ENTRY [range=R] [min=M] [go=1]` | Walk into range (default 20 yd) of the nearest ENTRY, face it, target it and cast SPELL. After a line-of-sight or range failure it moves closer (never nearer than `min=`) and tries again, up to 4 times; then STUCK. |
| `spellclick ENTRY [vehicle=1]` | Walk to the unit and spell-click it. `vehicle=1` also waits until you are seated. |
| `vehicle-cast SLOT\|SPELL [target=ENTRY] [at=x,y,z] [aim=PITCH] [speed=S]` | Cast an action-bar spell of the vehicle. `at=` / `aim=` add the ground target and the elevation and missile-speed fields used by aimed launchers. |
| `pet-cast SLOT\|SPELL [target=ENTRY\|at=x,y,z]` | `CMSG_PET_ACTION` with a button from the pet or charm bar. |
| `hold x,y,z seconds=S [fight=1] [entries=a,b] [radius=R] [until=COND]` | Stay at a spot and fight what attacks you (and any `entries` in `radius`), until S seconds pass or COND is met. |
| `follow ENTRY [range=R] [until=COND] [fight=1]` | Stay within R of a moving NPC (escort, summoned guide). Fights what attacks you or the NPC. |
| `wait-quest QUEST complete\|rewarded\|accepted [timeout=S]`, `wait-level N`, `wait-map MAP`, `vehicle-wait [on\|off]` | Wait for the state from the packets, or STUCK. |
| `quest-log`, `inventory`, `objects [ENTRY]` | Print what the client knows. `inventory` also lists worn items as `[EQUIP] slot=N`. |
| `buy VENDOR ITEM [slot=N] [count=N]` | Open the vendor list (`CMSG_LIST_INVENTORY`) and buy into the backpack (`CMSG_BUY_ITEM`); `slot` is the 1-based vendor list position. |
| `react SECONDS map=KEY:SPELL,... [until=COND]` | Watch incoming chat (whispers, emotes, raid-boss whispers) and cast the mapped spell (self or vehicle bar) when a new line contains KEY (`_` = space). For prompts such as "Use your Blastcrackers!". |
| `abandon QUEST` | Abandon the quest (`CMSG_QUESTLOG_REMOVE_QUEST`); a no-op when it is not in the log. |
| `pet-passive`, `pet-defensive`, `pet-assist` | The pet bar reaction buttons (`CMSG_PET_ACTION`, ACT_REACTION). |
| `cast SPELL pet`, `cast SPELL item=mainhand`/`offhand`/`ITEM` | `cast` on the character's own pet, or with an item target (for example a runeforge rune on the equipped weapon). |

More options: `kill ... until=COND` (ends when the objective is already done), `kill ... nopet=1` (the pet is not sent in),
`kill ... vehicle=1` (keep a vehicle seat; otherwise a seated character steps out first, or logs STUCK `in-vehicle`),
`kill ... deaths=N`. `kill` counts a target that yields with quest credit (a faction swap or "not attackable") or vanishes right
after the credit as a kill, and rests out of combat before the next pull when health is below 60 % (`HEADLESS_REST_S`, default 45 s).
`cast-on ... fresh=1` (one cast per target), `use-item ... at=go:ENTRY|npc:ENTRY` (ground target at the nearest such object),
`use-item ... fail=ok` (a failed use is logged, not STUCK; `on=` never picks a dead unit or one whose use already failed),
`spellclick ... optional=1` and `talk ... optional=1` (nothing to click / no gossip is OK), `talk ... go=1` (a gameobject
with a gossip menu). `talk` accepts a quest window (quest list, details) as the NPC's answer. `equip` of an item that is
already worn is OK. When `pump()` has not run for `HEADLESS_STALL_DUMP` seconds (default 45, 0 = off), the client prints
`STALL-DUMP` with every thread's Python stack.

Control lines:

```
repeat N                 ...  end      run the block N times
repeat until=COND [max=N] ...  end      run the block until COND is met (STUCK if it never is)
retry VERB ARGS... until=COND [every=S] [max=N]   run VERB, wait up to S s for COND, up to N tries
repeat until=COND { VERB ... ; VERB ... } end      a block on one line (`;` separates the lines)
```

`repeat` and `end` each go on their own line, and blocks can be nested. The `until=`, `every=` and
`max=` tokens of a `retry` line belong to `retry`; every other token goes to the inner verb. A STUCK
inside a retried verb counts as one failed try.

Conditions: `quest:Q:complete|rewarded|accepted|none`, `obj:Q:i:COUNT` (objective counter `i` of
quest Q has reached COUNT), `level:N`, `item:ENTRY:N`, `vehicle:on|off`, `map:M`, `dead:0|1`,
`visible:ENTRY`, `gone:ENTRY`.

Cinematics and movies are acknowledged automatically, as a client that skips them would
(`CMSG_NEXT_CINEMATIC_CAMERA`, then `CMSG_COMPLETE_CINEMATIC` / `CMSG_COMPLETE_MOVIE` 3 s later).

Optional read-only spawn hints: if `MOP_MYSQL_BIN` (path of the `mysql` client) and `MOP_DB_HOST`,
`MOP_DB_PORT`, `MOP_DB_USER`, `MOP_DB_PASS` and `MOP_DB_WORLD` are set, `goto-npc`, `goto-go` and
`kill` look up the spawn positions of an entry that is not in view. The lookup only runs a
`SELECT` on the `creature` / `gameobject` table, and the verbs still act only on objects the client
can see.

`--cmd create` now waits for `SMSG_CHAR_CREATE` and prints
`CREATE-RESULT code=<n> name=<ResponseCodes name>`. The exit code is 0 for
`CHAR_CREATE_SUCCESS`, 1 for any other result, and 2 when no reply arrives.

## Logging out

By default the character stays in the world until the server times the socket out, so a test
driver can keep using it after the script exits. Set `HEADLESS_LOGOUT=1` to send a clean logout
when the session ends.
