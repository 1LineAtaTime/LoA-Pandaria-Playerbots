#!/usr/bin/env python3
"""headless.py - minimal REAL WoW 5.4.8 client (SkyFire/Trinity family) for this core.

Logs in over a genuine TCP socket exactly like the game client does: SRP6 against the
auth server, then the world handshake, then character login. Because the resulting
session is a real socket session, the engine treats it like a manually logged-in
player (walked by Map::Update) - unlike the injected socketless PlayerbotMgr::LoginBot
sessions that the engine never dispatches for.

Typical use (see README.md next to this file):
  python headless.py --account ACC --password PASS --cmd test
  python headless.py --account ACC --password PASS --cmd play --queue 9 --accept --hold 300
  python headless.py --account ACC --password PASS --cmd play --queue 9   # just queue, no accept

Connection settings come from, in order: the command line, environment variables
(MOP_ACCOUNT, MOP_PASSWORD, MOP_CHAR_GUID, MOP_HOST, MOP_AUTH_PORT, MOP_WORLD_PORT, MOP_SOAP_HOST,
MOP_SOAP_PORT, MOP_SOAP_USER, MOP_SOAP_PASSWORD, MOP_SERVER_LOG), an optional JSON file of local
defaults (MOP_HEADLESS_DEFAULTS, else headless.local.json next to this script), then neutral
built-ins (127.0.0.1, ports 3724/8085/7878). There is no built-in account or password.

Protocol facts (verified against the SkyFire/Trinity 5.4.8 sources):
  * auth: SRP6 (N/g from SRP6.cpp), B/g/N/s/crc/flags challenge, 75-byte proof.
    Session key K = SHA1Interleave(S) (40 bytes); M1 = SHA1(H(N)^H(g) | H(I) | s | A | B | K).
  * world: 4-byte server header (uint16 size=payload+2, uint16 opcode); client pre-auth
    header is 6 bytes (uint16 size=payload+4, uint32 opcode). After CMSG_AUTH_SESSION all
    headers are 4 bytes: uint32 (size << 13 | opcode) encrypted with ARC4-drop1024 keyed by
    HMAC-SHA1(ServerEncryptionKey, K) for receive and HMAC-SHA1(ServerDecryptionKey, K) for
    send. Payloads are never encrypted.
  * digest = SHA1(account | 00000000 | clientSeed | serverSeed | K), sent scattered.
"""
import argparse
import atexit
import base64
import hashlib
import hmac
import http.client
import json
import math
import os
import random
import re
import select
import socket
import struct
import subprocess
import sys
import time
import threading
import zlib
import faulthandler


# Stall watchdog: a verb that spins without pumping the socket shows nothing in the log. When pump() has not run for
# HEADLESS_STALL_DUMP seconds (default 45, 0 = off), dump every thread's Python stack once per stall.
_STALL_LAST = [time.time(), False]


def _stall_watchdog_touch():
    _STALL_LAST[0] = time.time()
    _STALL_LAST[1] = False


def _stall_watchdog_start():
    limit = float(os.environ.get("HEADLESS_STALL_DUMP", "45") or 0)
    if limit <= 0:
        return

    def _run():
        while True:
            time.sleep(5)
            if not _STALL_LAST[1] and time.time() - _STALL_LAST[0] > limit:
                _STALL_LAST[1] = True
                print("STALL-DUMP: no socket pump for %.0f s - Python stacks follow" % (time.time() - _STALL_LAST[0]), flush=True)
                try:
                    faulthandler.dump_traceback(file=sys.stdout, all_threads=True)
                    sys.stdout.flush()
                except Exception as exc:  # noqa: BLE001 - diagnostics only
                    print("STALL-DUMP failed: %s" % exc, flush=True)
    threading.Thread(target=_run, name="stall-watchdog", daemon=True).start()

# w17: every print (log() and emit()) must survive characters the Windows console encoding cannot represent - a
# party-member-stats line with such characters killed two dungeon masters (UnicodeEncodeError in cp1252).
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(errors="replace")
    except (AttributeError, ValueError):
        pass

# ---------------------------------------------------------------------------
# SRP6 / crypt constants (SkyFire src/server/shared/Cryptography/...)
# ---------------------------------------------------------------------------
N_HEX = "894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7"
N_BYTES_LE = bytes.fromhex(N_HEX)[::-1]          # SRP6::N is little-endian on the wire
N_INT = int.from_bytes(N_BYTES_LE, "little")
G_INT = 7
K_MULT = 3
SERVER_ENCRYPTION_KEY = bytes([0x08, 0xF1, 0x95, 0x9F, 0x47, 0xE5, 0xD2, 0xDB,
                               0xA1, 0x3D, 0x77, 0x8F, 0x3F, 0x3E, 0xE7, 0x00])
SERVER_DECRYPTION_KEY = bytes([0x40, 0xAA, 0xD3, 0x92, 0x26, 0x71, 0x43, 0x47,
                               0x3A, 0x31, 0x08, 0xA6, 0xE7, 0xDC, 0x98, 0x2A])

CLIENT_BUILD = 18414

# opcodes (Opcodes.cpp wire numbers, 5.4.8 18414)
CMSG_PING = 0x0012
CMSG_TIME_SYNC_RESPONSE = 0x01DB
CMSG_AUTH_SESSION = 0x00B2
CMSG_ENUM_CHARACTERS = 0x00E0
CMSG_PLAYER_LOGIN = 0x158F
CMSG_BATTLEMASTER_JOIN = 0x0769
CMSG_BATTLEMASTER_JOIN_ARENA = 0x02D2   # #570: body = uint8 slot (BattleGroundHandler.cpp:575-577)
CMSG_BATTLEFIELD_PORT = 0x1379
CMSG_DUEL_PROPOSED = 0x1A26
CMSG_GROUP_INVITE = 0x072D
CMSG_DUEL_RESPONSE = 0x03E2
MSG_VERIFY_CONNECTIVITY = 0x4F57
MSG_MOVE_WORLDPORT_ACK = 0x1FAD

SMSG_AUTH_CHALLENGE = 0x0949
SMSG_AUTH_RESPONSE = 0x0ABA
SMSG_ENUM_CHARACTERS_RESULT = 0x11C3
SMSG_LOGIN_VERIFY_WORLD = 0x1C0F
SMSG_TIME_SYNC_REQUEST = 0x1A8F
CMSG_LOGOUT_REQUEST = 0x1349     # Opcodes.h:277
SMSG_LOGOUT_RESPONSE = 0x008F    # Opcodes.h:843: u32 result (0 = ok), u8 instant
SMSG_LOGOUT_COMPLETE = 0x142F    # Opcodes.h:842
SMSG_PONG = 0x1969
SMSG_TRANSFER_PENDING = 0x061B
SMSG_NEW_WORLD = 0x1C3B
SMSG_BATTLEFIELD_STATUS = 0x0433
SMSG_BATTLEFIELD_STATUS_QUEUED = 0x122E
SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION = 0x1EAF
SMSG_BATTLEFIELD_STATUS_ACTIVE = 0x1AAF
SMSG_BATTLEFIELD_STATUS_FAILED = 0x1140
SMSG_COMPRESSED_PACKET = 0x1568
SMSG_DUEL_REQUESTED = 0x0022
SMSG_DUEL_COMPLETE = 0x1C0A
SMSG_DUEL_COUNTDOWN = 0x129F
SMSG_DUEL_WINNER = 0x10E1
SMSG_CAST_FAILED = 0x143A
SMSG_SPELL_GO = 0x09D8                       # Opcodes.h:1091
SMSG_INVENTORY_CHANGE_FAILURE = 0x0C1E      # Opcodes.h:804 (use-item rejected: ITEM_NOT_FOUND etc.)
SMSG_UPDATE_OBJECT = 0x1792     # bug fix (issue #165): was 0x1BAB; Opcodes.h:1162 says 0x1792
SMSG_UPDATE_WORLD_STATE = 0x121B

# --- S1a additions: near-teleport ack, chat, group, char create -----------
# Opcodes verified against src/server/game/{Handlers,Server/Protocol/Opcodes.h}
# on this repo, not from the doc alone (issue #165 asks for this).
CMSG_MOVE_TELEPORT_ACK = 0x0078
SMSG_MOVE_TELEPORT = 0x0B39

CMSG_MESSAGECHAT_SAY = 0x0A9A
CMSG_MESSAGECHAT_PARTY = 0x109A
CMSG_MESSAGECHAT_RAID = 0x083E
CMSG_MESSAGECHAT_WHISPER = 0x123E
# channel + guild chat, channel join, guild invite accept (Opcodes.h:300,303,251,202,646,766)
CMSG_MESSAGECHAT_CHANNEL = 0x00BB
CMSG_MESSAGECHAT_GUILD = 0x0CAE
CMSG_JOIN_CHANNEL = 0x148E
CMSG_GUILD_ACCEPT = 0x18A2
SMSG_CHANNEL_NOTIFY = 0x0F06
SMSG_GUILD_INVITE = 0x0F71
SMSG_MESSAGECHAT = 0x1A9A

CMSG_GROUP_INVITE_RESPONSE = 0x0D61
CMSG_GROUP_UNINVITE_GUID = 0x0CE1
CMSG_GROUP_SET_LEADER = 0x15BB
CMSG_GROUP_DISBAND = 0x1798
# #373: dungeon difficulty (Opcodes.h:459 CMSG, :1050 SMSG; handler MiscHandler.cpp:1950 reads one uint32 = Difficulty enum,
# DBCEnums.h:409-412: 1 = DUNGEON_DIFFICULTY_NORMAL, 2 = DUNGEON_DIFFICULTY_HEROIC). The server answers with
# SMSG_SET_DUNGEON_DIFFICULTY (Player.cpp:22163, one uint32) to every group member via Group::SetDungeonDifficulty (Group.cpp:2583).
CMSG_SET_DUNGEON_DIFFICULTY = 0x1A36
SMSG_SET_DUNGEON_DIFFICULTY = 0x1283
CMSG_LOOT_METHOD = 0x0DE1
CMSG_GROUP_RAID_CONVERT = 0x032C
CMSG_RAID_TARGET_UPDATE = 0x0886
CMSG_RAID_READY_CHECK = 0x0817
CMSG_RAID_READY_CHECK_CONFIRM = 0x158B
# #210/#215 S10 P6: the broadcast side of the ready check, needed to observe bot confirms.
SMSG_RAID_READY_CHECK = 0x1C8E
SMSG_RAID_READY_CHECK_CONFIRM = 0x02AF
SMSG_GROUP_LIST = 0x0CBB
SMSG_PARTY_MEMBER_STATS = 0x0A9A

CMSG_CHAR_CREATE = 0x0F1D
SMSG_CHAR_CREATE = 0x1CAA

# --- S1b additions (issue #172): movement, mount/taxi, quest, loot, trade,
# LFG/BG group queue, death. Opcodes verified against src/server/game
# {Handlers,Server/Packets/MovementPackets.cpp,Server/Protocol/Opcodes.h}
# on this repo, not from the doc alone.
MSG_MOVE_START_FORWARD = 0x095A
MSG_MOVE_HEARTBEAT = 0x01F2
MSG_MOVE_STOP = 0x08F1
MSG_MOVE_SET_FACING = 0x1050

CMSG_CAST_SPELL = 0x0206
CMSG_CANCEL_AUTO_REPEAT_SPELL = 0x1272
CMSG_ACTIVATE_TAXI = 0x03C9

CMSG_PUSHQUESTTOPARTY = 0x03D2
CMSG_QUESTGIVER_ACCEPT_QUEST = 0x06D1
CMSG_QUESTGIVER_COMPLETE_QUEST = 0x0659
CMSG_QUESTGIVER_CHOOSE_REWARD = 0x07CB

CMSG_LOOT = 0x1CE2
CMSG_LOOT_RELEASE = 0x0840
CMSG_LOOT_ROLL = 0x15C2
CMSG_LOOT_MASTER_GIVE = 0x1DE1
SMSG_LOOT_START_ROLL = 0x0EAA
SMSG_LOOT_ROLL = 0x1840
SMSG_LOOT_ROLL_WON = 0x0A3A

CMSG_INITIATE_TRADE = 0x0267
CMSG_SET_TRADE_ITEM = 0x03D5
CMSG_SET_TRADE_GOLD = 0x14E3
CMSG_ACCEPT_TRADE = 0x144D
CMSG_CANCEL_TRADE = 0x1941
SMSG_TRADE_STATUS = 0x1963

CMSG_LFG_JOIN = 0x046B
CMSG_LFG_SET_ROLES = 0x08A2
CMSG_LFG_PROPOSAL_RESULT = 0x1D9D
CMSG_LFG_TELEPORT = 0x1AA6
# #210/#215 S10 P6: LFG queue-state opcodes (Server/Protocol/Opcodes.h on this repo).
CMSG_LFG_LEAVE = 0x01E0
SMSG_LFG_ROLE_CHECK_UPDATE = 0x12BB
SMSG_LFG_PROPOSAL_UPDATE = 0x1E3B
SMSG_LFG_UPDATE_STATUS = 0x0C2E
CMSG_BATTLEFIELD_LEAVE = 0x0257
SMSG_RAID_READY_CHECK_COMPLETED = 0x15C2

CMSG_SET_SELECTION = 0x0740
CMSG_REPOP_REQUEST = 0x134A
CMSG_RECLAIM_CORPSE = 0x03D3
CMSG_RESURRECT_RESPONSE = 0x0B0C
CMSG_SUMMON_RESPONSE = 0x0A33
SMSG_RESURRECT_REQUEST = 0x1062
SMSG_SUMMON_REQUEST = 0x081F

# --- S2 additions (issue #191): area triggers, transport-relative movement,
# duel step verb, item-on-item use. Opcodes verified against src/server/game/
# Server/Protocol/Opcodes.h (wire numbers match the enum values directly on
# this build, same convention already used above for every other opcode).
CMSG_AREATRIGGER = 0x1C44
CMSG_SELECT_FACTION = 0x0027   # Handlers/MiscHandler.cpp HandleSelectFactionOpcode: u32 choice (0 = Horde, 1 = Alliance)
CMSG_USE_ITEM = 0x1CC1
TARGET_FLAG_UNIT = 0x00000002
TARGET_FLAG_DEST_LOCATION = 0x00000040
TARGET_FLAG_ITEM = 0x00000010
TARGET_FLAG_GAMEOBJECT = 0x00000800   # SpellDefines.h:198; targetGuid is a GO guid (HighGuid::GameObject 0xF11)
CMSG_AUTOSTORE_LOOT_ITEM = 0x0354   # Opcodes.h:63
CMSG_LOOT_MONEY = 0x02F6            # Opcodes.h:283
SMSG_LOOT_RESPONSE = 0x128A         # Opcodes.h:855

# --- #470 additions: start-zone quest-chain verbs. Opcode wire numbers from
# src/server/game/Server/Protocol/Opcodes.h; layouts replayed from the handler read order.
CMSG_GOSSIP_HELLO = 0x12F3
CMSG_GOSSIP_SELECT_OPTION = 0x0748
CMSG_SELECT_FACTION = 0x0027
CMSG_GAMEOBJ_USE = 0x06D8
CMSG_GAMEOBJ_REPORT_USE = 0x06D9
CMSG_ATTACKSWING = 0x02E7
CMSG_ATTACKSTOP = 0x0345
CMSG_SPELLCLICK = 0x067A
SMSG_GOSSIP_MESSAGE = 0x0244
HIGHGUID_GAMEOBJECT = 0xF11
# #470 response observers (logged by name+size so a run can prove the server accepted the verb)
OBSERVED_470 = {0x0B01: "SMSG_TRIGGER_CINEMATIC", 0x1A9E: "SMSG_ATTACKSTART", 0x12AF: "SMSG_ATTACKSTOP", 0x06AA: "SMSG_ATTACKERSTATEUPDATE",
                0x128A: "SMSG_LOOT_RESPONSE", 0x123F: "SMSG_LOOT_RELEASE_RESPONSE",
                0x034E: "SMSG_GOSSIP_COMPLETE", 0x06AF: "SMSG_AI_REACTION",
                0x134C: "SMSG_QUESTGIVER_QUEST_DETAILS", 0x02D4: "SMSG_QUESTGIVER_QUEST_LIST",
                0x15E0: "SMSG_SHOW_NEURTRAL_PLAYER_FACTION_SELECT_UI",
                0x1AAE: "SMSG_LIST_INVENTORY", 0x189F: "SMSG_TRAINER_LIST", 0x09D8: "SMSG_SPELL_GO"}

# --- #969 additions: BattlePay (in-game shop). Wire numbers from src/server/game/Server/Protocol/Opcodes.h:77-81
# and :589-599; layouts replayed from Handlers/BattlePayHandler.cpp (read order) and BattlePay/BattlePayMgr.cpp
# (SendBattlePayPurchaseUpdate write order). Verbs: bp-products, bp-buy (see run_verb).
CMSG_BATTLE_PAY_GET_PRODUCT_LIST = 0x0DE0            # empty body
CMSG_BATTLE_PAY_START_PURCHASE = 0x1886              # u32 productId, u32 clientToken, guid bits [2,3,5,1,4,7,0,6], bytes [5,3,7,1,4,0,6,2]
CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE = 0x0812   # u64 currentPrice, u32 serverToken, u8 confirm
SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE = 0x1ABF
SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE = 0x0612     # u64 purchaseId, u32 result, u32 clientToken
SMSG_BATTLE_PAY_CONFIRM_PURCHASE = 0x14E3            # u32 serverToken, u64 currentPrice, u64 purchaseId
SMSG_BATTLE_PAY_PURCHASE_UPDATE = 0x14E2             # bits19 count, per: bits8 walletLen; per: u32 result, u64 purchaseId, u32 product, u32 status, wallet
SMSG_BATTLE_PAY_DELIVERY_ENDED = 0x020B
SMSG_BATTLE_PAY_DISTRIBUTION_UPDATE = 0x1E1B
BATTLE_PAY_STATUS_NAMES = {2: "READY", 3: "ALLOWED_TO_BUY", 5: "BUYING", 6: "BUYED"}   # BattlePayMgr.h:143-146
BATTLE_PAY_RESULT_NAMES = {0: "OK", 3: "SHOP_ERROR", 4: "DECLINED", 25: "UNAVAILABLE"}  # BattlePayMgr.h:151-154

# Group.h GROUP_UPDATE_FLAG_* (ascending bit order == field write order)
GROUP_UPDATE_FIELDS = [
    ("status", 0x00000001, "u16"), ("mop_unk", 0x00000002, "u8u8"),
    ("cur_hp", 0x00000004, "u32"), ("max_hp", 0x00000008, "u32"),
    ("power_type", 0x00000010, "u8"), ("unk20", 0x00000020, "u16"),
    ("cur_power", 0x00000040, "u16"), ("max_power", 0x00000080, "u16"),
    ("level", 0x00000100, "u16"), ("area", 0x00000200, "u16"),
    ("unk400", 0x00000400, "u16"), ("position", 0x00000800, "pos16"),
    ("auras", 0x00001000, "auras"), ("pet_guid", 0x00002000, "u64"),
    ("pet_name", 0x00004000, "cstr"), ("pet_model", 0x00008000, "u16"),
    ("pet_cur_hp", 0x00010000, "u32"), ("pet_max_hp", 0x00020000, "u32"),
    ("pet_power_type", 0x00040000, "u8"), ("pet_cur_power", 0x00080000, "u16"),
    ("pet_max_power", 0x00100000, "u16"), ("mop_unk2", 0x00200000, "u16"),
    ("pet_auras", 0x00400000, "auras"), ("vehicle_seat", 0x00800000, "i32"),
]

# race -> racial language (SharedDefines.h LANG_ORCISH=1 Horde, LANG_COMMON=7
# Alliance; used so chat isn't sent in the rejected LANG_UNIVERSAL).
LANG_UNIVERSAL = 0
LANG_ORCISH = 1
LANG_COMMON = 7
HORDE_RACES = {2, 5, 6, 8, 9, 10, 26}     # orc, undead, tauren, troll, goblin, blood elf, pandaren(h)
RACE_LANGUAGE = {race: LANG_ORCISH for race in HORDE_RACES}
for _r in (1, 3, 4, 7, 11, 22, 25):        # human, dwarf, night elf, gnome, draenei, worgen, pandaren(a)
    RACE_LANGUAGE[_r] = LANG_COMMON

# Battleground objective world states. These are what the client's scoreboard reads, so
# watching them is end-to-end proof that an objective actually changed hands - no server-side
# instrumentation required. Ids from src/server/game/Battlegrounds/Zones/Battleground{AB,WS,EY}.h.
WORLD_STATES = {
    # Arathi Basin (map 529)
    1776: "AB/alliance-resources", 1777: "AB/horde-resources",
    1778: "AB/horde-bases",        1779: "AB/alliance-bases",
    # Warsong Gulch (map 489)
    1581: "WS/alliance-captures",  1582: "WS/horde-captures",
    2338: "WS/horde-flag-state",   2339: "WS/alliance-flag-state",
    # Eye of the Storm (map 566)
    2749: "EY/alliance-resources", 2750: "EY/horde-resources",
    2752: "EY/alliance-bases",     2753: "EY/horde-bases",
    2757: "EY/netherstorm-flag",
    2769: "EY/flag-state-alliance", 2770: "EY/flag-state-horde",
}
# #898 Tol Barad (BattlefieldTB.h TolBaradWorldStates) so --trace / world_states show the TB war UI values.
for _id, _n in {
    5385: "alliance-controls-show", 5384: "horde-controls-show", 5546: "alliance-attacking-show",
    5547: "horde-attacking-show", 5348: "buildings-captured", 5349: "buildings-captured-show",
    5347: "towers-destroyed", 5350: "towers-destroyed-show", 5334: "faction-controlling",
    5332: "time-next-battle", 5387: "time-next-battle-show", 5333: "time-battle-end",
    5346: "time-battle-end-show", 5684: "state-preparations", 5344: "state-battle",
    5376: "progress-show", 5377: "progress", 5378: "progress-grey", 5469: "keep-horde",
    5470: "keep-alliance", 5418: "garrison-horde-controlled", 5419: "garrison-horde-capturing",
    5421: "garrison-alliance-capturing", 5422: "garrison-alliance-controlled",
    5423: "vigil-horde-controlled", 5424: "vigil-horde-capturing", 5426: "vigil-alliance-capturing",
    5427: "vigil-alliance-controlled", 5428: "slagworks-horde-controlled", 5429: "slagworks-horde-capturing",
    5431: "slagworks-alliance-capturing", 5432: "slagworks-alliance-controlled",
}.items():
    WORLD_STATES[_id] = "TB/" + _n

SMSG_COMPRESSED_UPDATE_OBJECT = 0x1BD0
# #900/#898 battlefield manager (Opcodes.h:68-71,574-578): entry invite = "join the war", queue invite = "join the queue"
CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE = 0x1806
CMSG_BATTLEFIELD_MGR_QUEUE_INVITE_RESPONSE = 0x0A97
SMSG_BATTLEFIELD_MGR_ENTRY_INVITE = 0x1226
SMSG_BATTLEFIELD_MGR_QUEUE_INVITE = 0x142E
SMSG_BATTLEFIELD_MGR_ENTERED = 0x081B
SMSG_BATTLEFIELD_MGR_EJECTED = 0x18C2
SMSG_BATTLEFIELD_MGR_QUEUE_REQUEST_RESPONSE = 0x08BE

SOTA_BG_TYPE = 9          # strand_of_the_ancients
SOTA_MAP = 607
BG_MAPS = {30, 489, 529, 566, 607, 628, 726}   # AV, WSG, AB, IoC, SotA, Gilneas, TP

OPCODE_NAMES = {
    0x0ABA: "SMSG_AUTH_RESPONSE", 0x11C3: "SMSG_ENUM_CHARACTERS_RESULT",
    0x1C0F: "SMSG_LOGIN_VERIFY_WORLD", 0x1A8F: "SMSG_TIME_SYNC_REQUEST",
    0x1969: "SMSG_PONG", 0x061B: "SMSG_TRANSFER_PENDING", 0x1C3B: "SMSG_NEW_WORLD",
    0x0433: "SMSG_BATTLEFIELD_STATUS", 0x122E: "SMSG_BATTLEFIELD_STATUS_QUEUED",
    0x1EAF: "SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION",
    0x1AAF: "SMSG_BATTLEFIELD_STATUS_ACTIVE", 0x1140: "SMSG_BATTLEFIELD_STATUS_FAILED",
    0x1568: "SMSG_COMPRESSED_PACKET",
    0x1226: "SMSG_BATTLEFIELD_MGR_ENTRY_INVITE", 0x142E: "SMSG_BATTLEFIELD_MGR_QUEUE_INVITE",
    0x081B: "SMSG_BATTLEFIELD_MGR_ENTERED", 0x18C2: "SMSG_BATTLEFIELD_MGR_EJECTED",
    0x08BE: "SMSG_BATTLEFIELD_MGR_QUEUE_REQUEST_RESPONSE",
}


def log(msg):
    line = "%s %s" % (time.strftime("%H:%M:%S"), msg)
    try:
        print(line, flush=True)
    except UnicodeEncodeError:
        # w17: a party-member-stats line with non-cp1252 characters killed the client (Windows console encoding).
        enc = getattr(sys.stdout, "encoding", None) or "ascii"
        print(line.encode(enc, errors="replace").decode(enc, errors="replace"), flush=True)


TRACE = False


def sha1(data):
    return hashlib.sha1(data).digest()


# ---------------------------------------------------------------------------
# ARC4 (standard RC4, as EVP_rc4) and the world header crypt
# ---------------------------------------------------------------------------
class Arc4:
    def __init__(self, key):
        s = list(range(256))
        j = 0
        for i in range(256):
            j = (j + s[i] + key[i % len(key)]) & 0xFF
            s[i], s[j] = s[j], s[i]
        self.s = s
        self.i = 0
        self.j = 0

    def crypt(self, data):
        out = bytearray(data)
        s, i, j = self.s, self.i, self.j
        for n in range(len(out)):
            i = (i + 1) & 0xFF
            j = (j + s[i]) & 0xFF
            s[i], s[j] = s[j], s[i]
            out[n] ^= s[(s[i] + s[j]) & 0xFF]
        self.i, self.j = i, j
        return bytes(out)


class WorldCrypt:
    """Client side of AuthCrypt: receive key from ServerEncryptionKey, send key from
    ServerDecryptionKey; both ARC4-drop1024. Only the 4-byte headers are crypted."""

    def __init__(self, session_key):
        self.recv = Arc4(hmac.new(SERVER_ENCRYPTION_KEY, session_key, hashlib.sha1).digest())
        self.send = Arc4(hmac.new(SERVER_DECRYPTION_KEY, session_key, hashlib.sha1).digest())
        self.recv.crypt(bytes(1024))
        self.send.crypt(bytes(1024))
        self.enabled = False


# ---------------------------------------------------------------------------
# bit writer (mirrors ByteBuffer: bits are MSB-first, partial byte pads to zero
# and raw bytes always start on a byte boundary)
# ---------------------------------------------------------------------------
class BitWriter:
    def __init__(self):
        self.buf = bytearray()
        self.cur = 0
        self.pos = 8

    def bit(self, value):
        self.pos -= 1
        if value:
            self.cur |= 1 << self.pos
        if self.pos == 0:
            self.buf.append(self.cur)
            self.cur = 0
            self.pos = 8

    def bits(self, value, count):
        for i in range(count - 1, -1, -1):
            self.bit((value >> i) & 1)

    def flush(self):
        if self.pos != 8:
            self.buf.append(self.cur)
            self.cur = 0
            self.pos = 8
        return bytes(self.buf)


def packed_guid(guid, mask_order, byte_order):
    bw = BitWriter()
    b = guid.to_bytes(8, "little")
    for off in mask_order:
        bw.bit((b[off] != 0))
    out = bytearray(bw.flush())
    for off in byte_order:
        if b[off]:
            out.append(b[off] ^ 1)
    return bytes(out)


def guid_mask_bits(guid, mask_order, extra_bits=None):
    """Like BitWriter-driven packed_guid(), but lets extra (value, nbits) fields
    share the same bit-flush as the guid mask (several CMSG group packets pack
    a length or a flag into the same run of bits as the guid mask, e.g.
    CMSG_GROUP_UNINVITE_GUID's reasonLen)."""
    bw = BitWriter()
    b = guid.to_bytes(8, "little")
    for off in mask_order:
        bw.bit(b[off] != 0)
    if extra_bits:
        for value, nbits in extra_bits:
            bw.bits(value, nbits)
    return bw.flush()


def guid_byte_seq(guid, byte_order):
    b = guid.to_bytes(8, "little")
    out = bytearray()
    for off in byte_order:
        if b[off]:
            out.append(b[off] ^ 1)
    return bytes(out)


def player_guid(counter):
    """Full raw player guid: HighGuid::Player(0x070) << 52 | counter.

    ObjectAccessor::FindPlayer keys on the full raw value, so client packets
    that reference a player must send the type bits, not the bare counter.
    """
    return (0x070 << 52) | (int(counter) & 0xFFFFFFFFFFFF)


class ByteReader:
    """Mirror of the server's ByteBuffer read side.

    WriteByteSeq only emits a byte when the value is non-zero, so a packet's
    field offsets are data-dependent. Decoding therefore has to replay the
    writer's exact sequence, reading a seq byte only when the corresponding
    guid byte was non-zero (the mask bit read earlier tells us that).
    """

    def __init__(self, data):
        self.data = data
        self.pos = 0
        self.bitpos = 8
        self.curbit = 0

    def _reset_bits(self):
        # Mirror ByteBuffer::ResetBitPos: a raw read discards pending bits.
        self.bitpos = 8
        self.curbit = 0

    def read_u8(self):
        self._reset_bits()
        b = self.data[self.pos]
        self.pos += 1
        return b

    def read_u32(self):
        self._reset_bits()
        v = struct.unpack_from("<I", self.data, self.pos)[0]
        self.pos += 4
        return v

    def read_bit(self):
        self.bitpos += 1
        if self.bitpos > 7:
            self.curbit = self.data[self.pos]
            self.pos += 1
            self.bitpos = 0
        return (self.curbit >> (7 - self.bitpos)) & 1

    def read_byte_seq(self, nonzero):
        """Consume a byte-seq byte iff the guid byte was non-zero.

        Bug fix (issue #172, found while verifying loot/resurrect/summon parses
        against ByteBuffer.h): ReadByteSeq(uint8& b) is `if (b != 0) b ^= read<uint8>();`
        with b initialised to the mask bit (0/1), i.e. WriteByteSeq's wire byte is
        (actual_byte ^ 1) and must be XORed with 1 again here to recover it. The
        pre-#172 version of this method returned the raw wire byte unXORed, which
        was wrong for every guid reconstructed through it (SMSG_GROUP_LIST,
        CHAR_ENUM, SMSG_MESSAGECHAT sender/receiver, PARTY_MEMBER_STATS pet guid).
        """
        return (self.read_u8() ^ 1) if nonzero else 0

    # -- S1a additions: multi-bit fields and byte-aligned strings/floats ---
    def read_bits(self, count):
        v = 0
        for _ in range(count):
            v = (v << 1) | self.read_bit()
        return v

    def read_string(self, length):
        """Byte-aligned, non-null-terminated string (ByteBuffer::ReadString);
        the first call after a run of bits mirrors FlushBits by realigning."""
        self._reset_bits()
        s = bytes(self.data[self.pos:self.pos + length])
        self.pos += length
        return s

    def read_f32(self):
        self._reset_bits()
        v = struct.unpack_from("<f", self.data, self.pos)[0]
        self.pos += 4
        return v

    def read_i32(self):
        self._reset_bits()
        v = struct.unpack_from("<i", self.data, self.pos)[0]
        self.pos += 4
        return v

    def read_u16(self):
        self._reset_bits()
        v = struct.unpack_from("<H", self.data, self.pos)[0]
        self.pos += 2
        return v

    def read_u64(self):
        self._reset_bits()
        v = struct.unpack_from("<Q", self.data, self.pos)[0]
        self.pos += 8
        return v

    def read_cstr(self):
        self._reset_bits()
        end = self.data.index(b"\x00", self.pos)
        s = bytes(self.data[self.pos:end])
        self.pos = end + 1
        return s

    def skip(self, n):
        self._reset_bits()
        self.pos += n

    def remaining(self):
        return len(self.data) - self.pos


# ---------------------------------------------------------------------------
# S1b (issue #172): table-driven MovementStatusElements encoder.
#
# 5.4.8 does not send a fixed MovementInfo layout: each MSG_MOVE_* opcode has
# its own field/bit ORDER (but the same field SET for the four opcodes used
# here), read back element-by-element by
# WorldPackets::Movement::ReadMovementInfo (Server/Packets/MovementPackets.cpp:
# 24-272) via GetMovementStatusElementsSequence(opcode) (Movement/
# MovementStructures.cpp:6670). The tables below are transcribed verbatim from
# MovementStructures.cpp (MovementHeartBeat:176, MovementSetFacing:330,
# MovementStartForward:561, MovementStop:946), with the MSE prefix stripped.
#
# Bit vs. byte semantics (verified against ByteBuffer.h): WriteBit/ReadBit bits
# accumulate in a single bitstream that is only flushed to a byte boundary when
# a raw (non-bit) field is read/written (ByteBuffer::read<T>() calls
# ResetBitPos() first; the real client's writer must FlushBits() at the same
# point). guid mask bits are inverted-free booleans; guid byte values use
# WriteByteSeq, which XORs the wire byte with 1 (ByteBuffer.h:186-196) - the
# same trick already used by packed_guid()/guid_mask_bits()/guid_byte_seq()
# below for CMSG_GROUP_INVITE etc.
#
# "Has*" bit polarity (from ReadMovementInfo's case list): MSEHasMovementFlags,
# MSEHasMovementFlags2, MSEHasTimestamp, MSEHasOrientation, MSEHasPitch,
# MSEHasSplineElevation, MSEHasCounter are INVERTED (`has = !ReadBit()`, so a
# 0 bit means "present"); MSEHasTransportData, MSEHasFallData,
# MSEHasFallDirection are NOT inverted. This encoder only ever sends grounded,
# non-transport, non-falling movement (mount/flight speed still uses plain
# heartbeats; no MSETransport*/MSEFall*/MSEPitch/MSESplineElevation payload is
# produced), which is everything the walk/goto/face/stop verbs and T4/T5 need;
# on-transport relative movement (T6/T7) is out of scope here (see report).
MSE_HEARTBEAT = [
    "PositionZ", "PositionX", "PositionY", "ForcesCount", "HasMovementFlags",
    "ZeroBit", "HasCounter", "HasGuidByte3", "HasGuidByte6", "HasPitch",
    "ZeroBit", "ZeroBit", "HasGuidByte7", "HasGuidByte2", "HasGuidByte4",
    "HasMovementFlags2", "HasOrientation", "HasTimestamp", "HasTransportData",
    "HasFallData", "HasGuidByte5", "HasSplineElevation", "HasGuidByte1",
    "HasGuidByte0", "HasTransportGuidByte5", "HasTransportGuidByte3",
    "HasTransportGuidByte6", "HasTransportGuidByte0", "HasTransportGuidByte7",
    "HasTransportTime3", "HasTransportGuidByte1", "HasTransportGuidByte2",
    "HasTransportGuidByte4", "HasTransportTime2", "MovementFlags",
    "HasFallDirection", "MovementFlags2", "GuidByte2", "GuidByte3",
    "GuidByte6", "GuidByte1", "GuidByte4", "GuidByte7", "Forces", "GuidByte5",
    "GuidByte0", "FallSinAngle", "FallCosAngle", "FallHorizontalSpeed",
    "FallVerticalSpeed", "FallTime", "TransportGuidByte1",
    "TransportGuidByte3", "TransportGuidByte2", "TransportGuidByte0",
    "TransportTime3", "TransportSeat", "TransportGuidByte7",
    "TransportPositionX", "TransportGuidByte4", "TransportTime2",
    "TransportPositionY", "TransportGuidByte6", "TransportGuidByte5",
    "TransportPositionZ", "TransportTime", "TransportOrientation", "Counter",
    "Orientation", "Pitch", "Timestamp", "SplineElevation",
]

MSE_SET_FACING = [
    "PositionY", "PositionX", "PositionZ", "HasGuidByte5", "HasMovementFlags2",
    "HasGuidByte3", "HasGuidByte2", "ForcesCount", "ZeroBit", "HasPitch",
    "HasGuidByte0", "HasOrientation", "HasTimestamp", "ZeroBit", "HasCounter",
    "HasGuidByte4", "ZeroBit", "HasGuidByte1", "HasGuidByte6", "HasFallData",
    "HasMovementFlags", "HasSplineElevation", "HasTransportData",
    "HasGuidByte7", "HasTransportGuidByte7", "HasTransportGuidByte0",
    "HasTransportTime2", "HasTransportGuidByte3", "HasTransportGuidByte6",
    "HasTransportTime3", "HasTransportGuidByte2", "HasTransportGuidByte5",
    "HasTransportGuidByte1", "HasTransportGuidByte4", "HasFallDirection",
    "MovementFlags2", "MovementFlags", "Forces", "GuidByte0", "GuidByte6",
    "GuidByte3", "GuidByte1", "GuidByte2", "GuidByte7", "GuidByte4",
    "GuidByte5", "TransportGuidByte0", "TransportGuidByte2",
    "TransportOrientation", "TransportGuidByte7", "TransportTime3",
    "TransportGuidByte5", "TransportTime", "TransportPositionX",
    "TransportTime2", "TransportPositionZ", "TransportSeat",
    "TransportPositionY", "TransportGuidByte4", "TransportGuidByte3",
    "TransportGuidByte6", "TransportGuidByte1", "FallTime",
    "FallVerticalSpeed", "FallHorizontalSpeed", "FallSinAngle", "FallCosAngle",
    "Counter", "Timestamp", "SplineElevation", "Orientation", "Pitch",
]

MSE_START_FORWARD = [
    "PositionZ", "PositionX", "PositionY", "HasMovementFlags2", "ZeroBit",
    "HasCounter", "ZeroBit", "HasGuidByte0", "HasOrientation", "HasFallData",
    "ForcesCount", "HasGuidByte4", "HasGuidByte1", "HasTimestamp",
    "HasGuidByte7", "HasPitch", "HasTransportData", "HasGuidByte5",
    "HasMovementFlags", "HasGuidByte3", "HasSplineElevation", "HasGuidByte2",
    "HasGuidByte6", "ZeroBit", "HasTransportGuidByte1", "HasTransportTime3",
    "HasTransportGuidByte3", "HasTransportGuidByte4", "HasTransportGuidByte2",
    "HasTransportGuidByte5", "HasTransportGuidByte0", "HasTransportGuidByte7",
    "HasTransportGuidByte6", "HasTransportTime2", "HasFallDirection",
    "MovementFlags2", "MovementFlags", "GuidByte1", "GuidByte6", "GuidByte7",
    "Forces", "GuidByte5", "GuidByte0", "GuidByte3", "GuidByte2", "GuidByte4",
    "TransportGuidByte3", "TransportGuidByte1", "TransportGuidByte6",
    "TransportPositionZ", "TransportGuidByte4", "TransportTime3",
    "TransportSeat", "TransportGuidByte7", "TransportOrientation",
    "TransportTime2", "TransportGuidByte5", "TransportGuidByte2",
    "TransportPositionX", "TransportGuidByte0", "TransportPositionY",
    "TransportTime", "FallCosAngle", "FallSinAngle", "FallHorizontalSpeed",
    "FallTime", "FallVerticalSpeed", "Timestamp", "Pitch", "SplineElevation",
    "Orientation", "Counter",
]

MSE_STOP = [
    "PositionX", "PositionY", "PositionZ", "HasGuidByte5", "HasGuidByte2",
    "HasFallData", "HasGuidByte0", "ZeroBit", "ZeroBit", "HasCounter",
    "HasGuidByte1", "ForcesCount", "HasPitch", "HasGuidByte3", "HasGuidByte4",
    "HasTransportData", "ZeroBit", "HasGuidByte6", "HasMovementFlags",
    "HasTimestamp", "HasMovementFlags2", "HasOrientation",
    "HasSplineElevation", "HasGuidByte7", "HasTransportTime2",
    "HasTransportGuidByte7", "HasTransportGuidByte4", "HasTransportGuidByte1",
    "HasTransportGuidByte0", "HasTransportGuidByte5", "HasTransportGuidByte2",
    "HasTransportGuidByte3", "HasTransportTime3", "HasTransportGuidByte6",
    "HasFallDirection", "MovementFlags2", "MovementFlags", "GuidByte0",
    "GuidByte3", "Forces", "GuidByte6", "GuidByte1", "GuidByte4", "GuidByte2",
    "GuidByte5", "GuidByte7", "Orientation", "FallVerticalSpeed",
    "FallHorizontalSpeed", "FallSinAngle", "FallCosAngle", "FallTime",
    "SplineElevation", "TransportPositionX", "TransportTime",
    "TransportGuidByte3", "TransportOrientation", "TransportPositionY",
    "TransportGuidByte2", "TransportGuidByte6", "TransportGuidByte7",
    "TransportGuidByte1", "TransportGuidByte4", "TransportTime3",
    "TransportGuidByte0", "TransportSeat", "TransportPositionZ",
    "TransportGuidByte5", "TransportTime2", "Counter", "Pitch", "Timestamp",
]

MOVEMENTFLAG_NONE = 0x00000000
MOVEMENTFLAG_FORWARD = 0x00000001

# Known area-trigger boxes for the walk/goto auto-fire check and the
# `areatrigger` verb (issue #191). id 2230 is shared with the AC sibling
# client (confirmed: world.areatrigger_teleport id=2230 -> target_map=389,
# "OrgrimmarInstance Entrance Target 001", i.e. Ragefire Chasm - the same
# trigger id AC's own AreaTrigger.dbc row uses). The x/y/z anchor is AC's
# documented entrance point (issue #191 task); MoP's Cataclysm-revamped
# Azeroth terrain was not independently re-surveyed here, and radius is an
# estimated entrance-trigger radius (AreaTrigger.dbc rows for instance mouths
# are typically a few yards) - calibrate against a running server if this
# silently fails to fire (see the issue #191 report for the caveat).
KNOWN_AREATRIGGERS = {
    2230: {"map": 1, "pos": (1818.4, -4427.26, -10.45), "radius": 4.0, "name": "Ragefire Chasm entrance"},
    # Wandering Isle (map 860): AreaTrigger.dbc rows (Pos / Radius), trigger scripts in zone_wandering_island_north.cpp
    7835: {"map": 860, "pos": (964.26, 3603.69, 196.52), "radius": 15.15, "name": "Temple of Five Dawns entrance (29423)"},
}


def build_movement_body(table, guid, x, y, z, o, movement_flags=MOVEMENTFLAG_NONE,
                         movement_flags2=0, timestamp_ms=0, transport=None):
    """Encode one client MSG_MOVE_* body for `table` (write side of
    ReadMovementInfo, see the module comment above). hasMovementFlags/
    hasMovementFlags2/hasTimestamp/hasOrientation are always sent;
    hasPitch/hasFallData/hasSplineElevation/hasCounter are always false.

    `transport`: optional (tguid, tx, ty, tz, to, ttime, tseat) tuple (issue
    #191). When given, hasTransportData is sent true and every Transport*
    table element is populated; HasTransportTime2/HasTransportTime3 are
    always sent false (no spline/interpolated-transport support needed for
    the docked-boat test cases this is for). Confirmed against
    MovementPackets.cpp:24-272 (WorldPackets::Movement::ReadMovementInfo):
    MSEHasTransportData -> hasTransportData = ReadBit() (direct, not
    inverted); MSEHasTransportGuidByteN -> only consumed `if
    (hasTransportData)`, then `tguid[N] = ReadBit()` (direct, same convention
    as the ordinary guid-mask bits); MSETransportGuidByteN -> ReadByteSeq,
    only `if (hasTransportData)`; MSETransportPositionX/Y/Z/Orientation/Seat/
    Time -> plain float/int8/uint32 reads, all unconditional once
    hasTransportData is true (no further per-field has-bit)."""
    gb = guid.to_bytes(8, "little")
    tgb = transport[0].to_bytes(8, "little") if transport else None

    def transport_field(name):
        _, tx, ty, tz, to, ttime, tseat = transport
        return {"TransportPositionX": tx, "TransportPositionY": ty,
                "TransportPositionZ": tz, "TransportOrientation": to,
                "TransportSeat": tseat, "TransportTime": ttime}[name]

    out = bytearray()
    bw = BitWriter()

    def flush():
        nonlocal bw
        b = bw.flush()
        if b:
            out.extend(b)
        bw = BitWriter()

    for el in table:
        if el.startswith("HasTransportGuidByte"):
            if transport:
                idx = int(el[len("HasTransportGuidByte"):])
                bw.bit(tgb[idx] != 0)
            # else: hasTransportData is false, no bit consumed (read side guard)
        elif el.startswith("HasGuidByte"):
            idx = int(el[len("HasGuidByte"):])
            bw.bit(gb[idx] != 0)
        elif el.startswith("TransportGuidByte"):
            if transport:
                idx = int(el[len("TransportGuidByte"):])
                if tgb[idx]:
                    flush()
                    out.append(tgb[idx] ^ 1)
        elif el in ("TransportPositionX", "TransportPositionY", "TransportPositionZ", "TransportSeat", "TransportTime"):
            if transport:
                flush()
                v = transport_field(el)
                out += struct.pack("<b", v) if el == "TransportSeat" else                        struct.pack("<I", int(v) & 0xFFFFFFFF) if el == "TransportTime" else                        struct.pack("<f", v)
        elif el == "TransportOrientation":
            if transport:
                flush(); out += struct.pack("<f", transport_field(el))
        elif el.startswith("GuidByte"):
            idx = int(el[len("GuidByte"):])
            if gb[idx] != 0:
                flush()
                out.append(gb[idx] ^ 1)
        elif el == "HasMovementFlags":
            bw.bit(0)     # inverted has-bit: 0 = present, we always send flags
        elif el == "HasMovementFlags2":
            bw.bit(0)
        elif el == "HasTimestamp":
            bw.bit(0)
        elif el == "HasOrientation":
            bw.bit(0)
        elif el == "HasTransportData":
            bw.bit(1 if transport else 0)
        elif el == "HasTransportTime2":
            if transport:
                bw.bit(0)     # never send time2 (no interpolation)
        elif el == "HasTransportTime3":
            if transport:
                bw.bit(0)
        elif el == "HasPitch":
            bw.bit(1)     # inverted: 1 = absent
        elif el == "HasFallData":
            bw.bit(0)     # direct: 0 = absent
        elif el == "HasFallDirection":
            pass          # only consumed `if (hasFallData)`, which is false
        elif el == "HasSplineElevation":
            bw.bit(1)     # inverted: 1 = absent
        elif el == "HasCounter":
            bw.bit(1)     # inverted: 1 = absent
        elif el == "ZeroBit":
            bw.bit(0)
        elif el == "ForcesCount":
            bw.bits(0, 22)
        elif el == "Forces":
            pass
        elif el == "MovementFlags":
            bw.bits(movement_flags & 0x3FFFFFFF, 30)
        elif el == "MovementFlags2":
            bw.bits(movement_flags2 & 0x1FFF, 13)
        elif el == "Timestamp":
            flush(); out += struct.pack("<I", int(timestamp_ms) & 0xFFFFFFFF)
        elif el == "PositionX":
            flush(); out += struct.pack("<f", x)
        elif el == "PositionY":
            flush(); out += struct.pack("<f", y)
        elif el == "PositionZ":
            flush(); out += struct.pack("<f", z)
        elif el == "Orientation":
            flush(); out += struct.pack("<f", o)
        elif el in ("TransportTime2", "TransportTime3"):
            pass          # HasTransportTime2/3 were always sent false above
        elif el in ("Pitch", "FallTime", "FallVerticalSpeed", "FallCosAngle",
                    "FallSinAngle", "FallHorizontalSpeed", "SplineElevation",
                    "Counter"):
            pass          # every corresponding Has* bit above was sent false
        else:
            raise ValueError("unhandled movement element %r" % el)
    flush()
    return bytes(out)


# ---------------------------------------------------------------------------
# auth server
# ---------------------------------------------------------------------------
def srp6_interleave(s_le):
    buf0 = s_le[0::2]
    buf1 = s_le[1::2]
    p = 0
    while p < 32 and s_le[p] == 0:
        p += 1
    if p & 1:
        p += 1
    p //= 2
    h0 = sha1(buf0[p:])
    h1 = sha1(buf1[p:])
    out = bytearray(40)
    for i in range(20):
        out[2 * i] = h0[i]
        out[2 * i + 1] = h1[i]
    return bytes(out)


def _wow_fail_name(code):
    """#8: authserver AuthResult (AuthCodes.h:20) so a rejected login names the cause."""
    return {
        0x00: "WOW_SUCCESS", 0x03: "WOW_FAIL_BANNED", 0x04: "WOW_FAIL_UNKNOWN_ACCOUNT",
        0x05: "WOW_FAIL_INCORRECT_PASSWORD", 0x06: "WOW_FAIL_ALREADY_ONLINE",
        0x07: "WOW_FAIL_NO_TIME", 0x08: "WOW_FAIL_DB_BUSY", 0x09: "WOW_FAIL_VERSION_INVALID",
        0x0A: "WOW_FAIL_VERSION_UPDATE", 0x0B: "WOW_FAIL_INVALID_SERVER",
        0x0C: "WOW_FAIL_SUSPENDED", 0x0D: "WOW_FAIL_FAIL_NOACCESS",
        0x10: "WOW_FAIL_LOCKED_ENFORCED",
    }.get(code, "unknown")


def _world_auth_fail_name(code):
    """#8: SMSG_AUTH_RESPONSE error byte (SharedDefines.h ResponseCodes) as the world server sends it."""
    return {
        13: "AUTH_FAILED", 14: "AUTH_REJECT", 16: "AUTH_UNAVAILABLE", 17: "AUTH_SYSTEM_ERROR",
        20: "AUTH_VERSION_MISMATCH", 21: "AUTH_UNKNOWN_ACCOUNT", 22: "AUTH_INCORRECT_PASSWORD",
        23: "AUTH_SESSION_EXPIRED", 24: "AUTH_SERVER_SHUTTING_DOWN", 25: "AUTH_ALREADY_LOGGING_IN",
        27: "AUTH_WAIT_QUEUE", 28: "AUTH_BANNED", 29: "AUTH_ALREADY_ONLINE", 32: "AUTH_SUSPENDED",
    }.get(code, "unknown")


def auth_login(host, port, account, password, timeout=15.0):
    s = socket.create_connection((host, port), timeout=timeout)
    ident = account.upper().encode()
    req = (bytes([0x00, 0x00]) + struct.pack("<H", 30 + len(ident)) + b"WoW\x00" +
           bytes([5, 4, 8]) + struct.pack("<H", CLIENT_BUILD) + b"x86\x00" +
           b"niW\x00" + b"SUne" + struct.pack("<I", 0) + struct.pack("<I", 0) +
           bytes([len(ident)]) + ident)
    s.sendall(req)

    def recv(n):
        out = b""
        while len(out) < n:
            chunk = s.recv(n - len(out))
            if not chunk:
                raise RuntimeError("auth connection closed")
            out += chunk
        return out

    # #8: a rejected challenge is only cmd, 0x00, result (3 bytes, AuthSession.cpp LogonChallengeCallback),
    # so reading 119 at once blocked for the full socket timeout and then reported a generic failure.
    head = recv(3)
    if head[1] != 0x00 or head[2] != 0x00:
        raise RuntimeError("AUTH_LOGON_CHALLENGE rejected: result 0x%02X (%s) for account %r"
                           % (head[2], _wow_fail_name(head[2]), account))
    data = head + recv(116)

    # This authserver appends cmd, error, result (0x00) before the SRP6 block, so B starts at 3
    # (stock 5.4.8 starts it at 2). Detect from the N constant instead of assuming.
    B_le = salt = None
    flags = None
    for base in (3, 2):
        if len(data) < base + 36:
            continue
        cand_b = data[base:base + 32]
        glen = data[base + 32]
        g = data[base + 33:base + 33 + glen]
        nlen = data[base + 33 + glen]
        cand_n = data[base + 34 + glen:base + 34 + glen + nlen]
        off = base + 34 + glen + nlen
        if g == bytes([7]) and cand_n == N_BYTES_LE and len(data) >= off + 49:
            B_le, salt, flags = cand_b, data[off:off + 32], data[off + 48]
            break
    if B_le is None:
        raise RuntimeError("cannot locate SRP6 block in the %d-byte challenge" % len(data))
    if flags != 0:
        raise RuntimeError("auth server requests 2FA/PIN (securityFlags=%d)" % flags)

    a = random.getrandbits(152)
    A_int = pow(G_INT, a, N_INT)
    A_le = A_int.to_bytes(32, "little")
    B_int = int.from_bytes(B_le, "little")

    x = int.from_bytes(sha1(salt + sha1(("%s:%s" % (account.upper(), password.upper())).encode())), "little")
    u = int.from_bytes(sha1(A_le + B_le), "little")
    S_int = pow((B_int - K_MULT * pow(G_INT, x, N_INT)) % N_INT, a + u * x, N_INT)
    K = srp6_interleave(S_int.to_bytes(32, "little"))

    ng = bytes(p ^ q for p, q in zip(sha1(N_BYTES_LE), sha1(bytes([7]))))
    M1 = sha1(ng + sha1(account.upper().encode()) + salt + A_le + B_le + K)

    s.sendall(bytes([0x01]) + A_le + M1 + bytes(20) + bytes([0, 0]))
    resp = recv(2)
    if resp[1] != 0x00:
        raise RuntimeError("AUTH_LOGON_PROOF rejected: result 0x%02X (%s) for account %r"
                           % (resp[1], _wow_fail_name(resp[1]), account))
    rest = recv(30)
    M2 = rest[:20]
    expect = sha1(A_le + M1 + K)
    if M2 != expect:
        raise RuntimeError("M2 mismatch: server=%s expected=%s resp=%s" % (M2.hex(), expect.hex(), (resp + rest).hex()))
    log("auth OK account=%s K=%s" % (account, K.hex()[:16] + "..."))
    s.close()
    return K


# ---------------------------------------------------------------------------
# world connection
# ---------------------------------------------------------------------------
SMSG_SPELLNONMELEEDAMAGELOG = 0x1450                 # Opcodes.h


def decode_spellnonmeleedamagelog(body):
    """SMSG_SPELLNONMELEEDAMAGELOG, mirroring Unit::SendSpellNonMeleeDamageLog (Unit.cpp): 20 mask bits, then
    blocked, overkill, resist, absorb, damage, school, hit info and the spell id between the guid byte-seq bytes.
    Returns a dict with attacker, victim, spell, dmg. A spell hit (Gorerot's Smash 71774) is otherwise invisible."""
    r = ByteReader(body)
    a, t = [0] * 8, [0] * 8
    for kind, i in (("t", 2), ("a", 7), ("a", 6), ("a", 1), ("a", 5), ("x", 0), ("a", 0), ("t", 0), ("t", 7), ("a", 3),
                    ("t", 6), ("x", 0), ("p", 0), ("t", 1), ("f", 0), ("t", 5), ("a", 2), ("a", 4), ("t", 3), ("t", 4)):
        v = r.read_bit()
        if kind == "a":
            a[i] = v
        elif kind == "t":
            t[i] = v
        elif kind in ("p", "f") and v:
            raise ValueError("power/float data not supported")
    out = {}
    out["blocked"] = r.read_u32()
    a[1] = r.read_byte_seq(a[1])
    out["overkill"] = r.read_u32()
    t[3] = r.read_byte_seq(t[3])
    a[0] = r.read_byte_seq(a[0])
    t[6] = r.read_byte_seq(t[6])
    t[4] = r.read_byte_seq(t[4])
    a[7] = r.read_byte_seq(a[7])
    out["resist"] = r.read_u32()
    out["absorb"] = r.read_u32()
    a[5] = r.read_byte_seq(a[5])
    t[5] = r.read_byte_seq(t[5])
    a[3] = r.read_byte_seq(a[3])
    a[2] = r.read_byte_seq(a[2])
    t[2] = r.read_byte_seq(t[2])
    a[6] = r.read_byte_seq(a[6])
    t[0] = r.read_byte_seq(t[0])
    a[4] = r.read_byte_seq(a[4])
    out["dmg"] = r.read_u32()
    out["school"] = r.read_u8()
    t[7] = r.read_byte_seq(t[7])
    out["hit"] = r.read_u32()
    t[1] = r.read_byte_seq(t[1])
    out["spell"] = r.read_u32()
    out["attacker"] = int.from_bytes(bytes(a), "little")
    out["victim"] = int.from_bytes(bytes(t), "little")
    return out


def decode_attackerstateupdate(body):
    """SMSG_ATTACKERSTATEUPDATE, mirroring Unit::SendAttackStateUpdate: a flag bit, a size
    placeholder, hit info, both packed guids, damage, overkill, the sub-damage list, absorb and
    resist per sub-damage by hit info, victim state and the blocked amount.  Returns a dict."""
    if not body:
        raise ValueError("empty")
    if body[0] & 0x80:
        raise ValueError("unk-flags block present")
    pos = 5                                    # flag byte + u32 size placeholder

    def pguid(pos):
        mask = body[pos]
        pos += 1
        val = 0
        for i in range(8):
            if mask & (1 << i):
                val |= body[pos] << (8 * i)
                pos += 1
        return val, pos

    hit = struct.unpack_from("<I", body, pos)[0]
    pos += 4
    attacker, pos = pguid(pos)
    victim, pos = pguid(pos)
    dmg, over = struct.unpack_from("<II", body, pos)
    pos += 8
    count = body[pos]
    pos += 1
    subs = []
    for _ in range(count):
        school, fdmg, idmg = struct.unpack_from("<IfI", body, pos)
        pos += 12
        subs.append((school, fdmg, idmg))
    absorb = resist = blocked = 0
    if hit & 0x60:                             # FULL_ABSORB | PARTIAL_ABSORB
        for _ in range(count):
            absorb += struct.unpack_from("<I", body, pos)[0]
            pos += 4
    if hit & 0x180:                            # FULL_RESIST | PARTIAL_RESIST
        for _ in range(count):
            resist += struct.unpack_from("<I", body, pos)[0]
            pos += 4
    state = body[pos]
    pos += 1 + 4 + 4                           # victim state, unknown attacker state, melee spell id
    if hit & 0x2000:                           # HITINFO_BLOCK
        blocked = struct.unpack_from("<I", body, pos)[0]
    entry = 0
    hi = (attacker >> 48) & 0xFFFF
    if (hi >> 4) in (0xF13, 0xF15, 0xF11):     # Unit / Vehicle / GameObject carry the entry
        entry = (attacker >> 32) & 0xFFFFF
    return dict(hit=hit, attacker=attacker, victim=victim, entry=entry, dmg=dmg, over=over,
                subs=subs, absorb=absorb, resist=resist, blocked=blocked, state=state)


class WorldClient:
    def __init__(self, host, port, session_key, account, timeout=20.0):
        self.s = socket.create_connection((host, port), timeout=timeout)
        self.s.settimeout(timeout)
        self.crypt = WorldCrypt(session_key)
        # The real client normalizes the account name to uppercase for both the CMSG_AUTH_SESSION packet and its digest; the server signs the name it receives, so keep them identical.
        self.account = account.upper()
        self.login_time = time.time()
        self.last_activity = time.time()
        self.map_id = None
        self.pos = None
        self.orientation = 0.0
        self.in_bg_map = False
        self.char_guid = None
        self.char_name = None
        self.char_race = None
        self.language = LANG_COMMON
        self.bg_slot = None
        # #210/#215 S10 P6: captured from SMSG_LFG_PROPOSAL_UPDATE / SMSG_LFG_UPDATE_STATUS so
        # `lfg-accept auto` / `lfg-leave` don't need the id passed on the command line.
        self.lfg_proposal_id = None
        self.lfg_client_queue_id = None
        self.duel_request = None
        self.events = []          # notable observation events for the runner
        self.bp_confirm = None    # #969: last SMSG_BATTLE_PAY_CONFIRM_PURCHASE (server token + price)
        self.bp_updates = []      # #969: every SMSG_BATTLE_PAY_PURCHASE_UPDATE entry seen
        self.world_states = {}    # field id -> latest value (battleground objectives)
        self.bf_accept = False    # #900/#898 --bf-accept: answer battlefield-manager entry/queue invites with accept
        self.ws_log = []          # (seconds since login, name, value) on every change
        self.decompressor = zlib.decompressobj(-15)
        # S1a: group state kept up to date from SMSG_GROUP_LIST/PARTY_MEMBER_STATS
        self.group_members = {}   # name (lower) -> raw guid
        self.group_guid = None
        self.leader_guid = None
        self.loot_method = None
        self.dungeon_difficulty = None   # #373: last SMSG_SET_DUNGEON_DIFFICULTY value (sent at login and on every change)
        # S1b (issue #172): harness-supplied guids (from `.botscenario` SOAP
        # replies, see run_verb's `botscenario` verb), loot/roll/trade/death state.
        self.harness_guid = {}        # "npc"/"corpse"/etc -> raw guid
        self.move_guid = None         # `mover` verb: guid that move_stop/face/goto/walk stamp into MSG_MOVE_* (a driven vehicle); None = the character
        self.loot_source_guid = None
        self.loot_windows = []        # parsed SMSG_LOOT_RESPONSE windows not yet taken (loot-all)
        self.roll_guid = None
        self.roll_item_slot = None
        self.roll_item_id = None
        self.resurrect_request_guid = None
        self.summon_request_guid = None
        # S2 (issue #191): area-trigger enter/leave edge tracking and
        # on-transport movement state.
        self._at_inside = set()
        self.on_transport = False
        self.transport_guid = None
        self.transport_seat = -1
        self.transport_anchor = None
        # S9 owner requirement (#206): fixed local offset for the "natural walk-on" boarding
        # path (board_natural(), below) - see its docstring. None means "use the anchor/self.pos
        # arithmetic instead" (the older board_transport() path).
        self.transport_local = None
        self.gossip = None                # last SMSG_GOSSIP_MESSAGE (gossip-select / talk)
        self.char_class = None
        self.char_create_result = None    # last SMSG_CHAR_CREATE result byte (ResponseCodes)
        self.play = PlayState(self)       # client-side world model + real-player verbs (no GM help)

    # --- S1a: common EVT output (kept identical to headless335.py) --------
    def emit(self, kind, **fields):
        ms = int((time.time() - self.login_time) * 1000)
        parts = ["EVT", str(ms), kind]
        for k, v in fields.items():
            parts.append("%s=%s" % (k, v))
        print(" ".join(parts), flush=True)

    def resolve_target(self, arg):
        """Verb argument -> raw guid: a known group member name, or an integer
        (either a raw guid or a DB character-guid counter, whichever is larger
        than a plausible counter is treated as already-raw)."""
        if arg is None:
            return None
        low = str(arg).lower()
        if low in self.group_members:
            return self.group_members[low]
        try:
            val = int(arg)
        except ValueError:
            return None
        return val if val > 0xFFFFFFFF else player_guid(val)

    # --- raw io -----------------------------------------------------------
    def recv_exact(self, n):
        out = b""
        while len(out) < n:
            chunk = self.s.recv(n - len(out))
            if not chunk:
                raise RuntimeError("world connection closed")
            out += chunk
        return out

    def send_packet(self, opcode, body=b""):
        if not self.crypt.enabled:
            self.s.sendall(struct.pack("<HI", len(body) + 4, opcode) + body)
        else:
            value = (len(body) << 13) | (opcode & 0x1FFF)
            hdr = self.crypt.send.crypt(struct.pack("<I", value))
            self.s.sendall(hdr + body)
        self.last_activity = time.time()

    def read_packet(self):
        hdr = self.recv_exact(4)
        if self.crypt.enabled:
            hdr = self.crypt.recv.crypt(hdr)
            value = struct.unpack("<I", hdr)[0]
            opcode = value & 0x1FFF
            size = value >> 13
        else:
            # pre-auth server header: uint16 size = payload + 2, uint16 opcode
            raw_size, opcode = struct.unpack("<HH", hdr)
            size = raw_size - 2
        if size > 0x7FFFF:   # the header's 19-bit size field; the 10236 limit is for client->server only
            raise RuntimeError("bad packet size %d opcode 0x%X (raw %s)" % (size, opcode, hdr.hex()))
        body = self.recv_exact(size) if size else b""
        if opcode == SMSG_COMPRESSED_PACKET:
            if len(body) < 12:
                raise RuntimeError("short SMSG_COMPRESSED_PACKET (%d bytes)" % len(body))
            raw = self.decompressor.decompress(body[12:])
            if len(raw) < 4:
                raise RuntimeError("empty decompressed payload")
            opcode = struct.unpack_from("<I", raw, 0)[0]
            body = raw[4:]
        self.last_activity = time.time()
        if TRACE:
            log("S->C 0x%04X size=%d%s" % (opcode, size,
                (" head=" + body[:16].hex()) if size > 0x400 else ""))
        return opcode, body

    # --- handshake --------------------------------------------------------
    def handshake(self, session_key, client_seed=None, realm_id=1):
        hdr = self.recv_exact(4)
        size, opc = struct.unpack("<HH", hdr)
        payload = self.recv_exact(size - 2)
        if opc != MSG_VERIFY_CONNECTIVITY:
            raise RuntimeError("expected connectivity, got 0x%X" % opc)
        # Core's WorldSocket::InitializeHandler waits for exactly 2 + 47 + 1 = 50 bytes
        # (ClientConnectionInitialize is 47 chars) and compares bytes[2..49] with the full
        # string; the client must also send a trailing byte (the buffer must fill completely).
        init_payload = b"WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER"
        self.s.sendall(struct.pack("<H", len(init_payload)) + init_payload + b"\x00")

        op, body = self.read_packet()
        if op != SMSG_AUTH_CHALLENGE:
            raise RuntimeError("expected SMSG_AUTH_CHALLENGE, got 0x%X" % op)
        if len(body) != 39:
            raise RuntimeError("unexpected auth challenge size %d" % len(body))
        server_seed = body[35:39]

        client_seed = client_seed or os.urandom(4)
        digest = sha1(self.account.upper().encode() + bytes(4) + client_seed + server_seed + session_key)

        pkt = bytearray()
        pkt += struct.pack("<II", 0, 0)
        pkt += bytes([digest[18], digest[14], digest[3], digest[4], digest[0]])
        pkt += struct.pack("<I", realm_id)
        pkt += bytes([digest[11]])
        pkt += client_seed
        pkt += bytes([digest[19], 0, 0, digest[2], digest[9], digest[12]])
        pkt += struct.pack("<QI", 0, 0)
        pkt += bytes([digest[16], digest[5], digest[6], digest[8]])
        pkt += struct.pack("<H", CLIENT_BUILD)
        pkt += bytes([digest[17], digest[7], digest[13], digest[15], digest[1], digest[10]])
        # Addon blob: the core calls addonsData.contents() which throws on empty
        # storage, so send a 4-byte "size = 0" blob; WorldSession::ReadAddonsInfo
        # returns early when that size is 0.
        pkt += struct.pack("<I", 4) + b"\x00\x00\x00\x00"
        bw = BitWriter()
        bw.bit(0)
        bw.bits(len(self.account), 11)
        pkt += bw.flush()
        pkt += self.account.encode()

        self.s.sendall(struct.pack("<HI", len(pkt) + 4, CMSG_AUTH_SESSION) + bytes(pkt))
        self.crypt.enabled = True
        log("sent CMSG_AUTH_SESSION (%d bytes) seed=%s pkt=%s" %
            (len(pkt), client_seed.hex(), bytes(pkt[:24]).hex()))

        # w14: under a login burst (6 wave chains starting at once) the auth response can take > 20 s; give the
        # handshake 60 s, then restore the normal socket timeout.
        _old_to = self.s.gettimeout(); self.s.settimeout(max(_old_to or 0, 60.0))
        for _ in range(10):
            op, body = self.read_packet()
            if op == SMSG_AUTH_RESPONSE:
                self.s.settimeout(_old_to)
                break
            # Warden's module-use packet is sent before the auth response; ignore it.
            log("pre-auth packet 0x%X (%d bytes) - ignored" % (op, len(body)))
        else:
            raise RuntimeError("no SMSG_AUTH_RESPONSE after auth session")
        if not body or not (body[0] & 0x80):
            # #8: error form is flags byte 0x00 then the ResponseCodes byte (WorldSocket::SendAuthResponseError)
            code = body[1] if len(body) > 1 else 0xFF
            raise RuntimeError("AUTH_RESPONSE not OK (first byte 0x%02X, code %d %s) for account %r"
                               % (body[0] if body else 0xFF, code, _world_auth_fail_name(code), account))
        log("world auth OK (AUTH_OK bit set)")

    def char_enum(self, timeout=60.0):
        # w15: under wave load the MoP world thread ran 1-2 s ticks and the enum answer took longer than the 20 s
        # socket timeout (two headless legs died in recv). A real client waits; widen the socket timeout like
        # player_login does.
        self.send_packet(CMSG_ENUM_CHARACTERS)
        deadline = time.time() + timeout
        chars = []
        _old_to = self.s.gettimeout(); self.s.settimeout(max(_old_to or 0, timeout))
        try:
            while time.time() < deadline:
                op, body = self.read_packet()
                if op == SMSG_ENUM_CHARACTERS_RESULT:
                    chars = self.parse_chars(body)
                    return chars
        finally:
            self.s.settimeout(_old_to)
        return chars

    def parse_chars(self, body):
        """SMSG_ENUM_CHARACTERS_RESULT parse (5.4.8, bit-packed).

        Replays Player::BuildEnumData/WorldSession::HandleCharEnum exactly
        (src/server/game/Entities/Player/Player.cpp:1602-1809 and
        Handlers/CharacterHandler.cpp:278-338): a header of 21+16 bits, then
        per character a run of 19 bits (guid/guild-guid mask + a 6-bit name
        length), a trailing success bit, FlushBits, and only then the
        byte-aligned per-character data (23*9 bytes of equipment visuals we
        don't need, so they are skipped rather than decoded).
        """
        result = []
        try:
            r = ByteReader(body)
            r.read_bits(21)                       # factionChangeRestrictions
            count = r.read_bits(16)
            entries = []
            for _ in range(count):
                guild_mask = [0] * 8
                guid_mask = [0] * 8
                guild_mask[4] = r.read_bit()
                guid_mask[0] = r.read_bit()
                guild_mask[3] = r.read_bit()
                guid_mask[3] = r.read_bit()
                guid_mask[7] = r.read_bit()
                r.read_bit()                       # boosted
                r.read_bit()                       # AT_LOGIN_FIRST
                guid_mask[6] = r.read_bit()
                guild_mask[6] = r.read_bit()
                name_len = r.read_bits(6)
                guid_mask[1] = r.read_bit()
                guild_mask[1] = r.read_bit()
                guild_mask[0] = r.read_bit()
                guid_mask[4] = r.read_bit()
                guild_mask[7] = r.read_bit()
                guid_mask[2] = r.read_bit()
                guid_mask[5] = r.read_bit()
                guild_mask[2] = r.read_bit()
                guild_mask[5] = r.read_bit()
                entries.append((guid_mask, guild_mask, name_len))
            r.read_bit()                           # success
            r.skip(0)                              # force the FlushBits realignment

            for guid_mask, guild_mask, name_len in entries:
                guid = [0] * 8
                guild = [0] * 8
                r.read_u32()                       # UNK02
                guid[1] = r.read_byte_seq(guid_mask[1])
                slot = r.read_u8()
                hair_style = r.read_u8()
                guild[2] = r.read_byte_seq(guild_mask[2])
                guild[0] = r.read_byte_seq(guild_mask[0])
                guild[6] = r.read_byte_seq(guild_mask[6])
                name = r.read_string(name_len).decode("utf-8", "replace")
                guild[3] = r.read_byte_seq(guild_mask[3])
                x = r.read_f32()
                r.read_u32()                       # UNK00 / boost
                face = r.read_u8()
                klass = r.read_u8()
                guild[5] = r.read_byte_seq(guild_mask[5])
                r.skip(23 * 9)                     # visible equipment (unused)
                r.read_u32()                       # customization flags
                guid[3] = r.read_byte_seq(guid_mask[3])
                guid[5] = r.read_byte_seq(guid_mask[5])
                r.read_u32()                       # pet family
                guild[4] = r.read_byte_seq(guild_mask[4])
                map_id = r.read_u32()
                race = r.read_u8()
                r.read_u8()                        # skin
                guild[1] = r.read_byte_seq(guild_mask[1])
                level = r.read_u8()
                guid[0] = r.read_byte_seq(guid_mask[0])
                guid[2] = r.read_byte_seq(guid_mask[2])
                r.read_u8()                        # hair color
                r.read_u8()                        # gender
                r.read_u8()                        # facial hair
                r.read_u32()                       # pet level
                guid[4] = r.read_byte_seq(guid_mask[4])
                guid[7] = r.read_byte_seq(guid_mask[7])
                y = r.read_f32()
                r.read_u32()                       # pet display id
                r.read_u32()                       # UNK03
                guid[6] = r.read_byte_seq(guid_mask[6])
                r.read_u32()                       # character flags
                zone = r.read_u32()
                guild[7] = r.read_byte_seq(guild_mask[7])
                z = r.read_f32()

                full_guid = int.from_bytes(bytes(guid), "little")
                result.append({"guid": full_guid, "name": name, "level": level,
                                "race": race, "class": klass, "map": map_id,
                                "zone": zone, "pos": (x, y, z), "slot": slot})
        except (IndexError, struct.error) as exc:
            log("char list parse stopped after %d/%d characters: %s" %
                (len(result), body[0] if body else -1, exc))
        return result

    def player_login(self, guid, timeout=60.0):
        # This core stores player guids as counter | (HighGuid::Player(0x070) << 52)
        # (ObjectGuid::ObjectGuid(HighGuid, counter)) - send the FULL raw value, not just the counter.
        raw_guid = (0x070 << 52) | (int(guid) & 0xFFFFFFFFFFFF)
        self.char_guid = raw_guid
        body = struct.pack("<I", 0) + packed_guid(raw_guid, (1, 4, 7, 3, 2, 6, 5, 0),
                                                  (5, 1, 0, 6, 2, 4, 7, 3))
        self.send_packet(CMSG_PLAYER_LOGIN, body)
        deadline = time.time() + timeout
        # #412 w14: under wave load (6 chains) the server sent nothing for > 20 s after CMSG_PLAYER_LOGIN and the 20 s
        # socket timeout raised TimeoutError out of main() (a MoP fishing master never logged in). Same remedy as
        # the auth handshake above: wait up to `timeout` (60 s) for SMSG_LOGIN_VERIFY_WORLD, then restore the timeout.
        _old_to = self.s.gettimeout(); self.s.settimeout(max(_old_to or 0, timeout))
        try:
            return self._player_login_wait(deadline)
        except socket.timeout:
            log("player_login: no SMSG_LOGIN_VERIFY_WORLD within %.0fs" % timeout)
            return False
        finally:
            self.s.settimeout(_old_to)

    def _player_login_wait(self, deadline):
        while time.time() < deadline:
            op, body = self.read_packet()
            if op == SMSG_LOGIN_VERIFY_WORLD:
                # bug fix (issue #165): CharacterHandler.cpp:918-923 writes x, o
                # (orientation), y, mapId, z - in that order - so z was never
                # read before; `o` is the facing, not part of the xyz tuple.
                x, o, y = struct.unpack("<fff", body[:12])
                self.map_id = struct.unpack("<I", body[12:16])[0]
                z = struct.unpack("<f", body[16:20])[0]
                self.pos = (x, y, z)
                self.orientation = o
                log("in world: map=%d pos=(%.1f, %.1f, %.1f) o=%.2f" %
                    (self.map_id, x, y, z, o))
                return True
            self.auto(op, body)
        return False

    # --- observation helpers ---------------------------------------------
    def _parse_bg_status(self, op, body):
        """Decode the queue slot from a bit-packed SMSG_BATTLEFIELD_STATUS_*.

        Replays the exact write order of
        BattlegroundMgr::BuildBattlegroundStatusPacket: guid mask bits first,
        then the interleaved fields and conditional byte-seq bytes. WriteByteSeq
        only emits a byte when the guid byte is non-zero, so offsets are
        data-dependent and cannot be hardcoded.
        """
        try:
            r = ByteReader(body)
            if op == SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION:
                # Bits in exact write order (BattlegroundMgr.cpp:265-282).
                # Names: p=playerGuid, b=bgGuid; - = literal (MissingRole/IsRated).
                bits = ['p7', 'p5', 'p4', 'p1', 'b3', 'b2', '-', 'b0',
                        'p0', 'p6', 'b7', 'b4', 'b1', '-', 'p2', 'b6', 'p3', 'b5']
                m = {}
                for name in bits:
                    v = r.read_bit()
                    if name != '-':
                        m[name] = v
                r.read_byte_seq(m['p1'])
                r.read_byte_seq(m['b1'])
                r.read_byte_seq(m['p2'])
                r.read_u8()          # player count
                self.bg_slot = r.read_u32() & 0xFF
            elif op == SMSG_BATTLEFIELD_STATUS_QUEUED:
                # Bits in exact write order (BattlegroundMgr.cpp:211-230).
                bits = ['b1', 'b5', 'p0', 'b7', '-', '-', '-', 'p6',
                        'p4', 'p1', 'p3', 'p7', 'p5', '-', 'b3', 'b0',
                        'b2', 'p2', 'b6', 'b4']
                m = {}
                for name in bits:
                    v = r.read_bit()
                    if name != '-':
                        m[name] = v
                r.read_byte_seq(m['b4'])
                r.read_byte_seq(m['p6'])
                r.read_u32()      # Id
                r.read_byte_seq(m['b5'])
                r.read_u32()      # Time1
                r.read_byte_seq(m['b3'])
                r.read_byte_seq(m['p3'])
                r.read_byte_seq(m['p4'])
                r.read_byte_seq(m['p0'])
                r.read_u8()       # MinLevel
                r.read_byte_seq(m['b0'])
                r.read_u32()      # Time2
                r.read_u8()       # premade
                r.read_byte_seq(m['b1'])
                r.read_u32()      # sinceJoined
                r.read_byte_seq(m['b7'])
                self.bg_slot = r.read_u32() & 0xFF
        except (IndexError, struct.error):
            self.bg_slot = None

    def _parse_duel_requested(self, body):
        """Decode the caster (initiator) guid of SMSG_DUEL_REQUESTED.

        Replays SpellEffects.cpp (Spell::EffectDuel): 16 mask bits (arbiter and
        caster interleaved) then conditional byte seqs, in the exact write order.
        """
        try:
            r = ByteReader(body)
            # (guid, byte-index) in write order; arbiter=a, caster=c.
            mask_order = [('a', 5), ('c', 4), ('c', 2), ('c', 7), ('a', 0), ('c', 5),
                          ('a', 4), ('a', 6), ('c', 1), ('c', 3), ('c', 6), ('a', 7),
                          ('a', 3), ('a', 2), ('a', 1), ('c', 0)]
            mask = {'a': [0] * 8, 'c': [0] * 8}
            for which, idx in mask_order:
                mask[which][idx] = r.read_bit()
            byte_order = [('a', 5), ('a', 3), ('c', 7), ('c', 4), ('a', 7), ('c', 3),
                          ('c', 6), ('c', 0), ('a', 4), ('c', 2), ('c', 1), ('a', 0),
                          ('a', 2), ('a', 6), ('a', 1), ('c', 5)]
            raw = {'a': [0] * 8, 'c': [0] * 8}
            for which, idx in byte_order:
                if mask[which][idx]:
                    raw[which][idx] = r.read_u8() ^ 1
            return int.from_bytes(bytes(raw['c']), "little")
        except (IndexError, struct.error):
            return None

    def _parse_duel_winner(self, body):
        """Decode SMSG_DUEL_WINNER (Player.cpp:8124-8134): fled-bit,
        opponent-name-length(6 bits), self-name-length(6 bits), then
        [byte-phase] realm id(u32), opponent name string, realm id(u32), self
        name string. There is no explicit FlushBits() between the 13 packed
        bits and the following u32/string on the write side (unlike
        SMSG_DUEL_COMPLETE); ByteReader.read_u32()/read_string() both call
        _reset_bits() first (mirrors ByteBuffer::ResetBitPos), so this only
        decodes correctly if the write side auto-aligns the same way every
        other raw field in this file does - flagged as unverified against a
        real capture in the issue #191 report."""
        try:
            r = ByteReader(body)
            fled = r.read_bit()
            opp_len = r.read_bits(6)
            self_len = r.read_bits(6)
            r.read_u32()  # realm id
            opp_name = r.read_string(opp_len).decode("utf-8", errors="replace")
            r.read_u32()  # realm id
            self_name = r.read_string(self_len).decode("utf-8", errors="replace")
            return fled, opp_name, self_name
        except (IndexError, struct.error):
            return None

    # --- housekeeping -----------------------------------------------------
    def auto(self, op, body):
        self.play.on_packet(op, body)
        if op == SMSG_TIME_SYNC_REQUEST:
            counter = struct.unpack("<I", body[:4])[0] if len(body) >= 4 else 0
            # w32: a sync request before login_time is set made ticks = epoch ms (1.8e12) and struct.pack('<I') raised, killing the
            # session (t6xx-ssb-w32-1818). The client's tick counter is a wrapping uint32, so mask it.
            ticks = int((time.time() - (self.login_time or time.time())) * 1000) & 0xFFFFFFFF
            self.send_packet(CMSG_TIME_SYNC_RESPONSE, struct.pack("<II", counter, ticks))
        elif op == SMSG_BATTLEFIELD_STATUS_QUEUED:
            self._parse_bg_status(op, body)
            log("BF status: QUEUED")
            self.events.append(("bg_queued", self.bg_slot))
            self.emit("bg", status="queued", slot=self.bg_slot, map=self.map_id)
        elif op == SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION:
            self._parse_bg_status(op, body)
            log("BF status: NEED CONFIRMATION (popup, slot %s)" % self.bg_slot)
            self.events.append(("bg_need_confirmation", self.bg_slot))
            self.emit("bg", status="confirm", slot=self.bg_slot, map=self.map_id)
        elif op == SMSG_BATTLEFIELD_STATUS_ACTIVE:
            self._parse_bg_status(op, body)
            log("BF status: ACTIVE (in battleground)")
            self.events.append(("bg_active", self.bg_slot))
            self.emit("bg", status="active", slot=self.bg_slot, map=self.map_id)
        elif op in (SMSG_BATTLEFIELD_MGR_ENTRY_INVITE, SMSG_BATTLEFIELD_MGR_QUEUE_INVITE):
            self._on_bf_invite(op, body)
        elif op in (SMSG_BATTLEFIELD_MGR_ENTERED, SMSG_BATTLEFIELD_MGR_EJECTED, SMSG_BATTLEFIELD_MGR_QUEUE_REQUEST_RESPONSE):
            log("%s (battlefield manager)" % OPCODE_NAMES.get(op, hex(op)))
            self.events.append(("bf_" + OPCODE_NAMES[op].split("MGR_")[1].lower(), body[:16].hex()))
        elif op == SMSG_SET_DUNGEON_DIFFICULTY:
            # #373: Player::SendDungeonDifficulty (Player.cpp:22163): one uint32 (sent at login, Player.cpp:25336, and to every
            # group member when the leader changes it).
            if len(body) >= 4:
                self.dungeon_difficulty = struct.unpack("<I", body[:4])[0]
                log("SMSG_SET_DUNGEON_DIFFICULTY mode=%d" % self.dungeon_difficulty)
                self.events.append(("dungeon_difficulty", self.dungeon_difficulty))
                self.emit("dungeon_difficulty", mode=self.dungeon_difficulty)
        elif op == SMSG_UPDATE_WORLD_STATE:
            # Player::SendUpdateWorldState: one bit (0), then uint32 value, uint32 field.
            # The leading bit is flushed to a whole byte, so the payload is 1 + 4 + 4.
            if len(body) >= 9:
                value, field = struct.unpack("<II", body[1:9])
                if field in WORLD_STATES:
                    name = WORLD_STATES[field]
                    if self.world_states.get(field) != value:
                        self.world_states[field] = value
                        self.ws_log.append((round(time.time() - self.login_time, 1), name, value))
                        log("worldstate %s = %d" % (name, value))
        elif op == SMSG_BATTLEFIELD_STATUS_FAILED:
            # BuildStatusFailedPacket (BattlegroundMgr.cpp) body: uint32 joinTime, uint32 id,
            # uint32 queueSlot, uint32 result. The result is the GroupJoinBattlegroundResult code and
            # was previously thrown away, which made every refused BG join indistinguishable - the
            # same diagnosability gap as the silent `return;` sites in HandleBattlemasterJoinOpcode.
            reason = struct.unpack("<I", body[12:16])[0] if len(body) >= 16 else -1
            log("BF status: FAILED reason=%d (body=%s)" % (reason, body[:24].hex()))
            self.events.append(("bg_failed", reason))
            self.emit("bg", status="none", slot=self.bg_slot, map=self.map_id)
        elif op == SMSG_LFG_PROPOSAL_UPDATE:
            self._parse_lfg_proposal_update(body)
        elif op == SMSG_LFG_ROLE_CHECK_UPDATE:
            self._parse_lfg_role_check_update(body)
        elif op == SMSG_LFG_UPDATE_STATUS:
            self._parse_lfg_update_status(body)
        elif op == SMSG_RAID_READY_CHECK:
            log("SMSG_RAID_READY_CHECK: ready check started")
            self.emit("readycheck", event="start")
        elif op == SMSG_RAID_READY_CHECK_CONFIRM:
            self._parse_ready_check_confirm(body)
        elif op == SMSG_RAID_READY_CHECK_COMPLETED:
            log("SMSG_RAID_READY_CHECK_COMPLETED")
            self.events.append(("readycheck_done", None))
            self.emit("readycheck", event="done")
        elif op == SMSG_DUEL_REQUESTED:
            # SMSG_DUEL_REQUESTED: bit-packed initiator guid then arbiter guid.
            self.duel_request = self._parse_duel_requested(body)
            log("SMSG_DUEL_REQUESTED from guid %s" % self.duel_request)
            self.events.append(("duel_requested", self.duel_request))
            self.emit("duel_requested", frm=self.duel_request)
        elif op == SMSG_DUEL_COUNTDOWN:
            self.events.append(("duel_countdown", None))
            self.emit("duel_countdown")
        elif op == SMSG_DUEL_COMPLETE:
            log("SMSG_DUEL_COMPLETE")
            self.events.append(("duel_complete", None))
            # Player.cpp:8108-8122: WriteBit(type != DUEL_INTERRUPTED) then
            # FlushBits() -> a single byte, nonzero iff completed normally.
            self.emit("duel_complete", completed=1 if (body and body[0]) else 0)
        elif op == SMSG_DUEL_WINNER:
            log("SMSG_DUEL_WINNER")
            self.events.append(("duel_winner", None))
            parsed = self._parse_duel_winner(body)
            if parsed:
                fled, opp_name, self_name = parsed
                self.emit("duel_winner", fled=fled, opponent=json.dumps(opp_name), self_name=json.dumps(self_name))
            else:
                self.emit("duel_winner", fled="?")
        elif op == SMSG_CAST_FAILED:
            # Spell.cpp:4401-: spellId(4) + result(4) + cast_count(1), diagnostic only - not decoding
            # the result-specific trailing payload some failure codes append.
            try:
                spell_id, result = struct.unpack("<II", body[0:8])
                # The server repeats an Auto Shot failure on every world tick (Unit::_UpdateAutoRepeatSpell),
                # thousands per second: log a repeat of the same spell/result at most once a second.
                now_cf = time.time()
                last_cf = getattr(self, "_cast_fail_last", None)
                if last_cf and last_cf[0] == (spell_id, result) and now_cf - last_cf[1] < 1.0:
                    self._cast_fail_last = (last_cf[0], last_cf[1], last_cf[2] + 1)
                else:
                    rep_n = last_cf[2] if last_cf and last_cf[0] == (spell_id, result) else 0
                    log("SMSG_CAST_FAILED spell=%d result=%d%s" % (spell_id, result,
                        (" (+%d repeats)" % rep_n) if rep_n else ""))
                    self.events.append(("cast_failed", "%d:%d" % (spell_id, result)))
                    self._cast_fail_last = ((spell_id, result), now_cf, 0)
            except struct.error as exc:
                log("SMSG_CAST_FAILED parse failed: %s" % exc)
        elif op == SMSG_SPELL_GO:
            # Spell.cpp:4873-5358 SendSpellGo (bit-packed, variable length). The spellId is a uint32
            # near the END of the packet, followed by [elevation float if ADJUST_MISSILE] and the
            # non-zero bytes of casterUnitGuid[0,3,4] and casterGuid[7] (5343-5351). For a self-cast
            # we know casterUnitGuid, so report the candidate ids (casterGuid[7] is the item guid's
            # byte for an item use, else the same unit guid byte). Casts by other units decode as
            # garbage candidates - match them against the spell you expect.
            cands = []
            try:
                cg = (self.char_guid or 0).to_bytes(8, "little")
                n = sum(1 for i in (0, 3, 4) if cg[i])
                for k in (n, n + 1):
                    if len(body) >= k + 4:
                        v = struct.unpack("<I", body[len(body) - k - 4:len(body) - k])[0]
                        if v not in cands:
                            cands.append(v)
            except (struct.error, AttributeError, OverflowError):
                pass
            cand_s = "|".join(str(c) for c in cands) or "?"
            log("SMSG_SPELL_GO (%d bytes) spell_candidates=%s %s" % (len(body), cand_s, body[:16].hex()))
            if os.environ.get("HEADLESS_DUMP_SPELLGO") == "1":
                log("SMSG_SPELL_GO-FULL %s" % body.hex())   # diagnostics: the whole packet, to decode the spell id offline
            self.events.append(("spell_go", cand_s))
            self.emit("spell_go", size=len(body), spell=cand_s)
        elif op == SMSG_INVENTORY_CHANGE_FAILURE:
            # Player::SendEquipError - the usual reply when CMSG_USE_ITEM is rejected before a cast
            # (ITEM_NOT_FOUND on a bad bag/slot/guid, cooldown, level, ...). Diagnostic hex only.
            log("SMSG_INVENTORY_CHANGE_FAILURE (%d bytes) %s" % (len(body), body[:24].hex()))
            self.emit("inventory_change_failure", size=len(body))
        elif op == SMSG_TRANSFER_PENDING:
            log("transfer pending (teleport), map=%s" %
                (struct.unpack("<I", body[1:5])[0] if len(body) >= 5 else "?"))
        elif op == SMSG_NEW_WORLD:
            self.map_id = struct.unpack("<I", body[4:8])[0]
            log("SMSG_NEW_WORLD map=%d -> ack worldport" % self.map_id)
            self.send_packet(MSG_MOVE_WORLDPORT_ACK)
            self.emit("worldport", map=self.map_id)
        elif op == SMSG_LOGIN_VERIFY_WORLD:
            x, o, y = struct.unpack("<fff", body[:12])
            self.map_id = struct.unpack("<I", body[12:16])[0]
            self.pos = (x, y, struct.unpack("<f", body[16:20])[0])
            log("SMSG_LOGIN_VERIFY_WORLD map=%d" % self.map_id)
        elif op == SMSG_MOVE_TELEPORT:
            # S1a near-teleport ack (issue #165): the server sends this to
            # itself/us to trigger CMSG_MOVE_TELEPORT_ACK and finish moving the
            # player server-side (Unit.cpp:18621 SendTeleportPacket). Without
            # an ack every subsequent movement packet from a same-map .go/.tele
            # is dropped (MovementHandler.cpp:321 IsBeingTeleportedNear gate).
            self.teleport_ack()
            log("SMSG_MOVE_TELEPORT -> sent CMSG_MOVE_TELEPORT_ACK")
            self.emit("teleport_near", pos="%.1f,%.1f,%.1f" % self.pos if self.pos else "?")
        elif op == SMSG_MESSAGECHAT:
            msg = self._parse_messagechat(body)
            if msg:
                # kept for `react` (a player answering a whispered/emoted prompt, e.g. the Kezan vault tools)
                if not hasattr(self, "chat_lines"):
                    self.chat_lines = []
                self.chat_lines.append((time.time(), msg["type_name"], str(msg.get("message", ""))))
                del self.chat_lines[:-200]
                self.emit("chat", type=msg["type_name"], lang=msg["lang"],
                           frm=msg.get("sender_name") or msg.get("sender_guid"),
                           msg=json.dumps(msg["message"]))
        elif op == SMSG_GUILD_INVITE:
            gi = self._parse_guild_invite(body)
            if gi:
                self.guild_invite = gi
                log("[GUILD] SMSG_GUILD_INVITE from=%s guild=%s" % (gi[0], gi[1]))
                self.emit("guild_invite", frm=gi[0], guild=json.dumps(gi[1]))
        elif op == SMSG_CHANNEL_NOTIFY:
            if body:
                log("[CHANNEL] notify type=%d channel=%s" % (body[0], body[1:].split(b"\0")[0].decode("utf-8", "replace")))
                self.emit("channel_notify", ntype=body[0],
                           channel=body[1:].split(b"\0")[0].decode("utf-8", "replace"))
        elif op == SMSG_GROUP_LIST:
            self._parse_group_list(body)
        elif op == SMSG_PARTY_MEMBER_STATS:
            self._parse_party_member_stats(body)
        elif op == SMSG_CHAR_CREATE:
            code = body[0] if body else 0xFF
            self.char_create_result = code
            log("SMSG_CHAR_CREATE result=%d (%s)" % (code, CHAR_CREATE_CODES.get(code, "unknown")))
            self.emit("char_create", result=code, name=CHAR_CREATE_CODES.get(code, "unknown"))
        elif op == SMSG_LOOT_RESPONSE:
            self._parse_loot_response(body)
        elif op == SMSG_LOOT_START_ROLL:
            self._parse_loot_start_roll(body)
        elif op == SMSG_LOOT_ROLL:
            self._parse_loot_roll(body)
        elif op == SMSG_LOOT_ROLL_WON:
            self._parse_loot_roll_won(body)
        elif op == SMSG_TRADE_STATUS:
            self._parse_trade_status(body)
        elif op == SMSG_RESURRECT_REQUEST:
            self._parse_resurrect_request(body)
        elif op == SMSG_SUMMON_REQUEST:
            self._parse_summon_request(body)
        elif op in (SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE, SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE,
                    SMSG_BATTLE_PAY_CONFIRM_PURCHASE, SMSG_BATTLE_PAY_PURCHASE_UPDATE,
                    SMSG_BATTLE_PAY_DELIVERY_ENDED, SMSG_BATTLE_PAY_DISTRIBUTION_UPDATE):
            try:
                self._parse_battle_pay(op, body)
            except (IndexError, struct.error) as exc:
                log("BattlePay 0x%04X parse failed: %s (body=%s)" % (op, exc, body[:48].hex()))
        elif op == 0x12AF and body:
            # SMSG_ATTACKSTOP: a stun / knock-down stops this character's own auto-attack server-side; remember it so the
            # fight sends CMSG_ATTACKSWING again (a raptor's Head Butt left a warrior swinging at nothing for the rest of the fight)
            self.attackstop_t = time.time()
        elif op == SMSG_SPELLNONMELEEDAMAGELOG:
            # a spell hit on (or by) this character - logged, and counted as a hit in melee_log so attackers() /
            # hit_by() see a caster whose target field never showed this character (four Smashes, no pull-back)
            try:
                m = decode_spellnonmeleedamagelog(body)
                me_g = self.char_guid or 0
                if me_g and me_g in (m["attacker"], m["victim"]):
                    log("[SPELLDMG] attacker=0x%016X spell=%d dmg=%d overkill=%d absorb=%d resist=%d blocked=%d victim=0x%016X"
                        % (m["attacker"], m["spell"], m["dmg"], m["overkill"], m["absorb"], m["resist"], m["blocked"],
                           m["victim"]))
                if not hasattr(self, "melee_log"):
                    self.melee_log = []
                self.melee_log.append((time.time(), m["attacker"], m["victim"], m["dmg"]))
                del self.melee_log[:-300]
            except (IndexError, struct.error, ValueError) as exc:
                log("SMSG_SPELLNONMELEEDAMAGELOG parse failed: %s (body=%s)" % (exc, body[:48].hex()))
        elif op == 0x06AA:
            try:
                m = decode_attackerstateupdate(body)
                log("[MELEE] attacker=0x%016X entry=%d hit=0x%x dmg=%d absorb=%d resist=%d blocked=%d state=%d victim=0x%016X"
                    % (m["attacker"], m["entry"], m["hit"], m["dmg"], m["absorb"], m["resist"],
                       m["blocked"], m["state"], m.get("victim") or 0))
                if not hasattr(self, "melee_log"):
                    self.melee_log = []
                self.melee_log.append((time.time(), m["attacker"], m.get("victim") or 0, m["dmg"]))
                del self.melee_log[:-300]
            except (IndexError, struct.error, ValueError) as exc:
                log("SMSG_ATTACKERSTATEUPDATE parse failed: %s (body=%s)" % (exc, body[:48].hex()))
            self.emit("observed", op="SMSG_ATTACKERSTATEUPDATE", size=len(body))
        elif op in OBSERVED_470:
            log("%s (%d bytes) %s" % (OBSERVED_470[op], len(body), body[:16].hex()))
            self.emit("observed", op=OBSERVED_470[op], size=len(body))
        elif op == SMSG_GOSSIP_MESSAGE:
            try:
                self._parse_gossip_message(body)
            except (IndexError, struct.error) as exc:
                log("SMSG_GOSSIP_MESSAGE parse failed: %s (body=%s)" % (exc, body[:64].hex()))

    def logout(self, timeout=30.0):
        """#8 (wave27): end the session like a real client. A bare socket close leaves the character IN the
        world for the server's 60 s disconnect linger (World.cpp offline sessions), during which Player::Update
        keeps sending SMSG_TIME_SYNC_REQ to a dead socket: exactly 6 requests, and 'Not received
        CMSG_TIME_SYNC_RESP ... possible cheater' on the 4th, 5th and 6th = 3 lines per client exit. A GM
        account logs out instantly (MiscHandler.cpp:551 HandleLogoutRequestOpcode -> SMSG_LOGOUT_COMPLETE at
        once); others sit for the 20 s timer, pumped."""
        try:
            self.send_packet(CMSG_LOGOUT_REQUEST)
            deadline = time.time() + timeout
            while time.time() < deadline:
                ready, _, _ = select.select([self.s], [], [], max(0.0, min(1.0, deadline - time.time())))
                if not ready:
                    continue
                op, body = self.read_packet()
                self.auto(op, body)
                if op == SMSG_LOGOUT_RESPONSE and len(body) >= 4 and struct.unpack("<I", body[:4])[0] != 0:
                    log("logout refused (result %d: combat/falling/duel) - the server keeps the character for its disconnect linger"
                        % struct.unpack("<I", body[:4])[0])
                    return False
                if op == SMSG_LOGOUT_COMPLETE:
                    log("logout complete")
                    return True
        except (OSError, RuntimeError, struct.error) as exc:
            log("logout: session ended (%s)" % exc)
            return False
        log("logout: no SMSG_LOGOUT_COMPLETE within %.0fs" % timeout)
        return False

    def pump(self, seconds, stop_on=(), stop_map=None):
        _stall_watchdog_touch()
        deadline = time.time() + seconds
        events = []
        while time.time() < deadline:
            # #209 w14: never block past the deadline waiting for the NEXT packet. read_packet() runs on
            # the 20 s socket timeout, so on a quiet map (the Deeprun Tram) pump(1) between two steps took
            # 11 s and the master stepped off the car 24 s after it left the Stormwind dock. Wait for the
            # first byte with select() bounded by the deadline (no byte consumed, so the crypt stream stays
            # aligned), then read the whole packet as before. Same bound headless335.py's pump() has.
            try:
                ready, _, _ = select.select([self.s], [], [], max(0.0, deadline - time.time()))
            except (OSError, ValueError):
                ready = [self.s]
            if not ready:
                break
            try:
                op, body = self.read_packet()
            except socket.timeout:
                break
            self.auto(op, body)
            if op in stop_on:
                events.append(op)
                if len(events) >= len(stop_on):
                    break
            if stop_map is not None and self.map_id == stop_map and op == SMSG_LOGIN_VERIFY_WORLD:
                events.append(op)
                break
        self.play.tick()
        return events

    def queue_bg(self, bg_type=SOTA_BG_TYPE):
        body = struct.pack("<II", 0, 0)
        bw = BitWriter()
        g = bg_type.to_bytes(8, "little")
        bw.bit(g[1])
        bw.bit(g[7])
        bw.bit(g[0])
        bw.bit(g[3])
        bw.bit(0)      # asGroup
        bw.bit(g[4])
        bw.bit(1)      # 1 = no role mask byte follows
        bw.bit(g[6])
        bw.bit(g[2])
        bw.bit(g[5])
        body += bw.flush()
        for off in (7, 2, 4, 5, 0, 6, 3, 1):
            if g[off]:
                body += bytes([g[off] ^ 1])
        self.send_packet(CMSG_BATTLEMASTER_JOIN, body)
        log("queued for bg type %d (packet %d bytes)" % (bg_type, len(body)))

    # Server read order for HandleBattleFieldPortOpcode (BattleGroundHandler.cpp:322-338).
    _BG_PORT_MASK = (6, 4, 2, 5, 0, 1, 7, 3)
    _BG_PORT_BYTES = (2, 5, 3, 0, 7, 4, 6, 1)

    def accept_bg(self, slot=None, time_ms=0):
        """CMSG_BATTLEFIELD_PORT, action=1 (enter).

        Layout verified against src/server/game/Handlers/BattleGroundHandler.cpp
        HandleBattleFieldPortOpcode: u8 action, u32 queueSlot, u32 id, u32 time,
        then a bit-packed 8-bit guid mask + 8 ReadByteSeq bytes.
        `slot` is the queue slot from the status packet; use `self.bg_slot`.
        """
        if slot is None:
            slot = self.bg_slot if self.bg_slot is not None else 0
        body = struct.pack("<BIII", 1, slot, 0, time_ms)
        body += packed_guid(self.char_guid or 0, self._BG_PORT_MASK, self._BG_PORT_BYTES)
        self.send_packet(CMSG_BATTLEFIELD_PORT, body)
        log("sent CMSG_BATTLEFIELD_PORT accept (slot %d, %d bytes)" % (slot, len(body)))

    # --- #900/#898 battlefield manager invites (Wintergrasp / Tol Barad) ------------------------------------------------
    @staticmethod
    def _bf_response(guid, bit_seq, byte_seq, accepted):
        # bit_seq: guid byte indices in the order the handler ReadBit()s them, with "A" = the accepted flag
        bw = BitWriter()
        b = guid.to_bytes(8, "little")
        for it in bit_seq:
            bw.bit(accepted if it == "A" else (b[it] != 0))
        out = bytearray(bw.flush())
        for gi in byte_seq:
            if b[gi]:
                out.append(b[gi] ^ 1)
        return bytes(out)

    @staticmethod
    def _bf_parse_guid(body, mask_pos, byte_seq, byte_start):
        # mask_pos: guid byte index -> bit position of its presence flag; byte_seq: guid byte index (None = skip 1 byte)
        g = bytearray(8)
        pos = byte_start
        for gi in byte_seq:
            if gi is None:
                pos += 1
                continue
            bp = mask_pos[gi]
            if (body[bp // 8] >> (7 - bp % 8)) & 1:
                g[gi] = body[pos] ^ 1
                pos += 1
        return int.from_bytes(bytes(g), "little")

    def _on_bf_invite(self, op, body):
        # SMSG_BATTLEFIELD_MGR_ENTRY_INVITE (BattlefieldHandler.cpp:35-60 SendBfInvitePlayerToWar): 8 guid mask bits in order
        #   g0 g2 g7 g6 g1 g4 g5 g3, byte seq g4 g2 g0 g1 g7 g3 g6 g5, then u32 zoneId, u32 expiry.
        # SMSG_BATTLEFIELD_MGR_QUEUE_INVITE (:63-96 SendBfInvitePlayerToQueue): 15 bits: 0 1 g3 g6 g5 g4 g2 1 g0 g7 1 0 1 g1 1,
        #   byte seq g5 g4 g7 g3 u8 g6 g2 g0 g1 (the u8 sits between g3 and g6).
        try:
            if op == SMSG_BATTLEFIELD_MGR_ENTRY_INVITE:
                mask = {0: 0, 2: 1, 7: 2, 6: 3, 1: 4, 4: 5, 5: 6, 3: 7}
                guid = self._bf_parse_guid(body, mask, [4, 2, 0, 1, 7, 3, 6, 5], 1)
                zone, expiry = struct.unpack("<II", body[-8:])
                kind = "war"
                # CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE (HandleBfEntryInviteResponse, :251-): bits g0 g7 A g4 g3 g1 g6 g2 g5,
                #   bytes g1 g6 g2 g5 g3 g4 g7 g0
                reply = (CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE,
                         self._bf_response(guid, [0, 7, "A", 4, 3, 1, 6, 2, 5], [1, 6, 2, 5, 3, 4, 7, 0], 1))
            else:
                mask = {3: 2, 6: 3, 5: 4, 4: 5, 2: 6, 0: 8, 7: 9, 1: 13}
                guid = self._bf_parse_guid(body, mask, [5, 4, 7, 3, None, 6, 2, 0, 1], 2)
                zone = expiry = 0
                kind = "queue"
                # CMSG_BATTLEFIELD_MGR_QUEUE_INVITE_RESPONSE (HandleBfQueueInviteResponse, :213-): bits g3 g6 g0 g2 A g1 g5 g7 g4,
                #   bytes g4 g1 g2 g6 g0 g7 g5 g3
                reply = (CMSG_BATTLEFIELD_MGR_QUEUE_INVITE_RESPONSE,
                         self._bf_response(guid, [3, 6, 0, 2, "A", 1, 5, 7, 4], [4, 1, 2, 6, 0, 7, 5, 3], 1))
        except (IndexError, struct.error) as e:
            log("BF-MGR invite parse failed (%s) op=0x%04X body=%s" % (e, op, body.hex()))
            return
        log("BF-MGR %s invite received: bf guid=0x%X zone=%d expiry=%d accept=%s" % (kind, guid, zone, expiry, self.bf_accept))
        self.events.append(("bf_%s_invite" % kind, guid, zone))
        self.emit("bf_invite", bf=kind, zone=zone)  # #898: not kind= (emit's own first parameter is `kind`: TypeError killed the session)
        if self.bf_accept:
            self.send_packet(reply[0], reply[1])
            log("BF-MGR accepted %s invite (sent 0x%04X, %d bytes)" % (kind, reply[0], len(reply[1])))

    def leave_bg(self, slot=None):
        if slot is None:
            slot = self.bg_slot if self.bg_slot is not None else 0
        body = struct.pack("<BIII", 0, slot, 0, 0)
        body += packed_guid(self.char_guid or 0, self._BG_PORT_MASK, self._BG_PORT_BYTES)
        self.send_packet(CMSG_BATTLEFIELD_PORT, body)
        log("sent CMSG_BATTLEFIELD_PORT leave (slot %d)" % slot)

    def duel(self, target_counter):
        """CMSG_DUEL_PROPOSED: bit-packed 8-bit guid mask + byte seqs.

        `target_counter` is the DB character guid; the full raw player guid
        (type bits included) is required or ObjectAccessor::FindPlayer misses.
        """
        guid = player_guid(target_counter)
        body = packed_guid(guid, [1, 5, 4, 6, 3, 2, 7, 0], [4, 2, 5, 7, 1, 3, 6, 0])
        self.send_packet(CMSG_DUEL_PROPOSED, body)
        log("sent CMSG_DUEL_PROPOSED -> counter %d (raw %d)" % (target_counter, guid))
        self.emit("duel_sent", target=target_counter)

    def duel_response(self, opponent_counter, accepted):
        """CMSG_DUEL_RESPONSE: guid bits with the accepted flag interleaved before bit 5."""
        b = player_guid(opponent_counter).to_bytes(8, "little")
        bw = BitWriter()
        for off in (7, 1, 3, 4, 0, 2, 6):
            bw.bit(b[off] != 0)
        bw.bit(1 if accepted else 0)
        bw.bit(b[5] != 0)
        body = bytearray(bw.flush())
        for off in (6, 4, 5, 0, 1, 2, 7, 3):
            if b[off]:
                body.append(b[off] ^ 1)
        self.send_packet(CMSG_DUEL_RESPONSE, bytes(body))
        log("sent CMSG_DUEL_RESPONSE accepted=%d -> counter %d" % (1 if accepted else 0, opponent_counter))

    def use_item_on(self, bag, slot, item_guid, target_bag, target_slot, target_item_guid, cast_count=1):
        """CMSG_USE_ITEM (Handlers/SpellHandler.cpp:40-498), 5.4.8 bit-packed.
        Mirrors HandleUseItemOpcode's exact read order: plain `slot` then
        `bagIndex` (reversed from the intuitive order, SpellHandler.cpp:87)
        BEFORE the bit region; then a scrambled run of has-bits interleaved
        with itemGuid's 8 mask bits (hasElevation/hasTargetString/
        hasCastFlags/hasDestLocation/hasTargetMask/hasMissileSpeed/
        hasMovement/hasCastCount/hasSpellId/hasGlyphIndex/hasSrcLocation + 2
        reserved bits + a 2-bit researchDataCount); then targetGuid's 8 mask
        bits and itemTargetGuid's 8 mask bits, BOTH read unconditionally
        regardless of targetMask (SpellHandler.cpp:192-208); then
        castFlags(5 bits, only if hasCastFlags) and targetMask(20 bits, only
        if hasTargetMask). Byte-phase: itemGuid byte-seq (7 bytes, then the
        empty researchDataCount=0 skip loop, then its 8th byte), itemTargetGuid
        byte-seq (8 bytes), targetGuid byte-seq (all zero here - no unit/GO
        target, so WriteByteSeq emits nothing), then the optional
        elevation/glyphIndex/missileSpeed/castCount tail (only castCount is
        sent here, last).

        This only sends TARGET_FLAG_ITEM (0x10) - use an item on another item,
        e.g. an enchant scroll or a tinker. `item_guid`/`target_item_guid`
        must be the items' real ObjectGuids: bag/slot alone can't be resolved
        to a guid without parsing SMSG_UPDATE_OBJECT (neither client does;
        out of scope, see the issue #191 report). `target_bag`/`target_slot`
        are accepted to match the task's verb signature but are not part of
        the wire packet - only the target's guid is."""
        item_guid = int(item_guid)
        target_item_guid = int(target_item_guid)
        igb = item_guid.to_bytes(8, "little")
        tgb = target_item_guid.to_bytes(8, "little")

        bw = BitWriter()
        bw.bit(1)                   # hasElevation=false (inverted)
        bw.bit(igb[6] != 0)          # itemGuid[6] mask
        bw.bit(1)                   # hasTargetString=false (inverted)
        bw.bit(igb[1] != 0)          # itemGuid[1] mask
        bw.bit(1)                   # hasCastFlags=false (inverted)
        bw.bit(0)                   # hasDestLocation=false (direct)
        bw.bit(igb[2] != 0)          # itemGuid[2] mask
        bw.bit(igb[7] != 0)          # itemGuid[7] mask
        bw.bit(igb[0] != 0)          # itemGuid[0] mask
        bw.bit(0)                   # hasTargetMask=true (inverted)
        bw.bit(1)                   # hasMissileSpeed=false (inverted)
        bw.bit(0)                   # hasMovement=false (direct)
        bw.bit(0)                   # hasCastCount=true (inverted)
        bw.bit(1)                   # hasSpellId=false (inverted)
        bw.bit(0)                   # reserved/unused bit
        bw.bit(1)                   # hasGlyphIndex=false (inverted)
        bw.bit(0)                   # reserved/unused bit
        bw.bit(igb[4] != 0)          # itemGuid[4] mask
        bw.bit(0)                   # hasSrcLocation=false (direct)
        bw.bit(igb[3] != 0)          # itemGuid[3] mask
        bw.bit(igb[5] != 0)          # itemGuid[5] mask
        bw.bits(0, 2)                # researchDataCount = 0
        for _ in range(8):
            bw.bit(0)                # targetGuid mask: all-zero (no unit/GO target)
        for idx in (4, 5, 0, 1, 3, 7, 6, 2):
            bw.bit(tgb[idx] != 0)    # itemTargetGuid mask
        bw.bits(TARGET_FLAG_ITEM, 20)  # targetMask (hasTargetMask=true)

        tail = bytearray()
        for idx in (0, 5, 6, 3, 4, 2, 1):
            if igb[idx]:
                tail.append(igb[idx] ^ 1)
        # researchDataCount == 0: the read_skip<uint32> loop consumes no bytes.
        if igb[7]:
            tail.append(igb[7] ^ 1)
        for idx in (6, 7, 2, 0, 3, 4, 1, 5):
            if tgb[idx]:
                tail.append(tgb[idx] ^ 1)
        # targetGuid byte-seq: all zero, WriteByteSeq emits nothing.
        tail += struct.pack("<B", int(cast_count) & 0xFF)   # hasCastCount=true

        body = struct.pack("<BB", int(slot), int(bag)) + bw.flush() + bytes(tail)
        self.send_packet(CMSG_USE_ITEM, body)
        log("sent CMSG_USE_ITEM bag=%d slot=%d item=%d -> target item %d (bag=%d slot=%d)"
            % (bag, slot, item_guid, target_item_guid, target_bag, target_slot))
        self.emit("use_item_on", bag=bag, slot=slot, target_bag=target_bag, target_slot=target_slot)

    def use_item(self, bag, slot, item_guid=0, dest=None):
        """CMSG_USE_ITEM for a PLAIN (untargeted) item use - e.g. the quest item 46856 "Keys to
        the Hot Rod", whose on-use spell 66393 summons and boards the vehicle (issue #207).

        Same opcode and same read order as use_item_on() above, with two differences that a plain
        use makes: itemTargetGuid is left at zero (so its mask bits and its byte-seq are both
        empty), and no second item guid is needed. targetMask is still written explicitly as
        TARGET_FLAG_ITEM, matching a real client's plain use.

        bag/slot are mandatory: this client does not parse SMSG_UPDATE_OBJECT (see the note in
        use_item_on's docstring and at SpellHandler.cpp's OpenItem note), so an itemEntry cannot
        be resolved to its inventory position client-side. Pass guid=<item guid> if the server
        ever needs the real ObjectGuid; it defaults to 0 and the server resolves the item by
        bag/slot.

        `use-item-at`: dest=(x,y,z) makes it a ground-targeted use (TARGET_FLAG_DEST_LOCATION 0x40
        instead of TARGET_FLAG_ITEM 0x10). Extra layout per HandleUseItemOpcode (SpellHandler.cpp):
          bits: hasDestLocation=1 (bit 6 of the has-bit run, :94); then, after researchDataCount
                (:110) and the (absent) movement/srcLocation bits (:115-175), the 8 destTransportGuid
                mask bits (:177-187, all zero = world coords), BEFORE the targetGuid mask (:192).
          bytes: after itemGuid[7] (:230) and the (absent) movement block (:232-305), the dest block
                (:307-324): [dT7] x [dT0,6,1,3] y [dT5] z [dT4,2]; the transport-guid bytes are all
                zero so only x, y, z floats are emitted. castCount stays last (:395).
        """
        igb = int(item_guid).to_bytes(8, "little")
        bw = BitWriter()
        bw.bit(1)                   # hasElevation=false (inverted)
        bw.bit(igb[6] != 0)          # itemGuid[6] mask
        bw.bit(1)                   # hasTargetString=false (inverted)
        bw.bit(igb[1] != 0)          # itemGuid[1] mask
        bw.bit(1)                   # hasCastFlags=false (inverted)
        bw.bit(1 if dest is not None else 0)  # hasDestLocation (direct)
        bw.bit(igb[2] != 0)          # itemGuid[2] mask
        bw.bit(igb[7] != 0)          # itemGuid[7] mask
        bw.bit(igb[0] != 0)          # itemGuid[0] mask
        bw.bit(0)                   # hasTargetMask=true (inverted)
        bw.bit(1)                   # hasMissileSpeed=false (inverted)
        bw.bit(0)                   # hasMovement=false (direct)
        bw.bit(0)                   # hasCastCount=true (inverted)
        bw.bit(1)                   # hasSpellId=false (inverted)
        bw.bit(0)                   # reserved/unused bit
        bw.bit(1)                   # hasGlyphIndex=false (inverted)
        bw.bit(0)                   # reserved/unused bit
        bw.bit(igb[4] != 0)          # itemGuid[4] mask
        bw.bit(0)                   # hasSrcLocation=false (direct)
        bw.bit(igb[3] != 0)          # itemGuid[3] mask
        bw.bit(igb[5] != 0)          # itemGuid[5] mask
        bw.bits(0, 2)                # researchDataCount = 0
        if dest is not None:
            for _ in range(8):
                bw.bit(0)            # destTransportGuid mask: all-zero (world coords), SpellHandler.cpp:177-187
        for _ in range(8):
            bw.bit(0)                # targetGuid mask: all-zero (no unit/GO target)
        for _ in range(8):
            bw.bit(0)                # itemTargetGuid mask: all-zero
        bw.bits(TARGET_FLAG_DEST_LOCATION if dest is not None else TARGET_FLAG_ITEM, 20)  # targetMask (hasTargetMask=true)
        tail = bytearray()
        for idx in (0, 5, 6, 3, 4, 2, 1):
            if igb[idx]:
                tail.append(igb[idx] ^ 1)
        # researchDataCount == 0: the read_skip<uint32> loop consumes no bytes.
        if igb[7]:
            tail.append(igb[7] ^ 1)
        # no movement block (hasMovement=0).
        if dest is not None:
            # dest block (SpellHandler.cpp:307-324): x, y, z; zero transport guid adds no bytes.
            tail += struct.pack("<fff", float(dest[0]), float(dest[1]), float(dest[2]))
        # itemTargetGuid + targetGuid byte-seqs: all zero, WriteByteSeq emits nothing.
        tail += struct.pack("<B", 1)  # hasCastCount=true -> castCount=1
        body = struct.pack("<BB", int(slot), int(bag)) + bw.flush() + bytes(tail)
        self.send_packet(CMSG_USE_ITEM, body)
        if dest is not None:
            log("sent CMSG_USE_ITEM bag=%d slot=%d guid=%d dest=(%.2f,%.2f,%.2f) mask=0x%02X (%d bytes)"
                % (bag, slot, int(item_guid), dest[0], dest[1], dest[2], TARGET_FLAG_DEST_LOCATION, len(body)))
            self.emit("use_item_at", bag=bag, slot=slot, guid=int(item_guid), dest="%g,%g,%g" % tuple(dest))
        else:
            log("sent CMSG_USE_ITEM bag=%d slot=%d guid=%d (plain use, no target)"
                % (bag, slot, int(item_guid)))
            self.emit("use_item", bag=bag, slot=slot, guid=int(item_guid))

    # --- #470: gossip / faction / gameobject / attack / spellclick -----------
    def obj_guid(self, arg, kind="unit"):
        """Verb argument -> raw object guid. Accepts: 'npc' (guid cached by the `npcguid` harness
        verb), a raw guid (> 0xFFFFFFFF), or '<entry>:<spawnId>' which builds the static
        GameObject guid exactly as ObjectGuid(HighGuid::GameObject, entry, counter) does
        (ObjectGuid.h:165; GameObject::LoadFromDB keeps the DB spawn id as the counter on
        non-instance maps, GameObject.cpp:909-913)."""
        if arg is None:
            return 0
        a = str(arg)
        if a.lower() == "npc":
            return int(self.harness_guid.get("npc", 0))
        if a.lower() in ("go", "corpse"):   # #553: guids cached by `goguid` / `nearcorpse` (harness_guid keys)
            return int(self.harness_guid.get(a.lower(), 0))
        if ":" in a:
            entry, spawn = a.split(":", 1)
            return (int(spawn) | (int(entry) << 32) | (HIGHGUID_GAMEOBJECT << 52)) & 0xFFFFFFFFFFFFFFFF
        return int(a)

    def gossip_hello(self, guid):
        """CMSG_GOSSIP_HELLO (NPCHandler.cpp:447): bits [2,4,0,3,6,7,5,1], byteseq [4,7,1,0,5,3,6,2]."""
        guid = int(guid)
        gb = guid.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (2, 4, 0, 3, 6, 7, 5, 1):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(guid, (4, 7, 1, 0, 5, 3, 6, 2))
        self.gossip = None
        self.send_packet(CMSG_GOSSIP_HELLO, body)
        log("sent CMSG_GOSSIP_HELLO guid=%d" % guid)
        self.emit("gossip_hello", guid=guid)

    def gossip_select(self, option, guid=None, menu_id=None, code=""):
        """CMSG_GOSSIP_SELECT_OPTION (MiscHandler.cpp:101-134): u32 gossipListId, u32 menuId, then
        bits [g3,g0,g1,g4,g7,g5,g6, boxTextLength(8), g2], byteseq [7,3,4,6,0,5], the coded-option
        string (only if the option IsCoded), byteseq [2,1]. gossipListId/menuId/guid default to the
        last SMSG_GOSSIP_MESSAGE this client saw; `option` is the listed option id (NOT its index)."""
        g = getattr(self, "gossip", None)
        if guid is None:
            guid = g["guid"] if g else 0
        if menu_id is None:
            menu_id = g["menu"] if g else 0
        guid = int(guid)
        gb = guid.to_bytes(8, "little")
        code_b = code.encode("utf-8") if code else b""
        bw = BitWriter()
        for idx in (3, 0, 1, 4, 7, 5, 6):
            bw.bit(gb[idx] != 0)
        bw.bits(len(code_b), 8)
        bw.bit(gb[2] != 0)
        body = (struct.pack("<II", int(option), int(menu_id)) + bw.flush()
                + guid_byte_seq(guid, (7, 3, 4, 6, 0, 5)) + code_b
                + guid_byte_seq(guid, (2, 1)))
        self.send_packet(CMSG_GOSSIP_SELECT_OPTION, body)
        log("sent CMSG_GOSSIP_SELECT_OPTION option=%d menu=%d guid=%d" % (int(option), int(menu_id), guid))
        self.emit("gossip_select", option=int(option), menu=int(menu_id), guid=guid)

    def _parse_gossip_message(self, body):
        """SMSG_GOSSIP_MESSAGE (GossipDef.cpp:188-278)."""
        r = ByteReader(body)
        nq = r.read_bits(19)
        quests = []
        for _ in range(nq):
            repeatable = r.read_bit()
            quests.append([repeatable, r.read_bits(9)])
        gb = [0] * 8
        for idx in (5, 7, 4, 0):
            gb[idx] = r.read_bit()
        no = r.read_bits(20)
        for idx in (6, 2):
            gb[idx] = r.read_bit()
        opts = []
        for _ in range(no):
            opts.append([r.read_bits(12), r.read_bits(12)])
        for idx in (3, 1):
            gb[idx] = r.read_bit()
        qlist = []
        for rep, tl in quests:
            title = r.read_string(tl).decode("utf-8", "replace")
            flags, level, icon, qid, flagsex = [r.read_i32() for _ in range(5)]
            qlist.append({"id": qid, "title": title, "level": level, "icon": icon})
        guid = [0] * 8
        for idx in (1, 0):
            guid[idx] = r.read_byte_seq(gb[idx])
        olist = []
        for boxlen, msglen in opts:
            money = r.read_i32()
            box = r.read_string(boxlen).decode("utf-8", "replace")
            oid = r.read_i32()
            coded = r.read_u8()
            msg = r.read_string(msglen).decode("utf-8", "replace")
            icon = r.read_u8()
            olist.append({"id": oid, "text": msg, "icon": icon, "coded": coded, "money": money, "box": box})
        for idx in (5, 3):
            guid[idx] = r.read_byte_seq(gb[idx])
        menu = r.read_i32()
        for idx in (2, 6, 4):
            guid[idx] = r.read_byte_seq(gb[idx])
        r.read_i32()
        guid[7] = r.read_byte_seq(gb[7])
        text_id = r.read_i32()
        raw = sum(b << (8 * i) for i, b in enumerate(guid))
        self.gossip = {"guid": raw, "menu": menu, "text": text_id, "options": olist, "quests": qlist}
        log("SMSG_GOSSIP_MESSAGE guid=%d menu=%d text=%d options=%d quests=%d" % (raw, menu, text_id, len(olist), len(qlist)))
        for o in olist:
            log("  gossip option id=%d icon=%d coded=%d text=%r" % (o["id"], o["icon"], o["coded"], o["text"]))
        for q in qlist:
            log("  gossip quest id=%d level=%d icon=%d title=%r" % (q["id"], q["level"], q["icon"], q["title"]))
        self.emit("gossip", guid=raw, menu=menu, options=len(olist), quests=len(qlist),
                  ids=",".join(str(o["id"]) for o in olist))

    # --- #969: BattlePay (in-game shop) -------------------------------------------------------------
    def _parse_battle_pay(self, op, body):
        """Decode the BattlePay server packets this client cares about and record them in self.events
        (kinds bp_products / bp_start / bp_confirm / bp_update / bp_delivery / bp_distribution)."""
        if op == SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE:
            # BattlePayMgr::SendBattlePayProductList: bits19 shop entries, bits19 products, ... (store disabled:
            # 0,0,0 counts then u32 currency, u32 status=1 UNAVAILABLE). Only the counts are decoded here.
            r = ByteReader(body)
            entries, products = r.read_bits(19), r.read_bits(19)
            log("SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE entries=%d products=%d (%d bytes)" % (entries, products, len(body)))
            self.emit("bp_products", entries=entries, products=products, size=len(body))
            self.events.append(("bp_products", products))
        elif op == SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE:
            purchase_id, result, client_token = struct.unpack_from("<QII", body, 0)
            log("SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE purchase=%d result=%d clientToken=%d" % (purchase_id, result, client_token))
            self.emit("bp_start", purchase=purchase_id, result=result, client_token=client_token)
            self.events.append(("bp_start", purchase_id))
        elif op == SMSG_BATTLE_PAY_CONFIRM_PURCHASE:
            server_token, price, purchase_id = struct.unpack_from("<IQQ", body, 0)
            self.bp_confirm = {"server_token": server_token, "price": price, "purchase": purchase_id}
            log("SMSG_BATTLE_PAY_CONFIRM_PURCHASE serverToken=%d price=%d (%.2f points) purchase=%d"
                % (server_token, price, price / 10000.0, purchase_id))
            self.emit("bp_confirm", server_token=server_token, price=price, purchase=purchase_id)
            self.events.append(("bp_confirm", self.bp_confirm))
        elif op == SMSG_BATTLE_PAY_PURCHASE_UPDATE:
            r = ByteReader(body)
            n = r.read_bits(19)
            wlens = [r.read_bits(8) for _ in range(n)]
            for wl in wlens:
                result = r.read_u32()
                r._reset_bits()
                purchase_id = struct.unpack_from("<Q", r.data, r.pos)[0]
                r.pos += 8
                product, status = r.read_u32(), r.read_u32()
                wallet = r.read_string(wl).decode("utf-8", "replace")
                upd = {"status": status, "result": result, "product": product, "purchase": purchase_id}
                self.bp_updates.append(upd)
                log("SMSG_BATTLE_PAY_PURCHASE_UPDATE product=%d purchase=%d status=%s result=%s wallet=%r"
                    % (product, purchase_id, BATTLE_PAY_STATUS_NAMES.get(status, status),
                       BATTLE_PAY_RESULT_NAMES.get(result, result), wallet))
                self.emit("bp_update", product=product, purchase=purchase_id,
                          status=BATTLE_PAY_STATUS_NAMES.get(status, status),
                          result=BATTLE_PAY_RESULT_NAMES.get(result, result))
                self.events.append(("bp_update", upd))
        elif op == SMSG_BATTLE_PAY_DELIVERY_ENDED:
            # bits22 item count, then u32 itemId each, then u64 DistributionID (BattlePayMgr.cpp BUYING step)
            r = ByteReader(body)
            n = r.read_bits(22)
            items = [r.read_u32() for _ in range(n)]
            log("SMSG_BATTLE_PAY_DELIVERY_ENDED items=%s" % items)
            self.emit("bp_delivery", items=",".join(str(i) for i in items))
            self.events.append(("bp_delivery", items))
        else:
            log("SMSG_BATTLE_PAY_DISTRIBUTION_UPDATE (%d bytes)" % len(body))
            self.emit("bp_distribution", size=len(body))
            self.events.append(("bp_distribution", len(body)))

    def bp_get_products(self):
        """CMSG_BATTLE_PAY_GET_PRODUCT_LIST (empty). The server answers with the product list and, when the store
        is enabled, the balance as a system chat line (BattlePayMgr::SendPointsBalance, trinity_string 15005)."""
        self.send_packet(CMSG_BATTLE_PAY_GET_PRODUCT_LIST, b"")
        log("sent CMSG_BATTLE_PAY_GET_PRODUCT_LIST")

    def bp_start_purchase(self, product_id, client_token=None):
        """CMSG_BATTLE_PAY_START_PURCHASE (BattlePayHandler.cpp HandleBattlePayStartPurchase): u32 productId,
        u32 clientToken, guid mask [2,3,5,1,4,7,0,6], byteseq [5,3,7,1,4,0,6,2]; the guid is the target
        character (this client's own)."""
        token = int(client_token) if client_token else random.randint(1, 0x7FFFFFFF)
        g = int(self.char_guid or 0)
        body = struct.pack("<II", int(product_id), token) + packed_guid(g, (2, 3, 5, 1, 4, 7, 0, 6), (5, 3, 7, 1, 4, 0, 6, 2))
        self.send_packet(CMSG_BATTLE_PAY_START_PURCHASE, body)
        log("sent CMSG_BATTLE_PAY_START_PURCHASE product=%d clientToken=%d guid=%d" % (int(product_id), token, g))
        self.emit("bp_start_sent", product=int(product_id), client_token=token)
        return token

    def bp_confirm_response(self, price, server_token, confirm=True):
        """CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE (HandleBattlePayConfirmPurchaseResponse): u64 currentPrice,
        u32 serverToken, u8 confirm."""
        self.send_packet(CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE,
                         struct.pack("<QIB", int(price), int(server_token), 1 if confirm else 0))
        log("sent CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE price=%d serverToken=%d confirm=%d"
            % (int(price), int(server_token), 1 if confirm else 0))
        self.emit("bp_confirm_sent", price=int(price), server_token=int(server_token), confirm=int(bool(confirm)))

    def select_faction(self, choice):
        """CMSG_SELECT_FACTION (MiscHandler.cpp:2709): a single u32; 0 = JOIN_THE_HORDE,
        1 = JOIN_THE_ALLIANCE. Only acts for race 24 (neutral pandaren)."""
        self.send_packet(CMSG_SELECT_FACTION, struct.pack("<I", int(choice)))
        log("sent CMSG_SELECT_FACTION choice=%d" % int(choice))
        self.emit("select_faction", choice=int(choice))

    def gameobject_use(self, guid, report=False):
        """CMSG_GAMEOBJ_USE (SpellHandler.cpp:596): bits [6,1,3,4,0,5,7,2], byteseq [0,1,6,2,3,4,5,7].
        report=True sends CMSG_GAMEOBJ_REPORT_USE (SpellHandler.cpp:633): bits [4,7,5,3,6,1,2,0],
        byteseq [7,1,6,5,0,3,2,4]."""
        guid = int(guid)
        gb = guid.to_bytes(8, "little")
        bw = BitWriter()
        if report:
            mo, bo, op = (4, 7, 5, 3, 6, 1, 2, 0), (7, 1, 6, 5, 0, 3, 2, 4), CMSG_GAMEOBJ_REPORT_USE
        else:
            mo, bo, op = (6, 1, 3, 4, 0, 5, 7, 2), (0, 1, 6, 2, 3, 4, 5, 7), CMSG_GAMEOBJ_USE
        for idx in mo:
            bw.bit(gb[idx] != 0)
        self.send_packet(op, bw.flush() + guid_byte_seq(guid, bo))
        log("sent %s guid=%d" % ("CMSG_GAMEOBJ_REPORT_USE" if report else "CMSG_GAMEOBJ_USE", guid))
        self.emit("gameobject_report_use" if report else "gameobject_use", guid=guid)

    def attack(self, guid):
        """CMSG_ATTACKSWING (CombatHandler.cpp:30): bits [6,5,7,0,3,1,4,2], byteseq [6,7,1,3,2,0,4,5]."""
        guid = int(guid)
        gb = guid.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (6, 5, 7, 0, 3, 1, 4, 2):
            bw.bit(gb[idx] != 0)
        self.send_packet(CMSG_ATTACKSWING, bw.flush() + guid_byte_seq(guid, (6, 7, 1, 3, 2, 0, 4, 5)))
        self.last_attack_guid = guid      # the unit last swung at - a kill/hold step finishes it before a fresh one
        log("sent CMSG_ATTACKSWING guid=%d" % guid)
        self.emit("attack", guid=guid)

    def cancel_auto_repeat(self):
        """CMSG_CANCEL_AUTO_REPEAT_SPELL (SpellHandler.cpp HandleCancelAutoRepeatSpellOpcode): empty body."""
        self.send_packet(CMSG_CANCEL_AUTO_REPEAT_SPELL, b"")
        log("sent CMSG_CANCEL_AUTO_REPEAT_SPELL")

    def attack_stop(self):
        """CMSG_ATTACKSTOP (CombatHandler.cpp:90): empty body."""
        self.send_packet(CMSG_ATTACKSTOP, b"")
        log("sent CMSG_ATTACKSTOP")
        self.emit("attack_stop")

    def spellclick(self, guid):
        """CMSG_SPELLCLICK (SpellHandler.cpp:1429): bits [7,4,0,3,6,5, unk(1), 1,2],
        byteseq [6,1,5,4,7,2,3,0]."""
        guid = int(guid)
        gb = guid.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (7, 4, 0, 3, 6, 5):
            bw.bit(gb[idx] != 0)
        bw.bit(0)
        for idx in (1, 2):
            bw.bit(gb[idx] != 0)
        self.send_packet(CMSG_SPELLCLICK, bw.flush() + guid_byte_seq(guid, (6, 1, 5, 4, 7, 2, 3, 0)))
        log("sent CMSG_SPELLCLICK guid=%d" % guid)
        self.emit("spellclick", guid=guid)

    def invite_to_group(self, name):
        """CMSG_GROUP_INVITE by character name.

        Layout verified against src/server/game/Handlers/GroupHandler.cpp
        HandleGroupInviteOpcode: u32 crossRealm, u8 unk, u32 always0, then bits
        [guid7, realmLen(9), guid3, nameLen(9), guid2, guid5, guid4, guid0,
        guid1, guid6], then ByteSeq 7,6,0,4, realmName, ByteSeq 1,2,3,
        memberName, ByteSeq 5.

        crossRealmGuid is unused by the handler, so it is sent as all-zero:
        every mask bit is 0 and WriteByteSeq therefore emits no bytes at all,
        which leaves realmName (empty) immediately followed by memberName.
        """
        nb = name.encode("utf-8")
        body = struct.pack("<IBI", 0, 0, 0)
        bw = BitWriter()
        bw.bit(0)              # crossRealmGuid[7]
        bw.bits(0, 9)          # realmLen - no cross-realm name
        bw.bit(0)              # crossRealmGuid[3]
        bw.bits(len(nb), 9)    # nameLen
        for _ in range(6):     # crossRealmGuid[2], [5], [4], [0], [1], [6]
            bw.bit(0)
        body += bw.flush()
        body += nb             # realmName is empty, so memberName lands here
        self.send_packet(CMSG_GROUP_INVITE, body)
        log("sent CMSG_GROUP_INVITE -> %r (%d bytes)" % (name, len(body)))
        self.emit("invite", target=name)

    # --- S1a: near-teleport ack --------------------------------------------
    def teleport_ack(self, move_time=0, flags=0):
        """CMSG_MOVE_TELEPORT_ACK (Server/Packets/MovementPackets.cpp:279-299
        MoveTeleportAck::Read): int32 MoveTime, uint32 Flags, then bits
        [guid0,7,3,5,4,6,1,2], ByteSeq 4,1,6,7,0,2,5,3. Only MoverGUID is
        checked server-side (== the teleporting player); MoveTime/Flags are
        just logged, so 0/0 is fine."""
        guid = self.char_guid or 0
        body = struct.pack("<iI", move_time, flags)
        body += guid_mask_bits(guid, (0, 7, 3, 5, 4, 6, 1, 2))
        body += guid_byte_seq(guid, (4, 1, 6, 7, 0, 2, 5, 3))
        self.send_packet(CMSG_MOVE_TELEPORT_ACK, body)

    # --- S1a: chat send/parse -----------------------------------------------
    _CHAT_SEND_OPCODES = {
        "say": CMSG_MESSAGECHAT_SAY, "party": CMSG_MESSAGECHAT_PARTY,
        "raid": CMSG_MESSAGECHAT_RAID, "whisper": CMSG_MESSAGECHAT_WHISPER,
        "guild": CMSG_MESSAGECHAT_GUILD,   # same layout as say (ChatHandler.cpp: textLen bits(8) + msg)
    }

    def send_chat(self, chat_type, message, target=None, language=None):
        """CMSG_MESSAGECHAT_{SAY,PARTY,RAID,WHISPER} (ChatHandler.cpp:46-266):
        u32 lang, then bits(8) textLen [+ bits(9) receiverLen for whisper],
        then the raw string(s) - msg first, then the whisper target name.
        LANG_UNIVERSAL is rejected for real messages, so default to the
        caller's racial language."""
        op = self._CHAT_SEND_OPCODES[chat_type]
        lang = language if language is not None else self.language
        msg_b = message.encode("utf-8")
        body = struct.pack("<I", lang)
        bw = BitWriter()
        bw.bits(len(msg_b), 8)
        to_b = b""
        if chat_type == "whisper":
            to_b = (target or "").encode("utf-8")
            bw.bits(len(to_b), 9)
        body += bw.flush()
        body += msg_b
        body += to_b
        self.send_packet(op, body)
        log("sent chat %s%s: %r" % (chat_type, (" -> %s" % target) if target else "", message))
        self.emit("chat_sent", type=chat_type, msg=json.dumps(message))

    def send_channel_chat(self, channel, message, language=None):
        """CMSG_MESSAGECHAT_CHANNEL (ChatHandler.cpp, case CHAT_MSG_CHANNEL): u32 lang, bits(9) channelLen,
        bits(8) textLen, then the TEXT first, then the channel name (the handler's variables are named
        confusingly: textLength -> channel, receiverLength -> msg, msg read first)."""
        lang = language if language is not None else self.language
        msg_b = message.encode("utf-8")
        ch_b = channel.encode("utf-8")
        body = struct.pack("<I", lang)
        bw = BitWriter()
        bw.bits(len(ch_b), 9)
        bw.bits(len(msg_b), 8)
        body += bw.flush() + msg_b + ch_b
        self.send_packet(CMSG_MESSAGECHAT_CHANNEL, body)
        log("sent chat channel %s: %r" % (channel, message))
        self.emit("chat_sent", type="channel", msg=json.dumps(message))

    def join_channel(self, name, channel_id=0, password=""):
        """CMSG_JOIN_CHANNEL (ChannelHandler.cpp HandleJoinChannel): u32 channelId, bit unk, bits(7) nameLen,
        bits(7) passLen, bit unk, then password, then name. channel_id 0 = custom channel by name; a DBC id
        (e.g. the LookingForGroup constant channel) joins the zone/global channel and ignores the name
        except for the city/zone pattern checks."""
        nb = name.encode("utf-8")
        pb = password.encode("utf-8")
        body = struct.pack("<I", channel_id)
        bw = BitWriter()
        bw.bit(0)
        bw.bits(len(nb), 7)
        bw.bits(len(pb), 7)
        bw.bit(0)
        body += bw.flush() + pb + nb
        self.send_packet(CMSG_JOIN_CHANNEL, body)
        log("sent CMSG_JOIN_CHANNEL id=%d name=%r" % (channel_id, name))
        self.emit("join_channel", name=name, id=channel_id)

    def guild_accept(self):
        """CMSG_GUILD_ACCEPT (GuildHandler.cpp HandleGuildAcceptOpcode): empty body."""
        self.send_packet(CMSG_GUILD_ACCEPT, b"")
        log("sent CMSG_GUILD_ACCEPT")
        self.emit("guild_accept")

    def _parse_guild_invite(self, body):
        """SMSG_GUILD_INVITE (Guild.cpp Guild::HandleInviteMember send): bit-packed, returns
        (inviter, guild_name, old_guild_name) or None."""
        try:
            r = ByteReader(body)
            ng = [0] * 8
            og = [0] * 8
            ng[4] = r.read_bit()
            guild_len = r.read_bits(7)
            og[4] = r.read_bit()
            ng[6] = r.read_bit()
            for i in (2, 1, 5, 7):
                og[i] = r.read_bit()
            ng[0] = r.read_bit()
            og[3] = r.read_bit()
            ng[5] = r.read_bit()
            og[6] = r.read_bit()
            inviter_len = r.read_bits(6)
            ng[1] = r.read_bit()
            ng[3] = r.read_bit()
            og[0] = r.read_bit()
            ng[7] = r.read_bit()
            old_len = r.read_bits(7)
            ng[2] = r.read_bit()
            r.read_byte_seq(ng[1]); r.read_u32()
            r.read_byte_seq(ng[4])
            inviter = r.read_string(inviter_len).decode("utf-8", "replace")
            r.read_u32()
            r.read_byte_seq(og[7]); r.read_byte_seq(ng[0]); r.read_byte_seq(ng[2])
            r.read_u32()
            r.read_byte_seq(og[2]); r.read_byte_seq(og[5])
            r.read_u32(); r.read_u32()
            r.read_byte_seq(ng[7]); r.read_byte_seq(ng[3]); r.read_byte_seq(og[4])
            r.read_u32()
            gname = r.read_string(guild_len).decode("utf-8", "replace")
            return inviter, gname, ""
        except (IndexError, struct.error) as exc:
            log("SMSG_GUILD_INVITE parse failed: %s" % exc)
            return None

    # ChatMsg values (SharedDefines.h:3826-3876) - verified against this repo,
    # NOT assumed: SYSTEM=0 shifts every other type up by one versus a naive
    # guess starting SAY at 0.
    _CHAT_TYPE_NAMES = {
        0: "SYSTEM", 1: "SAY", 2: "PARTY", 3: "RAID", 4: "GUILD", 5: "OFFICER",
        6: "YELL", 7: "WHISPER", 8: "WHISPER_FOREIGN", 9: "WHISPER_INFORM",
        10: "EMOTE", 11: "TEXT_EMOTE", 39: "RAID_LEADER", 40: "RAID_WARNING",
        49: "PARTY_LEADER",
    }

    def _parse_messagechat(self, body):
        """SMSG_MESSAGECHAT parse (Chat.cpp:351-571 BuildChatPacket). Bit-packed;
        the field order below is the exact write order, since ByteBuffer bits
        are read back in the order they were written."""
        try:
            r = ByteReader(body)
            no_sender_name = r.read_bit()
            r.read_bit()                       # hideInChatLog
            sender_name_len = r.read_bits(11) if not no_sender_name else 0
            r.read_bit()                       # fake bit
            no_channel_name = r.read_bit()
            r.read_bit()                       # unk
            r.read_bit()                       # sendFakeTime
            no_chat_tag = r.read_bit()
            r.read_bit()                       # !realmId1
            group_mask = [0] * 8
            for idx in (0, 1, 5, 4, 3, 2, 6, 7):
                group_mask[idx] = r.read_bit()
            chat_tag = r.read_bits(9) if not no_chat_tag else 0
            r.read_bit()                       # fake bit
            recv_mask = [0] * 8
            for idx in (7, 6, 1, 4, 0, 2, 3, 5):
                recv_mask[idx] = r.read_bit()
            r.read_bit()                       # fake bit
            no_language = r.read_bit()
            no_prefix = r.read_bit()
            sender_mask = [0] * 8
            for idx in (0, 3, 7, 2, 1, 5, 4, 6):
                sender_mask[idx] = r.read_bit()
            no_achievement = r.read_bit()
            no_message = r.read_bit()
            channel_name_len = r.read_bits(7) if not no_channel_name else 0
            message_len = r.read_bits(12) if not no_message else 0
            no_receiver_name = r.read_bit()
            prefix_len = r.read_bits(5) if not no_prefix else 0
            r.read_bit()                       # realmId2 (always 1)
            receiver_name_len = r.read_bits(11) if not no_receiver_name else 0
            r.read_bit()                       # fake bit
            guild_mask = [0] * 8
            for idx in (2, 5, 7, 4, 0, 1, 3, 6):
                guild_mask[idx] = r.read_bit()

            guild = [0] * 8
            for idx in (4, 5, 7, 3, 2, 6, 0, 1):
                guild[idx] = r.read_byte_seq(guild_mask[idx])
            channel_name = r.read_string(channel_name_len) if not no_channel_name else b""
            r.read_string(prefix_len) if not no_prefix else b""
            sender = [0] * 8
            for idx in (4, 7, 1, 5, 0, 6, 2, 3):
                sender[idx] = r.read_byte_seq(sender_mask[idx])
            chat_type = r.read_u8()
            achievement_id = r.read_u32() if not no_achievement else 0
            group = [0] * 8
            for idx in (1, 3, 4, 6, 0, 2, 5, 7):
                group[idx] = r.read_byte_seq(group_mask[idx])
            recv = [0] * 8
            for idx in (2, 5, 3, 6, 7, 4, 1, 0):
                recv[idx] = r.read_byte_seq(recv_mask[idx])
            language = r.read_u8() if not no_language else 0
            message = r.read_string(message_len).decode("utf-8", "replace") if not no_message else ""
            receiver_name = (r.read_string(receiver_name_len).decode("utf-8", "replace")
                              if not no_receiver_name else "")
            sender_name = (r.read_string(sender_name_len).decode("utf-8", "replace")
                            if not no_sender_name else "")

            return {
                "type": chat_type, "type_name": self._CHAT_TYPE_NAMES.get(chat_type, chat_type),
                "lang": language, "message": message,
                "sender_guid": int.from_bytes(bytes(sender), "little"),
                "receiver_guid": int.from_bytes(bytes(recv), "little"),
                "group_guid": int.from_bytes(bytes(group), "little"),
                "guild_guid": int.from_bytes(bytes(guild), "little"),
                "channel_name": channel_name.decode("utf-8", "replace") if channel_name else "",
                "sender_name": sender_name, "receiver_name": receiver_name,
                "achievement_id": achievement_id, "chat_tag": chat_tag,
            }
        except (IndexError, struct.error) as exc:
            log("SMSG_MESSAGECHAT parse failed: %s" % exc)
            return None

    # --- S1a: group verbs -----------------------------------------------
    def group_accept(self):
        """CMSG_GROUP_INVITE_RESPONSE, accept=1 (GroupHandler.cpp:298-304): u8
        skip, bits [unknown=0, accept]."""
        bw = BitWriter()
        bw.bit(0)
        bw.bit(1)
        self.send_packet(CMSG_GROUP_INVITE_RESPONSE, bytes([0]) + bw.flush())
        log("sent CMSG_GROUP_INVITE_RESPONSE accept")

    def group_decline(self):
        bw = BitWriter()
        bw.bit(0)
        bw.bit(0)
        self.send_packet(CMSG_GROUP_INVITE_RESPONSE, bytes([0]) + bw.flush())
        log("sent CMSG_GROUP_INVITE_RESPONSE decline")

    def group_disband(self):
        """CMSG_GROUP_DISBAND (GroupHandler.cpp:567-590): u8 unk. Used for both
        the `leave` and `disband` verbs - RemoveFromGroup() on a non-leader is
        a normal leave."""
        self.send_packet(CMSG_GROUP_DISBAND, bytes([0]))
        log("sent CMSG_GROUP_DISBAND")

    def set_dungeon_difficulty(self, mode, timeout=15.0):
        """#373: CMSG_SET_DUNGEON_DIFFICULTY (MiscHandler.cpp:1950-2007): a single uint32, 1 = normal, 2 = heroic.
        Only the group leader may change it (:1978), nobody in the group may be inside a non-raid dungeon (:1989), and a
        request equal to the player's current value returns silently (:1963) - so an equal value is not sent. Waits for the
        server's SMSG_SET_DUNGEON_DIFFICULTY echo (Group::SetDungeonDifficulty, Group.cpp:2603, sends it to every member
        including the leader) and logs a single greppable result line:
          DUNGEON-DIFFICULTY OK mode=<n>      (changed, or already <n>)
          DUNGEON-DIFFICULTY FAIL mode=<n> ... (no echo: not the leader / a member is inside a dungeon / invalid mode)
        Returns True on OK."""
        if self.dungeon_difficulty == mode:
            log("DUNGEON-DIFFICULTY OK mode=%d (already, nothing sent)" % mode)
            return True
        self.send_packet(CMSG_SET_DUNGEON_DIFFICULTY, struct.pack("<I", mode))
        log("sent CMSG_SET_DUNGEON_DIFFICULTY mode=%d" % mode)
        deadline = time.time() + timeout
        while time.time() < deadline and self.dungeon_difficulty != mode:
            self.pump(0.5)
        if self.dungeon_difficulty == mode:
            log("DUNGEON-DIFFICULTY OK mode=%d (server echoed SMSG_SET_DUNGEON_DIFFICULTY)" % mode)
            return True
        log("DUNGEON-DIFFICULTY FAIL mode=%d: no SMSG_SET_DUNGEON_DIFFICULTY within %.0fs (last known %s; the server drops the "
            "request if this player is not the group leader or any member is inside a dungeon)"
            % (mode, timeout, self.dungeon_difficulty))
        return False

    def group_uninvite(self, target_guid, reason=""):
        """CMSG_GROUP_UNINVITE_GUID (GroupHandler.cpp:375-401): u8 skip, bits
        [guid mask 6,4,3,2,0,1,7,5][reasonLen(8)], reason, ByteSeq 5,6,1,4,3,2,7,0."""
        reason_b = reason.encode("utf-8")
        body = bytes([0])
        body += guid_mask_bits(target_guid, (6, 4, 3, 2, 0, 1, 7, 5),
                                extra_bits=[(len(reason_b), 8)])
        body += reason_b
        body += guid_byte_seq(target_guid, (5, 6, 1, 4, 3, 2, 7, 0))
        self.send_packet(CMSG_GROUP_UNINVITE_GUID, body)
        log("sent CMSG_GROUP_UNINVITE_GUID -> %s" % target_guid)

    def group_set_leader(self, target_guid):
        """CMSG_GROUP_SET_LEADER (GroupHandler.cpp:444-483): bits
        [1,7,0,2,5,3,4,6], ByteSeq 1,5,7,6,0,2,4,3. The handler starts with read_skip<uint8>
        (GroupHandler.cpp:451), so a leading 0 byte is required (without it the guid is
        misread and the leader never changes)."""
        body = bytes([0]) + guid_mask_bits(target_guid, (1, 7, 0, 2, 5, 3, 4, 6))
        body += guid_byte_seq(target_guid, (1, 5, 7, 6, 0, 2, 4, 3))
        self.send_packet(CMSG_GROUP_SET_LEADER, body)
        log("sent CMSG_GROUP_SET_LEADER -> %s" % target_guid)

    def group_loot_method(self, method, master_guid=0, threshold=2):
        """CMSG_LOOT_METHOD (GroupHandler.cpp:592-636): u8 groupIndex skip, u8
        method, u32 threshold, bits [7,1,2,0,4,5,6,3], ByteSeq 7,1,3,4,6,5,0,2."""
        body = struct.pack("<BBI", 0, method, threshold)
        body += guid_mask_bits(master_guid, (7, 1, 2, 0, 4, 5, 6, 3))
        body += guid_byte_seq(master_guid, (7, 1, 3, 4, 6, 5, 0, 2))
        self.send_packet(CMSG_LOOT_METHOD, body)
        log("sent CMSG_LOOT_METHOD method=%d master=%s threshold=%d" %
            (method, master_guid, threshold))

    def group_raid_convert(self, to_raid=True):
        """CMSG_GROUP_RAID_CONVERT (GroupHandler.cpp:821-849): a single bit."""
        bw = BitWriter()
        bw.bit(1 if to_raid else 0)
        self.send_packet(CMSG_GROUP_RAID_CONVERT, bw.flush())
        log("sent CMSG_GROUP_RAID_CONVERT to_raid=%s" % to_raid)

    def group_raid_icon(self, symbol, target_guid=None):
        """MSG_RAID_TARGET_UPDATE (GroupHandler.cpp:775-819): u8 partyIndex, u8
        symbol (0xFF requests the current icon list; otherwise a guid follows),
        bits [3,2,1,5,0,6,7,4], ByteSeq 2,3,0,7,5,1,6,4."""
        body = struct.pack("<BB", 0, symbol)
        if symbol != 0xFF and target_guid:
            body += guid_mask_bits(target_guid, (3, 2, 1, 5, 0, 6, 7, 4))
            body += guid_byte_seq(target_guid, (2, 3, 0, 7, 5, 1, 6, 4))
        self.send_packet(CMSG_RAID_TARGET_UPDATE, body)
        log("sent MSG_RAID_TARGET_UPDATE symbol=%s target=%s" % (symbol, target_guid))

    def ready_check(self):
        """CMSG_RAID_READY_CHECK (GroupHandler.cpp:998-1069): u8 partyIndex."""
        self.send_packet(CMSG_RAID_READY_CHECK, bytes([0]))
        log("sent CMSG_RAID_READY_CHECK")

    def ready_check_confirm(self, accepted):
        """CMSG_RAID_READY_CHECK_CONFIRM (GroupHandler.cpp:1071-1104): u8 skip,
        bits [guid2,1,0,3,6,status,7,4,5] (status is interleaved with the guid
        mask, not appended after it), ByteSeq 1,0,3,2,4,5,7,6. The guid is
        unused server-side, so this sends our own."""
        guid = self.char_guid or 0
        b = guid.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (2, 1, 0, 3, 6):
            bw.bit(b[idx] != 0)
        bw.bit(1 if accepted else 0)
        for idx in (7, 4, 5):
            bw.bit(b[idx] != 0)
        body = bytes([0]) + bw.flush() + guid_byte_seq(guid, (1, 0, 3, 2, 4, 5, 7, 6))
        self.send_packet(CMSG_RAID_READY_CHECK_CONFIRM, body)
        log("sent CMSG_RAID_READY_CHECK_CONFIRM accepted=%s" % accepted)

    # --- S1a: SMSG_GROUP_LIST / SMSG_PARTY_MEMBER_STATS parse ---------------
    def _parse_group_removed(self, body):
        """SMSG_GROUP_LIST parse for Group::SendGroupRemoved (Group.cpp:2136-
        2193): the packet the server sends when the player has no group left
        (e.g. after `leave`/disband). Distinct, shorter layout - no loot-mode
        bits/fields at all, leaderGuid is always Empty, count is always 0."""
        r = ByteReader(body)
        group_mask = [0] * 8
        group_mask[0] = r.read_bit()
        r.read_bit(); r.read_bit()             # leaderGuid[7], [1] (always 0)
        r.read_bit()                           # has dungeon/raid difficulty (0)
        group_mask[7] = r.read_bit()
        r.read_bit(); r.read_bit()             # leaderGuid[6], [5]
        r.read_bits(21)                        # count (0)
        r.read_bit(); r.read_bit()             # leaderGuid[3], [0]
        r.read_bit()                           # has loot mode (0)
        group_mask[5] = r.read_bit()
        group_mask[2] = r.read_bit()
        group_mask[4] = r.read_bit()
        group_mask[1] = r.read_bit()
        r.read_bit()                           # isLFG (0)
        r.read_bit()                           # leaderGuid[2]
        group_mask[6] = r.read_bit()
        r.read_bit()                           # leaderGuid[4]
        group_mask[3] = r.read_bit()

        group = [0] * 8
        r.read_byte_seq(0)                     # leaderGuid[0] (always 0)
        group[1] = r.read_byte_seq(group_mask[1])
        r.read_byte_seq(0); r.read_byte_seq(0)  # leaderGuid[4], [2]
        group[6] = r.read_byte_seq(group_mask[6])
        group[4] = r.read_byte_seq(group_mask[4])
        r.read_u8(); r.read_u8(); r.read_i32()  # 0x10, slot, -1
        group[7] = r.read_byte_seq(group_mask[7])
        r.read_byte_seq(0); r.read_byte_seq(0)  # leaderGuid[3], [1]
        r.read_u32()                            # counter
        group[0] = r.read_byte_seq(group_mask[0])
        group[2] = r.read_byte_seq(group_mask[2])
        group[5] = r.read_byte_seq(group_mask[5])
        group[3] = r.read_byte_seq(group_mask[3])

        self.group_guid = int.from_bytes(bytes(group), "little")
        self.leader_guid = None
        self.loot_method = None
        self.group_members = {}
        log("SMSG_GROUP_LIST: no group (removed)")
        self.emit("group", members=0, names="-", leader=None, loot_method=None)

    def _parse_group_list(self, body):
        """SMSG_GROUP_LIST parse (Groups/Group.cpp:1902-2077 SendUpdateToPlayer).
        Bit-packed; replays the write order exactly (a normal, non-LFG group is
        assumed - the two LFG-only bit/field extensions are skipped). A player
        with no group gets the shorter SendGroupRemoved shape instead (below);
        that layout diverges right after the loot-mode bit, so a parse failure
        here is retried against it before giving up."""
        try:
            r = ByteReader(body)
            group_mask = [0] * 8
            leader_mask = [0] * 8
            group_mask[0] = r.read_bit()
            leader_mask[7] = r.read_bit()
            leader_mask[1] = r.read_bit()
            r.read_bit()                       # has dungeon/raid difficulty
            group_mask[7] = r.read_bit()
            leader_mask[6] = r.read_bit()
            leader_mask[5] = r.read_bit()
            count = r.read_bits(21)

            members = []
            for _ in range(count):
                m_mask = [0] * 8
                m_mask[1] = r.read_bit()
                m_mask[2] = r.read_bit()
                m_mask[5] = r.read_bit()
                m_mask[6] = r.read_bit()
                name_len = r.read_bits(6)
                m_mask[7] = r.read_bit()
                m_mask[3] = r.read_bit()
                m_mask[0] = r.read_bit()
                m_mask[4] = r.read_bit()
                members.append({"mask": m_mask, "name_len": name_len})

            leader_mask[3] = r.read_bit()
            leader_mask[0] = r.read_bit()
            r.read_bit()                       # has loot mode
            group_mask[5] = r.read_bit()
            looter_mask = [0] * 8
            for idx in (6, 4, 5, 2, 1, 0, 7, 3):
                looter_mask[idx] = r.read_bit()
            group_mask[2] = r.read_bit()
            group_mask[4] = r.read_bit()
            group_mask[1] = r.read_bit()
            is_lfg = r.read_bit()
            leader_mask[2] = r.read_bit()
            group_mask[6] = r.read_bit()
            if is_lfg:
                r.read_bit()
                r.read_bit()
            leader_mask[4] = r.read_bit()
            group_mask[3] = r.read_bit()

            leader = [0] * 8
            group = [0] * 8
            looter = [0] * 8
            leader[0] = r.read_byte_seq(leader_mask[0])
            dungeon_diff = r.read_u32()
            raid_diff = r.read_u32()

            for m in members:
                mm = m["mask"]
                g = [0] * 8
                g[6] = r.read_byte_seq(mm[6])
                g[3] = r.read_byte_seq(mm[3])
                m["roles"] = r.read_u8()
                m["online"] = r.read_u8()
                g[7] = r.read_byte_seq(mm[7])
                g[4] = r.read_byte_seq(mm[4])
                g[1] = r.read_byte_seq(mm[1])
                m["name"] = r.read_string(m["name_len"]).decode("utf-8", "replace")
                g[5] = r.read_byte_seq(mm[5])
                g[2] = r.read_byte_seq(mm[2])
                m["subgroup"] = r.read_u8()
                g[0] = r.read_byte_seq(mm[0])
                m["flags"] = r.read_u8()
                m["guid"] = int.from_bytes(bytes(g), "little")

            group[1] = r.read_byte_seq(group_mask[1])
            if is_lfg:
                r.read_f32()
                r.read_u8()
                r.read_u8()
                r.read_u32()
                r.read_u8()
                r.read_u8()
                r.read_u8()
                r.read_u32()
            leader[4] = r.read_byte_seq(leader_mask[4])
            leader[2] = r.read_byte_seq(leader_mask[2])
            loot_method = r.read_u8()
            looter[0] = r.read_byte_seq(looter_mask[0])
            looter[5] = r.read_byte_seq(looter_mask[5])
            looter[4] = r.read_byte_seq(looter_mask[4])
            looter[3] = r.read_byte_seq(looter_mask[3])
            looter[2] = r.read_byte_seq(looter_mask[2])
            loot_threshold = r.read_u8()
            looter[7] = r.read_byte_seq(looter_mask[7])
            looter[1] = r.read_byte_seq(looter_mask[1])
            looter[6] = r.read_byte_seq(looter_mask[6])
            group[6] = r.read_byte_seq(group_mask[6])
            group[4] = r.read_byte_seq(group_mask[4])
            group_type = r.read_u8()
            group_slot = r.read_u8()
            group_position = r.read_u32()
            group[7] = r.read_byte_seq(group_mask[7])
            leader[3] = r.read_byte_seq(leader_mask[3])
            leader[1] = r.read_byte_seq(leader_mask[1])
            counter = r.read_u32()
            group[0] = r.read_byte_seq(group_mask[0])
            group[2] = r.read_byte_seq(group_mask[2])
            group[5] = r.read_byte_seq(group_mask[5])
            group[3] = r.read_byte_seq(group_mask[3])
            leader[7] = r.read_byte_seq(leader_mask[7])
            r.read_u8()
            leader[5] = r.read_byte_seq(leader_mask[5])
            leader[6] = r.read_byte_seq(leader_mask[6])

            self.group_guid = int.from_bytes(bytes(group), "little")
            self.leader_guid = int.from_bytes(bytes(leader), "little")
            self.loot_method = loot_method
            self.group_members = {m["name"].lower(): m["guid"] for m in members if m["name"]}
            names = ",".join(m["name"] for m in members if m["name"]) or "-"
            log("SMSG_GROUP_LIST: %d member(s) [%s], leader=%s, lootMethod=%d" %
                (len(members), names, self.leader_guid, loot_method))
            self.emit("group", members=len(members), names=names,
                       leader=self.leader_guid, loot_method=loot_method)
        except (IndexError, struct.error) as exc:
            try:
                self._parse_group_removed(body)
            except (IndexError, struct.error):
                log("SMSG_GROUP_LIST parse failed: %s" % exc)

    def _parse_party_member_stats(self, body):
        """SMSG_PARTY_MEMBER_STATS parse (GroupHandler.cpp:1172-1514
        BuildPartyMemberStatsChangedPacket). The dataBuffer is length-prefixed
        (u32), so on any decode surprise we can still report the mask and stop
        cleanly instead of misaligning later packets."""
        try:
            r = ByteReader(body)
            mask_bits = [0] * 8
            for idx, is_guid in ((0, True), (5, True), (None, False), (1, True),
                                  (4, True), (None, False), (6, True), (2, True),
                                  (7, True), (3, True)):
                bit = r.read_bit()
                if is_guid:
                    mask_bits[idx] = bit
            guid = [0] * 8
            for idx in (3, 2, 6, 7, 5):
                guid[idx] = r.read_byte_seq(mask_bits[idx])
            mask = r.read_u32()
            for idx in (1, 4, 0):
                guid[idx] = r.read_byte_seq(mask_bits[idx])
            length = r.read_u32()
            end = r.pos + length
            fields = {}
            for name, flag, kind in GROUP_UPDATE_FIELDS:
                if not (mask & flag):
                    continue
                if r.pos >= end:
                    break
                if kind == "u8":
                    fields[name] = r.read_u8()
                elif kind == "u16":
                    fields[name] = r.read_u16()
                elif kind == "u32":
                    fields[name] = r.read_u32()
                elif kind == "i32":
                    fields[name] = r.read_i32()
                elif kind == "u64":
                    fields[name] = r.read_u64()
                elif kind == "u8u8":
                    fields[name] = (r.read_u8(), r.read_u8())
                elif kind == "pos16":
                    fields[name] = (r.read_u16(), r.read_u16(), r.read_u16())
                elif kind == "cstr":
                    fields[name] = r.read_cstr().decode("utf-8", "replace")
                elif kind == "auras":
                    # u8 unk, u64 auramask, u32 count, per set bit: u32 spell +
                    # u8 flags + u32 effectMask [+ variable amounts] - not
                    # needed for the group-parity smoke test, so stop cleanly.
                    fields[name] = "present"
                    break
            r.pos = end
            member_guid = int.from_bytes(bytes(guid), "little")
            log("SMSG_PARTY_MEMBER_STATS guid=%s mask=0x%X fields=%s" %
                (member_guid, mask, fields))
            self.emit("party_stats", guid=member_guid, mask="0x%X" % mask,
                       **{k: v for k, v in fields.items() if isinstance(v, (int, str))})
        except (IndexError, struct.error) as exc:
            log("SMSG_PARTY_MEMBER_STATS parse failed: %s" % exc)

    # --- S1a: character creation --------------------------------------------
    def char_create(self, name, race, klass, gender=0, skin=0, face=0,
                     hair_style=0, hair_color=0, facial_hair=0, outfit=0):
        """CMSG_CHAR_CREATE (CharacterHandler.cpp:365-378): u8 outfitId,
        hairStyle, class, skin, face, race, facialHair, gender, hairColor,
        bits [nameLen(6), unk(1)], name."""
        name_b = name.encode("utf-8")
        body = struct.pack("<BBBBBBBBB", outfit, hair_style, klass, skin, face,
                            race, facial_hair, gender, hair_color)
        bw = BitWriter()
        bw.bits(len(name_b), 6)
        bw.bit(0)
        body += bw.flush() + name_b
        self.send_packet(CMSG_CHAR_CREATE, body)
        log("sent CMSG_CHAR_CREATE name=%r race=%d class=%d" % (name, race, klass))

    def ping(self):
        self.send_packet(CMSG_PING, struct.pack("<II", int(time.time() * 1000) & 0xFFFFFFFF, 0))

    # --- S1b (issue #172): movement -----------------------------------------
    def _move_ts(self):
        return int((time.time() - self.login_time) * 1000) & 0xFFFFFFFF

    def _mguid(self):
        """Guid stamped into the movement packets of move_stop/face/goto/walk: the character, or - after the
        `mover <npc|rawguid>` verb - the vehicle it is driving (HandleMovementOpcode drops a packet whose guid
        is not _player->m_mover, MovementHandler.cpp:355; a seated driver's m_mover is the vehicle)."""
        return self.move_guid or self.char_guid

    def _transport_tuple(self):
        """Local-offset transport block for the current on_transport state, or
        None (issue #191). move_stop/face/goto/walk all call this so a
        script's ordinary movement verbs keep sending transport data once
        board_transport() has been used, exactly as a real client would (see
        board_transport's docstring).

        S9 owner requirement (#206, "walk-on platforms"): if transport_local is set
        (board_natural() below), it is used directly instead of the anchor/self.pos
        subtraction - the local offset is constant while standing still on a deck, so there is
        nothing to derive from self.pos (which is what made board_transport()'s anchor wrong in
        the first place - see its docstring)."""
        if not self.on_transport or not self.transport_guid:
            return None
        if self.transport_local is not None:
            lx, ly, lz, lo = self.transport_local
            return (self.transport_guid, lx, ly, lz, lo, 0, self.transport_seat)
        ax, ay, az, ao = self.transport_anchor
        x, y, z = self.pos
        return (self.transport_guid, x - ax, y - ay, z - az, ao, 0, self.transport_seat)

    def board_natural(self, transport_guid, lx, ly, lz, seat=-1):
        """S9 owner requirement (#206): the "natural" walk-on acceptance path. The harness never
        touches this character - board_natural() is called only after this client has already
        walked here (move_walk/move_goto, real movement packets) to the transport's CURRENT
        world position for local offset (lx,ly,lz), while genuinely docked. That world position
        comes from watching a SEPARATE scout character's `.botscenario <scout> transport ...`
        `[TRANSPORT] placed` line (BotScenario.cpp, unmodified - it already reports the live
        world x,y,z for a known local offset the moment the module itself confirms the transport
        is docked with a safe margin) - never from calling `board`/`teleport` on the master or
        the bot(s) under test.

        Sends one MSG_MOVE_STOP with hasTransportData so the server's MovementHandler attaches
        this character as a real transport passenger (Transport::AddPassenger) - the same
        server-side effect a real client's own collision detection would trigger by starting to
        send ONTRANSPORT movement on its own. No further packets are needed while stationary on
        the deck (a real client only heartbeats while actually moving); ordinary move_stop/
        move_walk/move_goto keep including transport data via _transport_tuple() above for as
        long as on_transport stays True.
        """
        self.transport_guid = int(transport_guid)
        self.transport_seat = seat
        self.transport_local = (float(lx), float(ly), float(lz), 0.0)
        self.on_transport = True
        x, y, z = self.pos
        body = build_movement_body(MSE_STOP, self.char_guid, x, y, z, self.orientation,
                                    transport=(self.transport_guid, float(lx), float(ly), float(lz), 0.0, 0, seat))
        self.send_packet(MSG_MOVE_STOP, body)
        log("sent MSG_MOVE_STOP with transport data (natural board, guid %d local=(%.2f,%.2f,%.2f))"
            % (self.transport_guid, lx, ly, lz))
        self.emit("board_natural", transport=self.transport_guid)

    def leave_transport_natural(self):
        """S9 owner requirement (#206): disembark the natural way - clear on_transport and send
        one plain (non-transport) MSG_MOVE_STOP from the current position. The caller is
        responsible for having already walked off the deck (move_walk/move_goto) to solid ground
        before calling this; this method itself does not move anything."""
        self.on_transport = False
        self.transport_guid = None
        self.transport_local = None
        x, y, z = self.pos
        body = build_movement_body(MSE_STOP, self.char_guid, x, y, z, self.orientation)
        self.send_packet(MSG_MOVE_STOP, body)
        log("sent MSG_MOVE_STOP without transport data (natural leave)")
        self.emit("leave_transport_natural")

    def board_transport(self, transport_guid, seat=-1):
        """Start sending transport-relative movement after the harness's
        `board <entry>` SOAP verb has already placed us on `transport_guid`
        server-side (BotScenario.cpp's `board` case: TeleportTo(transport's
        own position) + SetTransport + AddPassenger). That SOAP call does NOT
        make the *client* send hasTransportData, so the very next plain
        movement packet would hit MovementHandler.cpp's `else if (plrMover &&
        plrMover->GetTransport())` branch and knock us right back off - this
        was issue #191's T7 failure mode. Since `board` teleports us to the
        transport's own x,y,z,o, our current self.pos/self.orientation ARE the
        transport anchor at local offset (0,0,0,0); later movement calls
        derive their local offset from the delta off that anchor (see
        _transport_tuple). This is an approximation: it assumes the
        transport's own orientation stays close to constant for the rest of
        the test (true for a short docked-boat scenario, not for mid-voyage
        rotation - see the issue #191 report).

        S9 (#206) T7 correction: the assumption above is false. SMSG_MOVE_TELEPORT's
        handler (this file, near "SMSG_MOVE_TELEPORT -> sent CMSG_MOVE_TELEPORT_ACK")
        never parses the packet body, so self.pos/self.orientation still hold the
        PRE-teleport values at this point - the just-issued `board` SOAP call teleports
        the character server-side, but nothing has updated the client's local tracking
        yet. Sending a MSG_MOVE_STOP built from that stale world position (even with
        the correct transport guid/local-offset bits) let this fork's movement handler
        relocate the character back to the stale spot while still flagged as a
        passenger - the exact failure this test observed twice live (master ended up
        back near its pre-board teleport point instead of on the transport). Since BotScenario.cpp's `board` case
        (TeleportTo + SetTransport + AddPassenger, all server-authoritative) already
        places and attaches the character correctly, and the transport's own per-tick
        UpdatePassengerPositions carries every passenger regardless of client packets
        (no bot/AI motion generator is fighting it for a real player sitting idle), the
        safe fix is to send NOTHING here and just track the passenger state locally for
        any later move_* call. A correct transport-relative anchor would need the
        transport's live world position, which this client cannot learn without parsing
        SMSG_UPDATE_OBJECT (not implemented - see the transport-stage-design.md note on
        approximate offsets); do not re-enable this send without that.
        """
        self.transport_guid = int(transport_guid)
        self.transport_seat = seat
        x, y, z = self.pos
        self.transport_anchor = (x, y, z, self.orientation)
        self.on_transport = True
        log("board_transport: NOT sending a client movement packet (stale self.pos "
            "would corrupt the server-authoritative `board` placement - see docstring); "
            "relying on the transport's own per-tick passenger relocation instead")
        self.emit("board", transport=self.transport_guid)

    def move_stop(self):
        x, y, z = self.pos
        body = build_movement_body(MSE_STOP, self._mguid(), x, y, z, self.orientation,
                                    timestamp_ms=self._move_ts(), transport=self._transport_tuple())
        self.send_packet(MSG_MOVE_STOP, body)

    def move_face(self, orientation):
        """MSG_MOVE_SET_FACING: turn in place, no position change."""
        self.orientation = orientation % (2 * math.pi)
        x, y, z = self.pos
        body = build_movement_body(MSE_SET_FACING, self._mguid(), x, y, z, self.orientation,
                                    timestamp_ms=self._move_ts(), transport=self._transport_tuple())
        self.send_packet(MSG_MOVE_SET_FACING, body)
        log("sent MSG_MOVE_SET_FACING o=%.2f" % self.orientation)

    def move_goto(self, x, y, z):
        """Single-shot position set (no GM teleport involved): `.go xyz` is
        Console::No on both cores so SOAP cannot issue it, and the spec notes
        anticheat here is report-only, so a lone Stop packet at the target
        position is how a script places the player without walking there."""
        self.pos = (x, y, z)
        body = build_movement_body(MSE_STOP, self._mguid(), x, y, z, self.orientation,
                                    timestamp_ms=self._move_ts(), transport=self._transport_tuple())
        self.send_packet(MSG_MOVE_STOP, body)
        log("goto (x=%.1f, y=%.1f, z=%.1f)" % (x, y, z))
        self.emit("pos", x="%.1f" % x, y="%.1f" % y, z="%.1f" % z)
        self._check_area_triggers()

    def move_walk(self, waypoints, speed=7.0, heartbeat_ms=500):
        """Walk a straight-line path through `waypoints` (list of (x,y,z)) at
        `speed` yd/s, sending a real MSG_MOVE_HEARTBEAT every `heartbeat_ms` on
        the time-sync clock (matches the spec's 500 ms / <=7 y/s cadence).
        StartForward is sent once at the beginning, Stop once at the end.
        Keeps sending transport data (via _transport_tuple) while
        on_transport is set, so a script can walk around the deck after
        board_transport() (issue #191)."""
        if self.pos is None or not waypoints:
            return
        cx, cy, cz = self.pos
        tx, ty, tz = waypoints[0]
        o = math.atan2(ty - cy, tx - cx) % (2 * math.pi)
        self.orientation = o
        body = build_movement_body(MSE_START_FORWARD, self._mguid(), cx, cy, cz, o,
                                    movement_flags=MOVEMENTFLAG_FORWARD,
                                    timestamp_ms=self._move_ts(), transport=self._transport_tuple())
        self.send_packet(MSG_MOVE_START_FORWARD, body)
        dt = heartbeat_ms / 1000.0
        step = max(speed * dt, 0.01)
        for tx, ty, tz in waypoints:
            for _ in range(20000):    # safety cap: ~ a full heartbeat-run per waypoint
                dx, dy = tx - cx, ty - cy
                dist = math.hypot(dx, dy)
                if dist <= step:
                    cx, cy, cz = tx, ty, tz
                    break
                o = math.atan2(dy, dx) % (2 * math.pi)
                self.orientation = o
                cx += dx / dist * step
                cy += dy / dist * step
                cz = tz
                self.pos = (cx, cy, cz)
                body = build_movement_body(MSE_HEARTBEAT, self._mguid(), cx, cy, cz, o,
                                            movement_flags=MOVEMENTFLAG_FORWARD,
                                            timestamp_ms=self._move_ts(), transport=self._transport_tuple())
                self.send_packet(MSG_MOVE_HEARTBEAT, body)
                self.emit("pos", x="%.1f" % cx, y="%.1f" % cy, z="%.1f" % cz)
                self.pump(dt)
            self.pos = (cx, cy, cz)
        body = build_movement_body(MSE_STOP, self._mguid(), cx, cy, cz, self.orientation,
                                    timestamp_ms=self._move_ts(), transport=self._transport_tuple())
        self.send_packet(MSG_MOVE_STOP, body)
        self.emit("pos", x="%.1f" % cx, y="%.1f" % cy, z="%.1f" % cz)
        log("walk complete at (%.1f, %.1f, %.1f)" % (cx, cy, cz))
        self._check_area_triggers()

    # --- S2 (issue #191): area triggers --------------------------------------
    def _check_area_triggers(self):
        """Auto-fire CMSG_AREATRIGGER the way a real client does when
        walk/goto ends inside a known trigger box (KNOWN_AREATRIGGERS). Tracks
        enter/leave edges per trigger id so a script sitting still doesn't
        resend `entered` every call. See areatrigger() for the packet and
        MiscHandler.cpp:1133-1220 (HandleAreaTriggerOpcode) /
        Player::IsInAreaTrigger for the server's own geometry check (sphere/
        box/cylinder by AreaTrigger.dbc ShapeType) that this approximates."""
        if self.pos is None or self.map_id is None:
            return
        x, y, z = self.pos
        for tid, info in KNOWN_AREATRIGGERS.items():
            if info["map"] != self.map_id:
                continue
            tx, ty, tz = info["pos"]
            dist = math.sqrt((x - tx) ** 2 + (y - ty) ** 2 + (z - tz) ** 2)
            inside = dist <= info["radius"]
            was_inside = tid in self._at_inside
            if inside and not was_inside:
                self._at_inside.add(tid)
                log("walk/goto ended inside area trigger %d (%s), dist=%.1f" % (tid, info.get("name", ""), dist))
                self.areatrigger(tid, entered=1)
            elif not inside and was_inside:
                self._at_inside.discard(tid)
                self.areatrigger(tid, entered=0)

    def areatrigger(self, trigger_id, entered=1, from_client=1):
        """CMSG_AREATRIGGER (MiscHandler.cpp:1133-1141): plain uint32
        areaTriggerId, THEN two bits (fromClient, entered) - the u32 read
        resets the bit cursor, so the two ReadBit() calls start a fresh byte
        (ByteBuffer.h:198-208's self-refilling read, same convention already
        used throughout this file's movement/duel/use-item packets)."""
        bw = BitWriter()
        bw.bit(1 if from_client else 0)
        bw.bit(1 if entered else 0)
        body = struct.pack("<I", int(trigger_id)) + bw.flush()
        self.send_packet(CMSG_AREATRIGGER, body)
        log("sent CMSG_AREATRIGGER id=%d entered=%d" % (int(trigger_id), int(entered)))
        self.emit("areatrigger", id=trigger_id, entered=int(entered))

    # --- S1b: mount / taxi ---------------------------------------------------
    def mount(self, spell_id):
        """CMSG_CAST_SPELL, self-cast (no explicit target - see the module
        docstring above SpellHandler.cpp:685-1033 for the bit-by-bit derivation).
        31 header bits (auto-padded to 4 bytes), castCount(u8)=1, spellId(u32)."""
        bw = BitWriter()
        bw.bit(0)   # fake bit
        bw.bit(1)   # hasTargetString=false (inverted)
        bw.bit(0)   # fake bit
        bw.bit(0)   # hasCastCount=true (inverted)
        bw.bit(0)   # hasSrcLocation=false (direct)
        bw.bit(0)   # hasDestLocation=false (direct)
        bw.bit(0)   # hasSpellId=true (inverted)
        bw.bits(0, 2)  # researchDataCount=0
        bw.bit(1)   # hasTargetMask=false (inverted)
        bw.bit(1)   # hasMissileSpeed=false (inverted)
        bw.bit(1)   # hasGlyphIndex=false (inverted)
        bw.bit(0)   # hasMovement=false (direct)
        bw.bit(1)   # hasElevation=false (inverted)
        bw.bit(1)   # hasCastFlags=false (inverted)
        for _ in range(8):
            bw.bit(0)   # targetGuid mask: all-zero (implicit self target)
        for _ in range(8):
            bw.bit(0)   # itemTargetGuid mask: all-zero
        body = bw.flush() + struct.pack("<BI", 1, int(spell_id))
        self.send_packet(CMSG_CAST_SPELL, body)
        log("sent CMSG_CAST_SPELL (self-cast) spell=%d (%d bytes)" % (spell_id, len(body)))
        self.emit("cast", spell=spell_id)

    def cast(self, spell_id, target_guid=None, dest=None, elevation=None, speed=None, item_guid=None):
        """CMSG_CAST_SPELL with a real TARGET, for the player-facing "use this spell on that
        NPC" mechanic (issue #207: the Hot Rod's 61178 Grab Passenger on Ace/Izzy/Gobber).

        mount() above is the only other CMSG_CAST_SPELL sender and it is self-targeted: it leaves
        hasTargetMask=false and writes an all-zero targetGuid mask, so the server falls back to
        the implicit self target. A targeted cast instead needs (SpellHandler.cpp
        HandleCastSpellOpcode, 685-1033, read order verified against this build's source):

          bits, in read order:
            hasTargetString(730) .. hasCastFlags(748)   - identical prefix to mount(), see above
            targetGuid mask, 8 bits, order [5,4,2,7,1,6,3,0]   (SpellHandler.cpp:750-757)
            (destTransportGuid mask 759-768 - skipped: hasDestLocation=false)
            itemTargetGuid mask, 8 bits, all zero      (SpellHandler.cpp:822-829)
            hasTargetMask ? 20-bit targetMask : 0     (SpellHandler.cpp:843-844)
            hasCastFlags ? 5-bit castFlags : 0        (SpellHandler.cpp:846-847)
          then the byte seqs, in read order:
            itemTargetGuid[4,2,1,5,7,3,6,0]  (939-946) - all zero here, so nothing is emitted
            targetGuid[3,4,7,6,2,0,1,5]      (973-980)
            castCount(u8) then spellId(u32)  (1019-1020, matching mount()'s tail)

        target_guid is the full HighGuid-typed value as returned by the `npcguid` harness verb's
        SOAP reply ("guid=<N>"), passed straight through exactly like taxi() and quest_accept()
        below do. Omitting it keeps the old self-targeted behaviour bit-for-bit, so
        `cast <spell>` is a drop-in for `mount <spell>`.

        #513 `cast-at`: dest=(x,y,z) adds a ground target (TARGET_FLAG_DEST_LOCATION 0x40,
        hasDestLocation=1). Extra layout per HandleCastSpellOpcode:
          bits: hasDestLocation=1 (734); destTransportGuid mask 8 bits, order [1,3,5,0,2,6,7,4]
                (759-768) written all-zero (world coords, no transport; SpellCastTargets ctor
                Spell.cpp:142-146 then uses destPos as a world position)
          bytes: after itemTargetGuid seq (939-946) and BEFORE targetGuid seq (973-980):
                x,y,z floats in the order [destT2] x [destT4,1,0,3] y [destT7] z [destT5,6]
                (950-962) - transport guid bytes all zero so only x, y, z are emitted.
          optional: missileSpeed float (1007, hasMissileSpeed bit inverted) then elevation float
                (1010, hasElevation bit inverted) follow the target string, before castCount.
        Vehicle seats: the server picks the vehicle as caster when it HasSpell(spellId)
        (SpellHandler.cpp:1049-1060); nothing extra is sent.
        """
        bw = BitWriter()
        bw.bit(0)   # fake bit
        bw.bit(1)   # hasTargetString=false (inverted)
        bw.bit(0)   # fake bit
        bw.bit(0)   # hasCastCount=true (inverted)
        bw.bit(0)   # hasSrcLocation=false (direct)
        bw.bit(1 if dest is not None else 0)  # hasDestLocation (direct)
        bw.bit(0)   # hasSpellId=true (inverted)
        bw.bits(0, 2)  # researchDataCount=0
        # item_guid: an item target (TARGET_FLAG_ITEM), e.g. a runeforge rune engraved on the equipped weapon.
        # itemTargetGuid mask bits [1,0,7,4,6,5,3,2] (SpellHandler.cpp:822-829), byte seq [4,2,1,5,7,3,6,0] (939-946).
        ib = int(item_guid).to_bytes(8, "little") if item_guid else bytes(8)
        has_mask = target_guid is not None or dest is not None or bool(item_guid)
        bw.bit(0 if has_mask else 1)  # hasTargetMask (inverted)
        bw.bit(0 if speed is not None else 1)  # hasMissileSpeed (inverted)
        bw.bit(1)   # hasGlyphIndex=false (inverted)
        bw.bit(0)   # hasMovement=false (direct)
        bw.bit(0 if elevation is not None else 1)  # hasElevation (inverted)
        bw.bit(1)   # hasCastFlags=false (inverted)
        guid = int(target_guid) if target_guid is not None else 0
        gb = guid.to_bytes(8, "little")
        for idx in (5, 4, 2, 7, 1, 6, 3, 0):
            bw.bit(gb[idx] != 0)          # targetGuid mask
        if dest is not None:
            for _ in range(8):
                bw.bit(0)                 # destTransportGuid mask: all-zero (world coords)
        for idx in (1, 0, 7, 4, 6, 5, 3, 2):
            bw.bit(ib[idx] != 0)          # itemTargetGuid mask (all-zero without an item target)
        if has_mask:
            # a GameObject guid (HighGuid::GameObject 0xF11 in the top 12 bits) must carry TARGET_FLAG_GAMEOBJECT:
            # SpellCastTargets::Update resolves the object by mask, and the quest-chest lock cast
            # (OPEN_LOCK, GetGOTarget, SpellHandler.cpp:1078) needs the GO target, not a unit one.
            tflag = TARGET_FLAG_GAMEOBJECT if (guid >> 52) == 0xF11 else TARGET_FLAG_UNIT
            mask = (tflag if target_guid is not None else 0) |                    (TARGET_FLAG_DEST_LOCATION if dest is not None else 0) |                    (TARGET_FLAG_ITEM if item_guid else 0)
            bw.bits(mask, 20)             # hasTargetMask=true
        # hasCastFlags=false, so no 5-bit castFlags. researchDataCount==0 consumes no bytes.
        tail = bytearray()
        for idx in (4, 2, 1, 5, 7, 3, 6, 0):
            if ib[idx]:
                tail.append(ib[idx] ^ 1)  # itemTargetGuid byte-seq (nothing without an item target)
        if dest is not None:
            tail += struct.pack("<fff", float(dest[0]), float(dest[1]), float(dest[2]))
        if target_guid is not None:
            for idx in (3, 4, 7, 6, 2, 0, 1, 5):
                if gb[idx]:
                    tail.append(gb[idx] ^ 1)
        if speed is not None:
            tail += struct.pack("<f", float(speed))
        if elevation is not None:
            tail += struct.pack("<f", float(elevation))
        body = bw.flush() + bytes(tail) + struct.pack("<BI", 1, int(spell_id))
        self.send_packet(CMSG_CAST_SPELL, body)
        if dest is not None:
            log("sent CMSG_CAST_SPELL spell=%d dest=(%.2f,%.2f,%.2f) target=%s elev=%s speed=%s (%d bytes)"
                % (spell_id, dest[0], dest[1], dest[2], target_guid, elevation, speed, len(body)))
            self.emit("cast", spell=spell_id, dest="%g,%g,%g" % tuple(dest))
        elif item_guid:
            log("sent CMSG_CAST_SPELL spell=%d item=%d (%d bytes)" % (spell_id, int(item_guid), len(body)))
            self.emit("cast", spell=spell_id, item=int(item_guid))
        elif target_guid is not None:
            log("sent CMSG_CAST_SPELL spell=%d target=%d (%d bytes)"
                % (spell_id, guid, len(body)))
            self.emit("cast", spell=spell_id, target=guid)
        else:
            log("sent CMSG_CAST_SPELL (self-cast) spell=%d (%d bytes)" % (spell_id, len(body)))
            self.emit("cast", spell=spell_id)

    def taxi(self, src_node, dst_node, npc_guid=None):
        """CMSG_ACTIVATE_TAXI (TaxiHandler.cpp:429-466): u32 dst, u32 src (dst
        and src are REVERSED on the wire vs. their natural order), then the
        flightmaster npc's guid. The npc guid must come from the harness
        `npcguid` verb - it can't be derived client-side."""
        guid = npc_guid if npc_guid is not None else self.harness_guid.get("npc")
        if guid is None:
            log("taxi: no npc guid cached - run `npcguid <entry>` first")
            return
        guid = int(guid)
        body = struct.pack("<II", int(dst_node), int(src_node))
        body += guid_mask_bits(guid, (4, 0, 1, 2, 5, 6, 7, 3))
        body += guid_byte_seq(guid, (1, 0, 6, 5, 2, 4, 3, 7))
        self.send_packet(CMSG_ACTIVATE_TAXI, body)
        log("sent CMSG_ACTIVATE_TAXI %s -> %s (npc guid %d)" % (src_node, dst_node, guid))
        self.emit("taxi", src=src_node, dst=dst_node)

    # --- S1b: quests -----------------------------------------------------
    def quest_accept(self, quest_id, npc_guid=None):
        """CMSG_QUESTGIVER_ACCEPT_QUEST (QuestHandler.cpp:149-172): u32
        questId, bits [guid6,guid0,startCheat,guid2,guid7,guid5,guid4,guid3,
        guid1], byteseq [5,4,0,1,6,2,3,7]."""
        guid = int(npc_guid if npc_guid is not None else self.harness_guid.get("npc", 0))
        g = guid.to_bytes(8, "little")
        bw = BitWriter()
        bw.bit(g[6] != 0); bw.bit(g[0] != 0); bw.bit(0); bw.bit(g[2] != 0)
        bw.bit(g[7] != 0); bw.bit(g[5] != 0); bw.bit(g[4] != 0); bw.bit(g[3] != 0)
        bw.bit(g[1] != 0)
        body = struct.pack("<I", int(quest_id)) + bw.flush() + guid_byte_seq(guid, (5, 4, 0, 1, 6, 2, 3, 7))
        self.send_packet(CMSG_QUESTGIVER_ACCEPT_QUEST, body)
        log("sent CMSG_QUESTGIVER_ACCEPT_QUEST quest=%d npc=%d" % (quest_id, guid))
        self.emit("quest_accept", quest=quest_id)

    def quest_share(self, quest_id):
        """CMSG_PUSHQUESTTOPARTY (QuestHandler.cpp:758-765): u32 questId, no guid."""
        self.send_packet(CMSG_PUSHQUESTTOPARTY, struct.pack("<I", int(quest_id)))
        log("sent CMSG_PUSHQUESTTOPARTY quest=%d" % quest_id)
        self.emit("quest_share", quest=quest_id)

    def quest_complete(self, quest_id, npc_guid=None, auto_complete=False):
        """CMSG_QUESTGIVER_COMPLETE_QUEST (QuestHandler.cpp:669-696): u32
        questId, bits [guid4,guid2,guid1,guid5,guid6,guid7,guid3,
        autoCompleteMode,guid0], byteseq [0,2,1,4,3,6,7,5]."""
        guid = int(npc_guid if npc_guid is not None else self.harness_guid.get("npc", 0))
        g = guid.to_bytes(8, "little")
        bw = BitWriter()
        bw.bit(g[4] != 0); bw.bit(g[2] != 0); bw.bit(g[1] != 0); bw.bit(g[5] != 0)
        bw.bit(g[6] != 0); bw.bit(g[7] != 0); bw.bit(g[3] != 0)
        bw.bit(1 if auto_complete else 0)
        bw.bit(g[0] != 0)
        body = struct.pack("<I", int(quest_id)) + bw.flush() + guid_byte_seq(guid, (0, 2, 1, 4, 3, 6, 7, 5))
        self.send_packet(CMSG_QUESTGIVER_COMPLETE_QUEST, body)
        log("sent CMSG_QUESTGIVER_COMPLETE_QUEST quest=%d npc=%d" % (quest_id, guid))
        self.emit("quest_complete", quest=quest_id)

    def quest_reward(self, quest_id, item_id, npc_guid=None):
        """CMSG_QUESTGIVER_CHOOSE_REWARD (QuestHandler.cpp:454-480): u32
        reward (an ITEM ID on 5.4.8, not a choice index), u32 questId, bits
        [guid2,guid6,guid0,guid5,guid1,guid3,guid7,guid4], byteseq
        [1,2,5,7,0,3,6,4]."""
        guid = int(npc_guid if npc_guid is not None else self.harness_guid.get("npc", 0))
        g = guid.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (2, 6, 0, 5, 1, 3, 7, 4):
            bw.bit(g[idx] != 0)
        body = struct.pack("<II", int(item_id), int(quest_id)) + bw.flush() + guid_byte_seq(guid, (1, 2, 5, 7, 0, 3, 6, 4))
        self.send_packet(CMSG_QUESTGIVER_CHOOSE_REWARD, body)
        log("sent CMSG_QUESTGIVER_CHOOSE_REWARD quest=%d item=%d npc=%d" % (quest_id, item_id, guid))
        self.emit("quest_reward", quest=quest_id, item=item_id)

    # --- S1b: loot -------------------------------------------------------
    def loot_open(self, guid=None):
        """CMSG_LOOT (LootHandler.cpp:335-357): bits [4,5,2,7,0,1,3,6],
        byteseq [3,5,0,6,4,1,7,2]."""
        g = int(guid if guid is not None else
                (self.harness_guid.get("corpse") or self.harness_guid.get("npc") or 0))
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (4, 5, 2, 7, 0, 1, 3, 6):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (3, 5, 0, 6, 4, 1, 7, 2))
        self.send_packet(CMSG_LOOT, body)
        self.loot_source_guid = g
        log("sent CMSG_LOOT guid=%d" % g)
        self.emit("loot", guid=g)

    def loot_release(self, guid=None):
        """CMSG_LOOT_RELEASE (LootHandler.cpp:382-406): bits [7,4,2,3,0,5,6,1],
        byteseq [0,6,4,2,5,3,7,1]."""
        g = int(guid if guid is not None else (self.loot_source_guid or 0))
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (7, 4, 2, 3, 0, 5, 6, 1):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (0, 6, 4, 2, 5, 3, 7, 1))
        self.send_packet(CMSG_LOOT_RELEASE, body)
        log("sent CMSG_LOOT_RELEASE guid=%d" % g)
        self.emit("release", guid=g)

    def _parse_loot_response(self, body):
        """SMSG_LOOT_RESPONSE, full-permission layout (LootMgr.cpp:1468 LootView::WriteData, the path after the
        NONE_PERMISSION early-out at 1472-1513). Bits: 1 unk8, 1 lootType-present, guid[4], 20 currencyCount,
        loot[2,3,7,1], guid[6,7], !gold, personal, unk8, isAoE, guid[5], loot[6], 19 itemCount, loot[0],
        itemCount x (3 slotType + 5 flags = 8 bits, LootMgr.cpp:1421-1432), guid[1,0], currencyCount x 3 bits,
        loot[5], guid[3], loot[4], 1 unk8, guid[2]. Bytes: items (u32 suffix,count,itemid,4,0,randProp; u8 slot;
        u32 display = 29 B, 1441-1466), loot[2], u32 gold if present, loot[7], guid[5], loot[3], loot[4], u8 lootType,
        guid[4], loot[5], currencies (u32 count,u8 slot,u32 id), guid[2,3], loot[1], guid[0], loot[0], guid[6,7,1],
        loot[6] (1773-1800). guid = the looted object; loot = the Loot guid (HighGuid::Loot 0xF19) that
        CMSG_AUTOSTORE_LOOT_ITEM must carry (LootHandler.cpp:56-82 matches it against Player::m_lootView)."""
        try:
            r = ByteReader(body)
            r.read_bit()
            r.read_bit()
            g = [0] * 8
            L = [0] * 8
            g[4] = r.read_bit()
            ncur = r.read_bits(20)
            for i in (2, 3, 7, 1):
                L[i] = r.read_bit()
            g[6] = r.read_bit()
            g[7] = r.read_bit()
            no_gold = r.read_bit()
            r.read_bit()
            r.read_bit()
            r.read_bit()                      # personal, unk8, isAoE
            g[5] = r.read_bit()
            L[6] = r.read_bit()
            nitems = r.read_bits(19)
            L[0] = r.read_bit()
            for _ in range(nitems):
                r.read_bits(3)
                r.read_bits(5)
            g[1] = r.read_bit()
            g[0] = r.read_bit()
            for _ in range(ncur):
                r.read_bits(3)
            L[5] = r.read_bit()
            g[3] = r.read_bit()
            L[4] = r.read_bit()
            r.read_bit()
            g[2] = r.read_bit()
            items = []
            for _ in range(nitems):
                r.read_u32()
                cnt = r.read_u32()
                iid = r.read_u32()
                r.read_u32()
                r.read_u32()
                r.read_u32()
                slot = r.read_u8()
                r.read_u32()
                items.append((slot, iid, cnt))
            L[2] = r.read_byte_seq(L[2])
            gold = 0 if no_gold else r.read_u32()
            L[7] = r.read_byte_seq(L[7])
            g[5] = r.read_byte_seq(g[5])
            L[3] = r.read_byte_seq(L[3])
            L[4] = r.read_byte_seq(L[4])
            ltype = r.read_u8()
            g[4] = r.read_byte_seq(g[4])
            L[5] = r.read_byte_seq(L[5])
            for _ in range(ncur):
                r.read_u32()
                r.read_u8()
                r.read_u32()
            g[2] = r.read_byte_seq(g[2])
            g[3] = r.read_byte_seq(g[3])
            L[1] = r.read_byte_seq(L[1])
            g[0] = r.read_byte_seq(g[0])
            L[0] = r.read_byte_seq(L[0])
            g[6] = r.read_byte_seq(g[6])
            g[7] = r.read_byte_seq(g[7])
            g[1] = r.read_byte_seq(g[1])
            L[6] = r.read_byte_seq(L[6])
            src = int.from_bytes(bytes(g), "little")
            lg = int.from_bytes(bytes(L), "little")
            if r.remaining() != 0 or nitems > 64:
                raise ValueError("%d trailing bytes / %d items" % (r.remaining(), nitems))
        except Exception as e:
            log("SMSG_LOOT_RESPONSE parse failed (%s, %d bytes); a NONE_PERMISSION window has another layout" % (e, len(body)))
            self.emit("loot_response", ok=0)
            return
        self.loot_windows.append({"src": src, "loot": lg, "items": items, "gold": gold, "type": ltype})
        self.loot_source_guid = src
        log("SMSG_LOOT_RESPONSE src=%d loot=%d type=%d gold=%d items=%s currencies=%d" % (src, lg, ltype, gold, items, ncur))
        self.emit("loot_response", ok=1, src=src, loot=lg, items=len(items), gold=gold,
                  ids=",".join(str(i[1]) for i in items) or "-")

    def loot_all(self, wait=3.0):
        """Take everything from every pending loot window, then release it. Waits up to `wait` s for a window
        if none is pending (cast on a chest -> SMSG_LOOT_RESPONSE arrives a moment later).
        CMSG_AUTOSTORE_LOOT_ITEM (LootHandler.cpp:40-): bits count=ReadBits(23), per entry loot-guid mask order
        [2,7,0,6,5,3,1,4] (53-62); then per entry byte-seq order [0,4,1,7,6,5,3,2] (64-71) followed by u8 lootSlot (72).
        The guid is the LOOT guid (key of Player::m_lootView, 78-79), one packet for all slots.
        CMSG_LOOT_MONEY (LootHandler.cpp:161): empty body, sent only if the window has gold.
        CMSG_LOOT_RELEASE: loot_release() above, with the looted-object guid (LootHandler.cpp:393-402)."""
        if not self.loot_windows:
            self.pump(wait, stop_on=(SMSG_LOOT_RESPONSE,))
        wins, self.loot_windows = self.loot_windows, []
        if not wins:
            log("loot-all: no SMSG_LOOT_RESPONSE window pending")
            self.emit("loot_all", windows=0, items=0, gold=0)
            return 0
        n_items = 0
        n_gold = 0
        for win in wins:
            if win["items"]:
                bw = BitWriter()
                bw.bits(len(win["items"]), 23)
                lb = win["loot"].to_bytes(8, "little")
                for _ in win["items"]:
                    for idx in (2, 7, 0, 6, 5, 3, 1, 4):
                        bw.bit(lb[idx] != 0)
                body = bytearray(bw.flush())
                for slot, iid, cnt in win["items"]:
                    body += guid_byte_seq(win["loot"], (0, 4, 1, 7, 6, 5, 3, 2))
                    body.append(slot & 0xFF)
                self.send_packet(CMSG_AUTOSTORE_LOOT_ITEM, bytes(body))
                n_items += len(win["items"])
                log("sent CMSG_AUTOSTORE_LOOT_ITEM loot=%d slots=%s" % (win["loot"], [i[0] for i in win["items"]]))
            if win["gold"]:
                self.send_packet(CMSG_LOOT_MONEY, b"")
                n_gold += win["gold"]
                log("sent CMSG_LOOT_MONEY gold=%d" % win["gold"])
            self.pump(0.5)
            self.loot_release(win["src"])
        self.pump(0.5)
        self.emit("loot_all", windows=len(wins), items=n_items, gold=n_gold)
        return n_items

    _ROLL_CHOICES = {"pass": 0, "need": 1, "greed": 2}

    def loot_roll(self, choice):
        """CMSG_LOOT_ROLL (GroupHandler.cpp:638-666): u8 itemSlot, u8
        rollType, bits [7,1,2,0,6,3,4,5], byteseq [0,2,7,3,1,5,4,6]. Needs a
        prior SMSG_LOOT_START_ROLL (self.roll_guid/roll_item_slot)."""
        rt = self._ROLL_CHOICES.get(str(choice).lower())
        if rt is None or self.roll_guid is None:
            log("roll: no active roll (need an SMSG_LOOT_START_ROLL first) or bad choice %r" % choice)
            return
        g = self.roll_guid
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (7, 1, 2, 0, 6, 3, 4, 5):
            bw.bit(gb[idx] != 0)
        body = struct.pack("<BB", self.roll_item_slot or 0, rt) + bw.flush() + guid_byte_seq(g, (0, 2, 7, 3, 1, 5, 4, 6))
        self.send_packet(CMSG_LOOT_ROLL, body)
        log("sent CMSG_LOOT_ROLL choice=%s item=%s" % (choice, self.roll_item_id))
        self.emit("roll_sent", choice=choice, item=self.roll_item_id)

    def loot_master_give(self, slot, target_guid):
        """CMSG_LOOT_MASTER_GIVE (LootHandler.cpp:582-637), one item: bits
        target[0,2,4,1,7,6], size=ReadBits(23)=1, lootGuid mask
        [6,1,7,0,3,4,5,2], target[3,5]; bytes slot(u8), lootGuid byteseq
        [3,0,6,7,1,4,2,5], target byteseq [4,5,2,3,6,0,7,1]."""
        g = self.roll_guid or self.loot_source_guid or 0
        t = int(target_guid)
        gb = g.to_bytes(8, "little")
        tb = t.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (0, 2, 4, 1, 7, 6):
            bw.bit(tb[idx] != 0)
        bw.bits(1, 23)
        for idx in (6, 1, 7, 0, 3, 4, 5, 2):
            bw.bit(gb[idx] != 0)
        for idx in (3, 5):
            bw.bit(tb[idx] != 0)
        body = bw.flush()
        body += struct.pack("<B", int(slot))
        body += guid_byte_seq(g, (3, 0, 6, 7, 1, 4, 2, 5))
        body += guid_byte_seq(t, (4, 5, 2, 3, 6, 0, 7, 1))
        self.send_packet(CMSG_LOOT_MASTER_GIVE, body)
        log("sent CMSG_LOOT_MASTER_GIVE slot=%s target=%d loot=%d" % (slot, t, g))
        self.emit("master_give", slot=slot, target=t)

    # --- S1b: trade --------------------------------------------------------
    def trade_open(self, target_guid):
        """CMSG_INITIATE_TRADE (TradeHandler.cpp:636-657): bits
        [5,1,4,2,3,7,0,6], byteseq [4,6,2,0,3,7,5,1]."""
        g = int(target_guid)
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (5, 1, 4, 2, 3, 7, 0, 6):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (4, 6, 2, 0, 3, 7, 5, 1))
        self.send_packet(CMSG_INITIATE_TRADE, body)
        log("sent CMSG_INITIATE_TRADE -> %d" % g)
        self.emit("trade_open", target=g)

    def trade_item(self, trade_slot, bag, slot):
        """CMSG_SET_TRADE_ITEM (TradeHandler.cpp:799-808): u8 tradeSlot, u8
        slot (bag slot), u8 bag - in that order."""
        self.send_packet(CMSG_SET_TRADE_ITEM, struct.pack("<BBB", int(trade_slot), int(slot), int(bag)))
        log("sent CMSG_SET_TRADE_ITEM tradeSlot=%s bag=%s slot=%s" % (trade_slot, bag, slot))

    def trade_gold(self, amount):
        """CMSG_SET_TRADE_GOLD (TradeHandler.cpp:786-798): u64 gold."""
        self.send_packet(CMSG_SET_TRADE_GOLD, struct.pack("<Q", int(amount)))
        log("sent CMSG_SET_TRADE_GOLD amount=%d" % amount)

    def trade_accept(self):
        """CMSG_ACCEPT_TRADE (TradeHandler.cpp:333): empty body."""
        self.send_packet(CMSG_ACCEPT_TRADE, b"")
        log("sent CMSG_ACCEPT_TRADE")

    def trade_cancel(self):
        """CMSG_CANCEL_TRADE (TradeHandler.cpp:629, HandleCancelTradeOpcode): empty body. Used
        between S7 T17 sub-cases to guarantee a fresh trade session instead of reusing a stale one
        (a refused CheckTrade() never cancels the trade itself - #193)."""
        self.send_packet(CMSG_CANCEL_TRADE, b"")
        log("sent CMSG_CANCEL_TRADE")

    _TRADE_STATUS_NAMES = {
        0: "failed", 1: "target_stunned", 2: "initiated", 3: "currency_not_tradable",
        4: "player_not_found", 5: "too_far_away", 6: "accepted", 7: "dead",
        9: "state_changed", 10: "wrong_faction", 11: "already_trading",
        13: "restricted_account", 14: "complete", 15: "logging_out",
        16: "player_ignored", 17: "target_logging_out", 18: "petition",
        20: "stunned", 21: "player_busy", 22: "wrong_realm",
        23: "not_enough_currency", 24: "proposed", 27: "unaccepted",
        28: "target_dead", 30: "cancelled", 31: "not_on_taplist",
    }

    def _parse_trade_status(self, body):
        """SMSG_TRADE_STATUS (TradeHandler.cpp:35-75): 1 bit unk, 5 bits
        status, then a status-specific tail we don't need for the EVT name."""
        try:
            r = ByteReader(body)
            r.read_bit()
            status = r.read_bits(5)
            self.emit("trade", status=self._TRADE_STATUS_NAMES.get(status, str(status)))
        except (IndexError, struct.error) as exc:
            log("SMSG_TRADE_STATUS parse failed: %s" % exc)

    # --- S1b: loot roll SMSG parses ------------------------------------------
    def _parse_loot_start_roll(self, body):
        """SMSG_LOOT_START_ROLL (Group.cpp:848-895 BuildStartLootRollPacket)."""
        try:
            r = ByteReader(body)
            r.read_bits(3)                      # expect 7
            item_slot_zero = r.read_bit()
            r.read_bit()                        # always 0
            gm = [0] * 8
            for idx in (3, 1, 7, 6, 2, 4, 5, 0):
                gm[idx] = r.read_bit()
            r.read_bit()                        # !unk56, always 1
            r.read_bits(2)                       # expect 3
            g = [0] * 8
            g[7] = r.read_byte_seq(gm[7])
            r.read_i32()                          # itemRandomPropId
            g[5] = r.read_byte_seq(gm[5])
            r.read_u32()                          # mapid
            r.read_u32()                          # itemRandomSuffix
            item_slot = 0 if item_slot_zero else r.read_u8()
            g[4] = r.read_byte_seq(gm[4])
            g[0] = r.read_byte_seq(gm[0])
            g[3] = r.read_byte_seq(gm[3])
            g[2] = r.read_byte_seq(gm[2])
            r.read_u32()                          # unk counter
            item_id = r.read_u32()
            r.read_u8()                            # vote mask
            r.read_u32()                           # item count
            r.read_u8()                            # "I dunno" byte
            countdown = r.read_u32()
            g[6] = r.read_byte_seq(gm[6])
            r.read_u32()                            # display id
            g[1] = r.read_byte_seq(gm[1])
            loot_guid = int.from_bytes(bytes(g), "little")
            self.roll_guid = loot_guid
            self.roll_item_slot = item_slot
            self.roll_item_id = item_id
            self.emit("roll_start", item=item_id, slot=item_slot, guid=loot_guid, countdown=countdown)
        except (IndexError, struct.error) as exc:
            log("SMSG_LOOT_START_ROLL parse failed: %s" % exc)

    _ROLL_TYPE_NAMES = {0: "pass", 1: "need", 2: "greed", 3: "disenchant"}

    def _parse_loot_roll(self, body):
        """SMSG_LOOT_ROLL (Group.cpp:919-984 Group::SendLootRoll)."""
        try:
            r = ByteReader(body)
            lm = [0] * 8
            tm = [0] * 8
            lm[6] = r.read_bit(); tm[6] = r.read_bit(); lm[5] = r.read_bit(); lm[7] = r.read_bit()
            r.read_bit()                        # byte0x30==0, always true
            tm[3] = r.read_bit(); tm[7] = r.read_bit(); lm[4] = r.read_bit()
            item_slot_zero = r.read_bit()
            lm[0] = r.read_bit(); lm[2] = r.read_bit(); tm[1] = r.read_bit(); tm[0] = r.read_bit()
            r.read_bit()                        # 0x3C
            r.read_bits(2)                        # 0x34, expect 3
            lm[3] = r.read_bit()
            r.read_bit()                          # 0x64
            tm[5] = r.read_bit()
            r.read_bits(3)                        # 0x38, expect 3
            tm[4] = r.read_bit(); tm[2] = r.read_bit(); lm[1] = r.read_bit()

            lg = [0] * 8
            tg = [0] * 8
            lg[7] = r.read_byte_seq(lm[7]); tg[6] = r.read_byte_seq(tm[6])
            lg[0] = r.read_byte_seq(lm[0]); lg[5] = r.read_byte_seq(lm[5]); lg[3] = r.read_byte_seq(lm[3])
            item_slot = 0 if item_slot_zero else r.read_u8()
            r.read_u32()                          # display id
            r.read_i32()                           # itemRandomPropId
            r.read_u32()                           # itemRandomSuffix
            r.read_u32()                           # unk counter
            lg[1] = r.read_byte_seq(lm[1]); lg[4] = r.read_byte_seq(lm[4])
            tg[2] = r.read_byte_seq(tm[2]); lg[2] = r.read_byte_seq(lm[2])
            item_id = r.read_u32()
            tg[4] = r.read_byte_seq(tm[4])
            roll_number = r.read_u32()
            lg[6] = r.read_byte_seq(lm[6]); tg[1] = r.read_byte_seq(tm[1]); tg[0] = r.read_byte_seq(tm[0])
            roll_type = r.read_u8()
            tg[7] = r.read_byte_seq(tm[7])
            r.read_u32()                           # item count
            tg[5] = r.read_byte_seq(tm[5]); tg[3] = r.read_byte_seq(tm[3])

            looted_guid = int.from_bytes(bytes(lg), "little")
            target_guid = int.from_bytes(bytes(tg), "little")
            self.emit("roll", item=item_id, slot=item_slot, by=target_guid,
                      choice=self._ROLL_TYPE_NAMES.get(roll_type, roll_type),
                      number=roll_number, guid=looted_guid)
        except (IndexError, struct.error) as exc:
            log("SMSG_LOOT_ROLL parse failed: %s" % exc)

    def _parse_loot_roll_won(self, body):
        """SMSG_LOOT_ROLL_WON (Group.cpp:986-1045 Group::SendLootRollWon)."""
        try:
            r = ByteReader(body)
            tm = [0] * 8
            lm = [0] * 8
            tm[0] = r.read_bit(); lm[3] = r.read_bit()
            r.read_bits(2)                        # 0x30, expect 0
            tm[3] = r.read_bit(); tm[2] = r.read_bit(); tm[6] = r.read_bit(); tm[7] = r.read_bit()
            r.read_bit()                          # byte0x2C==0, always true
            lm[7] = r.read_bit(); lm[1] = r.read_bit(); tm[4] = r.read_bit(); tm[1] = r.read_bit()
            lm[4] = r.read_bit(); tm[5] = r.read_bit()
            r.read_bits(3)                        # 0x34, expect 0
            lm[0] = r.read_bit(); lm[2] = r.read_bit()
            item_slot_zero = r.read_bit()
            r.read_bit()                          # byte0x38!=0, always false
            lm[6] = r.read_bit(); lm[5] = r.read_bit()

            r.read_u32()                            # unk counter
            r.read_u32()                            # display id
            tg = [0] * 8
            lg = [0] * 8
            tg[2] = r.read_byte_seq(tm[2]); lg[7] = r.read_byte_seq(lm[7])
            r.read_i32()                             # itemRandomPropId
            tg[5] = r.read_byte_seq(tm[5]); lg[3] = r.read_byte_seq(lm[3])
            tg[7] = r.read_byte_seq(tm[7]); lg[1] = r.read_byte_seq(lm[1])
            lg[2] = r.read_byte_seq(lm[2]); lg[0] = r.read_byte_seq(lm[0])
            tg[3] = r.read_byte_seq(tm[3])
            roll_number = r.read_i32()
            r.read_u32()                             # itemRandomSuffix
            lg[6] = r.read_byte_seq(lm[6]); tg[1] = r.read_byte_seq(tm[1]); tg[4] = r.read_byte_seq(tm[4])
            roll_type = r.read_u8()
            tg[6] = r.read_byte_seq(tm[6])
            item_slot = 0 if item_slot_zero else r.read_u8()
            lg[4] = r.read_byte_seq(lm[4])
            item_id = r.read_u32()
            lg[5] = r.read_byte_seq(lm[5]); tg[0] = r.read_byte_seq(tm[0])
            r.read_u32()                             # item count

            target_guid = int.from_bytes(bytes(tg), "little")
            looted_guid = int.from_bytes(bytes(lg), "little")
            self.emit("roll_won", item=item_id, slot=item_slot, winner=target_guid,
                      choice=self._ROLL_TYPE_NAMES.get(roll_type, roll_type),
                      number=roll_number, guid=looted_guid)
        except (IndexError, struct.error) as exc:
            log("SMSG_LOOT_ROLL_WON parse failed: %s" % exc)

    # --- S1b: LFG / BG group queue -------------------------------------------
    def lfg_join(self, dungeon_id, roles=0, comment=""):
        """CMSG_LFG_JOIN (LFGHandler.cpp:26-101): u8 unk0, 3x u32 skip, u32
        roles, bits [numDungeons(22)=1, commentLen(8), unk1(1)], u32 dungeon
        (masked to 24 bits server-side), comment bytes."""
        body = struct.pack("<B", 0) + struct.pack("<III", 0, 0, 0) + struct.pack("<I", int(roles))
        cb = comment.encode("utf-8")
        bw = BitWriter()
        bw.bits(1, 22)
        bw.bits(len(cb), 8)
        bw.bit(0)
        body += bw.flush()
        body += struct.pack("<I", int(dungeon_id) & 0xFFFFFF)
        body += cb
        self.send_packet(CMSG_LFG_JOIN, body)
        log("sent CMSG_LFG_JOIN dungeon=%s roles=0x%X" % (dungeon_id, roles))
        self.emit("lfg", action="join", dungeon=dungeon_id, roles=roles)

    def lfg_set_roles(self, roles):
        """CMSG_LFG_SET_ROLES (LFGHandler.cpp:201-217): u32 roles, u8 skip."""
        self.send_packet(CMSG_LFG_SET_ROLES, struct.pack("<I", int(roles)) + b"\x00")
        log("sent CMSG_LFG_SET_ROLES roles=0x%X" % roles)
        self.emit("lfg", action="roles", roles=roles)

    def lfg_proposal_result(self, proposal_id, accept=True):
        """CMSG_LFG_PROPOSAL_RESULT (LFGHandler.cpp:147-198): u32 proposalID,
        u32 queueId, 2x u32 skip, then accept bit + 16 interleaved guid1/guid2
        mask bits, then 16 byteseq bytes. NOTE (lower confidence): the source
        only names these `guid1`/`guid2`; we send the player's own guid and
        group guid respectively, which is a best-effort guess at their
        semantics - flagged in the report for review."""
        guid1 = self.char_guid or 0
        guid2 = self.group_guid or 0
        g1 = guid1.to_bytes(8, "little")
        g2 = guid2.to_bytes(8, "little")
        body = struct.pack("<IIII", int(proposal_id), 0, 0, 0)
        bw = BitWriter()
        bw.bit(1 if accept else 0)
        mask_order = [(1, 6), (1, 0), (1, 2), (1, 4), (2, 6), (2, 7), (1, 3), (2, 4),
                      (1, 7), (2, 1), (1, 5), (2, 0), (1, 1), (2, 2), (2, 3), (2, 5)]
        for which, idx in mask_order:
            b = g1 if which == 1 else g2
            bw.bit(b[idx] != 0)
        body += bw.flush()
        byte_order = [(1, 3), (1, 6), (1, 4), (1, 1), (2, 7), (2, 0), (1, 7), (2, 6),
                      (1, 5), (2, 3), (2, 1), (2, 5), (2, 4), (1, 0), (1, 2), (2, 2)]
        for which, idx in byte_order:
            b = g1 if which == 1 else g2
            if b[idx]:
                body += bytes([b[idx] ^ 1])
        self.send_packet(CMSG_LFG_PROPOSAL_RESULT, body)
        log("sent CMSG_LFG_PROPOSAL_RESULT proposal=%s accept=%s" % (proposal_id, accept))
        self.emit("lfg", action="proposal_result", proposal=proposal_id, accept=int(accept))

    def lfg_teleport(self, direction="in"):
        """CMSG_LFG_TELEPORT (LFGHandler.cpp:229-236): a single bit (`out`)."""
        bw = BitWriter()
        bw.bit(1 if direction == "out" else 0)
        self.send_packet(CMSG_LFG_TELEPORT, bw.flush())
        log("sent CMSG_LFG_TELEPORT direction=%s" % direction)
        self.emit("lfg", action="teleport", direction=direction)

    # --- #210/#215 S10 P6: LFG queue-state parsers/verbs, ready-check confirm parse, and
    # bg-leave/bg-queue-leave. Self-contained block; leave_bg()/accept_bg() above are untouched.
    def lfg_leave(self, queue_id=None):
        """CMSG_LFG_LEAVE (LFGHandler.cpp:103-145): u32 8, u32 joinTime (unused, send 0), u32 3,
        u32 clientQueueId, then 8 mask bits + up to 8 conditional byteseq bytes for a guid the
        server only uses for a leader cheat-check (HandleLfgLeaveOpcode: "only leader can leave").
        `queue_id` defaults to the ClientQueueId last captured from SMSG_LFG_UPDATE_STATUS."""
        qid = queue_id if queue_id is not None else (self.lfg_client_queue_id or 0)
        body = struct.pack("<III", 8, 0, 3) + struct.pack("<I", int(qid))
        g = (self.char_guid or 0).to_bytes(8, "little")
        bw = BitWriter()
        for idx in (1, 6, 0, 7, 2, 4, 3, 5):
            bw.bit(g[idx])
        body += bw.flush()
        for idx in (4, 5, 2, 6, 1, 3, 7, 0):
            if g[idx]:
                body += bytes([g[idx] ^ 1])
        self.send_packet(CMSG_LFG_LEAVE, body)
        log("sent CMSG_LFG_LEAVE queueId=%s" % qid)
        self.emit("lfg", action="leave", queue_id=qid)

    def bg_leave(self):
        """CMSG_BATTLEFIELD_LEAVE (BattleGroundHandler.cpp:493-504): empty body - leaves the
        current match from inside (blocked only in combat before the match ends). Distinct from
        the existing leave_bg()/CMSG_BATTLEFIELD_PORT action=0, which drops a QUEUE slot - see
        bg_queue_leave() below."""
        self.send_packet(CMSG_BATTLEFIELD_LEAVE, b"")
        log("sent CMSG_BATTLEFIELD_LEAVE")
        self.emit("bg", action="leave")

    def bg_queue_leave(self, slot=None):
        """Drops this player's own battleground QUEUE slot before the pop - the existing
        leave_bg() (CMSG_BATTLEFIELD_PORT action=0) already does exactly this; only used at login
        until now. Named separately so the verb table matches the doc's "bg-leave (from inside)
        and bg-queue-leave" distinction 1:1."""
        self.leave_bg(slot=slot)

    def _parse_lfg_update_status(self, body):
        """SMSG_LFG_UPDATE_STATUS (LFGHandler.cpp:609-698, SendLfgUpdateStatus): 67 bits (comment
        length/flags/dungeon count, then the guid1-style presence flags for the player's own or
        original-group guid, interleaved with `join`/`queued`/a reserved 24-bit party-member
        field), then 3 conditional byteseq bytes, 3 raw skip bytes, 2 more conditional byteseq
        bytes, then plain u8 updateType / u32 Roles / u32 ClientQueueId. Only ClientQueueId is
        captured (for lfg_leave()); the bit order below mirrors the write order call-for-call."""
        try:
            r = ByteReader(body)
            r.read_bits(8)                  # comment length (comment is always empty server-side)
            r.read_bit()                    # always true
            join = r.read_bit()
            dungeon_count = r.read_bits(22)
            g = [0] * 8
            g[2] = r.read_bit()
            g[3] = r.read_bit()
            g[1] = r.read_bit()
            r.read_bit()                    # join, written a second time
            g[7] = r.read_bit()
            g[6] = r.read_bit()
            g[0] = r.read_bit()
            r.read_bit()                    # "not always true" per the server's own comment
            queued = r.read_bit()
            r.read_bits(24)                 # party members, always 0 on this core
            g[5] = r.read_bit()
            g[4] = r.read_bit()
            r.read_byte_seq(g[3])
            r.read_u8()
            r.read_u8()
            r.read_u8()
            r.read_byte_seq(g[4])
            r.read_byte_seq(g[6])
            update_type = r.read_u8()
            roles = r.read_u32()
            queue_id = r.read_u32()
            self.lfg_client_queue_id = queue_id
            if queued:
                self.events.append(("lfg_queued", queue_id))
            self.emit("lfg", action="status", type=update_type, join=int(join), queued=int(queued),
                       dungeons=dungeon_count, roles=roles, queue_id=queue_id)
        except (IndexError, struct.error) as exc:
            log("SMSG_LFG_UPDATE_STATUS parse failed: %s" % exc)

    def _parse_lfg_proposal_update(self, body):
        """SMSG_LFG_PROPOSAL_UPDATE (LFGHandler.cpp:1111-1201, SendLfgUpdateProposal): bit-packs
        guid1 (the original group, or the player) and guid2 (dungeonEntry|0x1F45<<48) around the
        fixed fields. Only proposal.id is captured - the exact same derivation as the server-side
        port (modules/mod_playerbots/src/strategy/actions/LfgActions.cpp, LfgAcceptAction::Execute),
        kept in sync with it."""
        try:
            r = ByteReader(body)
            g1 = [0] * 8
            g2 = [0] * 8
            g2[6] = r.read_bit()
            g2[0] = r.read_bit()
            g1[1] = r.read_bit()
            g1[7] = r.read_bit()
            g1[5] = r.read_bit()
            g2[5] = r.read_bit()
            g1[4] = r.read_bit()
            r.read_bit()                    # silent
            g2[2] = r.read_bit()
            g1[6] = r.read_bit()
            g2[3] = r.read_bit()
            g2[7] = r.read_bit()
            g1[3] = r.read_bit()
            player_count = r.read_bits(21)
            for _ in range(player_count):
                r.read_bit()                # is group in dungeon
                r.read_bit()                # is this player
                r.read_bit()                # answered
                r.read_bit()                # accepted
                r.read_bit()                # same group as the receiving player
            g1[2] = r.read_bit()
            g2[4] = r.read_bit()
            r.read_bit()                    # unk
            g1[0] = r.read_bit()
            g2[1] = r.read_bit()
            r.read_byte_seq(g2[1])
            r.read_byte_seq(g1[4])
            r.read_byte_seq(g2[4])
            r.read_byte_seq(g1[7])
            r.read_byte_seq(g1[2])
            r.read_byte_seq(g1[0])
            r.read_u32()                    # dungeonEntry
            state = r.read_u8()
            r.read_u32()                    # ClientQueueId
            r.read_byte_seq(g2[6])
            proposal_id = r.read_u32()
            self.lfg_proposal_id = proposal_id
            self.events.append(("lfg_proposal", proposal_id))
            self.emit("lfg", action="proposal", id=proposal_id, state=state, players=player_count)
        except (IndexError, struct.error) as exc:
            log("SMSG_LFG_PROPOSAL_UPDATE parse failed: %s" % exc)

    def _parse_lfg_role_check_update(self, body):
        """SMSG_LFG_ROLE_CHECK_UPDATE (LFGHandler.cpp:732-837): not decoded (same as AC - see this
        core's own bot-side port, strategy/actions/LfgActions.cpp, which never parses this packet's
        body either, only re-answers on every arrival). Emitted so a driver can gate on
        `EVT lfg rolecheck` without needing the field layout."""
        self.emit("lfg", action="rolecheck")

    def _parse_ready_check_confirm(self, body):
        """SMSG_RAID_READY_CHECK_CONFIRM (GroupHandler.cpp:1112-1158): 17 bits (16 guid-presence
        flags for groupGuid/playerGuid, with a `status` bit at position 4), FlushBits, then 16
        conditional byteseq bytes; the player guid is decoded and emitted as `player=` (#142 L7)."""
        try:
            r = ByteReader(body)
            order = ["g4", "p5", "p3", "status", "g2", "p6", "g3", "p0",
                     "p1", "g1", "g5", "p7", "p4", "g6", "p2", "g0", "g7"]
            status = 0
            mask = {}
            for name in order:
                v = r.read_bit()
                if name == "status":
                    status = v
                else:
                    mask[name] = v
            # #142 L7: decode the responder's guid too (byte order GroupHandler.cpp:1135-1150), so the
            # evaluator can count distinct responders; `status` alone is the 0/1 ready bit, identical
            # for every bot that answers "ready".
            pg = [0] * 8
            for name in ["p4", "p2", "p1", "g4", "g2", "p0", "g5", "g3",
                         "p7", "g6", "g1", "p6", "p3", "p5", "g0", "g7"]:
                v = r.read_byte_seq(mask[name])
                if name[0] == "p":
                    pg[int(name[1])] = v
            player = sum(b << (8 * i) for i, b in enumerate(pg))
            self.emit("readycheck", event="confirm", status=status, player=player)
        except (IndexError, struct.error) as exc:
            log("SMSG_RAID_READY_CHECK_CONFIRM parse failed: %s" % exc)

    def queue_bg_group(self, bg_type, join_as_group=False):
        body = struct.pack("<II", 0, 0)
        bw = BitWriter()
        g = bg_type.to_bytes(8, "little")
        bw.bit(g[1]); bw.bit(g[7]); bw.bit(g[0]); bw.bit(g[3])
        bw.bit(1 if join_as_group else 0)
        bw.bit(g[4]); bw.bit(1); bw.bit(g[6]); bw.bit(g[2]); bw.bit(g[5])
        body += bw.flush()
        for off in (7, 2, 4, 5, 0, 6, 3, 1):
            if g[off]:
                body += bytes([g[off] ^ 1])
        self.send_packet(CMSG_BATTLEMASTER_JOIN, body)
        log("bg-join type %d group=%s (%d bytes)" % (bg_type, join_as_group, len(body)))
        self.emit("bg", action="join", bg_type=bg_type, group=int(join_as_group))

    def queue_arena(self, slot):
        """#570: CMSG_BATTLEMASTER_JOIN_ARENA - the 5.4.8 rated arena join from the PvP UI.

        Body is a single uint8 slot (0 = 2v2, 1 = 3v3, 2 = 5v5; WorldSession::HandleBattlemasterJoinArena,
        BattleGroundHandler.cpp:571-669). Only the party leader may send it; the core checks faction,
        bracket and exact party size (Group::CanJoinBattlegroundQueue) and answers a refusal with
        SMSG_BATTLEFIELD_STATUS_FAILED (e.g. ERR_ARENA_TEAM_PARTY_SIZE for a wrong-size party)."""
        body = struct.pack("<B", slot)
        self.send_packet(CMSG_BATTLEMASTER_JOIN_ARENA, body)
        log("arena-join slot %d (%d bytes)" % (slot, len(body)))
        self.emit("bg", action="arena-join", slot=slot)

    # --- S1b: death / resurrect / summon --------------------------------------
    def set_selection(self, guid):
        """CMSG_SET_SELECTION (MiscHandler.cpp:721-744): bits [7..0] in
        descending order, byteseq [0,7,3,5,1,4,6,2]."""
        g = int(guid)
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (7, 6, 5, 4, 3, 2, 1, 0):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (0, 7, 3, 5, 1, 4, 6, 2))
        self.send_packet(CMSG_SET_SELECTION, body)
        log("sent CMSG_SET_SELECTION guid=%d" % g)

    def release_spirit(self):
        """CMSG_REPOP_REQUEST (MiscHandler.cpp:69-99): u8 skip."""
        self.send_packet(CMSG_REPOP_REQUEST, b"\x00")
        log("sent CMSG_REPOP_REQUEST")
        self.emit("release")

    def reclaim_corpse(self, guid=None):
        """CMSG_RECLAIM_CORPSE (MiscHandler.cpp:1035-1058): bits
        [1,5,7,2,6,3,0,4], byteseq [2,5,4,6,1,0,7,3]. The corpse guid must
        come from the harness `nearcorpse` verb - it can't be derived
        client-side (5.4.8 UPDATE_OBJECT parsing is impractical)."""
        g = int(guid if guid is not None else (self.harness_guid.get("corpse") or 0))
        if not g:
            log("reclaim: no corpse guid cached - run `nearcorpse` first")
            return
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (1, 5, 7, 2, 6, 3, 0, 4):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (2, 5, 4, 6, 1, 0, 7, 3))
        self.send_packet(CMSG_RECLAIM_CORPSE, body)
        log("sent CMSG_RECLAIM_CORPSE guid=%d" % g)
        self.emit("reclaim", guid=g)

    def res_accept(self, accept=True):
        """CMSG_RESURRECT_RESPONSE (MiscHandler.cpp:1091-...): u32 status
        (0 = accept, 1 = reject on MoP), bits [3,0,6,4,5,2,1,7], byteseq
        [7,0,1,3,4,6,2,5]. Needs a prior SMSG_RESURRECT_REQUEST."""
        if self.resurrect_request_guid is None:
            log("res-accept: no pending SMSG_RESURRECT_REQUEST")
            return
        g = self.resurrect_request_guid
        gb = g.to_bytes(8, "little")
        status = 0 if accept else 1
        bw = BitWriter()
        for idx in (3, 0, 6, 4, 5, 2, 1, 7):
            bw.bit(gb[idx] != 0)
        body = struct.pack("<I", status) + bw.flush() + guid_byte_seq(g, (7, 0, 1, 3, 4, 6, 2, 5))
        self.send_packet(CMSG_RESURRECT_RESPONSE, body)
        log("sent CMSG_RESURRECT_RESPONSE accept=%s (status=%d) guid=%d" % (accept, status, g))
        self.emit("res_response", accept=int(accept))
        self.resurrect_request_guid = None

    def summon_accept(self, accept=True):
        """CMSG_SUMMON_RESPONSE (MovementPackets.cpp:376-396): bits
        [1,3,5,2,Accept,7,0,4,6], byteseq [0,1,6,3,5,4,2,7]. Needs a prior
        SMSG_SUMMON_REQUEST."""
        if self.summon_request_guid is None:
            log("summon-accept: no pending SMSG_SUMMON_REQUEST")
            return
        g = self.summon_request_guid
        gb = g.to_bytes(8, "little")
        bw = BitWriter()
        for idx in (1, 3, 5, 2):
            bw.bit(gb[idx] != 0)
        bw.bit(1 if accept else 0)
        for idx in (7, 0, 4, 6):
            bw.bit(gb[idx] != 0)
        body = bw.flush() + guid_byte_seq(g, (0, 1, 6, 3, 5, 4, 2, 7))
        self.send_packet(CMSG_SUMMON_RESPONSE, body)
        log("sent CMSG_SUMMON_RESPONSE accept=%s guid=%d" % (accept, g))
        self.emit("summon_response", accept=int(accept))
        self.summon_request_guid = None

    def _parse_resurrect_request(self, body):
        """SMSG_RESURRECT_REQUEST (Spell.cpp:5774-5814)."""
        try:
            r = ByteReader(body)
            r.read_u32()                          # sentName.size()+1
            spell_id = r.read_u32()
            r.read_u32()                           # unk 0
            gm = [0] * 8
            gm[3] = r.read_bit()
            r.read_bit()                            # isCreature
            r.read_bit()                            # false, unused
            gm[1] = r.read_bit(); gm[5] = r.read_bit(); gm[2] = r.read_bit(); gm[6] = r.read_bit()
            gm[0] = r.read_bit(); gm[4] = r.read_bit(); gm[7] = r.read_bit()
            name_len = r.read_bits(6)
            g = [0] * 8
            g[7] = r.read_byte_seq(gm[7]); g[3] = r.read_byte_seq(gm[3]); g[5] = r.read_byte_seq(gm[5])
            r.read_string(name_len)
            g[2] = r.read_byte_seq(gm[2]); g[4] = r.read_byte_seq(gm[4]); g[1] = r.read_byte_seq(gm[1])
            g[6] = r.read_byte_seq(gm[6]); g[0] = r.read_byte_seq(gm[0])
            guid = int.from_bytes(bytes(g), "little")
            self.resurrect_request_guid = guid
            self.emit("res_request", frm=guid, spell=spell_id)
        except (IndexError, struct.error) as exc:
            log("SMSG_RESURRECT_REQUEST parse failed: %s" % exc)

    def _parse_summon_request(self, body):
        """SMSG_SUMMON_REQUEST (SpellEffects.cpp:4817-4839)."""
        try:
            r = ByteReader(body)
            gm = [0] * 8
            for idx in (0, 6, 3, 2, 1, 4, 7, 5):
                gm[idx] = r.read_bit()
            g = [0] * 8
            g[4] = r.read_byte_seq(gm[4])
            r.read_u32()                            # zone
            g[7] = r.read_byte_seq(gm[7]); g[3] = r.read_byte_seq(gm[3]); g[1] = r.read_byte_seq(gm[1])
            r.read_u32()                             # delay
            g[2] = r.read_byte_seq(gm[2]); g[6] = r.read_byte_seq(gm[6])
            g[5] = r.read_byte_seq(gm[5]); g[0] = r.read_byte_seq(gm[0])
            guid = int.from_bytes(bytes(g), "little")
            self.summon_request_guid = guid
            self.emit("summon_request", frm=guid)
        except (IndexError, struct.error) as exc:
            log("SMSG_SUMMON_REQUEST parse failed: %s" % exc)


# ---------------------------------------------------------------------------
# S1a: in-process SOAP helper (worldserver console over HTTP). MoP's TCSoap
# (src/server/worldserver/TCSoap/TCSoap.cpp:150-155) uses the "urn:TC"
# namespace and authenticates soap->userid/passwd against a SEC_ADMINISTRATOR
# game account, same envelope shape as AzerothCore's ACSoap.
def _xml_escape(text):
    return (text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                .replace('"', "&quot;"))


# #8 (wave27): every blocking SOAP call used to leave the world socket unread for its whole duration (up to
# `timeout` s per step, several steps back to back under load). The call now runs on a worker thread while the
# main thread keeps pumping the session (answering the time-sync request and everything else), so
# no step starves the time-sync queue. _PUMP_CLIENT is the live WorldClient (set once it is logged in).
_PUMP_CLIENT = None


def call_pumped(w, fn, *a, **kw):
    if w is None:
        return fn(*a, **kw)
    box = {}

    def run():
        try:
            box["r"] = fn(*a, **kw)
        except BaseException as exc:   # re-raised on the caller's thread below
            box["e"] = exc

    t = threading.Thread(target=run, daemon=True)
    t.start()
    while t.is_alive():
        w.pump(0.25)
    t.join()
    if "e" in box:
        raise box["e"]
    return box["r"]


def soap_command(host, port, user, password, command, timeout=15.0):
    return call_pumped(_PUMP_CLIENT, _soap_command_raw, host, port, user, password, command, timeout)


def _soap_command_raw(host, port, user, password, command, timeout=15.0):
    if not user or password is None:
        raise RuntimeError("SOAP credentials not set (--soap-user/--soap-pass or MOP_SOAP_USER/MOP_SOAP_PASSWORD)")
    body = (
        '<?xml version="1.0" encoding="utf-8"?>'
        '<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"'
        ' xmlns:ns1="urn:TC">'
        '<SOAP-ENV:Body><ns1:executeCommand><command>%s</command></ns1:executeCommand>'
        '</SOAP-ENV:Body></SOAP-ENV:Envelope>'
    ) % _xml_escape(command)
    auth = base64.b64encode(("%s:%s" % (user, password)).encode()).decode()
    conn = http.client.HTTPConnection(host, port, timeout=timeout)
    try:
        conn.request("POST", "/", body, {
            "Content-Type": "text/xml; charset=utf-8",
            "SOAPAction": '"urn:TC#executeCommand"',
            "Authorization": "Basic " + auth,
        })
        resp = conn.getresponse()
        payload = resp.read().decode(errors="replace")
        return resp.status, payload
    finally:
        conn.close()


def run_soap(args, command):
    log("SOAP %s:%d -> %r" % (args.soap_host, args.soap_port, command))
    try:
        status, payload = soap_command(args.soap_host, args.soap_port,
                                        args.soap_user, args.soap_pass, command)
    except (OSError, RuntimeError) as exc:
        log("SOAP helper failed: %s (check SOAP.Enabled/IP/Port in worldserver.conf)" % exc)
        return False
    text = payload.strip()
    if len(text) > 500:
        text = text[:500] + "..."
    log("SOAP HTTP %d: %s" % (status, text))
    return status == 200


# #209 round-6 rule (review of S9 round 5): "the driver gates
# every step ... on the SERVER-side master position from the probe log, never on the client's
# self.pos", because both S9 round-5 "FAILs" (MoP elevator, AC boat) turned out to be cases where
# the server never actually accepted this client's movement at all - the client's own EVT/self.pos
# trail proved nothing. `.botscenario <name> masterprobe on` (BotScenario.cpp) makes the server log
# one `[PROBE] <name> ... map=.. x=.. y=.. z=.. ...` line per grouped-bot AI tick (piggy-backed,
# since a real player has no AI tick of its own - see BotScenario.cpp's OnUpdate); this reads the
# latest one for `name` straight out of Server.log, exactly as the bash drivers already do for a
# bot's own probe (`tail -c ... | grep '[PROBE] $BOT '`).
_PROBE_POS_RE = re.compile(r"map=(\d+).*?\bx=(-?[\d.]+)\s+y=(-?[\d.]+)\s+z=(-?[\d.]+)")


def read_last_probe_pos(log_path, name):
    """Returns (map_id, x, y, z) from the last "[PROBE] <name> ..." line in `log_path`, or None
    if the file is missing/unreadable or has no such line yet. Only tails the last 400000 bytes
    (these Server.logs run into the hundreds of MB and open in overwrite mode across restarts;
    400000B is the same window the bash drivers already use and comfortably covers several
    minutes of probe lines at the default ~1s interval)."""
    try:
        size = os.path.getsize(log_path)
        with open(log_path, "rb") as f:
            if size > 400000:
                f.seek(size - 400000)
            data = f.read().decode("utf-8", "replace")
    except OSError:
        return None

    prefix = "[PROBE] " + name + " "
    last_line = None
    for line in data.splitlines():
        idx = line.find(prefix)
        if idx != -1:
            last_line = line[idx:]
    if last_line is None:
        return None

    m = _PROBE_POS_RE.search(last_line)
    if not m:
        return None
    return int(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4))


def wait_for_server_pos(log_path, name, tx, ty, tz, tol, timeout, poll=1.0, w=None):
    """Polls `log_path` for `name`'s server-side [PROBE] position until it is within `tol` yards
    (3D) of (tx,ty,tz), or `timeout` seconds elapse. Returns (ok, last_map, lx, ly, lz, dist) -
    `ok` is False, with the best position seen (or all-None if none was ever read), on timeout.
    #8: with `w` the poll interval is spent in w.pump(), so SMSG_TIME_SYNC_REQUEST keeps being
    answered through the wait (a bare time.sleep left the session silent for up to `timeout`
    seconds and the server logged 'Not received CMSG_TIME_SYNC_RESP ... possible cheater')."""
    deadline = time.time() + timeout
    last = None
    while True:
        got = read_last_probe_pos(log_path, name)
        if got is not None:
            last = got
            _, lx, ly, lz = got
            dist = math.sqrt((lx - tx) ** 2 + (ly - ty) ** 2 + (lz - tz) ** 2)
            if dist <= tol:
                return True, last[0], lx, ly, lz, dist
        if time.time() >= deadline:
            if last is None:
                return False, None, None, None, None, None
            _, lx, ly, lz = last
            dist = math.sqrt((lx - tx) ** 2 + (ly - ty) ** 2 + (lz - tz) ** 2)
            return False, last[0], lx, ly, lz, dist
        if w is not None:
            w.pump(poll)
        else:
            time.sleep(poll)


# --- S1a: common --script/--step verb dispatcher (identical verb set and
# EVT output on both headless clients). Movement/quest/loot/trade/LFG/BG/
# death verbs are S1b (issue #165 scopes only the verbs below into S1a).
def run_verb(w, args_cli, line):
    parts = line.strip().split()
    if not parts:
        return
    verb, rest = parts[0].lower(), parts[1:]
    if verb in PLAY_VERBS and run_play_verb(w, args_cli, verb, rest):
        return

    def resolve(arg):
        g = w.resolve_target(arg)
        if g is None:
            log("verb %r: cannot resolve target %r" % (verb, arg))
        return g

    if verb == "wait-for":
        # wait-for group members=N | map=<id> | evt=<kind> [timeout=S]
        # #210/#215 S10 P6: map= and evt= added (previously members= only) - evt= matches any
        # w.events entry (kind, data) appended to w.events after this wait-for call started (so a
        # script can call wait-for evt=X more than once across different phases without the first
        # occurrence satisfying a later wait). See _parse_lfg_proposal_update/_parse_lfg_update_status
        # for the "lfg_proposal"/"lfg_queued" kinds this can gate on.
        want = {}
        timeout = 30.0
        for tok in rest:
            if "=" in tok:
                k, v = tok.split("=", 1)
                if k == "timeout":
                    timeout = float(v)
                else:
                    want[k] = v
        start_events = len(w.events)
        deadline = time.time() + timeout
        while time.time() < deadline:
            ok = True
            if "members" in want and len(w.group_members) < int(want["members"]):  # MoP group list already includes the leader
                ok = False
            if "map" in want and w.map_id != int(want["map"]):
                ok = False
            if "evt" in want:
                # #210 S10 round 3 (B3 fix): comma-separated OR-match. Under `bgtest` 1v0, "queued"
                # and "confirm" can land in the very same tick (same ms timestamp observed in
                # practice) - a script step that only waits for "queued" can miss it and hang for the
                # full timeout, since by the time this wait-for call captured its baseline the real
                # status had already moved on to "confirm". `evt=bg_queued,bg_need_confirmation`
                # accepts whichever one is actually still there to see.
                wanted_kinds = want["evt"].split(",")
                if not any(kind in wanted_kinds for kind, _data in w.events[start_events:]):
                    ok = False
            if ok:
                return
            w.pump(1)
        log("wait-for %s timed out after %.0fs" % (want, timeout))
    elif verb == "wait-for-server-pos":
        # #209 round-6 rule: wait-for-server-pos <name> <x>,<y>,<z> [tol=10] [timeout=60]
        # Gates on the SERVER's own [PROBE] position for `name` (see wait_for_server_pos's
        # docstring above) instead of this client's self.pos. Fails fast (raises SystemExit,
        # ending the whole run) rather than silently proceeding to board/ride/walk-off steps
        # whose result would be meaningless if the server never actually saw `name` move there -
        # exactly what made S9 round 5's MoP-elevator and AC-boat "FAILs" master-position
        # artefacts instead of real bot-behaviour evidence.
        name = rest[0]
        tx, ty, tz = (float(v) for v in rest[1].split(","))
        tol = 10.0
        timeout = 60.0
        for tok in rest[2:]:
            if "=" in tok:
                k, v = tok.split("=", 1)
                if k == "tol":
                    tol = float(v)
                elif k == "timeout":
                    timeout = float(v)
        ok, smap, sx, sy, sz, dist = wait_for_server_pos(
            args_cli.server_log, name, tx, ty, tz, tol, timeout, w=w)
        if not ok:
            if sx is None:
                log("FAIL wait-for-server-pos: no [PROBE] %s line ever seen in %s within %.0fs "
                    "(is masterprobe on? is a grouped bot's AI actually ticking to piggy-back it?)"
                    % (name, args_cli.server_log, timeout))
            else:
                log("FAIL wait-for-server-pos: %s server-side pos (map=%s %.1f,%.1f,%.1f) never "
                    "within %.1fyd of target (%.1f,%.1f,%.1f) in %.0fs (last dist=%.1f) - the "
                    "server never accepted this client's movement; do not judge bot behaviour "
                    "from the rest of this run"
                    % (name, smap, sx, sy, sz, tol, tx, ty, tz, timeout, dist))
            raise SystemExit(3)
        log("wait-for-server-pos OK: %s server-side pos (map=%s %.1f,%.1f,%.1f) is within %.1fyd "
            "of target (%.1f,%.1f,%.1f)" % (name, smap, sx, sy, sz, tol, tx, ty, tz))
    elif verb == "wait-file":
        # wait-file <path> [timeout=S] (#206/#209 Phase 1): pump the session until a driver-side flag
        # file exists. Lets a driver sync a step to server state it polls itself (an elevator at its
        # landing, a zeppelin docked at the far stop) without the client logging out between phases.
        path = rest[0]
        timeout = 120.0
        for tok in rest[1:]:
            if tok.startswith("timeout="):
                timeout = float(tok.split("=", 1)[1])
        deadline = time.time() + timeout
        while time.time() < deadline and not os.path.exists(path):
            w.pump(0.2)
        log("wait-file %s %s" % (path, "seen" if os.path.exists(path) else "TIMED OUT after %.0fs" % timeout))
    elif verb == "soap":
        run_soap(args_cli, " ".join(rest))
    elif verb == "select":
        pass  # character selection happens before the script runs
    elif verb == "target":
        # target <guid>: CMSG_SET_SELECTION (S9/#206 T8 tooling gap - set_selection()
        # already existed below but no verb ever called it, so a real player could
        # never target anything, which EnterVehicleAction::Execute requires via
        # master->GetTarget()). <guid> is normally the raw guid text out of the
        # `npcguid` harness verb's SOAP reply ("guid=<N>", BotScenario.cpp's
        # non-bot npcguid case) - already a full HighGuid-typed value, so
        # resolve_target() passes it through unchanged (it is > 0xFFFFFFFF).
        g = w.obj_guid(rest[0]) if rest[0].lower() in ("npc", "go", "corpse") else resolve(rest[0])
        if g:
            w.set_selection(g)
    elif verb == "invite":
        w.invite_to_group(rest[0])
    elif verb == "accept":
        w.group_accept()
    elif verb in ("leave", "disband"):
        w.group_disband()
    elif verb == "uninvite":
        g = resolve(rest[0])
        if g:
            w.group_uninvite(g, rest[1] if len(rest) > 1 else "")
    elif verb == "leader":
        g = resolve(rest[0])
        if g:
            w.group_set_leader(g)
    elif verb == "loot-method":
        method = {"freeforall": 0, "roundrobin": 1, "master": 2, "group": 3, "needbeforegreed": 4}.get(
            rest[0].lower(), 3)
        master = resolve(rest[1]) if len(rest) > 1 else 0
        w.group_loot_method(method, master or 0)
    elif verb in ("difficulty", "dungeon-difficulty"):
        # difficulty <normal|heroic|1|2> [timeout=S] (#373): the group leader's dungeon-difficulty selector, sent like a real
        # client (CMSG_SET_DUNGEON_DIFFICULTY). Run it AFTER the group is formed (wait-for group members=N) and BEFORE the
        # first teleport into the dungeon; the server copies it to every bot in the group (Group::SetDungeonDifficulty).
        # Result line: "DUNGEON-DIFFICULTY OK mode=<n>" or "DUNGEON-DIFFICULTY FAIL mode=<n> ...".
        want = rest[0].lower() if rest else "heroic"
        mode = {"normal": 1, "n": 1, "heroic": 2, "h": 2}.get(want)
        if mode is None:
            mode = int(want)
        tmo = 15.0
        for tok in rest[1:]:
            if tok.startswith("timeout="):
                tmo = float(tok.split("=", 1)[1])
        w.set_dungeon_difficulty(mode, tmo)
    elif verb == "raid":
        w.group_raid_convert(rest[0].lower() != "group" if rest else True)
    elif verb == "icon":
        symbol = 0xFF if rest[0] in ("clear", "list") else int(rest[0])
        target = resolve(rest[1]) if len(rest) > 1 else None
        w.group_raid_icon(symbol, target)
    elif verb == "ready-check":
        w.ready_check()
    elif verb == "ready":
        w.ready_check_confirm(rest[0].lower() not in ("no", "decline", "0"))
    elif verb in ("say", "party", "raid-chat", "whisper"):
        chat_type = "raid" if verb == "raid-chat" else verb
        target = rest[0] if chat_type == "whisper" else None
        msg = " ".join(rest[1:]) if chat_type == "whisper" else " ".join(rest)
        w.send_chat(chat_type, msg, target=target)
    elif verb == "guild-chat":
        w.send_chat("guild", " ".join(rest))
    elif verb == "join-channel":
        # join-channel NAME [ID]   (ID = ChatChannels.dbc id for a constant channel; 0/omitted = custom channel)
        w.join_channel(rest[0], int(rest[1]) if len(rest) > 1 else 0)
    elif verb == "channel-say":
        # channel-say NAME text...
        w.send_channel_chat(rest[0], " ".join(rest[1:]))
    elif verb == "guild-accept":
        # accept the pending guild invite (SMSG_GUILD_INVITE is logged as "[GUILD] SMSG_GUILD_INVITE from=.. guild=..")
        w.guild_accept()
    elif verb == "create":
        # create <name> <race> <class> [gender]
        w.char_create(rest[0], int(rest[1]), int(rest[2]),
                       int(rest[3]) if len(rest) > 3 else 0)
    # --- S1b (issue #172) ---------------------------------------------------
    elif verb == "setpos":
        # S9 (#206) elevator tooling gap: neither SMSG_MOVE_TELEPORT (near teleport, the
        # board_transport()/on_transport docstrings already flagged this) nor SMSG_NEW_WORLD
        # (cross-map worldport - checked freshly this round: the handler only reads map_id out
        # of the packet, body[4:8], and never touches self.pos at all) update this client's own
        # tracked position after a SOAP `.botscenario <name> teleport <map> x y z` - so a `walk`
        # right after one computes its heading from a stale pre-teleport self.pos and can head
        # off in the wrong direction entirely (confirmed live: a same-map .botscenario teleport
        # sent the client walking ~4000yd across the continent instead of the intended ~short
        # hop). `setpos <x>,<y>,<z>` sets self.pos directly from values the driver already knows
        # authoritatively (it just issued that exact SOAP teleport) - the same "trust the SOAP
        # reply, not client-side tracking" pattern `board`'s reply text already uses for the
        # masterprobe gap. No packet is sent; this only corrects local bookkeeping.
        x, y, z = (float(v) for v in rest[0].split(","))
        w.pos = (x, y, z)
        log("setpos -> local pos set to (%.2f, %.2f, %.2f)" % (x, y, z))
    elif verb == "goto":
        x, y, z = (float(v) for v in rest[0].split(","))
        w.move_goto(x, y, z)
    elif verb == "walk":
        # walk [speed=<yd/s>] x,y,z [x,y,z ...]  (speed= is optional, default 7.0; a driven vehicle can be faster)
        kvw = dict(t.split("=", 1) for t in rest if "=" in t)
        waypoints = [tuple(float(v) for v in tok.split(",")) for tok in rest if "=" not in tok]
        w.move_walk(waypoints, speed=float(kvw.get("speed", 7.0)))
    elif verb == "mover":
        # mover <npc|rawguid|off>: from now on move_stop/face/goto/walk send their MSG_MOVE_* for that guid instead
        # of the character - what a real client does while it drives a vehicle (the seated driver's m_mover is the
        # vehicle). `npc` = the guid cached by the last `npcguid <entry>` (the Hot Rod 34840 is a fresh spawn per use).
        if not rest or rest[0].lower() in ("off", "0", "none"):
            w.move_guid = None
            log("mover: off (movement packets carry the character guid)")
        else:
            g = w.obj_guid(rest[0])
            if not g:
                log("mover: need <npc|rawguid|off> (run `npcguid <entry>` first)")
                return
            w.move_guid = g
            log("mover: movement packets now carry guid %d" % g)
    elif verb == "face":
        # face <orientation> | face (no argument: toward the `npcguid` creature, if this client can see it)
        if rest:
            w.move_face(float(rest[0]))
        else:
            o = w.play.objs.get(int(w.harness_guid.get("npc", 0)))
            if o is not None and o.position():
                w.play.face(o.position())
            else:
                log("face: no orientation given and no visible `npcguid` creature")
    elif verb == "stop":
        w.move_stop()
    elif verb == "mount":
        w.mount(int(rest[0]))
    elif verb == "taxi":
        w.taxi(rest[0], rest[1], int(rest[2]) if len(rest) > 2 else None)
    elif verb == "quest-accept":
        w.quest_accept(int(rest[0]), int(rest[1]) if len(rest) > 1 else None)
    elif verb == "quest-share":
        w.quest_share(int(rest[0]))
    elif verb == "quest-complete":
        w.quest_complete(int(rest[0]), int(rest[1]) if len(rest) > 1 else None)
    elif verb == "quest-reward":
        w.quest_reward(int(rest[0]), int(rest[1]), int(rest[2]) if len(rest) > 2 else None)
    elif verb == "loot":
        w.loot_open(resolve(rest[0]) if rest else None)
    elif verb == "loot-all":
        # loot-all [waitSeconds] - take every item + the money of each pending SMSG_LOOT_RESPONSE window, then release
        w.loot_all(float(rest[0]) if rest else 3.0)
    elif verb == "release":
        # loot release when a guid is given or a loot window is open; spirit
        # release (CMSG_REPOP_REQUEST) otherwise - the spec reuses one verb
        # name for both (see the Loot and Death rows of section 5.3).
        if rest:
            w.loot_release(resolve(rest[0]))
        elif w.loot_source_guid:
            w.loot_release()
        else:
            w.release_spirit()
    elif verb == "roll":
        w.loot_roll(rest[0])
    elif verb == "master-give":
        target = resolve(rest[1])
        if target:
            w.loot_master_give(int(rest[0]), target)
    elif verb == "trade-open":
        target = resolve(rest[0])
        if target:
            w.trade_open(target)
    elif verb == "trade-item":
        w.trade_item(int(rest[0]), int(rest[1]), int(rest[2]))
    elif verb == "trade-gold":
        w.trade_gold(int(rest[0]))
    elif verb == "trade-accept":
        w.trade_accept()
    elif verb == "trade-cancel":
        w.trade_cancel()
    elif verb == "lfg-join":
        roles = 0
        dungeon = None
        for tok in rest:
            if tok.startswith("roles="):
                for name in tok.split("=", 1)[1].split(","):
                    roles |= {"leader": 1, "tank": 2, "healer": 4, "heal": 4, "dps": 8, "damage": 8}.get(name, 0)
            else:
                dungeon = tok
        w.lfg_join(dungeon, roles=roles)
    elif verb == "lfg-accept":
        # #210/#215 S10 P6: "lfg-accept auto" uses the id SMSG_LFG_PROPOSAL_UPDATE's parser
        # (_parse_lfg_proposal_update) already captured, instead of needing it on the command
        # line (previously the only option, and unusable in practice - nothing parsed the id).
        if rest and rest[0].lower() == "auto":
            if w.lfg_proposal_id is None:
                log("lfg-accept auto: no SMSG_LFG_PROPOSAL_UPDATE seen yet")
            else:
                w.lfg_proposal_result(w.lfg_proposal_id,
                                       accept=(len(rest) < 2 or rest[1].lower() not in ("no", "decline", "0")))
        else:
            w.lfg_proposal_result(int(rest[0]), accept=(len(rest) < 2 or rest[1].lower() not in ("no", "decline", "0")))
    elif verb == "lfg-roles":
        roles = 0
        for name in rest[0].split(","):
            roles |= {"leader": 1, "tank": 2, "healer": 4, "heal": 4, "dps": 8, "damage": 8}.get(name, 0)
        w.lfg_set_roles(roles)
    elif verb == "lfg-teleport":
        w.lfg_teleport(rest[0] if rest else "in")
    elif verb == "lfg-leave":
        w.lfg_leave(int(rest[0]) if rest else None)
    elif verb == "bg-join":
        bg_type = int(rest[0]) if rest else SOTA_BG_TYPE
        group = len(rest) > 1 and rest[1].lower() == "group"
        w.queue_bg_group(bg_type, join_as_group=group)
    elif verb == "arena-join":
        # arena-join <slot 0|1|2> [rated] [bmguid]: #570. MoP takes the slot only (every 5.4.8 arena queue is
        # rated, no battlemaster); the extra args are accepted and ignored so one driver line runs on both cores.
        w.queue_arena(int(rest[0]) if rest else 1)
    elif verb == "bg-accept":
        w.accept_bg()
    elif verb == "bg-leave":
        w.bg_leave()
    elif verb == "bg-queue-leave":
        w.bg_queue_leave(int(rest[0]) if rest else None)
    elif verb == "die":
        run_soap(args_cli, ".botscenario %s kill" % (w.char_name or ""))
    elif verb == "reclaim":
        w.reclaim_corpse()
    elif verb == "res-accept":
        w.res_accept(accept=(not rest or rest[0].lower() not in ("no", "decline", "0")))
    elif verb == "summon-accept":
        w.summon_accept(accept=(not rest or rest[0].lower() not in ("no", "decline", "0")))
    # --- S2 (issue #191) -----------------------------------------------------
    elif verb == "areatrigger":
        w.areatrigger(int(rest[0]), entered=int(rest[1]) if len(rest) > 1 else 1)
    elif verb == "board-walk":
        # board-walk <transportEntry> [seat]: harness-boards the player onto
        # the docked transport (SOAP `board`, same as the plain `board` verb),
        # then switches this client's own movement packets to
        # transport-relative so the server doesn't immediately knock it back
        # off (see WorldClient.board_transport's docstring for why).
        entry = int(rest[0])
        seat = int(rest[1]) if len(rest) > 1 else -1
        cmd = ".botscenario %s board %s" % (w.char_name or "", entry)
        log("SOAP %s:%d -> %r" % (args_cli.soap_host, args_cli.soap_port, cmd))
        try:
            status, payload = soap_command(args_cli.soap_host, args_cli.soap_port,
                                            args_cli.soap_user, args_cli.soap_pass, cmd)
        except OSError as exc:
            log("SOAP helper failed: %s" % exc)
            return
        log("SOAP HTTP %d: %s" % (status, payload.strip()[:300]))
        m = re.search(r"guid=(\d+)", payload)
        if not m:
            log("board-walk: no transport guid in the SOAP reply, aborting")
            return
        w.board_transport(int(m.group(1)), seat=seat)
    elif verb == "board-natural":
        # S9 owner requirement (#206): board-natural <guid> <lx> <ly> <lz> [seat] - the "walk-on"
        # acceptance path. Caller must already be standing (via a real move_walk/move_goto) at
        # the transport's live world position for this exact local offset; see
        # board_natural()'s docstring for where that guid/offset/position come from (a separate
        # scout character's `.botscenario <scout> transport ...` [TRANSPORT] placed line - never
        # `board`/`teleport` on this character).
        guid = int(rest[0])
        lx, ly, lz = (float(v) for v in rest[1:4])
        seat = int(rest[4]) if len(rest) > 4 else -1
        w.board_natural(guid, lx, ly, lz, seat=seat)
    elif verb == "leave-transport":
        w.leave_transport_natural()
    elif verb == "duel":
        # duel <name|guid>: only usable under --cmd play (--duel-guid remains
        # the botchecks-only flag). resolve() returns a full raw player guid;
        # duel() takes the bare DB counter and re-derives the raw guid itself.
        target = resolve(rest[0])
        if target:
            w.duel(target & 0xFFFFFFFFFFFF)
    elif verb == "duel-accept":
        # duel-accept [decline]: answer the last SMSG_DUEL_REQUESTED (bot -> player challenge, #201 T4); wave 29.
        # HandleDuelResponseOpcode ignores the guid, the challenger's counter is sent as the client does.
        if w.duel_request is None:
            log("duel-accept: no SMSG_DUEL_REQUESTED seen yet")
        else:
            w.duel_response(w.duel_request & 0xFFFFFFFFFFFF, accepted=not (rest and rest[0].lower() == "decline"))
    elif verb == "use-item-on":
        # use-item-on <bag> <slot> <targetBag> <targetSlot> guid=<N> targetguid=<N>
        # the two real ObjectGuids can't be derived from bag/slot alone - see
        # use_item_on()'s docstring - so they're passed explicitly here.
        pos_args = [t for t in rest if "=" not in t]
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        bag, slot, tbag, tslot = (int(v) for v in pos_args[:4])
        item_guid = int(kv.get("guid", 0))
        target_guid = int(kv.get("targetguid", 0))
        if not item_guid or not target_guid:
            log("use-item-on: need guid=<item guid> and targetguid=<target item guid> "
                "(bag/slot alone can't be resolved to a guid client-side)")
            return
        w.use_item_on(bag, slot, item_guid, tbag, tslot, target_guid)
    elif verb == "cast":
        # cast <spellId> [targetGuid] - a real targeted CMSG_CAST_SPELL. With no target this is
        # the self-cast form (same packet as `mount`). The target is normally the raw guid text
        # out of the `npcguid` harness verb's SOAP reply ("guid=<N>"), already a full HighGuid-typed
        # value, so resolve_target() passes it through unchanged (> 0xFFFFFFFF).
        kvc = dict(t.split("=", 1) for t in rest if "=" in t)      # autoloot=1: loot-all after the cast
        rest = [t for t in rest if "=" not in t]
        if "item" in kvc:
            # cast SPELL item=mainhand|offhand|ENTRY: an item-targeted cast (runeforging a rune onto the weapon)
            me_ = w.play.me()
            ig = 0
            if me_ is not None:
                slot_ = {"mainhand": 15, "offhand": 16}.get(kvc["item"].lower())
                if slot_ is not None:
                    ig = me_.u64(UF_PLAYER_INV_SLOTS + 2 * slot_)
                else:
                    ig = next((g_ for _b, _s, g_, e_, _c in w.play.inventory() if e_ == int(kvc["item"])), 0)
            if not ig:
                log("cast %s: no item %s" % (rest[0], kvc["item"]))
                return
            w.cast(int(rest[0]), None, item_guid=ig)
            return
        if len(rest) > 1:
            key = rest[1].lower()
            if key == "pet":
                # the character's own summoned pet (UNIT_FIELD_SUMMON), e.g. `cast 2641 pet` (Dismiss Pet)
                me_ = w.play.me()
                g = me_.u64(UF_UNIT_SUMMON) if me_ else 0
                if not g:
                    log("cast %s: no pet out" % rest[0])
                    return
            elif key in ("npc", "corpse", "go"):
                # the guid cached by the `npcguid` / `nearcorpse` harness verb - the Hot Rod's
                # guid is generated fresh per spawn, so it cannot be hardcoded in a --step list
                # (same idiom taxi() uses, see its docstring).
                g = w.harness_guid.get(key)   # "go" = cached by `goguid <entry>` (#708)
                if not g:
                    log("cast %s: no cached %s guid - run `%s <entry>` first" % (rest[0], key, key))
                    return
            else:
                g = resolve(rest[1])
        else:
            g = None
        w.cast(int(rest[0]), g)
        if kvc.get("autoloot", "0") not in ("0", ""):
            w.loot_all(float(kvc.get("lootwait", 3)))

    elif verb == "cast-at":
        # cast-at <spellId> <x> <y> <z> [elev=<f>] [speed=<f>] - ground-targeted CMSG_CAST_SPELL
        # (TARGET_FLAG_DEST_LOCATION), #513. Also accepts x,y,z as one comma-separated token.
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        pos = [t for t in rest[1:] if "=" not in t]
        if len(pos) == 1 and "," in pos[0]:
            pos = pos[0].split(",")
        if len(pos) != 3:
            log("cast-at: need <spellId> <x> <y> <z>")
            return
        w.cast(int(rest[0]), None, dest=tuple(float(v) for v in pos),
               elevation=float(kv["elev"]) if "elev" in kv else None,
               speed=float(kv["speed"]) if "speed" in kv else None)

    elif verb in ("cast-at-near", "tele-near"):
        # t438 (w76 vehicle legs). Both ask the server (`.botscenario <name> npcpos <entry>`, nearest live creature of that
        # entry to the CHARACTER - a seated player is at his vehicle's position) where something is *now*:
        #   cast-at-near <spellId> <entry> [dz=<f>]   ground-targeted CMSG_CAST_SPELL (cast-at) at that creature's position.
        #       L11: the Cyclone flies a 105-yd loop, so fixed sailor coordinates are out of the 100-yd Lightning Strike range
        #       for ~40 % of the lap; the nearest sailor always is in range.
        #   tele-near <map> <entry> [off=<f>]   teleport the character (SOAP) next to that creature (off yd east of it, same z)
        #       and setpos: a wandering target behind terrain (L23 shaman: Charge result 50 = LINE_OF_SIGHT) is reached without
        #       a Charge spline / walk.
        kvn = dict(t.split("=", 1) for t in rest if "=" in t)
        pos_args = [t for t in rest if "=" not in t]
        want_entry = pos_args[1] if verb == "cast-at-near" and len(pos_args) > 1 else (pos_args[1] if len(pos_args) > 1 else "")
        if not want_entry:
            log("%s: need entry (cast-at-near <spell> <entry> | tele-near <map> <entry>)" % verb)
            return
        cmd = ".botscenario %s npcpos %s" % (w.char_name or "", want_entry)
        try:
            status, payload = soap_command(args_cli.soap_host, args_cli.soap_port,
                                           args_cli.soap_user, args_cli.soap_pass, cmd)
        except OSError as exc:
            log("%s: SOAP helper failed: %s" % (verb, exc))
            return
        mm = re.search(r"x=(-?[0-9.]+) y=(-?[0-9.]+) z=(-?[0-9.]+)", payload)
        if not mm:
            log("%s: npcpos %s found nothing: %s" % (verb, want_entry, payload.strip()[:160]))
            w.emit("harness", verb=verb, found=0)
            return
        nx, ny, nz = float(mm.group(1)), float(mm.group(2)), float(mm.group(3))
        if verb == "cast-at-near":
            nz += float(kvn.get("dz", 0))
            log("cast-at-near: nearest %s at (%.1f, %.1f, %.1f)" % (want_entry, nx, ny, nz))
            w.cast(int(pos_args[0]), None, dest=(nx, ny, nz))
        else:
            tx = nx + float(kvn.get("off", 3))
            log("tele-near: nearest %s at (%.1f, %.1f, %.1f) -> teleport to (%.1f, %.1f, %.1f)" % (want_entry, nx, ny, nz, tx, ny, nz + 0.5))
            run_soap(args_cli, ".botscenario %s teleport %s %.2f %.2f %.2f" % (w.char_name or "", pos_args[0], tx, ny, nz + 0.5))
            w.pos = (tx, ny, nz + 0.5)
    elif verb == "use-item":
        # use-item <bag> <slot> [guid=<N>] - plain CMSG_USE_ITEM (no target), e.g. quest item
        # 46856 "Keys to the Hot Rod". bag/slot are positional because this client does not parse
        # SMSG_UPDATE_OBJECT, so an itemEntry cannot be resolved to an inventory position.
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        pos_args = [t for t in rest if "=" not in t]
        if len(pos_args) < 2:
            log("use-item: need <bag> <slot> (this client does not parse SMSG_UPDATE_OBJECT, so "
                "it cannot look an itemEntry up for you); optional guid=<item guid>")
            return
        w.use_item(int(pos_args[0]), int(pos_args[1]), int(kv.get("guid", 0)))
    elif verb == "pump":
        # pump <seconds> - just keep the session alive and process packets (gossip menus, chat, shop steps).
        w.pump(float(rest[0]) if rest else 5.0)
    elif verb == "bp-products":
        # bp-products [timeout=S] (#969) - CMSG_BATTLE_PAY_GET_PRODUCT_LIST; logs the product count and the balance
        # system line ("Your new balance is: | <dp/10000> | ") the server sends with it.
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        w.bp_get_products()
        w.pump(float(kv.get("timeout", 10)), stop_on=(SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE,))
        w.pump(2)   # the balance chat line follows the list
    elif verb == "bp-buy":
        # bp-buy <productId> [confirms=N] [gap=S] [decline=1] [timeout=S] [tail=S] (#969)
        # The whole shop purchase a real client makes: START_PURCHASE -> (server) START_PURCHASE_RESPONSE +
        # PURCHASE_UPDATE READY -> ALLOWED_TO_BUY + CONFIRM_PURCHASE(serverToken, price) -> N x CONFIRM_PURCHASE_RESPONSE
        # (confirms=3 spams the confirm, gap=S between them; decline=1 answers "no") -> BUYING (8 s) -> DELIVERY_ENDED ->
        # BUYED. tail=S keeps pumping after the end to catch a second delivery. Prints one BP-BUY summary line.
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        pos_args = [t for t in rest if "=" not in t]
        if not pos_args:
            log("bp-buy: need <productId>")
            return
        product = int(pos_args[0])
        confirms = int(kv.get("confirms", 1))
        gap = float(kv.get("gap", 0))
        decline = kv.get("decline", "0") == "1"
        timeout = float(kv.get("timeout", 30))
        tail = float(kv.get("tail", 12))
        w.bp_confirm = None
        first_update = len(w.bp_updates)
        w.bp_start_purchase(product)

        def _failed():
            return any(u["result"] != 0 for u in w.bp_updates[first_update:])

        deadline = time.time() + timeout
        while time.time() < deadline and w.bp_confirm is None and not _failed():
            w.pump(0.5)
        if w.bp_confirm is not None and not _failed():
            for i in range(max(1, confirms)):
                w.bp_confirm_response(w.bp_confirm["price"], w.bp_confirm["server_token"], confirm=not decline)
                if gap and i + 1 < confirms:
                    w.pump(gap)
            deadline = time.time() + timeout
            while time.time() < deadline and not decline and not _failed() \
                    and not any(u["status"] == 6 for u in w.bp_updates[first_update:]):
                w.pump(0.5)
        elif _failed():
            log("bp-buy: the server refused the purchase (error update), no confirm sent")
        else:
            log("bp-buy: no SMSG_BATTLE_PAY_CONFIRM_PURCHASE within %.0fs" % timeout)
        w.pump(tail)
        ups = w.bp_updates[first_update:]
        summary = {"product": product, "confirm": w.bp_confirm, "confirms_sent": confirms if w.bp_confirm else 0,
                   "updates": ["%s/%s" % (BATTLE_PAY_STATUS_NAMES.get(u["status"], u["status"]),
                                          BATTLE_PAY_RESULT_NAMES.get(u["result"], u["result"])) for u in ups],
                   "buyed": sum(1 for u in ups if u["status"] == 6 and u["result"] == 0)}
        print("BP-BUY " + json.dumps(summary), flush=True)
    elif verb == "use-item-at":
        # use-item-at <bag> <slot> <x> <y> <z> [guid=<N>] - ground-targeted CMSG_USE_ITEM
        # (TARGET_FLAG_DEST_LOCATION 0x40, hasDestLocation=1), e.g. a placed/thrown item or an
        # item whose on-use spell has a dest target (summon/area spell). Outcome in the client log:
        # "SMSG_CAST_FAILED spell=<id> result=<n>", "SMSG_SPELL_GO ... spell_candidates=<id>" and
        # "SMSG_INVENTORY_CHANGE_FAILURE". Also accepts x,y,z as one comma-separated token.
        # guid=<item guid> is optional on the wire but SpellHandler.cpp:411 compares it with the
        # item found at bag/slot, so pass it when the server is the stock parser.
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        pos_args = [t for t in rest if "=" not in t]
        if len(pos_args) == 3 and "," in pos_args[2]:
            pos_args = pos_args[:2] + pos_args[2].split(",")
        if len(pos_args) != 5:
            log("use-item-at: need <bag> <slot> <x> <y> <z> (optional guid=<item guid>)")
            return
        w.use_item(int(pos_args[0]), int(pos_args[1]), int(kv.get("guid", 0)),
                   dest=tuple(float(v) for v in pos_args[2:5]))
    elif verb == "gossip-hello":
        # gossip-hello <npc|rawguid>  (CMSG_GOSSIP_HELLO; logs the SMSG_GOSSIP_MESSAGE options)
        g = w.obj_guid(rest[0] if rest else "npc")
        if not g:
            log("gossip-hello: need <npc|rawguid> (run `npcguid <entry>` first)")
            return
        w.gossip_hello(g)
        w.pump(20, stop_on=(SMSG_GOSSIP_MESSAGE,))   # server can be slow under bot load
    elif verb == "gossip-select":
        # gossip-select <optionId|text=<substring>> [code=<text>] [guid=<npc|rawguid>] [menu=<id>]
        # optionId is the id printed in the "gossip option id=" log line (not the index).
        kv = dict(t.split("=", 1) for t in rest if "=" in t)
        pos = [t for t in rest if "=" not in t]
        gm = getattr(w, "gossip", None)
        opt = None
        if "text" in kv:
            want = kv["text"].lower()
            for o in (gm["options"] if gm else []):
                if want in o["text"].lower():
                    opt = o["id"]
                    break
            if opt is None:
                log("gossip-select: no listed option matches text=%r" % kv["text"])
                return
        elif pos:
            opt = int(pos[0])
        else:
            log("gossip-select: need <optionId> or text=<substring>")
            return
        guid = w.obj_guid(kv["guid"]) if "guid" in kv else None
        w.gossip_select(opt, guid, int(kv["menu"]) if "menu" in kv else None, kv.get("code", ""))
    elif verb == "faction-select":
        # faction-select <horde|alliance|0|1>  (CMSG_SELECT_FACTION, neutral pandaren only)
        v = (rest[0].lower() if rest else "")
        choice = {"horde": 0, "0": 0, "alliance": 1, "1": 1}.get(v)
        if choice is None:
            log("faction-select: need horde|alliance")
            return
        w.select_faction(choice)
    elif verb in ("gameobject-use", "gameobject-report-use"):
        # gameobject-use <rawguid|entry:spawnId>  (CMSG_GAMEOBJ_USE / CMSG_GAMEOBJ_REPORT_USE)
        g = w.obj_guid(rest[0]) if rest else 0
        if not g:
            log("%s: need <rawguid|entry:spawnId>" % verb)
            return
        w.gameobject_use(g, report=(verb == "gameobject-report-use"))
    elif verb == "attack":
        g = w.obj_guid(rest[0] if rest else "npc")
        if not g:
            log("attack: need <npc|rawguid>")
            return
        w.attack(g)
    elif verb == "attack-stop":
        w.attack_stop()
    elif verb == "spellclick":
        g = w.obj_guid(rest[0] if rest else "npc")
        if not g:
            log("spellclick: need <npc|rawguid>")
            return
        w.spellclick(g)
    elif verb in ("npcguid", "goguid", "nearcorpse", "kill", "board", "equip", "learn", "masterprobe", "castself", "cansee"):  # cansee: #519
        # Harness verbs (.botscenario <player> <verb> [args], BotScenario.cpp;
        # issue #172): run over SOAP as the console, which returns the
        # handler's SendSysMessage() output directly in the HTTP response body
        # - no chat round-trip needed. npcguid/nearcorpse replies are parsed
        # for "guid=<n>" and cached in w.harness_guid for later verbs (taxi,
        # loot, reclaim) to use.
        cmd_verb = "masterprobe on" if verb == "masterprobe" else " ".join([verb] + rest)
        cmd = ".botscenario %s %s" % (w.char_name or "", cmd_verb)
        log("SOAP %s:%d -> %r" % (args_cli.soap_host, args_cli.soap_port, cmd))
        try:
            status, payload = soap_command(args_cli.soap_host, args_cli.soap_port,
                                            args_cli.soap_user, args_cli.soap_pass, cmd)
        except (OSError, RuntimeError) as exc:       # RuntimeError: no SOAP credentials configured
            log("SOAP helper failed: %s" % exc)
            return
        log("SOAP HTTP %d: %s" % (status, payload.strip()[:300]))
        m = re.search(r"guid=(\d+)", payload)
        if m and verb in ("npcguid", "goguid", "nearcorpse"):
            key = {"npcguid": "npc", "goguid": "go", "nearcorpse": "corpse"}[verb]   # goguid = #708 GameObject twin
            # #207: a 0 here means the harness found NO such creature nearby. Caching it
            # used to poison the single cache slot, so a later `cast <spell> npc` would
            # silently resolve to nothing (or, worse, to a stale guid from an earlier
            # lookup). Keep any previously cached guid and say the lookup failed.
            if int(m.group(1)):
                w.harness_guid[key] = int(m.group(1))
            elif not w.harness_guid.get(key):
                log("%s: no such creature nearby; %s cache left empty" % (verb, key))
            else:
                log("%s: no such creature nearby; keeping previous %s guid %s"
                    % (verb, key, w.harness_guid[key]))
            w.emit("harness", verb=verb, guid=m.group(1))
    else:
        log("verb %r not implemented (see issue #172)" % verb)


def _session_budget(args_cli, steps):
    """#875: wall-clock budget for a --step/--script session = --hold + every step's own timeout + ~4 s of
    per-step overhead (SOAP round trip + the 1 s pump after each step) + 60 s. --max-session N overrides it."""
    if getattr(args_cli, "max_session", 0):
        return float(args_cli.max_session)
    total = float(args_cli.hold) + 60.0
    # #875: a step without timeout= still blocks for its verb's DEFAULT wait (wait-for 30 s, wait-for-server-pos
    # 60 s, wait-file 120 s, walk = path length / speed), so count that and not a flat 10 s - otherwise a legitimate
    # chain step list would be killed by the budget it is supposed to merely backstop.
    verb_default = {"wait-for": 30.0, "wait-for-server-pos": 60.0, "wait-file": 120.0,
                    # real-player verbs: their default timeout= (a STUCK line ends the run earlier)
                    "goto-path": 300.0, "goto-npc": 300.0, "goto-go": 300.0, "kill": 900.0, "collect": 1200.0,
                    "accept": 300.0, "accept-go": 300.0, "turnin": 300.0, "turnin-go": 300.0, "talk": 300.0,
                    "use-go": 300.0, "use-item": 300.0, "spellclick": 300.0, "wait-quest": 60.0,
                    "vehicle-wait": 60.0, "wait-map": 120.0, "wait-level": 60.0, "hold": 300.0, "follow": 1800.0,
                    "repeat": 3600.0, "retry": 600.0}
    for line in steps:
        parts = line.split()
        verb = parts[0] if parts else ""
        t = max(10.0, verb_default.get(verb, 10.0))
        for m in re.finditer(r"(?:timeout|lootwait)=(\d+(?:\.\d+)?)", line):
            t = max(t, float(m.group(1)))
        if verb == "walk":
            spd = 7.0
            pts = []
            for tok in parts[1:]:
                if tok.startswith("speed="):
                    try:
                        spd = max(0.5, float(tok.split("=", 1)[1]))
                    except ValueError:
                        pass
                elif "=" not in tok:
                    try:
                        pts.append(tuple(float(v) for v in tok.split(",")))
                    except ValueError:
                        pass
            dist = sum(math.dist(a, b) for a, b in zip(pts, pts[1:]))
            t = max(t, dist / spd + 60.0)
        total += t + 4.0
    return total


def _session_timeout(w, n, line, budget):
    verb = line.split(None, 1)[0] if line.split() else "?"
    log("SESSION-TIMEOUT step=%d %s (budget %.0fs exceeded)" % (n, verb, budget))
    try:
        w.logout(timeout=10.0)
    except Exception as exc:
        log("SESSION-TIMEOUT logout failed: %r" % (exc,))
    sys.stdout.flush()
    os._exit(3)


def run_steps(w, args_cli, steps):
    budget = _session_budget(args_cli, steps)
    t0 = time.time()
    state = {"n": 0, "line": ""}

    def _watchdog():
        # Backstop for a step that blocks inside a socket/SOAP call past its own timeout.
        log("SESSION-TIMEOUT step=%d %s (watchdog, budget %.0fs + 30s exceeded)"
            % (state["n"], (state["line"].split() or ["?"])[0], budget))
        sys.stdout.flush()
        os._exit(3)
    wd = threading.Timer(budget + 30.0, _watchdog)
    wd.daemon = True
    wd.start()
    try:
        state["t0"], state["budget"] = t0, budget
        exec_steps(w, args_cli, expand_inline_blocks(steps), state)
    finally:
        wd.cancel()


# ---------------------------------------------------------------------------
# #747 --log-movement: what this real 5.4.8 client is TOLD about other units' movement. The owner saw Twin Peaks bots
# "fly around teleporting, extremely fast" in his client while the server [PROBE] positions were sane, so the evidence
# has to come from the packets. Decoded with this client's receive time (layouts from this tree):
#   SMSG_MONSTER_MOVE 0x1A07          MovementPacketBuilder.cpp WriteMonsterMove / WriteStopMovement, incl. the packed
#                                     intermediate points (ByteBuffer::appendPackXYZ: 11/11/10-bit signed quarter yards)
#   SMSG_PLAYER_MOVE 0x1A32           Unit::WriteMovementInfo + MovementStructures.cpp PlayerMove
#   SMSG_MOVE_UPDATE_RUN_SPEED 0x158E / _SWIM_SPEED 0x01E2 / _KNOCK_BACK 0x0251    (MovementUpdate* sequences)
#   SMSG_SPLINE_MOVE_SET_RUN_SPEED 0x02F1 / _SET_SWIM_SPEED 0x1D8E / _START_SWIM 0x0F29 / _STOP_SWIM 0x1798 /
#   _SET_FLYING 0x1046 / _UNSET_FLYING 0x0DE2                                      (SplineMove* sequences)
#   SMSG_MOVE_UPDATE_TELEPORT 0x15A9  Unit::BuildTeleportUpdateData
# Names: CMSG_NAME_QUERY 0x0328 once per player guid, SMSG_NAME_QUERY_RESPONSE 0x169B (QueryHandler.cpp:37-173).
# One grep-friendly line per packet ("MOVEPKT k=v ..."):
#   common        t=ms since login  wall=epoch s  op  guid=DB counter  hi=player|unit  name  map
#   MONSTER_MOVE  sid type s=x,y,z e=x,y,z npts dur(ms) sflags sfl dist(start->end) plen(client polyline length)
#                 spd=plen/dur  sspd=dist/dur  maxgap(longest client segment)  wrap(1 = maxgap > 100 yd on a packed
#                 path: packed-offset overflow)  snap=yd between where this client had the unit (previous spline, or a
#                 later status dead-reckoned along its facing) and the new start  snapS=same against the previous
#                 spline only  prevrem=ms the previous spline still had to run (>0 = re-path mid-spline)
#   status ops    p=x,y,z o mflags mfl midspline(1 = a spline was still running) rem(ms) dev=yd from the spline
#                 position at receipt  srvms=packet timestamp (server getMSTime)  spd=speed field (speed opcodes)
#   spline ops    midspline rem splspd=active spline speed  fastswim(1 = START_SWIM mid-spline on a spline faster
#                 than 1.6 x 4.72 yd/s: launched at run/mount speed into the water)  spd=speed field
#   TELEPORT      p=x,y,z snap
#   (2026-10-20) MONSTER_MOVE pts=x,y,z;... (every point of the polyline); status ops snap= (jump on this client when
#                 the status lands) drspd= (speed this client dead-reckons a FORWARD status at)
#   TRACK         every MV_TRACK_S per mover: p=x,y,z where this client draws it now, src=spline|status|hold, me=x,y,z
#                 (this client's own position: movers > ~533 yd away are out of view and not drawn at all)
#   OUT_OF_RANGE  the mover left this client's view (SMSG_UPDATE_OBJECT out-of-range block: range, stealth, phase)
#   VISIBLE       first=1: the mover is the first create block of an SMSG_UPDATE_OBJECT without an out-of-range block
#   CREATE        (#126) every create block of a wanted unit: spline=0|1 pos= [dest= passed= dur= npts= sid= sflags= type=] self=
#   TRANSPORT_CREATE / TRANSPORT_OUT  every create / out-of-range block of a moving transport (boat, zeppelin), whatever
#                 the wanted filter: guid= is its `transports`.guid. Shows which transports this client is sent (phases).
# MOVEPKT-SUMMARY at exit: packet counts and anomaly counts per bot.
# ---------------------------------------------------------------------------
MV_SMSG_MONSTER_MOVE = 0x1A07
MV_SMSG_UPDATE_OBJECT = 0x1792           # only its leading out-of-range block / first create block (MoveLog._update_object)


def mv_read_packed_guid(r):
    """ObjectGuid::WriteAsPacked: a mask byte, then the non-zero bytes low to high"""
    mask, guid = r.u8(), 0
    for i in range(8):
        if mask & (1 << i):
            guid |= r.u8() << (8 * i)
    return guid
MV_SMSG_MOVE_UPDATE_TELEPORT = 0x15A9
MV_CMSG_NAME_QUERY = 0x0328
MV_SMSG_NAME_QUERY_RESPONSE = 0x169B
MV_HIGH_PLAYER = 0x070
MV_HIGH_MO_TRANSPORT = 0x1FC             # ObjectGuid HighGuid::Mo_Transport; the counter is the `transports` guid
MV_SWIM_SPEED_BASE = 4.722222
MV_TRACK_S = 0.5           # TRACK line cadence (s): the client's drawn position of every mover

MV_SEQ_PLAYER_MOVE = [   # MovementStructures.cpp:22-97 (PlayerMove)
    "HasPitch", "HasGuidByte2", "ZeroBit", "ZeroBit", "HasGuidByte0", "HasOrientation", "HasFallData", "HasCounter",
    "HasGuidByte3", "HasFallDirection", "HasTransportData", "HasGuidByte4", "HasTransportGuidByte5",
    "HasTransportGuidByte4", "HasTransportGuidByte7", "HasTransportGuidByte2", "HasTransportGuidByte6",
    "HasTransportTime2", "HasTransportGuidByte3", "HasTransportGuidByte1", "HasTransportTime3",
    "HasTransportGuidByte0", "HasSplineElevation", "HasMovementFlags", "ZeroBit", "MovementFlags",
    "HasMovementFlags2", "HasGuidByte7", "HasGuidByte1", "HasTimestamp", "MovementFlags2", "HasGuidByte5",
    "ForcesCount", "HasGuidByte6", "PositionY", "TransportGuidByte7", "TransportTime2", "TransportPositionX",
    "TransportGuidByte5", "TransportSeat", "TransportGuidByte2", "TransportGuidByte0", "TransportGuidByte3",
    "TransportTime", "TransportGuidByte4", "TransportPositionZ", "TransportGuidByte1", "TransportPositionY",
    "TransportOrientation", "TransportGuidByte6", "TransportTime3", "GuidByte5", "GuidByte1", "PositionZ", "Forces",
    "Timestamp", "Orientation", "GuidByte3", "FallSinAngle", "FallHorizontalSpeed", "FallCosAngle",
    "FallVerticalSpeed", "FallTime", "GuidByte0", "Pitch", "GuidByte2", "GuidByte6", "SplineElevation", "Counter",
    "PositionX", "GuidByte4", "GuidByte7",
]
MV_SEQ_UPDATE_RUN_SPEED = [   # MovementStructures.cpp MovementUpdateRunSpeed
    "HasGuidByte0", "HasGuidByte3", "HasTimestamp", "HasGuidByte7", "HasGuidByte4", "ZeroBit", "HasGuidByte1",
    "HasPitch", "HasGuidByte2", "HasGuidByte5", "HasCounter", "HasGuidByte6", "HasMovementFlags", "ForcesCount",
    "MovementFlags", "HasMovementFlags2", "MovementFlags2", "HasTransportData", "HasTransportTime2",
    "HasTransportGuidByte5", "HasTransportGuidByte6", "HasTransportGuidByte1", "HasTransportGuidByte3",
    "HasTransportGuidByte2", "HasTransportTime3", "HasTransportGuidByte0", "HasTransportGuidByte7",
    "HasTransportGuidByte4", "HasSplineElevation", "HasOrientation", "HasFallData", "ZeroBit", "HasFallDirection",
    "ZeroBit", "FallVerticalSpeed", "FallSinAngle", "FallCosAngle", "FallHorizontalSpeed", "FallTime",
    "TransportGuidByte4", "TransportGuidByte2", "TransportSeat", "TransportPositionZ", "TransportGuidByte5",
    "TransportGuidByte7", "TransportGuidByte3", "TransportPositionY", "TransportTime2", "TransportGuidByte6",
    "TransportTime", "TransportOrientation", "TransportTime3", "TransportPositionX", "TransportGuidByte0",
    "TransportGuidByte1", "PositionX", "SplineElevation", "GuidByte1", "GuidByte2", "GuidByte6", "Counter",
    "GuidByte0", "Timestamp", "GuidByte3", "GuidByte5", "Forces", "Pitch", "GuidByte7", "Orientation", "PositionZ",
    "PositionY", "ExtraElement", "GuidByte4",
]
MV_SEQ_UPDATE_SWIM_SPEED = [   # MovementStructures.cpp MovementUpdateSwimSpeed
    "HasOrientation", "HasGuidByte0", "ZeroBit", "ZeroBit", "HasPitch", "HasGuidByte6", "ZeroBit", "HasCounter",
    "HasFallData", "HasTimestamp", "HasMovementFlags", "HasSplineElevation", "HasTransportData",
    "HasTransportGuidByte4", "HasTransportGuidByte7", "HasTransportGuidByte5", "HasTransportGuidByte3",
    "HasTransportGuidByte2", "HasTransportTime3", "HasTransportGuidByte6", "HasTransportGuidByte0",
    "HasTransportTime2", "HasTransportGuidByte1", "MovementFlags", "HasFallDirection", "HasGuidByte4",
    "HasGuidByte2", "HasMovementFlags2", "ForcesCount", "HasGuidByte3", "MovementFlags2", "HasGuidByte7",
    "HasGuidByte5", "HasGuidByte1", "FallSinAngle", "FallCosAngle", "FallHorizontalSpeed", "FallTime",
    "FallVerticalSpeed", "ExtraElement", "TransportPositionX", "TransportSeat", "TransportGuidByte3",
    "TransportPositionY", "TransportGuidByte6", "TransportGuidByte2", "TransportGuidByte0", "TransportGuidByte1",
    "TransportPositionZ", "TransportTime3", "TransportTime", "TransportGuidByte4", "TransportOrientation",
    "TransportGuidByte5", "TransportGuidByte7", "TransportTime2", "PositionX", "Counter", "GuidByte6", "GuidByte0",
    "SplineElevation", "GuidByte5", "Orientation", "Pitch", "Timestamp", "PositionY", "GuidByte1", "Forces",
    "GuidByte3", "PositionZ", "GuidByte4", "GuidByte7", "GuidByte2",
]
MV_SEQ_UPDATE_KNOCK_BACK = [   # MovementStructures.cpp MovementUpdateKnockBack
    "HasGuidByte5", "HasSplineElevation", "HasTimestamp", "ForcesCount", "HasMovementFlags2", "HasGuidByte2",
    "HasGuidByte4", "HasGuidByte6", "HasGuidByte1", "HasGuidByte0", "ZeroBit", "HasOrientation", "ZeroBit",
    "HasTransportData", "HasTransportGuidByte5", "HasTransportGuidByte2", "HasTransportGuidByte0",
    "HasTransportGuidByte7", "HasTransportGuidByte1", "HasTransportGuidByte6", "HasTransportGuidByte4",
    "HasTransportTime2", "HasTransportGuidByte3", "HasTransportTime3", "HasGuidByte3", "HasFallData", "HasCounter",
    "HasFallDirection", "HasGuidByte7", "ZeroBit", "HasPitch", "MovementFlags2", "HasMovementFlags", "MovementFlags",
    "GuidByte1", "TransportGuidByte5", "TransportTime3", "TransportGuidByte3", "TransportGuidByte1",
    "TransportGuidByte4", "TransportPositionZ", "TransportGuidByte7", "TransportGuidByte6", "TransportGuidByte2",
    "TransportPositionY", "TransportGuidByte0", "TransportSeat", "TransportOrientation", "TransportPositionX",
    "TransportTime2", "TransportTime", "GuidByte2", "SplineElevation", "FallSinAngle", "FallHorizontalSpeed",
    "FallCosAngle", "FallVerticalSpeed", "FallTime", "PositionY", "Orientation", "Forces", "GuidByte7", "GuidByte6",
    "GuidByte4", "PositionZ", "Counter", "GuidByte3", "GuidByte0", "PositionX", "Pitch", "GuidByte5", "Timestamp",
]


def _mv_guid_seq(mask_order, byte_order, extra_after=None):
    """A SplineMove* sequence: 8 mask bits, then the 8 guid bytes with the speed float inserted after byte index
    `extra_after` of byte_order (None = no speed field)."""
    seq = ["HasGuidByte%d" % i for i in mask_order]
    for k, i in enumerate(byte_order):
        seq.append("GuidByte%d" % i)
        if extra_after is not None and k == extra_after:
            seq.append("ExtraElement")
    return seq


# MovementStructures.cpp SplineMoveSetRunSpeed / SetSwimSpeed / StartSwim / StopSwim / SetFlying / UnsetFlying
MV_STATUS_OPS = {
    0x1A32: ("PLAYER_MOVE", MV_SEQ_PLAYER_MOVE, None),
    0x158E: ("MOVE_UPDATE_RUN_SPEED", MV_SEQ_UPDATE_RUN_SPEED, "run"),
    0x01E2: ("MOVE_UPDATE_SWIM_SPEED", MV_SEQ_UPDATE_SWIM_SPEED, "swim"),
    0x0251: ("MOVE_UPDATE_KNOCK_BACK", MV_SEQ_UPDATE_KNOCK_BACK, None),
}
MV_SPLINE_OPS = {
    0x02F1: ("SPLINE_SET_RUN_SPEED", _mv_guid_seq((3, 0, 1, 4, 7, 5, 6, 2), (4, 1, 5, 3, 7, 6, 2, 0), 0), "run"),
    0x1D8E: ("SPLINE_SET_SWIM_SPEED", _mv_guid_seq((5, 6, 7, 3, 4, 2, 1, 0), (4, 1, 6, 7, 3, 5, 0, 2), 4), "swim"),
    0x0F29: ("SPLINE_START_SWIM", _mv_guid_seq((7, 4, 2, 6, 5, 3, 1, 0), (4, 0, 6, 7, 1, 2, 3, 5)), None),
    0x1798: ("SPLINE_STOP_SWIM", _mv_guid_seq((3, 7, 4, 2, 1, 0, 5, 6), (0, 5, 1, 7, 3, 2, 6, 4)), None),
    0x1046: ("SPLINE_SET_FLYING", _mv_guid_seq((4, 1, 2, 0, 7, 5, 3, 6), (4, 7, 1, 0, 3, 5, 6, 2)), None),
    0x0DE2: ("SPLINE_UNSET_FLYING", _mv_guid_seq((1, 5, 7, 2, 6, 3, 0, 4), (2, 5, 4, 6, 1, 0, 7, 3)), None),
}
MV_MOVEMENT_FLAGS = [
    (0x1, "FORWARD"), (0x2, "BACKWARD"), (0x4, "STRAFE_L"), (0x8, "STRAFE_R"), (0x10, "LEFT"), (0x20, "RIGHT"),
    (0x40, "PITCH_UP"), (0x80, "PITCH_DOWN"), (0x100, "WALKING"), (0x200, "DISABLE_GRAVITY"), (0x400, "ROOT"),
    (0x800, "FALLING"), (0x1000, "FALLING_FAR"), (0x100000, "SWIMMING"), (0x200000, "ASCENDING"),
    (0x400000, "DESCENDING"), (0x800000, "CAN_FLY"), (0x1000000, "FLYING"), (0x2000000, "SPLINE_ELEVATION"),
    (0x4000000, "WATERWALKING"), (0x8000000, "FALLING_SLOW"), (0x10000000, "HOVER"),
]   # UnitDefines.h MovementFlags
MV_FLY_FLAGS = 0x200 | 0x800000 | 0x1000000          # DISABLE_GRAVITY | CAN_FLY | FLYING
MV_SPLINE_FLAGS = [
    (0x10, "FallingSlow"), (0x20, "Done"), (0x40, "Falling"), (0x80, "No_Spline"), (0x200, "Flying"),
    (0x400, "OrientationFixed"), (0x800, "Catmullrom"), (0x1000, "Cyclic"), (0x2000, "Enter_Cycle"),
    (0x8000, "TransportEnter"), (0x10000, "TransportExit"), (0x80000, "OrientationInversed"),
    (0x100000, "SmoothGroundPath"), (0x200000, "Walkmode"), (0x400000, "UncompressedPath"), (0x1000000, "Animation"),
    (0x2000000, "Parabolic"), (0x4000000, "Final_Point"), (0x8000000, "Final_Target"), (0x10000000, "Final_Angle"),
]   # MoveSplineFlag.h
MV_MOVE_TYPES = {0: "normal", 1: "stop", 2: "facepoint", 3: "facetarget", 4: "faceangle"}


class MvParseError(Exception):
    pass


class MvReader:
    """ByteBuffer read side: bits MSB first, a raw read drops pending bits (ResetBitPos)."""

    def __init__(self, data):
        self.d, self.p, self.bp, self.cur = data, 0, 8, 0

    def raw(self, n):
        self.bp = 8
        if self.p + n > len(self.d):
            raise MvParseError("short read %d at %d/%d" % (n, self.p, len(self.d)))
        v = self.d[self.p:self.p + n]
        self.p += n
        return v

    def u8(self):
        return self.raw(1)[0]

    def i8(self):
        return struct.unpack("<b", self.raw(1))[0]

    def u32(self):
        return struct.unpack("<I", self.raw(4))[0]

    def f32(self):
        return struct.unpack("<f", self.raw(4))[0]

    def bit(self):
        if self.bp >= 8:
            if self.p >= len(self.d):
                raise MvParseError("short bit read at %d" % self.p)
            self.cur = self.d[self.p]
            self.p += 1
            self.bp = 0
        b = (self.cur >> (7 - self.bp)) & 1
        self.bp += 1
        return b

    def bits(self, n):
        v = 0
        for _ in range(n):
            v = (v << 1) | self.bit()
        return v

    def flush(self):
        self.bp = 8


class MvGuid:
    """Bit-masked guid: mask bits, then WriteByteSeq bytes (wire byte = value ^ 1, only for non-zero bytes)."""

    def __init__(self):
        self.b = [0] * 8

    def mask(self, r, i):
        self.b[i] = r.bit()

    def masks(self, r, order):
        for i in order:
            self.b[i] = r.bit()

    def seq(self, r, i):
        if self.b[i]:
            self.b[i] = r.u8() ^ 1

    def seqs(self, r, order):
        for i in order:
            self.seq(r, i)

    def value(self):
        return int.from_bytes(bytes(x & 0xFF for x in self.b), "little")


def mv_unpack_xyz(p):
    """ByteBuffer::appendPackXYZ read back: x 11 bits, y 11 bits, z 10 bits, signed, 0.25 yd units."""
    x, y, z = p & 0x7FF, (p >> 11) & 0x7FF, (p >> 22) & 0x3FF
    x = x - 0x800 if x & 0x400 else x
    y = y - 0x800 if y & 0x400 else y
    z = z - 0x400 if z & 0x200 else z
    return x * 0.25, y * 0.25, z * 0.25


def mv_read_monster_move(body):
    """MovementPacketBuilder WriteMonsterMove / WriteStopMovement read back."""
    r = MvReader(body)
    z0 = r.f32(); x0 = r.f32(); sid = r.u32(); y0 = r.f32()
    r.f32(); r.f32(); r.f32()
    g, tg, fg = MvGuid(), MvGuid(), MvGuid()
    parab = not r.bit()
    g.mask(r, 0)
    mtype = r.bits(3)
    if mtype == 3:
        fg.masks(r, (6, 4, 3, 0, 5, 7, 1, 2))
    if not (r.bit() and r.bit()):
        raise MvParseError("monster move: constant bits (a)")
    has_seat = not r.bit()
    n_unc = r.bits(20)
    has_flags = not r.bit()
    g.mask(r, 3)
    if not (r.bit() and r.bit() and r.bit()):
        raise MvParseError("monster move: constant bits (b)")
    has_dur = not r.bit()
    g.masks(r, (7, 4))
    if not r.bit():
        raise MvParseError("monster move: constant bit (c)")
    g.mask(r, 5)
    n_cmp = r.bits(22)
    g.mask(r, 6)
    r.bit()
    tg.masks(r, (7, 1, 3, 0, 6, 4, 5, 2))
    r.bit(); r.bit()
    g.masks(r, (2, 1))
    r.flush()
    packed = [r.u32() for _ in range(n_cmp)]
    g.seq(r, 1)
    tg.seqs(r, (6, 4, 1, 7, 0, 3, 5, 2))
    unc = []
    for _ in range(n_unc):
        py = r.f32(); px = r.f32(); pz = r.f32()
        unc.append((px, py, pz))
    if mtype == 3:
        fg.seqs(r, (5, 7, 0, 4, 3, 2, 6, 1))
    g.seq(r, 5)
    if parab:
        r.f32()
    if mtype == 4:
        r.f32()
    g.seq(r, 3)
    flags = r.u32() if has_flags else 0
    g.seq(r, 6)
    if mtype == 2:
        r.f32(); r.f32(); r.f32()
    g.seq(r, 0)
    if has_seat:
        r.i8()
    g.seqs(r, (7, 2, 4))
    dur = r.u32() if has_dur else 0
    start = (x0, y0, z0)
    if flags & 0x400000 or not unc:          # UncompressedPath (all points as floats), or a stop (no points)
        pts = [start] + unc
        offsets = []
    else:                                      # linear: destination + offsets from the middle of start and end
        dest = unc[-1]
        mid = ((x0 + dest[0]) / 2.0, (y0 + dest[1]) / 2.0, (z0 + dest[2]) / 2.0)
        offsets = [mv_unpack_xyz(p) for p in packed]
        pts = [start] + [(mid[0] - o[0], mid[1] - o[1], mid[2] - o[2]) for o in offsets] + [dest]
    return dict(guid=g.value(), transport=tg.value(), start=start, pts=pts, offsets=offsets, type=mtype,
                flags=flags, duration=dur, id=sid)


def mv_read_status(body, seq):
    """Unit::WriteMovementInfo read back element by element (a MovementStructures.cpp sequence)."""
    r = MvReader(body)
    g, tg = MvGuid(), MvGuid()
    st = dict(mf=False, mf2=False, ts=False, orient=False, trans=False, t2=False, t3=False, pitch=False,
              fall=False, fdir=False, selev=False, counter=False)
    out = dict(x=0.0, y=0.0, z=0.0, o=0.0, mflags=0, ts=None, extra=None, transport=0)
    for el in seq:
        if el.startswith("HasGuidByte"):
            g.mask(r, int(el[-1]))
        elif el.startswith("HasTransportGuidByte"):
            if st["trans"]:
                tg.mask(r, int(el[-1]))
        elif el.startswith("GuidByte"):
            g.seq(r, int(el[-1]))
        elif el.startswith("TransportGuidByte"):
            if st["trans"]:
                tg.seq(r, int(el[-1]))
        elif el == "HasCounter":
            st["counter"] = not r.bit()
        elif el == "HasMovementFlags":
            st["mf"] = not r.bit()
        elif el == "HasMovementFlags2":
            st["mf2"] = not r.bit()
        elif el == "HasTimestamp":
            st["ts"] = not r.bit()
        elif el == "HasOrientation":
            st["orient"] = not r.bit()
        elif el == "HasTransportData":
            st["trans"] = bool(r.bit())
        elif el == "HasTransportTime2":
            if st["trans"]:
                st["t2"] = bool(r.bit())
        elif el == "HasTransportTime3":
            if st["trans"]:
                st["t3"] = bool(r.bit())
        elif el == "HasPitch":
            st["pitch"] = not r.bit()
        elif el == "HasFallData":
            st["fall"] = bool(r.bit())
        elif el == "HasFallDirection":
            if st["fall"]:
                st["fdir"] = bool(r.bit())
        elif el == "HasSplineElevation":
            st["selev"] = not r.bit()
        elif el == "MovementFlags":
            if st["mf"]:
                out["mflags"] = r.bits(30)
        elif el == "MovementFlags2":
            if st["mf2"]:
                r.bits(13)
        elif el == "ForcesCount":
            if r.bits(22):
                raise MvParseError("forces count != 0")
        elif el == "Forces":
            pass
        elif el == "ZeroBit":
            r.bit()
        elif el == "Timestamp":
            if st["ts"]:
                out["ts"] = r.u32()
        elif el in ("PositionX", "PositionY", "PositionZ"):
            out[el[-1].lower()] = r.f32()
        elif el == "Orientation":
            if st["orient"]:
                out["o"] = r.f32()
        elif el in ("TransportPositionX", "TransportPositionY", "TransportPositionZ", "TransportOrientation"):
            if st["trans"]:
                r.f32()
        elif el == "TransportSeat":
            if st["trans"]:
                r.i8()
        elif el == "TransportTime":
            if st["trans"]:
                r.u32()
        elif el == "TransportTime2":
            if st["trans"] and st["t2"]:
                r.u32()
        elif el == "TransportTime3":
            if st["trans"] and st["t3"]:
                r.u32()
        elif el == "Pitch":
            if st["pitch"]:
                r.f32()
        elif el == "FallTime":
            if st["fall"]:
                r.u32()
        elif el == "FallVerticalSpeed":
            if st["fall"]:
                r.f32()
        elif el in ("FallCosAngle", "FallSinAngle", "FallHorizontalSpeed"):
            if st["fall"] and st["fdir"]:
                r.f32()
        elif el == "SplineElevation":
            if st["selev"]:
                r.f32()
        elif el == "Counter":
            if st["counter"]:
                r.u32()
        elif el == "ExtraElement":
            out["extra"] = r.f32()
        else:
            raise MvParseError("element %s" % el)
    out["guid"] = g.value()
    if st["trans"]:
        out["transport"] = tg.value()
    return out


def mv_read_move_update_teleport(body):
    """Unit::BuildTeleportUpdateData read back."""
    r = MvReader(body)
    g, tg = MvGuid(), MvGuid()
    trans = r.bit()
    ts = not r.bit()
    fall = r.bit()
    if r.bit():
        raise MvParseError("teleport unk189")
    u98 = not r.bit()
    t2 = t3 = 0
    if trans:
        tg.masks(r, (7, 3, 0, 2, 6))
        t3 = r.bit()
        tg.masks(r, (5, 4))
        t2 = r.bit()
        tg.mask(r, 1)
    g.mask(r, 0)
    swim = r.bit()
    u188 = r.bit()
    mf = not r.bit()
    g.mask(r, 1)
    u228 = r.bit()
    runback = r.bit()
    u52 = r.bit()
    c240 = r.bits(19)
    selev = not r.bit()
    g.mask(r, 6)
    orient = not r.bit()
    u28 = r.bit()
    fdir = r.bit() if fall else 0
    flight = r.bit()
    u260 = r.bit()
    g.masks(r, (3, 4))
    run = r.bit()
    ub8 = not r.bit()
    r.bit()
    g.mask(r, 5)
    for _ in range(c240):
        r.bits(2)
    u236 = r.bit()
    g.mask(r, 2)
    if r.bits(22):
        raise MvParseError("teleport forces")
    mf2 = not r.bit()
    mflags = r.bits(30) if mf else 0
    g.mask(r, 7)
    if mf2:
        r.bits(13)
    if u188:
        raise MvParseError("teleport unk188")
    if flight:
        r.f32()
    if trans:
        tg.seqs(r, (3, 5))
        r.u32()
        r.f32(); r.f32()
        tg.seqs(r, (4, 1, 7))
        r.f32()
        if t2:
            r.u32()
        tg.seqs(r, (0, 2))
        if t3:
            r.u32()
        r.i8()
        r.f32()
        tg.seq(r, 6)
    g.seq(r, 3)
    if fall:
        if fdir:
            r.f32(); r.f32(); r.f32()
        r.u32(); r.f32()
    g.seq(r, 4)
    for _ in range(c240):
        r.u32(); r.u32(); r.f32(); r.f32(); r.f32(); r.f32()
    if u260:
        r.f32()
    y = r.f32()
    if ub8:
        r.f32()
    if u236:
        r.f32()
    if u98:
        r.f32()
    if runback:
        r.f32()
    g.seqs(r, (2, 7))
    if swim:
        r.f32()
    x = r.f32()
    if run:
        r.f32()
    if u28:
        r.f32()
    z = r.f32()
    if ts:
        r.u32()
    if selev:
        r.f32()
    g.seq(r, 6)
    if u52:
        r.f32()
    g.seqs(r, (5, 1))
    o = r.f32() if orient else 0.0
    g.seq(r, 0)
    if u228:
        r.f32()
    return dict(guid=g.value(), x=x, y=y, z=z, o=o, mflags=mflags)


def mv_name_query_body(guid):
    """CMSG_NAME_QUERY (QueryHandler.cpp:134-172): guid bits 4, bit14=0, 6, 0, 7, 1, bit1C=0, 5, 2, 3; bytes 7 5 1 2 6 3 0 4."""
    b = guid.to_bytes(8, "little")
    bw = BitWriter()
    for item in (4, None, 6, 0, 7, 1, None, 5, 2, 3):
        bw.bit(0 if item is None else (b[item] != 0))
    out = bytearray(bw.flush())
    for off in (7, 5, 1, 2, 6, 3, 0, 4):
        if b[off]:
            out.append(b[off] ^ 1)
    return bytes(out)


def mv_read_name_response(body):
    """SMSG_NAME_QUERY_RESPONSE (QueryHandler.cpp:37-131) -> (guid, name or None)."""
    r = MvReader(body)
    g = MvGuid()
    g.masks(r, (3, 6, 7, 2, 5, 4, 0, 1))
    g.seqs(r, (5, 4, 7, 6, 1, 2))
    missing = r.u8()
    if not missing:
        r.u32(); r.u32(); r.u8(); r.u8(); r.u8(); r.u8()
    g.seqs(r, (0, 3))
    if missing:
        return g.value(), None
    g2, g3 = MvGuid(), MvGuid()
    g2.mask(r, 2); g2.mask(r, 7); g3.mask(r, 7); g3.mask(r, 2); g3.mask(r, 0)
    r.bit()
    g2.mask(r, 4); g3.mask(r, 5); g2.mask(r, 1); g2.mask(r, 3); g2.mask(r, 0)
    for _ in range(5):                       # MAX_DECLINED_NAME_CASES
        r.bits(7)
    g3.mask(r, 6); g3.mask(r, 3); g2.mask(r, 5); g3.mask(r, 1); g3.mask(r, 4)
    nlen = r.bits(6)
    g2.mask(r, 6)
    r.flush()
    g3.seq(r, 6); g3.seq(r, 0)
    return g.value(), r.raw(nlen).decode("utf-8", "replace")


def _mv_flag_names(value, table):
    names = [n for bit, n in table if value & bit]
    return ",".join(names) if names else "-"


def _mv_dist(a, b):
    return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2)


class _MvSpline:
    """The client's view of one SMSG_MONSTER_MOVE: polyline walked at constant speed over `dur` ms."""

    def __init__(self, t0, pts, dur):
        self.t0, self.pts, self.dur = t0, pts, dur
        self.cum = [0.0]
        for a, b in zip(pts, pts[1:]):
            self.cum.append(self.cum[-1] + _mv_dist(a, b))
        self.plen = self.cum[-1]

    def end_t(self):
        return self.t0 + self.dur / 1000.0

    def speed(self):
        return self.plen / (self.dur / 1000.0) if self.dur > 0 else 0.0

    def pos_at(self, t):
        if self.dur <= 0 or self.plen <= 0.0 or len(self.pts) < 2:
            return self.pts[-1]
        d = min(max((t - self.t0) * 1000.0 / self.dur, 0.0), 1.0) * self.plen
        for i in range(1, len(self.pts)):
            if self.cum[i] >= d:
                seg = self.cum[i] - self.cum[i - 1]
                f = (d - self.cum[i - 1]) / seg if seg > 0 else 1.0
                a, b = self.pts[i - 1], self.pts[i]
                return (a[0] + (b[0] - a[0]) * f, a[1] + (b[1] - a[1]) * f, a[2] + (b[2] - a[2]) * f)
        return self.pts[-1]


class _MvStatus:
    """A movement-status packet: position + facing + flags. With FORWARD (and no ROOT) a client keeps the unit
    running along its facing; dead-reckoned here at `spd` for at most 10 s."""

    def __init__(self, t0, pos, o, mflags, spd):
        self.t0, self.pos, self.o, self.mflags, self.spd = t0, pos, o, mflags, spd

    def pos_at(self, t):
        if not (self.mflags & 0x1) or (self.mflags & 0x400):
            return self.pos
        dt = min(max(t - self.t0, 0.0), 10.0)
        return (self.pos[0] + math.cos(self.o) * self.spd * dt, self.pos[1] + math.sin(self.o) * self.spd * dt,
                self.pos[2])


# --- #126 create-block decode: does the SMSG_UPDATE_OBJECT create block of a unit carry its move spline? -----------------
# Layout read back from Object::BuildMovementUpdate (Object.cpp:423-835), PacketBuilder::WriteCreateBits/WriteCreateData
# (MovementPacketBuilder.cpp:308-352) and BuildValuesUpdate/BuildDynamicValuesUpdate (Object.cpp:840-905).
# Output: MOVEPKT op=CREATE spline=0|1 pos=x,y,z [dest=x,y,z passed=ms dur=ms npts=n sid=id sflags=0x.. type=n] self=0|1.
def mv_pop(m):
    return bin(m).count("1")


def mv_skip_values(r):
    """BuildValuesUpdate: u8 block count, count x u32 mask, one u32 per set bit; then BuildDynamicValuesUpdate."""
    nb = r.u8()
    n = 0
    for _ in range(nb):
        n += mv_pop(r.u32())
    r.raw(4 * n)
    if r.u8():                       # dynamic block count (the server writes 1 or 0)
        tabmask = r.u32()
        for i in range(32):
            if tabmask & (1 << i):
                r.u8()
                r.raw(4 * mv_pop(r.u32()))


def mv_read_create_movement(r):
    """BuildMovementUpdate read back for the CREATE block of any object. Returns a dict (has_living, spline, pos, ...)."""
    out = {"living": 0, "spline": 0, "self": 0, "pos": None}
    r.bit()
    has_anim = r.bit()
    has_living = r.bit()
    r.bit(); r.bit()
    n_stop = r.bits(22)
    has_vehicle = r.bit()
    r.bit(); r.bit()
    has_transport = r.bit()
    has_rot = r.bit()
    r.bit()
    out["self"] = r.bit()
    has_target = r.bit()
    r.bit(); r.bit(); r.bit()
    has_at = r.bit()
    has_tpos = r.bit()
    r.bit()
    has_stat = r.bit()
    out["living"] = has_living
    ut = None
    sp = {"has": 0}
    if has_living:
        g = MvGuid()
        g.mask(r, 2)
        r.bit()
        has_pitch = not r.bit()
        has_ut = r.bit()
        r.bit()
        ut = None
        if has_ut:
            ut = MvGuid()
            ut.masks(r, (4, 2))
            t3 = r.bit()
            ut.masks(r, (0, 1, 3, 6, 7))
            t2 = r.bit()
            ut.mask(r, 5)
            ut_t2, ut_t3 = t2, t3
        no_time = r.bit()
        g.masks(r, (6, 4, 3))
        orient0 = r.bit()
        no_counter = r.bit()
        g.mask(r, 5)
        if r.bits(22):
            raise MvParseError("create block: forces")
        no_mflags = r.bit()
        if r.bits(19):
            raise MvParseError("create block: 19-bit block")
        has_fall = r.bit()
        mflags = 0 if no_mflags else r.bits(30)
        no_selev = r.bit()
        has_spline = r.bit()
        r.bit()
        g.masks(r, (0, 7, 1))
        if has_spline:
            sp["has"] = r.bit()                      # !Finalized
            if sp["has"]:
                sp["pa"] = r.bit()
                sp["parab"] = r.bit()
                r.bit()
                sp["n"] = r.bits(20)
                sp["mode"] = r.bits(2)
                sp["flags"] = r.bits(25)
        no_extra = r.bit()
        fall_dir = r.bit() if has_fall else 0
        if not no_extra:
            r.bits(13)
    tp = None
    if has_tpos:
        tp = MvGuid()
        tp.masks(r, (4, 1, 0))
        tp_t2 = r.bit()
        tp.masks(r, (6, 5, 3, 2, 7))
        tp_t3 = r.bit()
    at_spline = at_scale = 0
    if has_at:                                   # Object.cpp hasAreaTriggerData bits
        for _ in range(6):
            r.bit()
        at_scale = r.bit()
        r.bit()
        at_spline = r.bit()
        r.bit()
        at_n = r.bits(20) if at_spline else 0
        r.bit(); r.bit(); r.bit()
    if has_anim:
        kit_mv, kit_me, kit_ai = r.bit(), r.bit(), r.bit()
    victim = None
    if has_target:
        victim = MvGuid()
        victim.masks(r, (4, 6, 5, 2, 0, 1, 3, 7))
    r.flush()
    for _ in range(n_stop):
        r.u32()
    if has_living:
        if ut is not None:
            ut.seq(r, 7); r.f32()
            if ut_t3:
                r.u32()
            r.f32(); r.f32()
            ut.seqs(r, (4, 1, 3)); r.f32()
            ut.seq(r, 5)
            if ut_t2:
                r.u32()
            ut.seq(r, 0); r.i8(); ut.seqs(r, (6, 2)); r.u32()
        g.seq(r, 4)
        spd = {}
        if has_spline:
            if sp["has"]:
                out["passed"] = r.u32()
                r.f32(); r.f32()
                pts = []
                for _ in range(sp["n"]):
                    x = r.f32(); z = r.f32(); y = r.f32()
                    pts.append((x, y, z))
                if sp["pa"]:
                    r.u32()
                mtype = r.u8()
                if mtype == 4:
                    r.f32()
                elif mtype == 2:
                    r.f32(); r.f32(); r.f32()
                if sp["parab"]:
                    r.f32()
                out["dur"] = r.u32()
                out["pts"] = pts
                out["mtype"] = mtype
                out["sflags"] = sp["flags"]
            dx = r.f32(); dz = r.f32(); out["sid"] = r.u32(); dy = r.f32()
            out["dest"] = (dx, dy, dz)
            out["spline"] = 1 if sp["has"] else 0
        spd["flight"] = r.f32()
        if not no_counter:
            r.u32()
        g.seq(r, 2)
        if has_fall:
            if fall_dir:
                r.f32(); r.f32(); r.f32()
            r.u32(); r.f32()
        g.seq(r, 1)
        spd["turn"] = r.f32()
        if not no_time:
            r.u32()
        spd["runback"] = r.f32()
        if not no_selev:
            r.f32()
        g.seq(r, 7)
        spd["pitch"] = r.f32()
        px = r.f32()
        if has_pitch:
            r.f32()
        out["orient"] = 0.0 if orient0 else r.f32()
        spd["walk"] = r.f32()
        py = r.f32()
        spd["flightback"] = r.f32()
        g.seqs(r, (3, 5, 6, 0))
        spd["swimback"] = r.f32(); spd["run"] = r.f32(); spd["swim"] = r.f32()
        pz = r.f32()
        out["pos"] = (px, py, pz)
        out["mflags"] = mflags
        out["speeds"] = spd
        out["transport"] = ut.value() if ut is not None else 0
    if has_at:
        r.raw(12 * at_n)
        r.u32()
        if at_scale:
            r.f32(); r.f32()
    if has_tpos:
        if tp_t2:
            r.u32()
        r.f32(); r.i8(); r.f32()
        tp.seqs(r, (2, 4, 1))
        if tp_t3:
            r.u32()
        r.u32(); r.f32(); r.f32()
        tp.seqs(r, (6, 0, 5, 3, 7))
    if has_target:
        victim.seqs(r, (7, 1, 5, 2, 6, 3, 0, 4))
    if has_vehicle:
        out["vehicle_id"] = r.u32(); r.f32()
    if has_stat:
        sy, sz, so, sx = r.f32(), r.f32(), r.f32(), r.f32()
        out["stat"] = (sx, sy, sz, so)
    if has_anim:
        if not kit_ai:
            r.raw(2)
        if not kit_me:
            r.raw(2)
        if not kit_mv:
            r.raw(2)
    if has_transport:
        r.u32()
    if has_rot:
        r.raw(8)
    if has_living and out["spline"] and out.get("mtype") == 3:      # WriteFacingTargetPart
        fg = MvGuid()
        fg.masks(r, (4, 7, 0, 5, 1, 2, 3, 6))
        fg.seqs(r, (4, 2, 0, 5, 6, 3, 1, 7))
    return out


class MoveLog:
    """--log-movement: MOVEPKT lines for the movers selected by `spec` ('all' = every other player, 'units' = every
    unit, else a comma list of names and/or DB guid counters)."""

    def __init__(self, client, spec, out_path=""):
        self.w = client
        spec = (spec or "").strip()
        self.all_players = spec.lower() == "all"
        self.all_units = spec.lower() == "units"
        self.want_names, self.want_counters = set(), set()
        for tok in spec.split(","):
            tok = tok.strip()
            if not tok or tok.lower() in ("all", "units"):
                continue
            if tok.isdigit():
                self.want_counters.add(int(tok))
            else:
                self.want_names.add(tok.lower())
        self.fh = open(out_path, "a", buffering=1, encoding="utf-8") if out_path else None
        self.names = {}            # raw guid -> name
        self.queried = set()
        self.spl = {}              # raw guid -> _MvSpline (last SMSG_MONSTER_MOVE)
        self.stat = {}             # raw guid -> _MvStatus newer than the last spline, else absent
        self.speeds = {}           # raw guid -> {"run": f, "swim": f}
        self.counts = {}
        self.anom = {}             # name -> {kind: n}
        self.last_trk = 0.0        # wall time of the last TRACK sweep (MV_TRACK_S)

    # --- selection / output ------------------------------------------------
    @staticmethod
    def _is_player(guid):
        return ((guid >> 52) & 0xFFF) == MV_HIGH_PLAYER

    def _self_guid(self, guid):
        cg = self.w.char_guid
        return cg is not None and (guid == cg or (guid & 0xFFFFFFFF) == (cg & 0xFFFFFFFF) and self._is_player(guid))

    def wanted(self, guid):
        if self._self_guid(guid):
            return False
        if self.all_units:
            return True
        if not self._is_player(guid):
            return False
        if self.all_players:
            return True
        if (guid & 0xFFFFFFFF) in self.want_counters:
            return True
        name = self.names.get(guid)
        return bool(name) and name.lower() in self.want_names

    def _name(self, guid):
        if guid not in self.names and self._is_player(guid) and guid not in self.queried:
            self.queried.add(guid)
            try:
                self.w.send_packet(MV_CMSG_NAME_QUERY, mv_name_query_body(guid))
            except (OSError, RuntimeError):
                pass
        return self.names.get(guid, "?")

    def emit_transport(self, t, op, guid):
        """TRANSPORT_CREATE / TRANSPORT_OUT for a moving transport (not subject to the wanted filter)."""
        line = "MOVEPKT t=%d wall=%.3f op=%s guid=%d hi=motransport map=%s" % (
            int((t - self.w.login_time) * 1000), t, op, guid & 0xFFFFFFFF, self.w.map_id)
        if self.fh:
            self.fh.write(line + "\n")
        else:
            print(line, flush=True)

    def emit(self, t, op, guid, fields):
        name = self._name(guid)
        if not self.wanted(guid):
            return
        parts = ["MOVEPKT", "t=%d" % int((t - self.w.login_time) * 1000), "wall=%.3f" % t, "op=%s" % op,
                 "guid=%d" % (guid & 0xFFFFFFFF), "hi=%s" % ("player" if self._is_player(guid) else "unit"),
                 "name=%s" % name, "map=%s" % self.w.map_id]
        parts.extend("%s=%s" % kv for kv in fields)
        line = " ".join(parts)
        if self.fh:
            self.fh.write(line + "\n")
        else:
            print(line, flush=True)

    def _count(self, op):
        self.counts[op] = self.counts.get(op, 0) + 1

    def _anomaly(self, guid, kind):
        if not self.wanted(guid):
            return
        per = self.anom.setdefault(self.names.get(guid, str(guid & 0xFFFFFFFF)), {})
        per[kind] = per.get(kind, 0) + 1

    # --- client model ------------------------------------------------------
    def _model_pos(self, guid, t, spline_only=False):
        st = self.stat.get(guid)
        if st is not None and not spline_only:
            return st.pos_at(t)
        sp = self.spl.get(guid)
        return sp.pos_at(t) if sp is not None else None

    def _dr_speed(self, guid, mflags):
        sp = self.speeds.get(guid, {})
        if mflags & 0x100000:
            return sp.get("swim", MV_SWIM_SPEED_BASE)
        if "run" in sp:
            return sp["run"]
        s = self.spl.get(guid)
        return s.speed() if s is not None and s.dur > 0 else 7.0

    def _track(self, t):
        """#747: every MV_TRACK_S, one TRACK line per mover this client draws: where it draws it now (p), from what
        (src = spline: the last SMSG_MONSTER_MOVE / status: dead-reckoned movement status / hold), and where this
        client itself stands (me), so the evaluator can compare the drawn track with the server [PROBE] track and
        skip movers that are out of visibility range."""
        if t - self.last_trk < MV_TRACK_S:
            return
        self.last_trk = t
        me = self.w.pos
        mes = "%.1f,%.1f,%.1f" % tuple(me[:3]) if me else "-"
        for guid in list(set(self.spl) | set(self.stat)):
            if not self.wanted(guid):
                continue
            p = self._model_pos(guid, t)
            if p is None:
                continue
            st = self.stat.get(guid)
            if st is not None:
                src = "status" if (st.mflags & 0x1) and not (st.mflags & 0x400) else "hold"
            else:
                src = "spline" if t < self.spl[guid].end_t() else "hold"
            self.emit(t, "TRACK", guid, [("p", "%.1f,%.1f,%.1f" % p), ("src", src), ("me", mes)])

    # --- packets -----------------------------------------------------------
    def handle(self, op, body, t):
        self._track(t)
        if op == MV_SMSG_NAME_QUERY_RESPONSE:
            guid, name = mv_read_name_response(body)
            if name:
                self.names[guid] = name
            return
        if op == MV_SMSG_MONSTER_MOVE:
            self._monster_move(mv_read_monster_move(body), t)
        elif op in MV_STATUS_OPS:
            label, seq, speed_kind = MV_STATUS_OPS[op]
            self._status(label, mv_read_status(body, seq), speed_kind, t)
        elif op in MV_SPLINE_OPS:
            label, seq, speed_kind = MV_SPLINE_OPS[op]
            self._spline_op(label, mv_read_status(body, seq), speed_kind, t)
        elif op == MV_SMSG_MOVE_UPDATE_TELEPORT:
            self._teleport(mv_read_move_update_teleport(body), t)
        elif op == MV_SMSG_UPDATE_OBJECT:
            self._update_object(body, t)
            return
        else:
            return
        self._count(op)

    def _update_object(self, body, t):
        """#747 visibility, without a full SMSG_UPDATE_OBJECT parser: UpdateData::BuildPacket writes the out-of-range
        block FIRST (uint16 map, uint32 blocks, uint8 3, uint32 n, n packed guids), so every unit that leaves this
        client's view (range, stealth, phase) is seen - OUT_OF_RANGE, and this client stops drawing it. VISIBLE is logged
        for the first create block of a packet that has no out-of-range block.
        #126 (2026-10-21): the blocks after it are walked too (mv_read_create_movement + mv_skip_values), and every CREATE
        block of a wanted unit logs "op=CREATE spline=0|1 pos=x,y,z [dest= passed= dur= npts= sid= sflags= type=]" so a late
        observer's create block shows whether it carried the unit's move spline (Object.cpp:497-502). A create block that
        carries one also seeds the drawn track (TRACK lines advance along it). A decode error stops the walk only."""
        r = MvReader(body)
        r.raw(2)
        nblocks = r.u32()
        if nblocks == 0:
            return
        first = True
        for _ in range(nblocks):
            typ = r.u8()
            if typ == 3:
                for _ in range(r.u32()):
                    guid = mv_read_packed_guid(r)
                    if ((guid >> 52) & 0xFFF) == MV_HIGH_MO_TRANSPORT:
                        self.emit_transport(t, "TRANSPORT_OUT", guid)
                    if self.wanted(guid) and (guid in self.spl or guid in self.stat):
                        self.emit(t, "OUT_OF_RANGE", guid, [])
                    self.spl.pop(guid, None)
                    self.stat.pop(guid, None)
                self._count(MV_SMSG_UPDATE_OBJECT)
                first = False
            elif typ in (1, 2):
                guid = mv_read_packed_guid(r)
                r.u8()                                   # object type id
                if ((guid >> 52) & 0xFFF) == MV_HIGH_MO_TRANSPORT:
                    self.emit_transport(t, "TRANSPORT_CREATE", guid)
                if first and self._is_player(guid) and self.wanted(guid):
                    self.emit(t, "VISIBLE", guid, [("first", 1)])
                first = False
                mv = mv_read_create_movement(r)
                mv_skip_values(r)
                if mv["living"] and self.wanted(guid):
                    f = [("spline", mv["spline"]), ("self", mv["self"])]
                    if mv["pos"] is not None:
                        f.append(("pos", "%.1f,%.1f,%.1f" % mv["pos"]))
                    if mv["spline"]:
                        pts = mv.get("pts") or []
                        f.append(("dest", "%.1f,%.1f,%.1f" % mv["dest"]))
                        f.append(("passed", mv.get("passed", 0)))
                        f.append(("dur", mv.get("dur", 0)))
                        f.append(("npts", len(pts)))
                        f.append(("sid", mv.get("sid", 0)))
                        f.append(("sflags", "0x%08X" % mv.get("sflags", 0)))
                        f.append(("type", mv.get("mtype", 0)))
                    self.emit(t, "CREATE", guid, f)
                    self._count(MV_SMSG_UPDATE_OBJECT)
                if mv["living"] and mv["spline"] and mv.get("pts") and mv.get("dur", 0) > 0:
                    self.spl[guid] = _MvSpline(t - mv.get("passed", 0) / 1000.0, mv["pts"], mv["dur"])
                    self.stat.pop(guid, None)
            elif typ == 0:
                mv_read_packed_guid(r)
                mv_skip_values(r)
                first = False
            else:
                return

    def _monster_move(self, mm, t):
        guid = mm["guid"]
        pts, dur = mm["pts"], mm["duration"]
        start, end = pts[0], pts[-1]
        prev = self.spl.get(guid)
        snap = snap_s = None
        if guid in self.spl or guid in self.stat:
            m = self._model_pos(guid, t)
            snap = _mv_dist(m, start) if m is not None else None
            ms = self._model_pos(guid, t, spline_only=True)
            snap_s = _mv_dist(ms, start) if ms is not None else None
        prevrem = max(0, int((prev.end_t() - t) * 1000)) if prev is not None else 0
        sp = _MvSpline(t, pts, dur)
        gaps = [_mv_dist(a, b) for a, b in zip(pts, pts[1:])]
        maxgap = max(gaps) if gaps else 0.0
        wrap = 1 if mm["offsets"] and maxgap > 100.0 else 0
        dist = _mv_dist(start, end)
        far = max([max(abs(o[0]), abs(o[1])) for o in mm["offsets"]] or [0.0])
        fields = [("sid", mm["id"]), ("type", MV_MOVE_TYPES.get(mm["type"], mm["type"])),
                  ("s", "%.1f,%.1f,%.1f" % start), ("e", "%.1f,%.1f,%.1f" % end), ("npts", len(pts)),
                  ("dur", dur), ("sflags", "0x%08X" % mm["flags"]), ("sfl", _mv_flag_names(mm["flags"], MV_SPLINE_FLAGS)),
                  ("dist", "%.1f" % dist), ("plen", "%.1f" % sp.plen),
                  ("spd", "%.2f" % sp.speed()), ("sspd", "%.2f" % (dist / (dur / 1000.0) if dur > 0 else 0.0)),
                  ("maxgap", "%.1f" % maxgap), ("far", "%.1f" % far), ("wrap", wrap),
                  ("snap", "-" if snap is None else "%.1f" % snap), ("snapS", "-" if snap_s is None else "%.1f" % snap_s),
                  ("prevrem", prevrem), ("transport", "%d" % (mm["transport"] & 0xFFFFFFFF)),
                  # every point the client walks (start, decoded packed offsets, end): t747-movepkt-eval.py rebuilds the
                  # drawn track from these instead of a straight start->end line
                  ("pts", ";".join("%.1f,%.1f,%.1f" % p for p in pts))]
        self.emit(t, "MONSTER_MOVE", guid, fields)
        if wrap:
            self._anomaly(guid, "wrap")
        if snap is not None and snap > 20.0:
            self._anomaly(guid, "snap>20")
        if mm["flags"] & (0x200 | 0x800):
            self._anomaly(guid, "flyspline")
        self.spl[guid] = sp
        self.stat.pop(guid, None)

    def _status(self, label, mi, speed_kind, t):
        guid = mi["guid"]
        pos = (mi["x"], mi["y"], mi["z"])
        sp = self.spl.get(guid)
        mid = sp is not None and sp.dur > 0 and t < sp.end_t()
        rem = int((sp.end_t() - t) * 1000) if mid else 0
        dev = _mv_dist(pos, sp.pos_at(t)) if mid else None
        # snap: how far the unit jumps on this client when the status lands (model position just before -> packet)
        mpos = self._model_pos(guid, t) if (guid in self.spl or guid in self.stat) else None
        snap = _mv_dist(mpos, pos) if mpos is not None else None
        if speed_kind and mi["extra"] is not None:
            self.speeds.setdefault(guid, {})[speed_kind] = mi["extra"]
        fields = [("p", "%.1f,%.1f,%.1f" % pos), ("o", "%.2f" % mi["o"]), ("mflags", "0x%08X" % mi["mflags"]),
                  ("mfl", _mv_flag_names(mi["mflags"], MV_MOVEMENT_FLAGS)), ("midspline", 1 if mid else 0),
                  ("rem", rem), ("dev", "-" if dev is None else "%.1f" % dev),
                  ("srvms", "-" if mi["ts"] is None else mi["ts"]),
                  ("spd", "-" if mi["extra"] is None else "%.2f" % mi["extra"]),
                  ("snap", "-" if snap is None else "%.1f" % snap),
                  ("drspd", "%.2f" % self._dr_speed(guid, mi["mflags"]))]
        self.emit(t, label, guid, fields)
        if snap is not None and snap > 20.0:
            self._anomaly(guid, "snap>20")
        if mid and mi["mflags"] & 0x1:
            self._anomaly(guid, "status-mid-spline-forward")
        if mi["mflags"] & MV_FLY_FLAGS:
            self._anomaly(guid, "flyflag")
        self.stat[guid] = _MvStatus(t, pos, mi["o"], mi["mflags"], self._dr_speed(guid, mi["mflags"]))

    def _spline_op(self, label, mi, speed_kind, t):
        guid = mi["guid"]
        sp = self.spl.get(guid)
        mid = sp is not None and sp.dur > 0 and t < sp.end_t()
        rem = int((sp.end_t() - t) * 1000) if mid else 0
        splspd = sp.speed() if mid else 0.0
        if speed_kind and mi["extra"] is not None:
            self.speeds.setdefault(guid, {})[speed_kind] = mi["extra"]
        fast_swim = 1 if label == "SPLINE_START_SWIM" and mid and splspd > 1.6 * MV_SWIM_SPEED_BASE else 0
        fields = [("midspline", 1 if mid else 0), ("rem", rem), ("splspd", "%.2f" % splspd), ("fastswim", fast_swim),
                  ("spd", "-" if mi["extra"] is None else "%.2f" % mi["extra"])]
        self.emit(t, label, guid, fields)
        if fast_swim:
            self._anomaly(guid, "fastswim")
        if label == "SPLINE_SET_FLYING":
            self._anomaly(guid, "flyflag")

    def _teleport(self, mi, t):
        guid = mi["guid"]
        pos = (mi["x"], mi["y"], mi["z"])
        m = self._model_pos(guid, t)
        snap = _mv_dist(m, pos) if m is not None else None
        fields = [("p", "%.1f,%.1f,%.1f" % pos), ("o", "%.2f" % mi["o"]), ("mflags", "0x%08X" % mi["mflags"]),
                  ("mfl", _mv_flag_names(mi["mflags"], MV_MOVEMENT_FLAGS)),
                  ("snap", "-" if snap is None else "%.1f" % snap)]
        self.emit(t, "MOVE_UPDATE_TELEPORT", guid, fields)
        self.spl.pop(guid, None)
        self.stat[guid] = _MvStatus(t, pos, mi["o"], 0, 0.0)

    def summary(self):
        ops = " ".join("0x%04X=%d" % kv for kv in sorted(self.counts.items()))
        line = "MOVEPKT-SUMMARY packets: %s" % (ops or "none")
        bots = " ".join("%s:%s" % (n, ",".join("%s=%d" % kv for kv in sorted(k.items())))
                        for n, k in sorted(self.anom.items()))
        line2 = "MOVEPKT-SUMMARY anomalies: %s" % (bots or "none")
        for ln in (line, line2):
            if self.fh:
                self.fh.write(ln + "\n")
            print(ln, flush=True)


def attach_movement_log(w, spec, out_path=""):
    """Wrap WorldClient.auto: the client's own handling runs first, then the decode; a decode error never reaches
    the run (it is logged as MOVEPKT-ERR, at most 50 per opcode)."""
    mlog = MoveLog(w, spec, out_path)
    orig = w.auto
    errs = {}

    def auto(op, body):
        t = time.time()
        res = orig(op, body)
        try:
            mlog.handle(op, body, t)
        except Exception as exc:          # noqa: BLE001 - the capture must never kill the session
            errs[op] = errs.get(op, 0) + 1
            if errs[op] <= 50:
                log("MOVEPKT-ERR op=0x%04X size=%d %s: %s" % (op, len(body), type(exc).__name__, exc))
        return res

    w.auto = auto
    return mlog

# ===========================================================================
# Playing without GM help.
#
# Everything below works only from what a real 5.4.8 client receives and sends: objects from
# SMSG_UPDATE_OBJECT (Object::BuildCreateUpdateBlockForPlayer / BuildValuesUpdate, UpdateFields.h),
# the quest log from PLAYER_FIELD_QUEST_LOG, inventory from PLAYER_FIELD_INV_SLOTS + container slots,
# known spells from SMSG_INITIAL_SPELLS / SMSG_LEARNED_SPELL, and the quest/loot/cast replies. The
# real-player verbs (goto-path, goto-npc, kill, accept, turnin, ...) never call a GM/SOAP verb.
# Read-only lookups (spawn positions from the world DB, see db_spawns) are optional hints only.
# ===========================================================================

NAVPATH_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "navpath")

SMSG_DESTROY_OBJECT = 0x14C2
SMSG_MONSTER_MOVE = 0x1A07
SMSG_PLAYER_MOVE = 0x1A32
SMSG_MOVE_UPDATE_TELEPORT = 0x15A9
SMSG_INITIAL_SPELLS = 0x045A
SMSG_LEARNED_SPELL = 0x129A
SMSG_REMOVED_SPELL = 0x14C3
SMSG_SPELL_FAILURE = 0x04AF
SMSG_ATTACK_SWING_ERROR = 0x11E1
SMSG_QUESTGIVER_OFFER_REWARD = 0x074F
SMSG_QUESTGIVER_QUEST_COMPLETE = 0x0346
SMSG_QUESTGIVER_QUEST_DETAILS = 0x134C
SMSG_QUESTGIVER_QUEST_FAILED = 0x12DE
SMSG_QUESTGIVER_QUEST_INVALID = 0x027D
SMSG_QUESTGIVER_QUEST_LIST = 0x02D4
SMSG_QUESTGIVER_REQUEST_ITEMS = 0x0277
SMSG_QUESTGIVER_STATUS = 0x1275
SMSG_QUESTGIVER_STATUS_MULTIPLE = 0x06CE
SMSG_QUESTUPDATE_ADD_CREDIT = 0x1645
SMSG_QUESTUPDATE_ADD_CREDIT_SIMPLE = 0x07F5
SMSG_QUEST_UPDATE_COMPLETE = 0x0776
SMSG_QUESTUPDATE_FAILED = 0x07DD
SMSG_QUESTUPDATE_FAILEDTIMER = 0x06FF
SMSG_QUESTLOG_FULL = 0x07FD
SMSG_CORPSE_QUERY = 0x0E0B
SMSG_DEATH_RELEASE_LOC = 0x1063
SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA = 0x1A2A
SMSG_CLIENT_CONTROL_UPDATE = 0x1043
SMSG_MOVE_ROOT = 0x15AE
SMSG_MOVE_UNROOT = 0x1FAE
CMSG_QUESTGIVER_HELLO = 0x02DB
CMSG_QUESTGIVER_QUERY_QUEST = 0x12F0
CMSG_QUESTGIVER_REQUEST_REWARD = 0x0378
CMSG_QUESTGIVER_STATUS_QUERY = 0x036A
CMSG_CORPSE_QUERY = 0x1FBE
CMSG_PET_ACTION = 0x025B
CMSG_AUTOEQUIP_ITEM = 0x025F
CMSG_SELL_ITEM = 0x1358
CMSG_SET_PRIMARY_TALENT_TREE = 0x06C6       # u32 specialization tab (SkillHandler.cpp HandeSetTalentSpecialization)
# `spec`: the damage specialization a player picks at level 10 when the step does not name one (tab order of
# ChrSpecialization per class): Arms, Retribution, Beast Mastery, Assassination, Shadow, Frost, Elemental, Arcane,
# Affliction, Windwalker, Balance
DEFAULT_DPS_SPEC_TAB = {1: 0, 2: 2, 3: 0, 4: 0, 5: 2, 6: 1, 7: 0, 8: 0, 9: 0, 10: 1, 11: 0}
CMSG_REPAIR_ITEM = 0x02C1        # NPCHandler.cpp HandleRepairItemOpcode (item guid 0 = repair all)
CMSG_QUESTLOG_REMOVE_QUEST = 0x0779   # QuestHandler.cpp HandleQuestLogRemoveQuest: u8 log slot
CMSG_BUY_ITEM = 0x02E2          # ItemHandler.cpp HandleBuyItemOpcode
CMSG_LIST_INVENTORY = 0x02D8    # ItemHandler.cpp HandleListInventoryOpcode
CMSG_DESTROY_ITEM = 0x0026
CMSG_SPIRIT_HEALER_ACTIVATE = 0x0340
SMSG_SPIRIT_HEALER_CONFIRM = 0x1EAA
SMSG_SPELL_START = 0x107A
# Spells with SPELL_EFFECT_OPEN_LOCK a character knows from creation. A locked chest / goober (Lock.dbc
# LOCK_KEY_SKILL) is opened by casting the matching one on it, as the real client does; the server accepts it
# even when it is not in the spellbook (GameObject::GetSpellForLock, SpellHandler.cpp "special lock cases").
OPENING_SPELLS = (3365, 6478, 21651, 22810, 6477, 6247, 61437, 21288, 6246)
CMSG_REQUEST_VEHICLE_EXIT = 0x1DC3
SMSG_TRIGGER_CINEMATIC = 0x0B01
SMSG_TRIGGER_MOVIE = 0x1C2E
CMSG_NEXT_CINEMATIC_CAMERA = 0x1124
CMSG_COMPLETE_CINEMATIC = 0x1F34
CMSG_COMPLETE_MOVIE = 0x1362
SMSG_PET_SPELLS_MESSAGE = 0x095A
CINEMATIC_WATCH_S = 3.0          # seconds a cinematic / movie "plays" before this client reports it complete

# SharedDefines.h ResponseCodes (the SMSG_CHAR_CREATE result byte)
CHAR_CREATE_CODES = {
    46: "CHAR_CREATE_IN_PROGRESS", 47: "CHAR_CREATE_SUCCESS", 48: "CHAR_CREATE_ERROR", 49: "CHAR_CREATE_FAILED",
    50: "CHAR_CREATE_NAME_IN_USE", 51: "CHAR_CREATE_DISABLED", 52: "CHAR_CREATE_PVP_TEAMS_VIOLATION",
    53: "CHAR_CREATE_SERVER_LIMIT", 54: "CHAR_CREATE_ACCOUNT_LIMIT", 55: "CHAR_CREATE_SERVER_QUEUE",
    56: "CHAR_CREATE_ONLY_EXISTING", 57: "CHAR_CREATE_EXPANSION", 58: "CHAR_CREATE_EXPANSION_CLASS",
    59: "CHAR_CREATE_LEVEL_REQUIREMENT", 60: "CHAR_CREATE_UNIQUE_CLASS_LIMIT", 61: "CHAR_CREATE_CHARACTER_IN_GUILD",
    62: "CHAR_CREATE_RESTRICTED_RACECLASS", 63: "CHAR_CREATE_CHARACTER_CHOOSE_RACE",
    64: "CHAR_CREATE_CHARACTER_ARENA_LEADER", 65: "CHAR_CREATE_CHARACTER_DELETE_MAIL",
    66: "CHAR_CREATE_CHARACTER_SWAP_FACTION", 67: "CHAR_CREATE_CHARACTER_RACE_ONLY",
    68: "CHAR_CREATE_CHARACTER_GOLD_LIMIT", 69: "CHAR_CREATE_FORCE_LOGIN", 70: "CHAR_CREATE_TRIAL",
    92: "CHAR_NAME_SUCCESS", 93: "CHAR_NAME_FAILURE", 94: "CHAR_NAME_NO_NAME", 95: "CHAR_NAME_TOO_SHORT",
    96: "CHAR_NAME_TOO_LONG", 97: "CHAR_NAME_INVALID_CHARACTER", 98: "CHAR_NAME_MIXED_LANGUAGES",
    99: "CHAR_NAME_PROFANE", 100: "CHAR_NAME_RESERVED", 101: "CHAR_NAME_INVALID_APOSTROPHE",
    102: "CHAR_NAME_MULTIPLE_APOSTROPHES", 103: "CHAR_NAME_THREE_CONSECUTIVE", 104: "CHAR_NAME_INVALID_SPACE",
    105: "CHAR_NAME_CONSECUTIVE_SPACES", 106: "CHAR_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS",
    107: "CHAR_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END", 108: "CHAR_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME",
}

# SharedDefines.h SpellCastResult (the ones a fight loop reacts to)
AUTO_REPEAT_SPELLS = (75, 5019)   # Auto Shot, Shoot (wand)
SPELL_FAILED_NAMES = {
    1: "AFFECTING_COMBAT", 13: "BAD_TARGETS", 24: "CASTER_AURASTATE", 25: "CASTER_DEAD", 30: "EQUIPPED_ITEM",
    34: "ERROR", 39: "HIGHLEVEL", 43: "INTERRUPTED", 47: "ITEM_NOT_FOUND", 48: "ITEM_NOT_READY",
    50: "LINE_OF_SIGHT", 54: "MOVING", 58: "NEED_MORE_ITEMS", 60: "NOT_BEHIND", 67: "NOT_MOUNTED",
    70: "NOT_READY", 71: "NOT_SHAPESHIFT", 78: "NO_AMMO", 81: "NO_COMBO_POINTS", 87: "NO_PET", 88: "NO_POWER",
    89: "NOTHING_TO_DISPEL", 98: "ONLY_STEALTHED", 100: "OUT_OF_RANGE", 103: "REAGENTS",
    105: "REQUIRES_SPELL_FOCUS", 108: "SPELL_IN_PROGRESS", 110: "SPELL_UNAVAILABLE", 112: "TARGETS_DEAD",
    114: "TARGET_AURASTATE", 116: "TARGET_ENEMY", 118: "TARGET_FRIENDLY", 132: "TOO_CLOSE",
    137: "UNIT_NOT_BEHIND", 138: "UNIT_NOT_INFRONT", 185: "CANT_DO_THAT_RIGHT_NOW",
}
SPELL_FAILED_OUT_OF_RANGE = 100
SPELL_FAILED_LINE_OF_SIGHT = 50
SPELL_FAILED_UNIT_NOT_INFRONT = 138

# UpdateFields.h (5.4.8 build 18414)
UF_OBJECT_END = 0x008
UF_ENTRY = 0x005
UF_DYNAMIC_FLAGS = 0x006
UF_ITEM_OWNER = UF_OBJECT_END + 0x000
UF_ITEM_CONTAINED = UF_OBJECT_END + 0x002
UF_ITEM_STACK = UF_OBJECT_END + 0x008
UF_ITEM_END = UF_OBJECT_END + 0x03D
UF_CONTAINER_SLOTS = UF_ITEM_END + 0x000            # 36 slot guids
UF_CONTAINER_NUM_SLOTS = UF_ITEM_END + 0x048
UF_UNIT_CHARM = UF_OBJECT_END + 0x000
UF_UNIT_SUMMON = UF_OBJECT_END + 0x002
UF_UNIT_CHARMED_BY = UF_OBJECT_END + 0x006
UF_UNIT_TARGET = UF_OBJECT_END + 0x00E
UF_UNIT_HEALTH = UF_OBJECT_END + 0x019
UF_UNIT_POWER = UF_OBJECT_END + 0x01A               # 5 power slots, slot 0 = the class's primary power
UF_UNIT_MAX_HEALTH = UF_OBJECT_END + 0x01F
UF_UNIT_MAX_POWER = UF_OBJECT_END + 0x020
UF_UNIT_LEVEL = UF_OBJECT_END + 0x02F
UF_UNIT_FACTION = UF_OBJECT_END + 0x031
UF_UNIT_FLAGS = UF_OBJECT_END + 0x035
UF_UNIT_FLAGS2 = UF_OBJECT_END + 0x036
UF_UNIT_NPC_FLAGS = UF_OBJECT_END + 0x04F
UF_UNIT_END = UF_OBJECT_END + 0x098
UF_PLAYER_FLAGS = UF_UNIT_END + 0x002
UF_PLAYER_QUEST_LOG = UF_UNIT_END + 0x00B           # 50 slots x MAX_QUEST_OFFSET 15: id, state, 12 x (2 x u16 counts), time
UF_PLAYER_INV_SLOTS = UF_UNIT_END + 0x325           # 86 slot guids
UF_PLAYER_XP = UF_UNIT_END + 0x3DF
UF_PLAYER_NEXT_XP = UF_UNIT_END + 0x3E0
UF_GO_FLAGS = UF_OBJECT_END + 0x003
UF_GO_BYTES_1 = UF_OBJECT_END + 0x00A
UF_CORPSE_OWNER = UF_OBJECT_END + 0x000
QUEST_LOG_SLOTS = 50
QUEST_SLOT_FIELDS = 15
QUEST_STATE_COMPLETE = 0x1
QUEST_STATE_FAIL = 0x2
PLAYER_FLAGS_GHOST = 0x10
INVENTORY_SLOT_BAG_0 = 255
UNIT_FLAG_NON_ATTACKABLE = 0x2
UNIT_FLAG_IMMUNE_TO_PC = 0x100
UNIT_FLAG_IN_COMBAT = 0x80000
UNIT_FLAG_NOT_SELECTABLE = 0x02000000
UNIT_DYNFLAG_LOOTABLE = 0x2
UNIT_DYNFLAG_TAPPED = 0x8
UNIT_DYNFLAG_TAPPED_BY_PLAYER = 0x10
UNIT_DYNFLAG_DEAD = 0x40
GO_DYNFLAG_LO_ACTIVATE = 0x2
UNIT_NPC_FLAG_SPELLCLICK = 0x01000000
SPLINEFLAG_TRANSPORT_ENTER = 0x8000
SPLINEFLAG_TRANSPORT_EXIT = 0x10000
TYPEID_NAMES = {0: "object", 1: "item", 2: "container", 3: "unit", 4: "player", 5: "gameobject",
                6: "dynobj", 7: "corpse", 8: "areatrigger", 9: "sceneobject"}

# A simple starter rotation per class (ChrClasses id): (spellId, kind, castSeconds, cooldownSeconds).
# kind: m = melee (5 yd), r = ranged (30 yd), dot = ranged, re-applied after `cooldown` on the same target,
# auto = start once per target (Auto Shot). Only spells the character knows (SMSG_INITIAL_SPELLS /
# SMSG_LEARNED_SPELL) are used; a failed cast (SMSG_CAST_FAILED) backs the spell off for a few seconds.
# rage costs (5.4.8) of the warrior rotation spells that spend rage; UF_UNIT_POWER holds rage x10
WARRIOR_RAGE_COST = {78: 30, 1464: 25, 5308: 30}

CLASS_ROTATIONS = {
    1: [(12294, "m", 0, 6), (23881, "m", 0, 4.5), (23922, "m", 0, 6), (34428, "m", 0, 2), (6572, "m", 0, 9),
        (20243, "m", 0, 1.5), (5308, "m", 0, 1.5), (78, "m", 0, 1.5), (88161, "m", 0, 1.5), (1464, "m", 0, 1.5)],
    2: [(35395, "m", 0, 4.5), (20271, "r", 0, 6), (24275, "r", 0, 6), (53595, "m", 0, 4.5), (85256, "m", 0, 1.5),
        (879, "r", 0, 15)],
    3: [(75, "auto", 0, 0), (34026, "r", 0, 6), (53351, "r", 0, 10), (3044, "r", 0, 1.5), (77767, "r", 2.0, 1.5),
        (56641, "r", 2.0, 1.5)],
    4: [(2098, "m", 0, 1.5), (1329, "m", 0, 1.5), (84617, "m", 0, 1.5), (1752, "m", 0, 1.5)],
    5: [(589, "dot", 0, 15), (8092, "r", 1.5, 8), (47540, "r", 2.0, 9), (14914, "r", 1.5, 10), (15407, "r", 3.0, 1.5),
        (585, "r", 1.5, 1.5)],
    6: [(45477, "r", 0, 3), (45462, "m", 0, 3), (49998, "m", 0, 3), (55050, "m", 0, 3), (49020, "m", 0, 3),
        (49143, "m", 0, 1.5), (47541, "r", 0, 1.5)],
    7: [(8050, "dot", 0, 18), (73899, "m", 0, 8), (17364, "m", 0, 8), (8042, "r", 0, 6), (51505, "r", 2.0, 8),
        (403, "r", 2.0, 1.5)],
    8: [(2136, "r", 0, 8), (30455, "r", 0, 1.5), (44614, "r", 2.75, 1.5), (133, "r", 2.25, 1.5), (116, "r", 2.0, 1.5),
        (30451, "r", 2.25, 1.5)],
    9: [(172, "dot", 0, 18), (348, "dot", 1.5, 15), (29722, "r", 2.0, 1.5), (686, "r", 2.0, 1.5), (77799, "r", 0, 1.5)],
    10: [(107428, "m", 0, 8), (100784, "m", 0, 1.5), (100787, "m", 0, 1.5), (100780, "m", 0, 1.5)],
    11: [(8921, "dot", 0, 15), (2912, "r", 2.7, 1.5), (5176, "r", 2.0, 1.5)],
}
RANGED_CLASSES = {3, 5, 7, 8, 9, 11}
MELEE_RANGE = 4.5
NAV_Z_LIFT = 0.5
SPELL_RANGE = 29.0

_NAV_MODULE = None


def nav_module():
    """The navmesh pathfinder (navpath/nav.py: find_path(map_id, start_xyz, end_xyz) -> [(x,y,z), ...] or None),
    imported once. Missing module = None: callers walk a straight line and log NAV-FALLBACK."""
    global _NAV_MODULE
    if _NAV_MODULE is None:
        try:
            if NAVPATH_DIR not in sys.path:
                sys.path.insert(0, NAVPATH_DIR)
            import nav as _nav            # noqa: E402 - optional module, imported on first use
            _NAV_MODULE = _nav
            log("NAV module loaded from %s" % getattr(_nav, "__file__", NAVPATH_DIR))
        except Exception as exc:          # noqa: BLE001 - any import problem means "no navmesh"
            _NAV_MODULE = False
            log("NAV-FALLBACK navmesh module unavailable (%s: %s); walking straight lines" % (type(exc).__name__, exc))
    return _NAV_MODULE or None


def db_spawns(kind, entry, map_id):
    """Optional read-only hint: spawn positions of a creature/gameobject entry on `map_id` from the world DB, via the
    mysql command-line client. Needs MOP_MYSQL_BIN (path of mysql) and MOP_DB_HOST / MOP_DB_PORT / MOP_DB_USER /
    MOP_DB_PASS / MOP_DB_WORLD; returns [] when they are not set or the query fails. Never writes."""
    mysql = os.environ.get("MOP_MYSQL_BIN")
    db = os.environ.get("MOP_DB_WORLD")
    if not mysql or not db:
        return []
    table = "creature" if kind == "unit" else "gameobject"
    query = "SELECT position_x, position_y, position_z FROM %s WHERE id=%d AND map=%d" % (table, int(entry), int(map_id))
    cmd = [mysql, "-h", os.environ.get("MOP_DB_HOST", "127.0.0.1"), "-P", os.environ.get("MOP_DB_PORT", "3306"),
           "-u", os.environ.get("MOP_DB_USER", ""), "-N", "-B", db, "-e", query]
    env = dict(os.environ)
    if os.environ.get("MOP_DB_PASS") is not None:
        env["MYSQL_PWD"] = os.environ["MOP_DB_PASS"]
    try:
        out = subprocess.run(cmd, capture_output=True, text=True, timeout=15, env=env).stdout
    except (OSError, subprocess.SubprocessError) as exc:
        log("db_spawns: lookup failed (%s)" % exc)
        return []
    pts = []
    for line in out.splitlines():
        try:
            pts.append(tuple(float(v) for v in line.split("\t")[:3]))
        except ValueError:
            pass
    return pts


def db_items(entries):
    """Optional read-only item facts for gear-up , the way a real client knows them from its item cache:
    {entry: (class, subclass, InventoryType, ItemLevel, RequiredLevel, ContainerSlots, Quality, SellPrice)} from item_template, via the
    same MOP_MYSQL_BIN / MOP_DB_* settings as db_spawns. {} when they are not set. Never writes."""
    mysql = os.environ.get("MOP_MYSQL_BIN")
    db = os.environ.get("MOP_DB_WORLD")
    ids = sorted({int(e) for e in entries if int(e) > 0})
    if not mysql or not db or not ids:
        return {}
    query = ("SELECT entry, class, subclass, InventoryType, ItemLevel, RequiredLevel, ContainerSlots, Quality, SellPrice FROM item_template "
             "WHERE entry IN (%s)" % ",".join(str(i) for i in ids))
    cmd = [mysql, "-h", os.environ.get("MOP_DB_HOST", "127.0.0.1"), "-P", os.environ.get("MOP_DB_PORT", "3306"),
           "-u", os.environ.get("MOP_DB_USER", ""), "-N", "-B", db, "-e", query]
    env = dict(os.environ)
    if os.environ.get("MOP_DB_PASS") is not None:
        env["MYSQL_PWD"] = os.environ["MOP_DB_PASS"]
    try:
        out = subprocess.run(cmd, capture_output=True, text=True, timeout=15, env=env).stdout
    except (OSError, subprocess.SubprocessError) as exc:
        log("db_items: lookup failed (%s)" % exc)
        return {}
    res = {}
    for line in out.splitlines():
        try:
            v = [int(x) for x in line.split("\t")[:9]]
        except ValueError:
            continue
        res[v[0]] = tuple(v[1:])
    return res


# gear-up: InventoryType -> candidate equipment slots (5.4.8: ranged weapons go in the main hand)
GEAR_SLOTS = {1: (0,), 2: (1,), 3: (2,), 5: (4,), 20: (4,), 6: (5,), 7: (6,), 8: (7,), 9: (8,), 10: (9,),
              11: (10, 11), 12: (12, 13), 16: (14,), 13: (15,), 17: (15,), 21: (15,), 15: (15,), 26: (15,),
              25: (15,), 22: (16,), 14: (16,), 23: (16,)}

def po_read_values(r):
    """BuildValuesUpdate read back: u8 mask block count, the mask (bit i = field i), one u32 per set bit in index
    order; then BuildDynamicValuesUpdate (skipped). Returns {field index: u32}."""
    nb = r.u8()
    mask = r.raw(4 * nb)
    vals = {}
    for bi, b in enumerate(mask):
        if b:
            for bit in range(8):
                if b & (1 << bit):
                    vals[bi * 8 + bit] = struct.unpack("<I", r.raw(4))[0]
    if r.u8():
        tabmask = r.u32()
        for i in range(32):
            if tabmask & (1 << i):
                n = r.u8() & 0x7F
                cnt = 0
                for _ in range(n):
                    cnt += mv_pop(r.u32())
                r.raw(4 * cnt)
    return vals


def _d3(a, b):
    return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2)


def _d2(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


class TrackedObject:
    """One object this client has been told about (create block + later values / movement packets)."""

    def __init__(self, guid, typeid):
        self.guid = guid
        self.typeid = typeid
        self.fields = {}
        self.pos = None
        self.o = 0.0
        self.spline = None
        self.speeds = {}
        self.seen = time.time()

    @property
    def kind(self):
        return {3: "unit", 4: "player", 5: "go", 7: "corpse", 1: "item", 2: "item"}.get(self.typeid, "other")

    def u32(self, idx, default=0):
        return self.fields.get(idx, default)

    def u64(self, idx):
        return self.fields.get(idx, 0) | (self.fields.get(idx + 1, 0) << 32)

    @property
    def entry(self):
        e = self.fields.get(UF_ENTRY)
        if e:
            return e
        if (self.guid >> 52) in (0xF11, 0xF13, 0xF15, 0xF14):
            return (self.guid >> 32) & 0xFFFFF
        return 0

    @property
    def health(self):
        return self.fields.get(UF_UNIT_HEALTH, 0)

    @property
    def max_health(self):
        return self.fields.get(UF_UNIT_MAX_HEALTH, 0)

    @property
    def dynflags(self):
        return self.fields.get(UF_DYNAMIC_FLAGS, 0)

    @property
    def unit_flags(self):
        return self.fields.get(UF_UNIT_FLAGS, 0)

    def alive(self):
        return self.health > 0 and not (self.dynflags & UNIT_DYNFLAG_DEAD)

    def attackable(self):
        f = self.unit_flags
        return not (f & (UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC))

    def tapped_by_other(self):
        d = self.dynflags
        return bool(d & UNIT_DYNFLAG_TAPPED) and not (d & UNIT_DYNFLAG_TAPPED_BY_PLAYER)

    def position(self, t=None):
        if self.spline is not None:
            return self.spline.pos_at(time.time() if t is None else t)
        return self.pos

    def short(self):
        p = self.position()
        ps = "%.1f,%.1f,%.1f" % p if p else "-"
        extra = ""
        if self.typeid in (3, 4):
            extra = " hp=%d/%d lvl=%d flags=0x%X dyn=0x%X npc=0x%X" % (
                self.health, self.max_health, self.u32(UF_UNIT_LEVEL), self.unit_flags, self.dynflags,
                self.u32(UF_UNIT_NPC_FLAGS))
        elif self.typeid == 5:
            extra = " dyn=0x%X state=%d" % (self.dynflags, self.u32(UF_GO_BYTES_1) & 0xFF)
        return "guid=%d type=%s entry=%d pos=%s%s" % (self.guid, TYPEID_NAMES.get(self.typeid, self.typeid),
                                                      self.entry, ps, extra)


class PlayState:
    """Client-side world model and the real-player step verbs. Attached to a WorldClient as `w.play`."""

    def __init__(self, w):
        self.w = w
        self.objs = {}
        self.known_spells = set()
        self.quest_prev = None          # qid -> (state, counts) at the last own-player update
        self.quest_required = {}        # qid -> {count_value: required} from SMSG_QUESTUPDATE_ADD_CREDIT
        self.quest_credit = {}          # (qid, objectId) -> (count, required)
        self.quest_rewarded = set()
        self.quest_completed_pkt = set()
        self.quest_failed = set()
        self.quest_details = None       # last SMSG_QUESTGIVER_QUEST_DETAILS quest id
        self.offer = None               # last SMSG_QUESTGIVER_OFFER_REWARD {quest, choices}
        self.request_items = None       # last SMSG_QUESTGIVER_REQUEST_ITEMS {quest, can_complete}
        self.quest_invalid = None
        self.qg_status = {}             # guid -> SMSG_QUESTGIVER_STATUS value
        self.cast_fail = []             # (t, spell, result)
        self.swing_error = None         # (t, code)
        self.corpse_pos = None          # (map, x, y, z) from SMSG_CORPSE_QUERY
        self.vehicle = False
        self.vehicle_guid = None
        self.vehicle_t = 0.0
        self.rooted = False
        self.pos_corrected = 0          # bumps on every server-side change of this client's position
        self.own_spline = None          # (t_end, end_pos) of a server spline moving this character
        self.errors = {}
        self.parsed_updates = 0
        self.auto_shot = set()
        self.rest_s = float(os.environ.get("HEADLESS_REST_S", "45"))   # kill: max out-of-combat rest before a pull (0 = off)
        self.spell_cd = {}
        self.gcd_until = 0.0
        self.dot_until = {}
        self.char_class = None
        self.used_go = {}               # guid -> time (collect from GOs: do not reuse a just-used node)
        self._nav_fallback_logged = False
        self.cine_complete_at = 0.0
        self.movie_complete_at = 0.0
        self.action_bar = []
        self.action_bar_t = 0.0
        self.used_targets = {}          # (verb, entry) -> guids already used (distinct-target selection)
        self.extra_move_flags = 0       # MOVEMENTFLAG_SWIMMING while a `swim=1` walk runs
        self.last_seen = {}             # (kind, entry) -> {guid: (map, pos)} of objects that left this client's view
        self.cur_near = None            # the running verb's near=x,y,z hint (goto_object uses it when no target is in view)
        self.spirit_confirm = None      # time of the last SMSG_SPIRIT_HEALER_CONFIRM
        self.spirit_healer_pos = None   # (map, x, y, z) from SMSG_DEATH_RELEASE_LOC
        self.spell_seen_t = 0.0         # last SMSG_SPELL_START / SMSG_SPELL_GO
        self.max_deaths = 3             # a kill/collect verb that dies more often than this logs STUCK (deaths=N)
        self.max_dry_kills = 10         # collect from=: kills without the item count changing before STUCK (dry=N)

    # ------------------------------------------------------------------ helpers
    def me(self):
        return self.objs.get(self.w.char_guid)

    def run_speed(self):
        me = self.me()
        spd = me.speeds.get("run") if me else None
        return spd if spd and 1.0 < spd < 60.0 else 7.0

    def is_dead(self):
        me = self.me()
        if me is None:
            return False
        return (me.max_health > 0 and me.health == 0) or bool(me.u32(UF_PLAYER_FLAGS) & PLAYER_FLAGS_GHOST)

    def is_ghost(self):
        me = self.me()
        return bool(me and me.u32(UF_PLAYER_FLAGS) & PLAYER_FLAGS_GHOST)

    def _forget(self, guid):
        """Drop an object, remembering where it was last seen (a player remembers where the quest giver stood)."""
        o = self.objs.pop(guid, None)
        if o is not None and o.kind in ("unit", "go") and o.position():
            per = self.last_seen.setdefault((o.kind, o.entry), {})
            per[guid] = (self.w.map_id, o.position())
            if len(per) > 20:
                per.pop(next(iter(per)))

    def remembered(self, entry, kinds):
        """Last-seen positions (this map) of objects of `entry` that are out of view, nearest first."""
        pts = []
        for k in kinds:
            for _g, (m, p) in self.last_seen.get((k, int(entry)), {}).items():
                if m == self.w.map_id:
                    pts.append(p)
        return sorted(pts, key=lambda p: _d3(p, self.w.pos)) if self.w.pos else pts

    def objects_of(self, entry, kinds):
        return [o for o in self.objs.values() if o.entry == int(entry) and o.kind in kinds and o.position()]

    def nearest(self, entry, kinds, pred=None, exclude=()):
        best, bd = None, 1e18
        me = self.w.pos
        if me is None:
            return None
        # never pick the vehicle this character sits in (riding a Mountain Horse 36540 and lassoing other
        # 36540s: the nearest one was always the own mount, OUT_OF_RANGE / no credit)
        own = (self.vehicle_guid, getattr(self.w, "move_guid", None)) if getattr(self, "vehicle", False) else ()
        for o in self.objects_of(entry, kinds):
            if o.guid in exclude or (own and o.guid in own) or (pred and not pred(o)):
                continue
            d = _d3(o.position(), me)
            if d < bd:
                best, bd = o, d
        return best

    def attackers(self):
        """Live units whose target is this character and that are in combat (fight back)."""
        me = self.w.char_guid
        out = []
        for o in self.objs.values():
            if o.typeid == 3 and o.alive() and o.u64(UF_UNIT_TARGET) == me and o.unit_flags & UNIT_FLAG_IN_COMBAT:
                if o.position() and self.w.pos and _d3(o.position(), self.w.pos) < 40:
                    out.append(o)
        # a unit that hit this character in the last 4 s is an attacker even when the 40 yd check (own position
        # out of date after a server spline) or its target field says otherwise - a careful rest sat through 40 hits
        seen = {o.guid for o in out}
        now = time.time()
        for t, att, vic, _dmg in getattr(self.w, "melee_log", [])[-60:]:
            if vic == me and now - t <= 4.0 and att not in seen:
                o = self.objs.get(att)
                if o is not None and o.typeid == 3 and o.alive():
                    out.append(o)
                    seen.add(att)
        return out

    def too_strong(self, o):
        """careful: a unit with more than 8x this character's health (a quest elite such as Koroth the Hillbreaker,
        7410 hp against 484) is not fought back - a player runs from it and lets the quest script handle it."""
        me_ = self.me()
        if not (getattr(self, "careful", False) and me_ is not None and me_.max_health > 0
                and o.max_health > 8 * me_.max_health):
            return False
        # owner 2026-11-15: an elite that a friendly NPC is tanking (Gorerot on Prince Liam in the 24904 battle) is
        # helped down like a player would - only when its victim is an ally, never when it is on this character
        tv = o.u64(UF_UNIT_TARGET)
        # a unit that hit this character in the last 6 s (melee or spell) is on it, whatever its target field says
        if tv and tv != self.w.char_guid and not self.hit_by(o.guid, 6.0):
            ally = self.objs.get(tv)
            if ally is not None and ally.typeid == 3 and ally.alive() and not ally.attackable():
                return False
        return True

    def pick_attacker(self, att):
        """which attacker to fight first, like a player keeps hitting the mob already on them: the unit this
        character last swung at, else the most damaged one, else the nearest (a hold left a Slavedriver at 8 hp and
        the next kill step turned to a fresh one at full health, and the character died between the two)."""
        last = getattr(self.w, "last_attack_guid", None)
        for a in att:
            if last is not None and a.guid == last:
                return a

        def frac(a):
            return (a.health / float(a.max_health)) if getattr(a, "max_health", 0) else 1.0
        return min(att, key=lambda a: (round(frac(a), 2), _d3(a.position(), self.w.pos)))

    def in_avoid(self, pt, avoid):
        """follow avoid=: the live unit of an avoided entry whose radius contains `pt` (2D), else None."""
        for e, r in avoid:
            for o in self.objects_of(e, ("unit",)):
                op = o.position()
                if o.alive() and op and math.hypot(pt[0] - op[0], pt[1] - op[1]) < r:
                    return (o, r)
        return None

    def avoid_point(self, pt, avoid, margin=2.0):
        """follow avoid=: `pt` moved out to the nearest point just outside every avoided unit's radius (straight
        out from the unit; from the unit toward this character when `pt` is the unit's own spot)."""
        for _k in range(4):
            hit = self.in_avoid(pt, avoid)
            if hit is None:
                return pt
            o, r = hit
            op = o.position()
            vx, vy = pt[0] - op[0], pt[1] - op[1]
            if math.hypot(vx, vy) < 0.5 and self.w.pos:
                vx, vy = self.w.pos[0] - op[0], self.w.pos[1] - op[1]
            vl = math.hypot(vx, vy) or 1.0
            pt = (op[0] + (r + margin) * vx / vl, op[1] + (r + margin) * vy / vl, pt[2])
        return pt

    def camp_foes(self, center, radius, entries=()):
        """positions of the live hostile units within `radius` yd of `center` (units of `entries` when given, else
        every attackable unit without NPC flags above level 1)."""
        out = []
        for o in self.objs.values():
            if o.typeid != 3 or not o.alive() or not o.attackable() or not o.position():
                continue
            if entries:
                if o.entry not in entries:
                    continue
            elif o.u32(UF_UNIT_NPC_FLAGS) or o.u32(UF_UNIT_LEVEL) <= 1:
                continue
            if _d3(o.position(), center) < radius:
                out.append(o.position())
        return out

    def quiet_point(self, center, foes, rings=(30.0,), n=16):
        """the point on a ring around `center` farthest from every foe position. Returns (point, nearest-foe yd)."""
        best_pt, best_d = None, -1.0
        for r in rings:
            for k in range(n):
                ang = k * 2.0 * math.pi / n
                pt = (center[0] + r * math.cos(ang), center[1] + r * math.sin(ang), center[2])
                dmin = min(math.hypot(pt[0] - f[0], pt[1] - f[1]) for f in foes) if foes else 1e9
                if dmin > best_d:
                    best_pt, best_d = pt, dmin
        return best_pt, best_d

    def hit_by(self, guid, secs=4.0):
        """`guid` hit this character in the last `secs` seconds."""
        me = self.w.char_guid
        now = time.time()
        return any(att == guid and vic == me and now - t <= secs for t, att, vic, _d in getattr(self.w, "melee_log", [])[-60:])

    def hit_recently(self, secs=3.0):
        """this character took a melee hit in the last `secs` seconds."""
        me = self.w.char_guid
        now = time.time()
        return any(vic == me and now - t <= secs for t, _a, vic, _d in getattr(self.w, "melee_log", [])[-60:])

    def quest_log(self):
        me = self.me()
        out = {}
        if me is None:
            return out
        for s in range(QUEST_LOG_SLOTS):
            base = UF_PLAYER_QUEST_LOG + s * QUEST_SLOT_FIELDS
            qid = me.fields.get(base, 0)
            if not qid:
                continue
            counts = []
            for i in range(12):
                v = me.fields.get(base + 2 + i, 0)
                counts += [v & 0xFFFF, v >> 16]
            out[qid] = {"slot": s, "state": me.fields.get(base + 1, 0), "counts": counts}
        return out

    def quest_status(self, qid):
        """none | incomplete | complete | failed | rewarded (rewarded = the reward packet came and it left the log)."""
        qid = int(qid)
        log_ = self.quest_log()
        if qid in log_:
            st = log_[qid]["state"]
            if st & QUEST_STATE_FAIL:
                return "failed"
            if st & QUEST_STATE_COMPLETE or qid in self.quest_completed_pkt:
                return "complete"
            return "incomplete"
        if qid in self.quest_rewarded:
            return "rewarded"
        return "none"

    def quest_state_str(self, qid):
        if not qid:
            return "-"
        qid = int(qid)
        st = self.quest_status(qid)
        q = self.quest_log().get(qid)
        cnt = ""
        if q:
            nz = [c for c in q["counts"] if c]
            if nz:
                cnt = "(" + ",".join(str(c) for c in nz) + ")"
        return "%d:%s%s" % (qid, st, cnt)

    def inventory(self):
        """[(bag, slot, item guid, entry, count)] for the backpack and the four bags (CMSG_USE_ITEM bag/slot)."""
        me = self.me()
        items = []
        if me is None:
            return items
        for slot in range(23, 39):
            g = me.u64(UF_PLAYER_INV_SLOTS + 2 * slot)
            if g:
                it = self.objs.get(g)
                items.append((INVENTORY_SLOT_BAG_0, slot, g, it.entry if it else 0,
                              max(1, it.u32(UF_ITEM_STACK)) if it else 1))
        for bslot in range(19, 23):
            bg = me.u64(UF_PLAYER_INV_SLOTS + 2 * bslot)
            bag = self.objs.get(bg) if bg else None
            if bag is None:
                continue
            for s in range(min(36, bag.u32(UF_CONTAINER_NUM_SLOTS))):
                g = bag.u64(UF_CONTAINER_SLOTS + 2 * s)
                if g:
                    it = self.objs.get(g)
                    items.append((bslot, s, g, it.entry if it else 0, max(1, it.u32(UF_ITEM_STACK)) if it else 1))
        return items

    def item_count(self, entry):
        return sum(c for _b, _s, _g, e, c in self.inventory() if e == int(entry))

    def free_bag_slots(self):
        """empty slots in the backpack (16) and the equipped bags."""
        me = self.me()
        if me is None:
            return 0
        total = 16
        for bslot in range(19, 23):
            bg = me.u64(UF_PLAYER_INV_SLOTS + 2 * bslot)
            bag = self.objs.get(bg) if bg else None
            if bag is not None:
                total += min(36, bag.u32(UF_CONTAINER_NUM_SLOTS))
        return total - len(self.inventory())

    def _sell_item(self, vendor_guid, g, c):
        """CMSG_SELL_ITEM (ItemHandler.cpp): u32 count, then the vendor/item guid bits and bytes interleaved."""
        w = self.w
        vb, ib = int(vendor_guid).to_bytes(8, "little"), int(g).to_bytes(8, "little")
        bw = BitWriter()
        for which, idx in (("i", 4), ("i", 3), ("i", 7), ("v", 6), ("v", 5), ("v", 1), ("i", 5), ("i", 2),
                           ("i", 1), ("v", 2), ("i", 6), ("v", 4), ("v", 0), ("v", 7), ("v", 3), ("i", 0)):
            bw.bit((vb if which == "v" else ib)[idx] != 0)
        body = bytearray(struct.pack("<I", c) + bw.flush())
        for which, idx in (("v", 6), ("v", 3), ("v", 1), ("i", 1), ("v", 2), ("i", 7), ("i", 5), ("v", 7),
                           ("i", 2), ("v", 0), ("v", 5), ("i", 3), ("i", 6), ("v", 4), ("i", 4), ("i", 0)):
            v = (vb if which == "v" else ib)[idx]
            if v:
                body.append(v ^ 1)
        w.send_packet(CMSG_SELL_ITEM, bytes(body))
        w.pump(0.3)

    def find_item(self, entry):
        for b, s, g, e, c in self.inventory():
            if e == int(entry):
                return b, s, g
        return None

    def pos_str(self):
        p = self.w.pos or (0.0, 0.0, 0.0)
        return "%s %.1f %.1f %.1f" % (self.w.map_id, p[0], p[1], p[2])

    def stuck(self, verb, args, reason, quest=None):
        try:
            self.w.move_stop()
        except Exception:                  # noqa: BLE001 - best effort before exiting
            pass
        log("STUCK verb=%s args=%s reason=%s pos=%s quest=%s"
            % (verb, ",".join(args) or "-", str(reason).replace(" ", "_"), self.pos_str(), self.quest_state_str(quest)))
        raise SystemExit(3)

    def ok(self, verb, args, extra=""):
        log("OK %s %s%s" % (verb, " ".join(args), (" " + extra) if extra else ""))

    # ------------------------------------------------------------------ packets
    def on_packet(self, op, body):
        try:
            self._on_packet(op, body)
        except Exception as exc:          # noqa: BLE001 - a decode error must never end the session
            n = self.errors.get(op, 0) + 1
            self.errors[op] = n
            if n <= 20:
                log("PLAY-PARSE-ERR op=0x%04X size=%d %s: %s" % (op, len(body), type(exc).__name__, exc))
        # a server spline moving this character (knockback, charge, vehicle exit): adopt its end when it is done
        if self.own_spline is not None and time.time() >= self.own_spline[0]:
            self.w.pos = self.own_spline[1]
            self.own_spline = None
            self.pos_corrected += 1

    def _on_packet(self, op, body):
        w = self.w
        if op == SMSG_UPDATE_OBJECT:
            self._update_object(body)
        elif op == SMSG_COMPRESSED_UPDATE_OBJECT:
            self._update_object(zlib.decompress(body[4:]))
        elif op == SMSG_DESTROY_OBJECT:
            r = MvReader(body)
            g = MvGuid()
            g.masks(r, (3, 2, 4, 1))
            on_death = r.bit()
            g.masks(r, (7, 0, 6, 5))
            r.flush()
            g.seqs(r, (0, 4, 7, 2, 6, 3, 1, 5))
            self._forget(g.value())
            if on_death:
                pass
        elif op == SMSG_MONSTER_MOVE:
            mm = mv_read_monster_move(body)
            g = mm["guid"]
            if g == w.char_guid:
                self._own_spline(mm)
            o = self.objs.get(g)
            if o is not None and not mm["transport"]:
                o.spline = _MvSpline(time.time(), mm["pts"], mm["duration"]) if mm["duration"] > 0 else None
                o.pos = mm["pts"][-1]
        elif op == SMSG_PLAYER_MOVE:
            mi = mv_read_status(body, MV_SEQ_PLAYER_MOVE)
            o = self.objs.get(mi["guid"])
            if o is not None and not mi["transport"]:
                o.pos, o.o, o.spline = (mi["x"], mi["y"], mi["z"]), mi["o"], None
        elif op == SMSG_MOVE_UPDATE_TELEPORT:
            mi = mv_read_move_update_teleport(body)
            o = self.objs.get(mi["guid"])
            if o is not None:
                o.pos, o.o, o.spline = (mi["x"], mi["y"], mi["z"]), mi["o"], None
        elif op == SMSG_MOVE_TELEPORT:
            self._move_teleport(body)
        elif op == SMSG_NEW_WORLD and len(body) >= 20:
            x, mapid, y, z, o = struct.unpack("<fIfff", body[:20])
            w.pos, w.orientation = (x, y, z), o
            self.pos_corrected += 1
            keep = {g: ob for g, ob in self.objs.items() if g == w.char_guid or ob.typeid in (1, 2)}
            self.objs = keep
            log("[MOVE] worldport map=%d pos=(%.1f, %.1f, %.1f)" % (mapid, x, y, z))
        elif op == SMSG_INITIAL_SPELLS:
            r = ByteReader(body)
            r.read_bit()
            n = r.read_bits(22)
            self.known_spells = set(r.read_u32() for _ in range(n))
            rot = [s for s, _k, _c, _d in CLASS_ROTATIONS.get(self.char_class or 0, []) if s in self.known_spells]
            log("[SPELLS] initial known=%d rotation=%s" % (len(self.known_spells), rot or "-"))
        elif op == SMSG_LEARNED_SPELL:
            r = ByteReader(body)
            n = r.read_bits(22)
            r.read_bit()
            new = [r.read_u32() for _ in range(n)]
            self.known_spells.update(new)
            log("[SPELLS] learned %s" % new)
        elif op == SMSG_REMOVED_SPELL:
            r = ByteReader(body)
            n = r.read_bits(22)
            gone = [r.read_u32() for _ in range(n)]
            self.known_spells.difference_update(gone)
        elif op == SMSG_CAST_FAILED and len(body) >= 8:
            spell, res = struct.unpack("<II", body[:8])
            self.cast_fail.append((time.time(), spell, res))
            del self.cast_fail[:-50]
        elif op == SMSG_SPELL_FAILURE:
            pass
        elif op == SMSG_QUESTGIVER_QUEST_LIST:
            self.quest_window_t = time.time()     # a quest-giver answer to a hello (talk on a quest-only NPC)
        elif op == SMSG_ATTACK_SWING_ERROR and body:
            self.swing_error = (time.time(), body[0] >> 6)
        elif op == SMSG_QUESTGIVER_QUEST_DETAILS and len(body) >= 92:
            self.quest_window_t = time.time()
            self.quest_details = struct.unpack_from("<I", body, 88)[0]
            log("[QUEST] id=%d event=details" % self.quest_details)
        elif op == SMSG_QUESTGIVER_OFFER_REWARD and len(body) >= 66 * 4:
            u = struct.unpack_from("<66I", body, 0)
            choices = [u[i] for i in (37, 40, 55, 23, 58, 65)]
            n = u[25]
            self.offer = {"quest": u[1], "choices": [c for c in choices[:max(n, 0)] if c] if n else [],
                          "t": time.time()}
            self.quest_window_t = time.time()
            log("[QUEST] id=%d event=offer-reward choices=%s" % (u[1], self.offer["choices"] or "-"))
        elif op == SMSG_QUESTGIVER_REQUEST_ITEMS and len(body) >= 36:
            u = struct.unpack_from("<9I", body, 0)
            self.request_items = {"quest": u[8], "can_complete": u[3] == 0x5F, "t": time.time()}
            self.quest_window_t = time.time()
            log("[QUEST] id=%d event=request-items can_complete=%d" % (u[8], int(u[3] == 0x5F)))
        elif op == SMSG_QUESTGIVER_QUEST_COMPLETE and len(body) >= 13:
            qid = struct.unpack_from("<I", body, 9)[0]
            self.quest_rewarded.add(qid)
            self.quest_completed_pkt.discard(qid)
            log("[QUEST] id=%d event=rewarded" % qid)
        elif op == SMSG_QUEST_UPDATE_COMPLETE and len(body) >= 4:
            qid = struct.unpack_from("<I", body, 0)[0]
            if qid not in self.quest_completed_pkt:
                self.quest_completed_pkt.add(qid)
                log("[QUEST] id=%d event=complete" % qid)
        elif op in (SMSG_QUESTUPDATE_FAILED, SMSG_QUESTUPDATE_FAILEDTIMER, SMSG_QUESTGIVER_QUEST_FAILED) and len(body) >= 4:
            qid = struct.unpack_from("<I", body, 0)[0]
            self.quest_failed.add(qid)
            reason = struct.unpack_from("<I", body, 4)[0] if op == SMSG_QUESTGIVER_QUEST_FAILED and len(body) >= 8 else None
            # SMSG_QUESTGIVER_QUEST_FAILED carries an InventoryResult (e.g. 50 = bags full): the reward did not fit
            log("[QUEST] id=%d event=failed%s" % (qid, "" if reason is None else " reward-error=%d" % reason))
            if reason is not None:
                self.quest_reward_error = (time.time(), qid, reason)
        elif op == SMSG_QUESTUPDATE_ADD_CREDIT and len(body) >= 15:
            cnt, otype, qid, req, obj = struct.unpack_from("<HBIHI", body, 0)
            self.quest_credit[(qid, obj)] = (cnt, req)
            self.quest_required.setdefault(qid, {})[cnt] = req
        elif op == SMSG_QUESTUPDATE_ADD_CREDIT_SIMPLE and len(body) >= 9:
            obj, qid, otype = struct.unpack_from("<IIB", body, 0)
            self.quest_credit[(qid, obj)] = (1, 1)
            log("[QUEST] id=%d event=progress i=1/1 object=%d" % (qid, obj))
        elif op == SMSG_QUESTGIVER_QUEST_INVALID and len(body) >= 5:
            self.quest_invalid = (time.time(), struct.unpack_from("<I", body, 1)[0])
            log("[QUEST] questgiver refused: QUEST_INVALID reason=%d" % self.quest_invalid[1])
        elif op == SMSG_QUESTLOG_FULL:
            self.quest_invalid = (time.time(), -1)
            log("[QUEST] quest log full")
        elif op == SMSG_QUESTGIVER_STATUS:
            r = MvReader(body)
            g = MvGuid()
            g.masks(r, (1, 7, 4, 2, 5, 3, 6, 0))
            g.seq(r, 7)
            st = r.u32()
            g.seqs(r, (4, 6, 1, 5, 2, 0, 3))
            self.qg_status[g.value()] = st
        elif op == SMSG_QUESTGIVER_STATUS_MULTIPLE and len(body) >= 3:
            r = MvReader(body)
            n = r.bits(21)
            gs = []
            for _ in range(n):
                g = MvGuid()
                g.masks(r, (4, 0, 3, 6, 5, 7, 1, 2))
                gs.append(g)
            r.flush()
            for g in gs:
                g.seqs(r, (6, 2, 7, 5, 4))
                st = r.u32()
                g.seqs(r, (1, 3, 0))
                self.qg_status[g.value()] = st
        elif op == SMSG_CORPSE_QUERY and len(body) >= 2:
            r = ByteReader(body)
            bits = [r.read_bit() for _ in range(9)]
            if bits[3] and r.remaining() >= 20 and not any(bits[i] for i in (0, 1, 2, 4, 5, 6, 7, 8)):
                z = r.read_f32(); mapid = r.read_u32(); x = r.read_f32(); r.read_i32(); y = r.read_f32()
                self.corpse_pos = (mapid, x, y, z)
                log("[DEATH] corpse at map=%d (%.1f, %.1f, %.1f)" % (mapid, x, y, z))
        elif op == SMSG_DEATH_RELEASE_LOC and len(body) >= 16:
            mapid, y, x, z = struct.unpack("<Ifff", body[:16])
            if mapid != 0xFFFFFFFF:
                self.spirit_healer_pos = (mapid, x, y, z)
                log("[DEATH] spirit healer at map=%d (%.1f, %.1f, %.1f)" % (mapid, x, y, z))
        elif op == SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA:
            self._set_vehicle(True, "ride-aura")
        elif op == SMSG_CLIENT_CONTROL_UPDATE and body:
            r = MvReader(body)
            g = MvGuid()
            g.masks(r, (2, 7))
            allow = r.bit()
            g.masks(r, (0, 3, 6, 5, 1, 4))
            r.flush()
            g.seqs(r, (1, 5, 7, 4, 2, 6, 3, 0))
            gv = g.value()
            log("[VEHICLE] client control guid=%d allowMove=%d" % (gv, allow))
            if gv and gv != w.char_guid:
                self.vehicle_guid = gv
                self._set_vehicle(True, "control")
                if allow:
                    w.move_guid = gv          # this client now moves the vehicle (MovementHandler drops other guids)
                    log("[VEHICLE] driving: movement packets now carry the vehicle guid %d" % gv)
            elif gv == w.char_guid:
                if w.move_guid and w.move_guid != w.char_guid:
                    log("[VEHICLE] control back to the character")
                w.move_guid = None
                if self.vehicle:
                    self._set_vehicle(False, "control")
        elif op == SMSG_TRIGGER_CINEMATIC:
            cid = struct.unpack_from("<I", body, 0)[0] if len(body) >= 4 else 0
            w.send_packet(CMSG_NEXT_CINEMATIC_CAMERA, b"")
            self.cine_complete_at = time.time() + CINEMATIC_WATCH_S
            log("[CINEMATIC] cinematic %d started -> CMSG_NEXT_CINEMATIC_CAMERA (complete in %.0fs)" % (cid, CINEMATIC_WATCH_S))
        elif op == SMSG_TRIGGER_MOVIE:
            self.movie_complete_at = time.time() + CINEMATIC_WATCH_S
            log("[CINEMATIC] movie started (complete in %.0fs)" % CINEMATIC_WATCH_S)
        elif op == SMSG_PET_SPELLS_MESSAGE:
            self._parse_action_bar(body)
        elif op in (SMSG_SPELL_START, SMSG_SPELL_GO):
            self.spell_seen_t = time.time()
        elif op == SMSG_SPIRIT_HEALER_CONFIRM:
            self.spirit_confirm = time.time()
            log("[DEATH] spirit healer asks to confirm the resurrection")
        elif op == SMSG_MOVE_ROOT:
            self.rooted = True
        elif op == SMSG_MOVE_UNROOT:
            self.rooted = False

    def _set_vehicle(self, on, why):
        if self.vehicle != on:
            self.vehicle = on
            self.vehicle_t = time.time()
            self.vehicle_why = why
            if not on:
                self.vehicle_guid = None      # a stale seat guid must not make the next ride-aura look like a seat
            log("[VEHICLE] %s (%s)" % ("entered" if on else "left", why))

    def _own_spline(self, mm):
        fl = mm["flags"]
        if fl & SPLINEFLAG_TRANSPORT_ENTER or mm["transport"]:
            self.vehicle_guid = mm["transport"] or self.vehicle_guid
            self._set_vehicle(True, "spline-enter")
            return
        if fl & SPLINEFLAG_TRANSPORT_EXIT:
            self._set_vehicle(False, "spline-exit")
            self.w.move_guid = None
        if mm["pts"]:
            self.own_spline = (time.time() + mm["duration"] / 1000.0, mm["pts"][-1])

    def _move_teleport(self, body):
        """SMSG_MOVE_TELEPORT (MovementStructures.cpp MoveTeleport, Unit::WriteMovementInfo)."""
        r = MvReader(body)
        g, tg = MvGuid(), MvGuid()
        g.masks(r, (0, 6, 5, 7, 2))
        trans = r.bit()
        g.mask(r, 4)
        if trans:
            tg.masks(r, (1, 3, 6, 4, 5, 2, 0, 7))
        g.masks(r, (3, 1))
        r.bit()
        r.flush()
        if trans:
            tg.seqs(r, (4, 3, 7, 1, 6, 0, 2, 5))
        g.seqs(r, (4, 7))
        z = r.f32()
        y = r.f32()
        g.seqs(r, (2, 3, 5))
        x = r.f32()
        r.u32()
        g.seqs(r, (0, 6, 1))
        o = r.f32()
        self.w.pos = (x, y, z)
        self.w.orientation = o
        self.pos_corrected += 1
        log("[MOVE] server teleport -> (%.1f, %.1f, %.1f)%s" % (x, y, z, " on transport" if trans else ""))

    def _update_object(self, body):
        r = MvReader(body)
        r.raw(2)
        n = r.u32()
        for _ in range(n):
            typ = r.u8()
            if typ == 3:
                for _ in range(r.u32()):
                    g = mv_read_packed_guid(r)
                    if g != self.w.char_guid:
                        self._forget(g)
            elif typ in (1, 2):
                g = mv_read_packed_guid(r)
                tid = r.u8()
                mv = mv_read_create_movement(r)
                vals = po_read_values(r)
                self._on_create(g, tid, mv, vals)
            elif typ == 0:
                g = mv_read_packed_guid(r)
                vals = po_read_values(r)
                o = self.objs.get(g)
                if o is None:
                    o = self.objs[g] = TrackedObject(g, None)
                self._merge(o, vals)
            else:
                raise MvParseError("update block type %d" % typ)
        self.parsed_updates += 1

    def _on_create(self, g, tid, mv, vals):
        o = TrackedObject(g, tid)
        prev = self.objs.get(g)
        if prev is not None and g == self.w.char_guid:
            o.speeds = prev.speeds
        self.objs[g] = o
        if mv.get("pos") is not None:
            o.pos = mv["pos"]
            o.o = mv.get("orient", 0.0)
        elif mv.get("stat") is not None:
            o.pos = mv["stat"][:3]
            o.o = mv["stat"][3]
        if mv.get("speeds"):
            o.speeds = mv["speeds"]
        if mv.get("spline") and mv.get("pts") and mv.get("dur", 0) > 0:
            o.spline = _MvSpline(time.time() - mv.get("passed", 0) / 1000.0, mv["pts"], mv["dur"])
        self._merge(o, vals, created=True)
        if g == self.w.char_guid:
            if mv.get("pos") is not None and mv.get("self"):
                self.w.pos = mv["pos"]
                self.w.orientation = mv.get("orient", self.w.orientation)
            log("[SELF] create level=%d hp=%d/%d run=%.2f quests=%d items=%d" % (
                o.u32(UF_UNIT_LEVEL), o.health, o.max_health, self.run_speed(), len(self.quest_log()),
                len(self.inventory())))

    def _merge(self, o, vals, created=False):
        me = o.guid == self.w.char_guid
        if me:
            was_dead = self.is_dead() if o.fields else False
            old_lvl = o.u32(UF_UNIT_LEVEL)
        o.fields.update(vals)
        o.seen = time.time()
        if me:
            if any(UF_PLAYER_QUEST_LOG <= k < UF_PLAYER_QUEST_LOG + QUEST_LOG_SLOTS * QUEST_SLOT_FIELDS for k in vals) \
                    or self.quest_prev is None:
                self._quest_diff()
            lvl = o.u32(UF_UNIT_LEVEL)
            if old_lvl and lvl > old_lvl:
                log("[SELF] level up %d -> %d" % (old_lvl, lvl))
            dead = self.is_dead()
            if dead and not was_dead:
                log("[DEATH] this character died at %s (hp=%d ghost=%d)" % (self.pos_str(), o.health, int(self.is_ghost())))
            elif was_dead and not dead:
                log("[DEATH] alive again at %s hp=%d/%d" % (self.pos_str(), o.health, o.max_health))

    def _quest_diff(self):
        cur = self.quest_log()
        snap = {q: (v["state"], tuple(v["counts"])) for q, v in cur.items()}
        if self.quest_prev is None:
            self.quest_prev = snap
            for q, (st, cnt) in snap.items():
                log("[QUEST] id=%d event=inlog state=%d counts=%s" % (q, st, [c for c in cnt if c] or 0))
            return
        prev = self.quest_prev
        for q, (st, cnt) in snap.items():
            if q not in prev:
                log("[QUEST] id=%d event=accepted" % q)
                pst, pcnt = 0, (0,) * len(cnt)
            else:
                pst, pcnt = prev[q]
            for i, (a, b) in enumerate(zip(pcnt, cnt)):
                if b != a:
                    req = self.quest_required.get(q, {}).get(b, "?")
                    log("[QUEST] id=%d event=progress i=%d/%s obj=%d" % (q, b, req, i))
            if st & QUEST_STATE_COMPLETE and not pst & QUEST_STATE_COMPLETE:
                if q not in self.quest_completed_pkt:
                    log("[QUEST] id=%d event=complete" % q)
                self.quest_completed_pkt.add(q)
            if st & QUEST_STATE_FAIL and not pst & QUEST_STATE_FAIL:
                log("[QUEST] id=%d event=failed" % q)
        for q in prev:
            if q not in snap:
                if q in self.quest_rewarded:
                    log("[QUEST] id=%d event=left-log (rewarded)" % q)
                else:
                    log("[QUEST] id=%d event=removed" % q)
        self.quest_prev = snap

    # ------------------------------------------------------------------ client packets
    def _guid_packet(self, op, guid, mask_order, byte_order, prefix=b"", extra_bit_after=None):
        gb = int(guid).to_bytes(8, "little")
        bw = BitWriter()
        for k, idx in enumerate(mask_order):
            bw.bit(gb[idx] != 0)
            if extra_bit_after is not None and k == extra_bit_after:
                bw.bit(0)
        body = prefix + bw.flush() + guid_byte_seq(guid, byte_order)
        self.w.send_packet(op, body)

    def questgiver_hello(self, guid):
        """CMSG_QUESTGIVER_HELLO (QuestHandler.cpp:102): bits [5,6,7,3,4,2,1,0], bytes [4,1,7,3,6,0,5,2]."""
        self.w.gossip = None
        self._guid_packet(CMSG_QUESTGIVER_HELLO, guid, (5, 6, 7, 3, 4, 2, 1, 0), (4, 1, 7, 3, 6, 0, 5, 2))
        log("sent CMSG_QUESTGIVER_HELLO guid=%d" % guid)

    def questgiver_query(self, guid, qid):
        """CMSG_QUESTGIVER_QUERY_QUEST (QuestHandler.cpp:365): u32 quest, bits [2,6,5,0,4,3,1,7] + 1 unused bit,
        bytes [2,0,4,7,5,1,3,6]."""
        self._guid_packet(CMSG_QUESTGIVER_QUERY_QUEST, guid, (2, 6, 5, 0, 4, 3, 1, 7), (2, 0, 4, 7, 5, 1, 3, 6),
                          prefix=struct.pack("<I", int(qid)), extra_bit_after=7)
        log("sent CMSG_QUESTGIVER_QUERY_QUEST quest=%d guid=%d" % (qid, guid))

    def questgiver_request_reward(self, guid, qid):
        """CMSG_QUESTGIVER_REQUEST_REWARD (QuestHandler.cpp:521): u32 quest, bits [6,3,1,2,4,0,5,7],
        bytes [3,0,7,6,2,1,5,4]."""
        self._guid_packet(CMSG_QUESTGIVER_REQUEST_REWARD, guid, (6, 3, 1, 2, 4, 0, 5, 7), (3, 0, 7, 6, 2, 1, 5, 4),
                          prefix=struct.pack("<I", int(qid)))
        log("sent CMSG_QUESTGIVER_REQUEST_REWARD quest=%d guid=%d" % (qid, guid))

    def questgiver_status_query(self, guid):
        """CMSG_QUESTGIVER_STATUS_QUERY (QuestHandler.cpp:37): bits [4,3,2,1,0,5,7,6], bytes [5,7,4,0,2,1,6,3]."""
        self._guid_packet(CMSG_QUESTGIVER_STATUS_QUERY, guid, (4, 3, 2, 1, 0, 5, 7, 6), (5, 7, 4, 0, 2, 1, 6, 3))

    def corpse_query(self):
        self.w.send_packet(CMSG_CORPSE_QUERY, b"")

    def pet_attack(self, target):
        """CMSG_PET_ACTION (PetHandler.cpp:116) with ACT_COMMAND(7) << 24 | COMMAND_ATTACK(2) on the target."""
        me = self.me()
        pet = me.u64(UF_UNIT_SUMMON) if me else 0
        if not pet:
            return False
        pb, tb = pet.to_bytes(8, "little"), int(target).to_bytes(8, "little")
        bw = BitWriter()
        for which, idx in (("p", 1), ("p", 0), ("p", 6), ("p", 7), ("p", 5), ("t", 7), ("p", 2), ("p", 3), ("t", 6),
                           ("t", 3), ("t", 0), ("t", 2), ("t", 5), ("p", 4), ("t", 4), ("t", 1)):
            bw.bit((pb if which == "p" else tb)[idx] != 0)
        body = bytearray(struct.pack("<Ifff", 0x07000002, 0.0, 0.0, 0.0) + bw.flush())
        for which, idx in (("p", 7), ("p", 6), ("p", 1), ("p", 2), ("p", 5), ("p", 4), ("t", 5), ("p", 3), ("t", 0),
                           ("t", 1), ("t", 7), ("t", 4), ("t", 6), ("t", 2), ("t", 3), ("p", 0)):
            b = (pb if which == "p" else tb)[idx]
            if b:
                body.append(b ^ 1)
        self.w.send_packet(CMSG_PET_ACTION, bytes(body))
        log("sent CMSG_PET_ACTION attack pet=%d target=%d" % (pet, target))
        return True

    def use_item_target(self, bag, slot, item_guid, target_guid=None, dest=None):
        """CMSG_USE_ITEM (SpellHandler.cpp HandleUseItemOpcode) with a unit/GO target (targetGuid mask [1,0,5,3,6,4,7,2],
        byte-seq [1,4,3,6,2,0,7,5] after the itemTargetGuid seq) or a ground dest; plain use otherwise."""
        if target_guid is None:
            return self.w.use_item(bag, slot, item_guid, dest=dest)
        igb = int(item_guid).to_bytes(8, "little")
        tgb = int(target_guid).to_bytes(8, "little")
        bw = BitWriter()
        bw.bit(1); bw.bit(igb[6] != 0); bw.bit(1); bw.bit(igb[1] != 0); bw.bit(1); bw.bit(0)
        bw.bit(igb[2] != 0); bw.bit(igb[7] != 0); bw.bit(igb[0] != 0); bw.bit(0); bw.bit(1); bw.bit(0)
        bw.bit(0); bw.bit(1); bw.bit(0); bw.bit(1); bw.bit(0); bw.bit(igb[4] != 0); bw.bit(0)
        bw.bit(igb[3] != 0); bw.bit(igb[5] != 0); bw.bits(0, 2)
        for idx in (1, 0, 5, 3, 6, 4, 7, 2):
            bw.bit(tgb[idx] != 0)
        for _ in range(8):
            bw.bit(0)
        tflag = TARGET_FLAG_GAMEOBJECT if (int(target_guid) >> 52) == 0xF11 else TARGET_FLAG_UNIT
        bw.bits(tflag, 20)
        tail = bytearray()
        for idx in (0, 5, 6, 3, 4, 2, 1):
            if igb[idx]:
                tail.append(igb[idx] ^ 1)
        if igb[7]:
            tail.append(igb[7] ^ 1)
        for idx in (1, 4, 3, 6, 2, 0, 7, 5):
            if tgb[idx]:
                tail.append(tgb[idx] ^ 1)
        tail += struct.pack("<B", 1)
        self.w.send_packet(CMSG_USE_ITEM, struct.pack("<BB", int(slot), int(bag)) + bw.flush() + bytes(tail))
        log("sent CMSG_USE_ITEM bag=%d slot=%d item=%d target=%d" % (bag, slot, item_guid, target_guid))

    def use_gameobject(self, go, wait=2.0, open_wait=8.0):
        """Use a gameobject like a client: CMSG_GAMEOBJ_USE + CMSG_GAMEOBJ_REPORT_USE; if a chest / goober (type
        from GAMEOBJECT_BYTES_1) opens nothing, cast the character's opening spell on it. Returns loot windows."""
        w = self.w
        w.loot_windows = []
        w.gameobject_use(go.guid)
        w.gameobject_use(go.guid, report=True)
        self.used_go[go.guid] = time.time()
        self._wait(lambda: bool(w.loot_windows), wait)
        gotype = (go.u32(UF_GO_BYTES_1) >> 8) & 0xFF
        if w.loot_windows or gotype not in (3, 10):
            return w.loot_windows
        cands = [s for s in OPENING_SPELLS if s in self.known_spells] +                 [s for s in OPENING_SPELLS if s not in self.known_spells]
        # the spell that opened this entry before goes first (e.g. Kaja'mite Chunks: 6478, while 22810 is accepted but opens nothing)
        good = getattr(self, "open_spell_ok", {}).get(go.entry)
        if good in cands:
            cands = [good] + [c for c in cands if c != good]
        for sp in cands:
            t0 = time.time()
            w.cast(sp, go.guid)
            if not self._wait(lambda: self.spell_seen_t >= t0 or any(t >= t0 for t, _s, _r in self.cast_fail), 1.0):
                continue                                  # silently ignored: not this lock's spell
            if any(t >= t0 and s == sp for t, s, _r in self.cast_fail):
                continue
            log("[GO] opening guid=%d with spell %d" % (go.guid, sp))
            if self._wait(lambda: bool(w.loot_windows), open_wait):
                if not hasattr(self, "open_spell_ok"):
                    self.open_spell_ok = {}
                self.open_spell_ok[go.entry] = sp
                break                                     # (a spell event from another caster just costs one wait)
        return w.loot_windows

    def face(self, p):
        if p is None or self.w.pos is None:
            return
        o = math.atan2(p[1] - self.w.pos[1], p[0] - self.w.pos[0]) % (2 * math.pi)
        diff = abs((o - (self.w.orientation or 0.0) + math.pi) % (2 * math.pi) - math.pi)
        if diff > 0.25:
            self.w.move_face(o)

    # ------------------------------------------------------------------ movement
    def nav_path(self, start, end):
        nav = nav_module()
        if nav is None:
            return None, "nonav"
        status = "ok"
        try:
            pts = nav.find_path(int(self.w.map_id), tuple(start), tuple(end))
            if not pts:
                # a partial path (transport, portal, off-mesh goal) still gets this character most of the way
                pts = nav.find_path(int(self.w.map_id), tuple(start), tuple(end), allow_partial=True)
                status = "partial"
        except Exception as exc:          # noqa: BLE001
            log("NAV-FALLBACK find_path raised %s: %s" % (type(exc).__name__, exc))
            return None, "error"
        if not pts:
            return None, "nopath"
        # the mesh z is the raw ground surface: send positions NAV_Z_LIFT above it like the server's own paths
        out = [(float(p[0]), float(p[1]), float(p[2]) + NAV_Z_LIFT) for p in pts]
        if status == "partial":
            log("NAV partial path (%d points) toward (%.1f, %.1f, %.1f); the rest is a straight line"
                % (len(out), end[0], end[1], end[2]))
        return out, status

    def walk_to(self, goal_fn, stop_range, verb, args, timeout=300.0, stuck_s=30.0, abort_fn=None, quest=None,
                repath=4.0, fail="stuck"):
        """Walk with real movement packets (MSG_MOVE_START_FORWARD, a HEARTBEAT every 500 ms at the run speed,
        MSG_MOVE_STOP) along the navmesh path to goal_fn() until within stop_range. Re-paths when the goal moves
        more than `repath` yd or the server moves this character. Returns ok | gone | abort; on no progress it
        logs STUCK and exits 3 (fail="return" returns "stuck" instead)."""
        w = self.w
        t0 = time.time()
        dt = 0.5
        path, idx, goal_used = None, 0, None
        moving = False
        best, last_prog = 1e18, time.time()
        corr = self.pos_corrected
        status = "ok"
        result = None
        try:
            while True:
                goal = goal_fn()
                if goal is None:
                    result = "gone"
                    return result
                d = _d3(w.pos, goal)
                # arrived: within range on the ground plane and not on another floor (mesh z differs from DB z)
                if d <= stop_range or (_d2(w.pos, goal) <= stop_range and abs(w.pos[2] - goal[2]) <= 4.0):
                    result = "ok"
                    return result
                if abort_fn is not None and abort_fn():
                    result = "abort"
                    return result
                now = time.time()
                # progress = the remaining length of the path being walked (a navmesh detour may first lead away
                # from the goal); before the first path and after a re-path the new length is the baseline
                rem = self._path_remaining(w.pos, path, idx) if path else d
                if rem < best - 0.5:
                    best, last_prog = rem, now
                if now - t0 > timeout or now - last_prog > stuck_s:
                    reason = "timeout" if now - t0 > timeout else "no-position-progress"
                    if status == "nopath":
                        reason = "path-not-found"
                    if fail == "return":
                        result = "stuck"
                        return result
                    self.stuck(verb, args, "%s dist=%.1f" % (reason, d), quest)
                # SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA alone is also sent when the character itself becomes a vehicle base
                # (aura 296 SET_VEHICLE, e.g. the Kezan villa disguise 67435): with no seat after 3 s it is not a ride
                if (self.vehicle and getattr(self, "vehicle_why", "") == "ride-aura" and not self.vehicle_guid
                        and not w.move_guid and time.time() - self.vehicle_t > 3.0):
                    self._set_vehicle(False, "ride-aura without a seat: own vehicle kit")
                # killed mid-walk and still dead (not released) after 10 s, i.e. no scripted resurrection
                # coming: release, run back to the corpse, then go on walking from there (was: STUCK no-position-progress)
                if self.is_dead() and not self.is_ghost() and not getattr(self, "_walk_corpse_run", False):
                    if getattr(self, "_walk_dead_since", None) is None:
                        self._walk_dead_since = time.time()
                    elif time.time() - self._walk_dead_since > 10.0:
                        if moving:
                            w.move_stop()
                            moving = False
                        self._walk_corpse_run = True
                        try:
                            self.corpse_run(verb, args, quest)
                        finally:
                            self._walk_corpse_run = False
                            self._walk_dead_since = None
                        if getattr(self, "careful", False):
                            self._careful_rest(verb)   # back to full before walking on into what killed it
                        path, idx, goal_used = None, 0, None
                        best, last_prog, t0 = 1e18, time.time(), time.time()
                        continue
                elif not self.is_dead():
                    self._walk_dead_since = None
                # careful: hit while walking -> stop and fight back like a player, then walk on (walking on while
                # nine Slavedrivers hit the character killed it without a swing)
                if (getattr(self, "careful", False) and not self.vehicle and not self.is_dead()
                        and not getattr(self, "_walk_fightback", False) and self.hit_recently(1.5)
                        and [a for a in self.attackers() if not self.too_strong(a)]):
                    if moving:
                        w.move_stop()
                        moving = False
                    self._walk_fightback = True
                    try:
                        n_fb = 0
                        while self.attackers() and not self.is_dead() and n_fb < 8:
                            n_fb += 1
                            if not self.fight_nearby(verb, args, stuck_s, quest, (), 15.0):
                                break
                    finally:
                        self._walk_fightback = False
                    path, idx, goal_used = None, 0, None
                    best, last_prog, t0 = 1e18, time.time(), time.time()
                    continue
                # seated (unless this client drives the vehicle via `mover`), rooted, or dead before release: wait
                if self.rooted or (self.vehicle and not w.move_guid) or (self.is_dead() and not self.is_ghost()):
                    if moving:
                        w.move_stop()
                        moving = False
                    w.pump(dt)
                    continue
                if (path is None or idx >= len(path) or self.pos_corrected != corr
                        or (goal_used is not None and _d3(goal, goal_used) > repath)):
                    if self.pos_corrected != corr and moving:
                        moving = False
                    corr = self.pos_corrected
                    # fly=1 (goto-path): a flying mount/vehicle goes straight through the air, no navmesh
                    pts, status = ((None, "fly") if self.extra_move_flags & 0x1000000 else self.nav_path(w.pos, goal))
                    if pts is None:
                        if status != "nonav" or not self._nav_fallback_logged:
                            log("NAV-FALLBACK %s (%s) -> straight line to (%.1f, %.1f, %.1f)"
                                % (verb, status, goal[0], goal[1], goal[2]))
                            self._nav_fallback_logged = True
                        path = [goal]
                    else:
                        while len(pts) > 1 and _d2(pts[0], w.pos) < 0.5:
                            pts = pts[1:]
                        path = pts
                        if _d3(path[-1], goal) > max(stop_range, 2.0):
                            path.append(goal)
                    idx, goal_used = 0, goal
                    # a new path gets its own baseline: the switch itself earns no progress credit
                    best = self._path_remaining(w.pos, path, idx)
                nxt = path[min(idx, len(path) - 1)]
                o = math.atan2(nxt[1] - w.pos[1], nxt[0] - w.pos[0]) % (2 * math.pi)
                if not moving:
                    w.orientation = o
                    x, y, z = w.pos
                    w.send_packet(MSG_MOVE_START_FORWARD, build_movement_body(
                        MSE_START_FORWARD, w._mguid(), x, y, z, o,
                        movement_flags=MOVEMENTFLAG_FORWARD | self.extra_move_flags,
                        timestamp_ms=w._move_ts(), transport=w._transport_tuple()))
                    moving = True
                    w.pump(dt)
                    continue
                # do not overshoot: the last step stops just inside stop_range (min ranges, e.g. Charge, matter)
                remaining = min(self.run_speed() * dt, max(0.3, self._path_remaining(w.pos, path, idx) - stop_range + 0.5))
                x, y, z = w.pos
                while remaining > 1e-6 and idx < len(path):
                    tx, ty, tz = path[idx]
                    seg = math.hypot(tx - x, ty - y)
                    if seg <= remaining:
                        x, y, z = tx, ty, tz
                        remaining -= seg
                        idx += 1
                    else:
                        f = remaining / seg
                        x, y, z = x + (tx - x) * f, y + (ty - y) * f, z + (tz - z) * f
                        remaining = 0.0
                if idx < len(path):
                    o = math.atan2(path[idx][1] - y, path[idx][0] - x) % (2 * math.pi)
                w.pos, w.orientation = (x, y, z), o
                w.send_packet(MSG_MOVE_HEARTBEAT, build_movement_body(
                    MSE_HEARTBEAT, w._mguid(), x, y, z, o, movement_flags=MOVEMENTFLAG_FORWARD | self.extra_move_flags,
                    timestamp_ms=w._move_ts(), transport=w._transport_tuple()))
                w.pump(dt)
        finally:
            if moving:
                w.move_stop()
                w.emit("pos", x="%.1f" % w.pos[0], y="%.1f" % w.pos[1], z="%.1f" % w.pos[2])

    @staticmethod
    def _path_remaining(pos, path, idx):
        """Length still to walk: to the next waypoint, then along the rest of the path."""
        if not path or idx >= len(path):
            return 0.0 if not path else _d3(pos, path[-1])
        total = _d3(pos, path[idx])
        for a, b in zip(path[idx:], path[idx + 1:]):
            total += _d3(a, b)
        return total

    def goto_object(self, verb, args, entry, kinds, rng, timeout, stuck_s, wait_s=8.0, near=None, pred=None,
                    quest=None, exclude=(), fresh_key=None):
        """Walk to the nearest tracked object of `entry`; returns it. When none is in view: walk to `near` or the
        nearest DB spawn (read-only hint) while watching for it, else wait `wait_s` s; then STUCK not-visible."""
        w = self.w
        t0 = time.time()
        target = None
        used = self.used_targets.get(fresh_key, set()) if fresh_key else set()
        while time.time() - t0 < wait_s:
            target = self.nearest(entry, kinds, pred, set(exclude) | used)
            if target is None and used and time.time() - t0 > min(3.0, wait_s / 2):
                target = self.nearest(entry, kinds, pred, exclude)       # only used ones left: take one again
            if target:
                break
            w.pump(0.5)
        if target is None:
            near = near or self.cur_near
            hints = [tuple(near)] if near else []
            if not hints:
                hints = self.remembered(entry, kinds)[:3]
                if hints:
                    log("%s: entry %d not in view; walking to where it was last seen (%.1f, %.1f, %.1f)"
                        % (verb, entry, hints[0][0], hints[0][1], hints[0][2]))
            if not hints:
                sp = db_spawns("unit" if "unit" in kinds else "go", entry, w.map_id)
                if sp:
                    hints = sorted(sp, key=lambda p: _d3(p, w.pos))[:3]
                    log("%s: entry %d not in view; walking to the nearest DB spawn (%.1f, %.1f, %.1f)"
                        % (verb, entry, hints[0][0], hints[0][1], hints[0][2]))
            for h in hints:
                self.walk_to(lambda h=h: h, 5.0, verb, args, timeout=timeout, stuck_s=stuck_s, quest=quest,
                             abort_fn=lambda: self.nearest(entry, kinds, pred, exclude) is not None)
                target = self.nearest(entry, kinds, pred, exclude)
                if target:
                    break
                w.pump(2.0)
                target = self.nearest(entry, kinds, pred, exclude)
                if target:
                    break
        if target is None:
            self.stuck(verb, args, "not-visible entry=%d" % entry, quest)
        guid = target.guid
        if fresh_key:
            self.used_targets.setdefault(fresh_key, set()).add(guid)

        def goal():
            o = self.objs.get(guid)
            return o.position() if o else None
        res = self.walk_to(goal, rng, verb, args, timeout=timeout, stuck_s=stuck_s, quest=quest)
        if res == "gone":
            if getattr(self, "gone_ok", False):
                return None              # use-item ... fail=ok: a freed NPC that runs off / despawns is not a STUCK
            self.stuck(verb, args, "target-gone entry=%d" % entry, quest)
        return self.objs.get(guid)

    # ------------------------------------------------------------------ combat
    def _cast_result(self, spell, t0, wait):
        end = time.time() + wait
        while True:
            for t, s, res in self.cast_fail:
                if t >= t0 and s == spell:
                    return res
            if time.time() >= end:
                return None
            self.w.pump(min(0.1, max(0.0, end - time.time())))

    def _warrior_openers(self, verb, tgt, d):
        """Warrior (class-gated, additive): Battle Shout (6673) when it is not running, then Charge (100) from 8-25 yd,
        like a player opens a fight. Both only when the spellbook has them (SMSG_INITIAL_SPELLS / LEARNED_SPELL)."""
        now = time.time()
        if not getattr(self, "_war_known_logged", False):
            self._war_known_logged = True
            log("[WARRIOR] known: %s" % ", ".join("%d=%d" % (sp, int(sp in self.known_spells))
                                                 for sp in (100, 6673, 6343, 78, 34428, 5308, 1464, 12294)))
        if 6673 in self.known_spells and now - getattr(self, "_bshout_t", 0) > 110:
            t0 = time.time()
            self.w.cast(6673)
            if self._cast_result(6673, t0, 0.6) is None:
                self._bshout_t = t0
                log("%s: Battle Shout" % verb)
        if 100 in self.known_spells and 8.0 <= d <= 25.0 and self.spell_cd.get(100, 0) <= now:
            self.face(tgt.position())
            self.w.set_selection(tgt.guid)
            t0 = time.time()
            self.w.cast(100, tgt.guid)
            res = self._cast_result(100, t0, 0.8)
            self.spell_cd[100] = t0 + (20.0 if res is None else 3.0)
            log("%s: Charge -> %s" % (verb, "ok" if res is None else SPELL_FAILED_NAMES.get(res, res)))
            if res is None:
                self.w.pump(0.8)

    def rotation_step(self, tgt, dist):
        now = time.time()
        if now < self.gcd_until:
            return None
        rot_list = CLASS_ROTATIONS.get(self.char_class or 0, [])
        if (self.char_class == 1 and 6343 in self.known_spells and self.spell_cd.get(6343, 0) <= now and self.w.pos
                and sum(1 for a in self.attackers() if a.position() and _d3(a.position(), self.w.pos) <= 8.0) >= 2):
            # Thunder Clap (6343): warrior AoE when two or more are on us in melee
            t0 = time.time()
            self.w.cast(6343)
            res = self._cast_result(6343, t0, 0.5)
            self.spell_cd[6343] = t0 + (6.0 if res is None else 2.5)
            if res is None:
                self.gcd_until = t0 + 1.5
                return ("cast", 6343)
        if self.char_class == 10:
            # monk (class-gated): Jab (100780, energy) builds Chi; Tiger Palm (100787, 1 chi), Blackout Kick (100784, 2 chi) and
            # Rising Sun Kick (107428, 2 chi) spend it. Without the chi estimate Tiger Palm was tried first and only earned
            # SPELL_FAILED_NO_POWER back-offs. The estimate is local (jab +1, spend on success, 0 on a NO_POWER failure).
            me_ = self.me()
            if (me_ is not None and me_.max_health and me_.health < 0.55 * me_.max_health and 115072 in self.known_spells
                    and self.spell_cd.get(115072, 0) <= now):
                # Expel Harm (115072): the monk's self heal, used like a player would when hurt in a fight
                t0 = time.time()
                self.w.cast(115072, self.w.char_guid)
                res = self._cast_result(115072, t0, 0.7)
                self.spell_cd[115072] = t0 + (3.0 if res is None else 6.0)
                if res is None:
                    self.gcd_until = t0 + 1.5
                    return ("cast", 115072)
            chi = getattr(self, "_monk_chi", 0)
            if me_ is not None and (UF_UNIT_POWER + 4) in me_.fields:
                chi = me_.fields[UF_UNIT_POWER + 4]     # the server's Chi (power slot 4, seen live: 0-4); energy is slot 1
            by_spell = {sp: (sp, k, c, d) for sp, k, c, d in rot_list}
            pref = []
            if chi >= 2:
                pref += [by_spell[x] for x in (107428, 100784) if x in by_spell]
            if chi >= 1:
                pref += [by_spell[x] for x in (100787,) if x in by_spell]
            if 100780 in by_spell:
                pref.append(by_spell[100780])
            rot_list = pref or rot_list
        for spell, kind, cast_s, cd_s in rot_list:
            if spell not in self.known_spells or self.spell_cd.get(spell, 0) > now:
                continue
            rng = MELEE_RANGE if kind == "m" else SPELL_RANGE
            if dist > rng + 0.5:
                continue
            if kind == "auto" and tgt.guid in self.auto_shot:
                continue
            if kind == "dot" and self.dot_until.get((spell, tgt.guid), 0) > now:
                continue
            # warrior: Victory Rush (34428) only works for 20 s after a kill that gave XP (CASTER_AURASTATE otherwise, seen live)
            if self.char_class == 1 and spell == 34428 and time.time() - getattr(self, "last_kill_t", 0) > 20.0:
                continue
            # warrior rage gate: a low-level warrior lives on auto-attack rage; casting Heroic Strike / Slam /
            # Execute without the rage only earns SPELL_FAILED_NO_POWER (88) and a back-off
            if self.char_class == 1 and spell in WARRIOR_RAGE_COST:
                me_ = self.me()
                if me_ is not None and me_.fields.get(UF_UNIT_POWER, 0) < WARRIOR_RAGE_COST[spell] * 10:
                    continue
            t0 = time.time()
            self.w.cast(spell, tgt.guid)
            res = self._cast_result(spell, t0, max(0.35, cast_s + 0.35))
            if res is None:
                if self.char_class == 10:
                    self._monk_chi = (min(4, getattr(self, "_monk_chi", 0) + 1) if spell == 100780
                                      else max(0, getattr(self, "_monk_chi", 0) - {100787: 1, 100784: 2, 107428: 2}.get(spell, 0)))
                self.spell_cd[spell] = t0 + max(cd_s, 0.5)
                self.gcd_until = t0 + (0.2 if kind == "auto" else 1.5)
                if kind == "auto":
                    self.auto_shot.add(tgt.guid)
                if kind == "dot":
                    self.dot_until[(spell, tgt.guid)] = t0 + cd_s
                return ("cast", spell)
            if self.char_class == 10 and res == 88:
                self._monk_chi = 0 if spell != 100780 else getattr(self, "_monk_chi", 0)   # NO_POWER on a chi spell: no chi; on Jab: wait for energy
            backoff = {70: 1.5, 88: 2.5, 100: 1.0, 50: 1.0, 138: 0.5}.get(res, 5.0)
            self.spell_cd[spell] = time.time() + backoff
            if kind == "auto":
                self.w.cancel_auto_repeat()
            return ("fail", spell, res)
        return None

    def fight(self, verb, args, guid, stuck_s, quest=None):
        """Fight one unit until it dies: approach (melee or spell range by class), face it, auto-attack, pet attack,
        class rotation. Returns dead | gone | died | tapped | evade."""
        w = self.w
        ranged = self.char_class in RANGED_CLASSES and any(
            s in self.known_spells for s, k, _c, _d in CLASS_ROTATIONS.get(self.char_class or 0, []) if k != "m")
        engage = 24.0 if ranged else 2.5
        swing_sent = False
        swing_t = time.time()
        last_hp, last_dmg = None, time.time()
        fails = 0
        t_fight = time.time()
        t0_obj = self.objs.get(guid)
        fac0 = t0_obj.fields.get(UF_UNIT_FACTION) if t0_obj is not None else None
        was_attackable = t0_obj.attackable() if t0_obj is not None else True
        qsum_f = self._quest_count_sum(quest)
        # the leash is measured from where the target stood when the fight began (not from the character, which may
        # still be walking up to it)
        fight_anchor = t0_obj.position() if t0_obj is not None and t0_obj.position() else None
        while True:
            if self.is_dead():
                return "died"
            tgt = self.objs.get(guid)
            if tgt is None:
                return "gone"
            if not tgt.alive():
                return "dead"
            # careful: an elite helped down while an NPC tanked it (too_strong() exception) that turns on this
            # character - stop at once and let the caller pull back (Gorerot 38331 switched to Wolfplay and its Smash
            # 71774, a single-target hit on its victim, killed the level 14 warrior three times at -1702,1403)
            if (getattr(self, "careful", False) and (tgt.u64(UF_UNIT_TARGET) == w.char_guid or self.hit_by(tgt.guid, 3.0))
                    and self.too_strong(tgt)):
                log("%s: %s turned on this character - breaking off" % (verb, tgt.short()))
                try:
                    w.attack_stop()
                except OSError:
                    pass
                return "evade"
            # careful: a low-health target that runs (creatures flee) is not chased past 25 yd from where the fight
            # began - chasing a fleeing Slavedriver into the camp pulled the rest of it
            if (getattr(self, "careful", False) and fight_anchor and tgt.position() and tgt.max_health > 0
                    and tgt.health * 4 <= tgt.max_health and _d3(tgt.position(), fight_anchor) > 25.0
                    and tgt.u64(UF_UNIT_TARGET) != w.char_guid):
                log("%s: target guid=%d fled past the 25 yd leash - letting it go" % (verb, guid))
                try:
                    w.attack_stop()
                except OSError:
                    pass
                return "evade"
            fac = tgt.fields.get(UF_UNIT_FACTION)
            low_hp = tgt.max_health > 0 and tgt.health * 20 <= tgt.max_health
            # a faction swap at low health, or one that came with this quest's credit (e.g. "At 30% credit, faction 35")
            credited = quest is not None and self._quest_count_sum(quest) > qsum_f
            if ((low_hp or credited) and fac0 is not None and fac is not None and fac != fac0) or (was_attackable and not tgt.attackable()):
                log("%s: target guid=%d yielded (faction %s->%s attackable=%d)" % (verb, guid, fac0, fac, int(tgt.attackable())))
                try:
                    w.attack_stop()
                except OSError:
                    pass
                return "yielded"
            # an Auto Shot that started fine but then fails on every tick (LOS, range): cancel it and re-cast later
            if guid in self.auto_shot:
                bad = [r for t, s_, r in self.cast_fail if t >= t_fight and s_ in AUTO_REPEAT_SPELLS]
                if bad:
                    self.auto_shot.discard(guid)
                    w.cancel_auto_repeat()
                    t_fight = time.time()
                    if bad[-1] in (SPELL_FAILED_OUT_OF_RANGE, SPELL_FAILED_LINE_OF_SIGHT) and engage > 5:
                        engage = max(5.0, engage - 8.0)
                        log("%s: auto-repeat failed (%s) - closing in to %.0f yd" % (verb, SPELL_FAILED_NAMES.get(bad[-1]), engage))
            if tgt.tapped_by_other() and last_hp is not None and tgt.health == tgt.max_health:
                return "tapped"
            hp = tgt.health
            now = time.time()
            if last_hp is None or hp < last_hp:
                last_dmg = now
            last_hp = hp
            if now - last_dmg > stuck_s:
                return "evade"
            p = tgt.position()
            d = _d3(w.pos, p)
            thr = engage + 1.5 if engage > 5 else MELEE_RANGE
            # warrior opener: Charge (100, 8-25 yd) closes in, stuns and gives the rage a low-level warrior lacks
            if (d > thr and self.char_class == 1 and 100 in self.known_spells and 8.5 <= d <= 24.0
                    and self.spell_cd.get(100, 0) <= time.time() and time.time() >= self.gcd_until):
                self.face(p)
                t_c = time.time()
                w.cast(100, guid)
                res_c = self._cast_result(100, t_c, 0.6)
                self.spell_cd[100] = t_c + (15.0 if res_c is None else 3.0)
                if res_c is None:
                    log("%s: charge -> guid=%d (%.1f yd)" % (verb, guid, d))
                    w.pump(1.0)
                    swing_sent = False
                    continue
            if self.char_class == 1 and not swing_sent:
                self._warrior_openers(verb, tgt, d)
                d = _d3(w.pos, tgt.position()) if tgt.position() else d
            if d > thr:
                # walk until inside the same threshold: an engage of exactly 5 yd used to stop walk_to at 5 yd while this
                # check wanted MELEE_RANGE, so the loop spun without ever pumping the socket (the i27 Wandering Isle hang)
                res = self.walk_to(lambda: (self.objs[guid].position() if guid in self.objs else None),
                                   min(engage, thr), verb, args, timeout=stuck_s * 3, stuck_s=stuck_s, quest=quest,
                                   abort_fn=lambda: self.is_dead() or guid not in self.objs
                                   or not self.objs[guid].alive(), fail="return")
                if res == "stuck":
                    return "evade"
                swing_sent = False
                last_dmg = time.time()
                w.pump(0.1)                      # never spin: a walk that ends at once still yields to the socket
                continue
            self.face(p)
            # re-swing after the server stopped the auto-attack (stun, fear, knock-down: SMSG_ATTACKSTOP)
            if swing_sent and getattr(w, "attackstop_t", 0) > swing_t and time.time() - w.attackstop_t > 0.3:
                log("%s: auto-attack was stopped by the server - swinging again" % verb)
                swing_sent = False
            if not swing_sent:
                w.set_selection(guid)
                w.attack(guid)
                if not getattr(self, "no_pet_attack", False):
                    self.pet_attack(guid)
                swing_sent = True
                swing_t = time.time()
            if self.swing_error and now - self.swing_error[0] < 1.0:
                code = self.swing_error[1]
                self.swing_error = None
                if code == 0 and engage > 2.0:
                    engage = max(2.0, engage - 2.0)
                elif code == 1:
                    w.orientation = (w.orientation + 1.0) % (2 * math.pi)   # force a re-face next tick
                swing_sent = False
            if os.environ.get("HEADLESS_FIGHT_TRACE") == "1" and now - getattr(self, "_ftrace_t", 0) >= 1.0:
                # diagnostics: target and own health, the server's own power slots (UF_UNIT_POWER x5), the attackers on us
                self._ftrace_t = now
                me_t = self.me()
                pw = [me_t.fields.get(UF_UNIT_POWER + i, 0) for i in range(5)] if me_t is not None else []
                log("[FTRACE] tgt=%d hp=%d/%d dist=%.1f my_hp=%s/%s power=%s attackers=%d" % (
                    tgt.entry, tgt.health, tgt.max_health, d, getattr(me_t, "health", "?"), getattr(me_t, "max_health", "?"),
                    pw, len(self.attackers())))
            r = self.rotation_step(tgt, d)
            if r and r[0] == "fail":
                fails += 1
                if r[2] in (SPELL_FAILED_OUT_OF_RANGE, SPELL_FAILED_LINE_OF_SIGHT) and engage > 5:
                    engage = max(5.0, engage - 8.0)
                    log("%s: cast %d failed (%s) - closing in to %.0f yd" % (verb, r[1], SPELL_FAILED_NAMES.get(r[2]), engage))
                elif r[2] == SPELL_FAILED_UNIT_NOT_INFRONT:
                    w.orientation = (w.orientation + 1.0) % (2 * math.pi)
            w.pump(0.25)

    def loot_unit(self, verb, args, guid, wait=2.0):
        t0 = time.time()
        while time.time() - t0 < wait:
            o = self.objs.get(guid)
            if o is not None and o.dynflags & UNIT_DYNFLAG_LOOTABLE:
                break
            self.w.pump(0.25)
        o = self.objs.get(guid)
        if o is None or not o.dynflags & UNIT_DYNFLAG_LOOTABLE:
            return 0
        if self.walk_to(lambda: (self.objs[guid].position() if guid in self.objs else None), 3.0, verb, args,
                        timeout=60, stuck_s=20, fail="return") != "ok":
            return 0
        self.w.loot_windows = []
        self.w.loot_open(guid)
        return self.w.loot_all(3.0)

    def _quest_count_sum(self, quest):
        if not quest:
            return 0
        q = self.quest_log().get(int(quest))
        return sum(q["counts"]) if q else 0

    def _careful_rest(self, verb, full=0.95, max_s=None):
        """careful pull: out of combat, rest until `full` health (or max_s, default 2x HEADLESS_REST_S)."""
        w = self.w
        me_ = self.me()
        if me_ is None or me_.max_health <= 0 or me_.health >= me_.max_health * full or self.attackers() or self.is_dead():
            return
        log("%s: careful - resting to full (hp=%d/%d)" % (verb, me_.health, me_.max_health))
        t_rest, lim = time.time(), (max_s if max_s is not None else max(30.0, self.rest_s * 2))
        if os.environ.get("HEADLESS_EAT", "0") == "1" and me_.health < me_.max_health * 0.7:
            # a player eats instead of waiting out the slow out-of-combat regeneration (opt-in; food entries seen in the bags)
            for food in (117, 77272, 4540, 4541, 4542, 4544, 4601):
                fnd = self.find_item(food)
                if fnd:
                    try:
                        w.use_item(fnd[0], fnd[1], fnd[2])
                        log("%s: careful - eating item %d" % (verb, food))
                        w.pump(2.0)
                        lim = max(lim, 30.0)
                    except OSError:
                        pass
                    break
        while time.time() - t_rest < lim and not self.attackers() and not self.hit_recently() and not self.is_dead():
            m2 = self.me()
            if m2 is None or m2.health >= m2.max_health * full:
                break
            w.pump(1.0)

    def _careful_pick(self, verb, entry, black=(), radius=12.0):
        """careful pull: rest to full, then the live, attackable, untapped unit of `entry` with the fewest other
        live units within `radius` yd of it (packs are one entry; anything already in combat counts too), nearest
        first on a tie."""
        self._careful_rest(verb)
        if self.attackers() or self.is_dead():
            return None
        me = self.w.pos
        cands = [o for o in self.objects_of(entry, ("unit",))
                 if o.guid not in black and o.alive() and o.attackable() and not o.tapped_by_other() and o.position()]
        if not cands or me is None:
            return None
        near = [c for c in cands if _d3(c.position(), me) <= 50.0]
        cands = near or cands
        # with near=x,y,z: clear toward the objective (the camp around near=), still the least crowded one first
        if getattr(self, "cur_near", None):
            wn = getattr(self, "cur_within", 30.0)
            if wn > 0:
                # within=R (default 30 yd, 0 = the old behaviour): only targets that close to near= are candidates; none there
                # means no pull (the loop walks to near= and waits) instead of a unit 80-100 yd away in another camp
                around = [c for c in cands if _d3(c.position(), self.cur_near) <= wn]
                if not around:
                    if time.time() - getattr(self, "_within_log", 0) > 20:
                        self._within_log = time.time()
                        log("%s: careful - no live %d within %.0f yd of near=%.0f,%.0f,%.0f" % (
                            verb, entry, wn, self.cur_near[0], self.cur_near[1], self.cur_near[2]))
                    return None
                cands = around
            else:
                around = [c for c in cands if _d3(c.position(), self.cur_near) <= 25.0]
                cands = around or cands
        units = [o for o in self.objs.values() if o.typeid == 3 and o.alive() and o.position() and o.attackable()]

        def crowd(c):
            cp = c.position()
            # a pack mixes entries (Oomlot Shaman 38644 + 38574 + Tribesman 38575, all faction 14): the same faction template
            # counts as the pack too - "0 others within 28 yd" by entry pulled seven at once
            cf = c.fields.get(UF_UNIT_FACTION)
            return sum(1 for o in units if o.guid != c.guid and _d3(o.position(), cp) <= radius
                       and (o.entry == c.entry or o.unit_flags & UNIT_FLAG_IN_COMBAT
                            or (cf and o.fields.get(UF_UNIT_FACTION) == cf and o.u32(UF_UNIT_LEVEL) > 1)))
        # path-aware (HEADLESS_CAREFUL_PATH=1, opt-in): also count the pack members within aggro range (18 yd) of the navmesh
        # walk to the candidate - a "lone" shaman reached through two other packs pulled five (the walk fights back)
        path_r = float(os.environ.get("HEADLESS_CAREFUL_PATH_R", "18"))
        use_path = os.environ.get("HEADLESS_CAREFUL_PATH", "0") not in ("0", "")
        nav = nav_module() if use_path else None
        path_cache = {}

        def path_crowd(c):
            if not nav:
                return 0
            if c.guid in path_cache:
                return path_cache[c.guid]
            cp = c.position()
            try:
                pth = nav.find_path(self.w.map_id, tuple(me), tuple(cp)) or [tuple(me), tuple(cp)]
            except Exception:             # noqa: BLE001 - no path: the straight line
                pth = [tuple(me), tuple(cp)]
            cf = c.fields.get(UF_UNIT_FACTION)
            pts = []
            for a, b2 in zip(pth, pth[1:]):
                n = max(1, int(_d3(a, b2) / 3.0))
                pts += [tuple(a[k] + (b2[k] - a[k]) * i / n for k in range(3)) for i in range(n)]
            pts.append(tuple(cp))
            hit = set()
            for o in units:
                if o.guid == c.guid or o.u32(UF_UNIT_LEVEL) <= 1:
                    continue
                if not (o.entry == c.entry or o.unit_flags & UNIT_FLAG_IN_COMBAT
                        or (cf and o.fields.get(UF_UNIT_FACTION) == cf)):
                    continue
                op = o.position()
                if _d3(op, cp) <= radius:
                    continue                  # already in crowd(c)
                if any(_d3(op, q) <= path_r for q in pts):
                    hit.add(o.guid)
            path_cache[c.guid] = len(hit)
            return len(hit)
        best = min(cands, key=lambda c: (crowd(c) + path_crowd(c), _d3(c.position(), me)))
        log("%s: careful - picked %s (%d others within %.0f yd, %d along the walk)"
            % (verb, best.short(), crowd(best), radius, path_crowd(best)))
        return best

    def kill_loop(self, verb, args, entry, done_fn, loot, timeout, stuck_s, quest=None, progress_fn=None):
        """Kill units of `entry` until done_fn(). Fights back when attacked; runs back to the corpse after a death."""
        w = self.w
        t0 = time.time()
        kills = 0
        deaths = 0
        last_prog = time.time()
        black = {}
        prog_val = progress_fn() if progress_fn else None
        kills_at_prog = 0
        while not done_fn(kills):
            now = time.time()
            if now - t0 > timeout:
                self.stuck(verb, args, "timeout kills=%d" % kills, quest)
            if progress_fn:
                v = progress_fn()
                if v != prog_val:
                    prog_val, last_prog, kills_at_prog = v, now, kills
                elif kills - kills_at_prog >= self.max_dry_kills:
                    self.stuck(verb, args, "no-item-progress after %d kills have=%s" % (kills - kills_at_prog, v), quest)
            if self.is_dead():
                deaths += 1
                if deaths > self.max_deaths:
                    self.stuck(verb, args, "died-%d-times kills=%d" % (deaths, kills), quest)
                self.corpse_run(verb, args, quest)
                if getattr(self, "careful", False):
                    self._careful_rest(verb)   # careful pull: back to full before walking into the camp again
                last_prog = time.time()
                continue
            black = {g: t for g, t in black.items() if t > now}
            # a blacklisted unit that is hitting the character is fought anyway (a target let go at the leash kept
            # hitting, was skipped as blacklisted, and the character stood still for two minutes until it died)
            att = [a for a in self.attackers() if (a.guid not in black or self.hit_by(a.guid)) and not self.too_strong(a)]
            strong = [a for a in self.attackers() if self.too_strong(a)]
            if strong and not att:
                # careful: only a far stronger unit is on the character - run 35 yd straight away from it like a
                # player (the kill loop used to spin here without pumping the socket until the client stalled)
                big = min(strong, key=lambda a: _d3(a.position(), w.pos))
                bp = big.position()
                vx, vy = w.pos[0] - bp[0], w.pos[1] - bp[1]
                vl = math.hypot(vx, vy) or 1.0
                away = (w.pos[0] + 35.0 * vx / vl, w.pos[1] + 35.0 * vy / vl, w.pos[2])
                log("%s: careful - %s is far too strong, running away to (%.1f, %.1f)" % (verb, big.short(), away[0], away[1]))
                try:
                    w.attack_stop()
                except OSError:
                    pass
                self.walk_to(lambda: away, 4.0, verb, args, timeout=20.0, stuck_s=8.0, quest=quest,
                             abort_fn=lambda: self.is_dead(), fail="return")
                w.pump(0.5)
                continue
            if att and getattr(self, "careful", False) and len(att) >= 3 and getattr(self, "_careful_anchor", None):
                # careful pull: two or more adds joined - back off along the way we came, so the fight moves
                # away from the camp and no further adds join; then fight what followed, nearest first
                anchor = self._careful_anchor
                self._careful_anchor = None
                if _d3(anchor, w.pos) > 8.0:
                    log("%s: careful - %d attackers, backing off to (%.1f, %.1f, %.1f)" % (verb, len(att), anchor[0], anchor[1], anchor[2]))
                    try:
                        w.attack_stop()
                    except OSError:
                        pass
                    self.walk_to(lambda: anchor, 3.0, verb, args, timeout=25.0, stuck_s=10.0, quest=quest,
                                 abort_fn=lambda: self.is_dead(), fail="return")
                continue
            if att:
                tgt = self.pick_attacker(att)
            elif getattr(self, "careful", False):
                tgt = self._careful_pick(verb, entry, black, radius=getattr(self, "careful_radius", 12.0))
                if tgt is not None:
                    self._careful_anchor = tuple(w.pos)
                if self.attackers() or self.is_dead():
                    w.pump(0.2)                  # never spin without reading the socket
                    continue
            else:
                # like a player: rest out of combat before the next pull when health is low (crowded camps killed
                # a low-level character pull after pull, then again right after each corpse run)
                me_ = self.me()
                if me_ is not None and me_.max_health > 0 and me_.health * 10 < me_.max_health * 6 and self.rest_s > 0:
                    log("%s: resting (hp=%d/%d)" % (verb, me_.health, me_.max_health))
                    t_rest = time.time()
                    while time.time() - t_rest < self.rest_s and not self.attackers() and not self.is_dead():
                        m2 = self.me()
                        if m2 is None or m2.health * 10 >= m2.max_health * 9:
                            break
                        w.pump(1.0)
                    last_prog = time.time()
                    if self.attackers() or self.is_dead():
                        continue
                tgt = self.nearest(entry, ("unit",), lambda o: o.alive() and o.attackable() and not o.tapped_by_other(),
                                   exclude=black)
            if tgt is None:
                if now - last_prog > stuck_s * 2:
                    self.stuck(verb, args, "no-target entry=%d visible=%d kills=%d"
                               % (entry, len(self.objects_of(entry, ("unit",))), kills), quest)
                if not self.objects_of(entry, ("unit",)):
                    sp = ([self.cur_near] if self.cur_near else []) or self.remembered(entry, ("unit",))                         or db_spawns("unit", entry, w.map_id)
                    if sp:
                        h = min(sp, key=lambda p: _d3(p, w.pos))
                        if _d3(h, w.pos) > 20:
                            self.walk_to(lambda: h, 10.0, verb, args, timeout=stuck_s * 4, stuck_s=stuck_s, quest=quest,
                                         abort_fn=lambda: bool(self.objects_of(entry, ("unit",))), fail="return")
                w.pump(1.0)
                continue
            log("%s: engaging %s" % (verb, tgt.short()))
            qsum0 = self._quest_count_sum(quest)
            res = self.fight(verb, args, tgt.guid, stuck_s, quest)
            if res in ("yielded", "gone") and quest:
                # sparring/duel NPCs give credit and despawn instead of dying: give the quest update a moment
                self._wait(lambda: self._quest_count_sum(quest) > qsum0 or self.quest_status(quest) in ("complete", "rewarded"), 3.0)
                if self._quest_count_sum(quest) > qsum0 or self.quest_status(quest) in ("complete", "rewarded"):
                    kills += 1
                    last_prog = time.time()
                    log("[KILL] entry=%d guid=%d kills=%d (%s, quest credit)" % (tgt.entry, tgt.guid, kills, res))
                else:
                    log("[KILL] entry=%d guid=%d %s with NO quest credit" % (tgt.entry, tgt.guid, res))
                black[tgt.guid] = time.time() + 120
                continue
            if res == "yielded":
                black[tgt.guid] = time.time() + 120
                continue
            if res == "dead":
                o = self.objs.get(tgt.guid)
                if o is not None and o.entry == int(entry):
                    kills += 1
                    last_prog = time.time()
                    log("[KILL] entry=%d guid=%d kills=%d" % (o.entry, o.guid, kills))
                    self.last_kill_t = time.time()
                elif o is not None:
                    log("[KILL] entry=%d guid=%d (attacker, not counted)" % (o.entry, o.guid))
                try:
                    w.attack_stop()
                except OSError:
                    pass
                if loot:
                    self.loot_unit(verb, args, tgt.guid)
            elif res in ("evade", "tapped"):
                log("%s: giving up on guid=%d (%s)" % (verb, tgt.guid, res))
                black[tgt.guid] = time.time() + 120
                w.attack_stop()
                if time.time() - last_prog > stuck_s * 3:
                    self.stuck(verb, args, "no-damage entry=%d kills=%d" % (entry, kills), quest)
            elif res == "died":
                continue
        return kills

    def corpse_run(self, verb, args, quest=None, timeout=420.0):
        """Death: CMSG_REPOP_REQUEST, walk back to the corpse as a ghost (CMSG_CORPSE_QUERY position), and
        CMSG_RECLAIM_CORPSE once in range and the reclaim delay has passed."""
        w = self.w
        death_pos = (w.map_id,) + tuple(w.pos)
        log("[DEATH] releasing spirit (died at %s)" % self.pos_str())
        t0 = time.time()
        if not self.is_ghost():
            w.release_spirit()
            while not self.is_ghost() and time.time() - t0 < 20:
                if not self.is_dead():
                    break                 # resurrected on the spot (a scripted revive, e.g. aboard the Vengeance Wake)
                w.pump(0.5)
            if not self.is_dead():
                log("[DEATH] resurrected without a corpse run (%s)" % self.pos_str())
                return
            if not self.is_ghost():
                self.stuck(verb, args, "release-failed", quest)
        w.pump(2.0)
        # HEADLESS_SPIRIT_RES_BELOW=N (opt-in): below level N the spirit healer costs nothing (no resurrection sickness before
        # level 10), so take it instead of walking back into the pack that killed the character
        lim = int(os.environ.get("HEADLESS_SPIRIT_RES_BELOW", "0") or 0)
        me_ = self.me()
        shp = self.spirit_healer_pos
        if lim and me_ is not None and me_.u32(UF_UNIT_LEVEL) < lim and shp and shp[0] == w.map_id:
            log("[DEATH] level %d < %d: resurrecting at the spirit healer" % (me_.u32(UF_UNIT_LEVEL), lim))
            # the spirit healer entry differs per zone (6491, 39660 on the Lost Isles...): take any visible unit with the
            # spirit-healer NPC flag (0x4000), waiting a moment for the graveyard to come into view
            sh_entry = None
            t_sh = time.time()
            while time.time() - t_sh < 10.0 and sh_entry is None:
                shs = [o for o in self.objs.values() if o.typeid == 3 and o.u32(UF_UNIT_NPC_FLAGS) & 0x4000 and o.position()]
                if shs:
                    sh_entry = min(shs, key=lambda o: _d3(o.position(), w.pos)).entry
                else:
                    w.pump(0.5)
            npc = self.goto_object(verb, args, sh_entry or 6491, ("unit",), 4.0, 300.0, 45.0, wait_s=10.0,
                                   near=tuple(shp[1:]))
            self.face(npc.position())
            w.gossip_hello(npc.guid)
            self._wait(lambda: self.spirit_confirm is not None, 3.0)
            self._guid_packet(CMSG_SPIRIT_HEALER_ACTIVATE, npc.guid, (2, 7, 6, 0, 5, 4, 1, 3), (1, 5, 6, 3, 2, 0, 7, 4))
            if self._wait(lambda: not self.is_dead(), 10.0):
                log("[DEATH] resurrected at the spirit healer (%s)" % self.pos_str())
                w.pump(1.0)
                return
            log("[DEATH] spirit healer resurrection failed - walking back to the corpse")
        self.corpse_pos = None
        self.corpse_query()
        w.pump(2.0)
        if os.environ.get("HEADLESS_SPIRIT_RES") == "1" and self.spirit_healer_pos and self.spirit_healer_pos[0] == w.map_id:
            # opt-in: take the spirit healer's resurrection (a real player's choice when the corpse lies in a mob pack);
            # when the healer cannot be reached (not in view, a STUCK) fall back to the ordinary corpse run
            shp = self.spirit_healer_pos
            try:
                w.pump(4.0)   # the graveyard's objects arrive a few seconds after the release teleport
                npc = self.goto_object(verb, args, 6491, ("unit",), 4.0, timeout, 45, wait_s=20, near=tuple(shp[1:]))
                self.face(npc.position())
                w.gossip_hello(npc.guid)
                self._wait(lambda: self.spirit_confirm is not None, 3.0)
                self._guid_packet(CMSG_SPIRIT_HEALER_ACTIVATE, npc.guid, (2, 7, 6, 0, 5, 4, 1, 3), (1, 5, 6, 3, 2, 0, 7, 4))
                log("sent CMSG_SPIRIT_HEALER_ACTIVATE guid=%d" % npc.guid)
                if self._wait(lambda: not self.is_dead(), 10.0):
                    log("[DEATH] resurrected by the spirit healer (%s)" % self.pos_str())
                    w.pump(1.0)
                    return
            except SystemExit:
                log("[DEATH] spirit healer not reachable - falling back to the corpse run")
        cp = self.corpse_pos or death_pos
        if cp[0] != w.map_id:
            self.stuck(verb, args, "corpse-on-other-map map=%s" % cp[0], quest)
        goal = tuple(cp[1:4])
        # like a player: reclaim from the edge of the reclaim radius (39 yd), not on top of the corpse, so the mobs that killed the
        # character do not catch it again at half health (a corpse in a mob pack gave a death loop)
        # HEADLESS_RECLAIM_TOWARD=x,y,z (opt-in): reclaim from the side of the corpse that faces a safe spot (a quest camp), up to
        # 35 yd from the body, so the character comes back to life next to safety instead of inside the pack
        tow = os.environ.get("HEADLESS_RECLAIM_TOWARD")
        if tow:
            try:
                tx, ty, tz = (float(v) for v in tow.split(","))
                dx, dy = tx - goal[0], ty - goal[1]
                dl = math.hypot(dx, dy)
                if 0 < dl < 200:
                    k = min(35.0, dl) / dl
                    goal = (goal[0] + dx * k, goal[1] + dy * k, goal[2] + (tz - goal[2]) * k)
                    log("[DEATH] reclaiming toward (%.0f, %.0f): from (%.1f, %.1f, %.1f)" % (tx, ty, goal[0], goal[1], goal[2]))
            except ValueError:
                pass
        if not tow and getattr(self, "careful", False):
            # careful: as a ghost the pack around the body is in sight - come back to life on the point of the
            # reclaim circle (33 yd) farthest from every live hostile unit (two reclaims inside a pack ended in two deaths)
            body = goal
            self.walk_to(lambda: body, 45.0, verb, args, timeout=timeout, stuck_s=45, quest=quest)
            foes = [o.position() for o in self.objs.values() if o.typeid == 3 and o.alive() and o.attackable()
                    and o.position() and _d3(o.position(), body) < 80.0]
            if foes:
                best_pt, best_d = None, -1.0
                for k in range(16):
                    ang = k * math.pi / 8.0
                    pt = (body[0] + 30.0 * math.cos(ang), body[1] + 30.0 * math.sin(ang), body[2])
                    dmin = min(math.hypot(pt[0] - f[0], pt[1] - f[1]) for f in foes)
                    if dmin > best_d:
                        best_pt, best_d = pt, dmin
                goal = best_pt
                tow = "edge"
                log("[DEATH] reclaiming from the quiet edge (%.1f, %.1f): nearest hostile %.0f yd (%d in sight)"
                    % (goal[0], goal[1], best_d, len(foes)))
        self.walk_to(lambda: goal, 1.5 if tow else float(os.environ.get("HEADLESS_RECLAIM_DIST", "28")), verb, args,
                     timeout=timeout, stuck_s=45, quest=quest)
        # the server ignores CMSG_RECLAIM_CORPSE from more than 39 yd (CORPSE_RECLAIM_RADIUS, Corpse.h): make sure we are
        # within 38 yd of the body before asking (an edge point reached within the walk tolerance can be just outside)
        body_pos = tuple(cp[1:4])
        if w.pos and _d3(w.pos, body_pos) > 38.0:
            log("[DEATH] %.1f yd from the body - closing in to reclaim" % _d3(w.pos, body_pos))
            self.walk_to(lambda: body_pos, 30.0, verb, args, timeout=60, stuck_s=30, quest=quest)
        t1 = time.time()
        # the reclaim delay grows with repeated deaths (Player::GetCorpseReclaimDelay), so keep asking for a while
        closed_in = False
        while self.is_dead() and time.time() - t1 < 360:
            corpse = next((o for o in self.objs.values() if o.typeid == 7 and o.u64(UF_CORPSE_OWNER) == w.char_guid), None)
            w.reclaim_corpse(corpse.guid if corpse else w.char_guid)
            w.pump(5.0)
            if not closed_in and self.is_dead() and time.time() - t1 > 75.0 and w.pos and _d3(w.pos, body_pos) > (12.0 if os.environ.get("HEADLESS_CAREFUL", "0") in ("0", "") else 36.0):
                # the reclaim is silently ignored (no reply packet) when the ghost is not really inside the radius, e.g. its
                # last walk did not reach the edge point: after 75 s go to the body itself and ask there
                # (HEADLESS_CAREFUL: only when really outside the 39 yd radius - at 75 s the reclaim delay of repeated deaths is still running, and
                # walking onto the body walked the half-health ghost into the tiger that guards it: a death loop on the Wandering Isle)
                closed_in = True
                log("[DEATH] reclaim ignored for 75 s at %.1f yd - walking to the body" % _d3(w.pos, body_pos))
                try:
                    self.walk_to(lambda: body_pos, 5.0, verb, args, timeout=60, stuck_s=30, quest=quest)
                except SystemExit:
                    pass
        if self.is_dead():
            self.stuck(verb, args, "reclaim-failed", quest)
        log("[DEATH] resurrected at the corpse (%s)" % self.pos_str())
        w.pump(1.0)
        if tow == "edge" and len(self.attackers()) <= 2:
            # careful: alive at half health on the quiet edge - walk 30 yd further out, away from the body and the
            # pack, before resting (a rest on the reclaim spot was found by the pack 20 s later); one or two units already
            # on the character are led out too and fought there, away from the camp, by the step that follows
            vx, vy = goal[0] - body[0], goal[1] - body[1]
            vl = math.hypot(vx, vy) or 1.0
            out = (goal[0] + 30.0 * vx / vl, goal[1] + 30.0 * vy / vl, goal[2])
            try:
                self.walk_to(lambda: out, 4.0, verb, args, timeout=40, stuck_s=12, quest=quest)
            except SystemExit:
                pass

    # ------------------------------------------------------------------ quest giver flows
    def _wait(self, cond, timeout, step=0.25):
        end = time.time() + timeout
        while time.time() < end:
            if cond():
                return True
            self.w.pump(step)
        return cond()

    def accept(self, verb, args, entry, qid, kv):
        st = self.quest_status(qid)
        if st in ("incomplete", "complete"):
            self.ok(verb, args, "already-in-log")
            return
        is_go = kv.get("go", "0") not in ("0", "")
        stuck_s = float(kv.get("stuck", 30))
        npc = self.goto_object(verb, args, entry, ("go",) if is_go else ("unit",), float(kv.get("range", 3.5)),
                               float(kv.get("timeout", 300)), stuck_s, quest=qid)
        for attempt in range(3):
            self.quest_details = None
            self.quest_invalid = None
            if is_go:
                self.w.gameobject_use(npc.guid)
            else:
                self.face(npc.position())
                self.questgiver_hello(npc.guid)
            self._wait(lambda: self.w.gossip is not None or self.quest_details is not None, 3.0)
            if self.quest_details != qid:
                self.questgiver_query(npc.guid, qid)
                self._wait(lambda: self.quest_details == qid or self.quest_invalid is not None, 5.0)
            if self.quest_status(qid) not in ("incomplete", "complete"):
                self.w.quest_accept(qid, npc.guid)
            if self._wait(lambda: self.quest_status(qid) in ("incomplete", "complete"), 8.0):
                self.ok(verb, args, "state=%s" % self.quest_state_str(qid))
                return
            if self.quest_invalid is not None:
                break
        self.stuck(verb, args, "accept-failed%s" % (" invalid=%d" % self.quest_invalid[1] if self.quest_invalid else ""), qid)

    def choose_reward(self, verb, args, guid, qid, index):
        off = self.offer
        choices = off["choices"] if off and off["quest"] == qid else []
        item = choices[index] if 0 <= index < len(choices) else (choices[0] if choices else 0)
        self.w.quest_reward(qid, item, guid)
        if self._wait(lambda: qid in self.quest_rewarded, 8.0):
            self.ok(verb, args, "reward-item=%d" % item)
            return True
        return False

    def turnin(self, verb, args, entry, qid, kv):
        if qid in self.quest_rewarded and self.quest_status(qid) == "rewarded":
            self.ok(verb, args, "already-rewarded")
            return
        if self.quest_status(qid) == "none":
            self.stuck(verb, args, "quest-not-in-log", qid)
        is_go = kv.get("go", "0") not in ("0", "")
        idx = int(kv.get("reward", 0))
        npc = self.goto_object(verb, args, entry, ("go",) if is_go else ("unit",), float(kv.get("range", 3.5)),
                               float(kv.get("timeout", 300)), float(kv.get("stuck", 30)), quest=qid)
        for attempt in range(3):
            self.offer = None
            self.request_items = None
            if is_go:
                self.w.gameobject_use(npc.guid)
            else:
                self.face(npc.position())
                self.questgiver_hello(npc.guid)
            self.w.pump(1.0)
            self.w.quest_complete(qid, npc.guid)
            self._wait(lambda: (self.offer and self.offer["quest"] == qid)
                       or (self.request_items and self.request_items["quest"] == qid), 5.0)
            if self.request_items and self.request_items["quest"] == qid and not self.offer:
                if not self.request_items["can_complete"]:
                    self.stuck(verb, args, "quest-items-missing", qid)
                self.questgiver_request_reward(npc.guid, qid)
                self._wait(lambda: self.offer and self.offer["quest"] == qid, 5.0)
            if self.offer and self.offer["quest"] == qid:
                if self.choose_reward(verb, args, npc.guid, qid, idx):
                    return
            if qid in self.quest_rewarded:
                self.ok(verb, args)
                return
        err = getattr(self, "quest_reward_error", None)
        self.stuck(verb, args, "turnin-failed status=%s%s" % (
            self.quest_status(qid), " reward-error=%d" % err[2] if err and err[1] == qid else ""), qid)

    # ------------------------------------------------------------------ cinematics, action bars, conditions
    def tick(self):
        now = time.time()
        if self.cine_complete_at and now >= self.cine_complete_at:
            self.cine_complete_at = 0.0
            self.w.send_packet(CMSG_COMPLETE_CINEMATIC, b"")
            log("[CINEMATIC] sent CMSG_COMPLETE_CINEMATIC")
        if self.movie_complete_at and now >= self.movie_complete_at:
            self.movie_complete_at = 0.0
            self.w.send_packet(CMSG_COMPLETE_MOVIE, b"")
            log("[CINEMATIC] sent CMSG_COMPLETE_MOVIE")

    def _parse_action_bar(self, body):
        """SMSG_PET_SPELLS_MESSAGE (Player::PetSpellInitialize / PossessSpellInitialize / VehicleSpellInitialize):
        guid mask + three counts in bits, then the 10-button action bar (u32 MAKE_UNIT_ACTION_BUTTON(spell, type))."""
        if len(body) < 8 + 40:
            self.action_bar = []
            log("[ACTIONBAR] cleared")
            return
        r = ByteReader(body)
        r.read_bits(2)
        r.read_bits(21)
        r.read_bits(22)
        r.read_bit()
        r.read_bits(20)
        r.read_bits(5)
        bar = [r.read_u32() for _ in range(10)]
        self.action_bar = [(v & 0xFFFFFF, v >> 24) for v in bar]
        self.action_bar_t = time.time()
        log("[ACTIONBAR] %s" % " ".join("%d:%d/0x%X" % (i + 1, s, t) for i, (s, t) in enumerate(self.action_bar) if s))

    def bar_spell(self, tok):
        """SLOT (1-10, an action-bar button) or a spell id -> (spellId, raw button value)."""
        n = int(tok)
        if 1 <= n <= 10 and self.action_bar and self.action_bar[n - 1][0]:
            s, t = self.action_bar[n - 1]
            return s, s | (t << 24)
        for s, t in self.action_bar:
            if s == n:
                return s, s | (t << 24)
        return n, n | (0xC1 << 24)          # ACT_ENABLED spell

    def controlled_guid(self):
        me = self.me()
        if me is None:
            return 0
        return me.u64(UF_UNIT_CHARM) or me.u64(UF_UNIT_SUMMON)

    def pet_action(self, pet, action, target=0, pos=(0.0, 0.0, 0.0)):
        """CMSG_PET_ACTION (PetHandler.cpp:116): u32 action, y z x, pet/target guid bits and bytes interleaved."""
        pb, tb = int(pet).to_bytes(8, "little"), int(target).to_bytes(8, "little")
        bw = BitWriter()
        for which, idx in (("p", 1), ("p", 0), ("p", 6), ("p", 7), ("p", 5), ("t", 7), ("p", 2), ("p", 3), ("t", 6),
                           ("t", 3), ("t", 0), ("t", 2), ("t", 5), ("p", 4), ("t", 4), ("t", 1)):
            bw.bit((pb if which == "p" else tb)[idx] != 0)
        body = bytearray(struct.pack("<Ifff", int(action), float(pos[1]), float(pos[2]), float(pos[0])) + bw.flush())
        for which, idx in (("p", 7), ("p", 6), ("p", 1), ("p", 2), ("p", 5), ("p", 4), ("t", 5), ("p", 3), ("t", 0),
                           ("t", 1), ("t", 7), ("t", 4), ("t", 6), ("t", 2), ("t", 3), ("p", 0)):
            b = (pb if which == "p" else tb)[idx]
            if b:
                body.append(b ^ 1)
        self.w.send_packet(CMSG_PET_ACTION, bytes(body))
        log("sent CMSG_PET_ACTION action=0x%08X pet=%d target=%d" % (int(action), pet, target))

    def cond(self, spec):
        """until= conditions: quest:Q:complete|rewarded|accepted|none, obj:Q:i:COUNT, level:N, item:ENTRY:N,
        vehicle:on|off, map:M, dead:0|1, visible:ENTRY, gone:ENTRY."""
        p = spec.split(":")
        k = p[0]
        if k == "quest":
            st = self.quest_status(int(p[1]))
            want = p[2] if len(p) > 2 else "complete"
            return {"complete": st in ("complete", "rewarded"), "rewarded": st == "rewarded",
                    "accepted": st in ("incomplete", "complete", "rewarded"), "none": st == "none"}.get(want, st == want)
        if k == "obj":
            q = self.quest_log().get(int(p[1]))
            if q is None:
                return self.quest_status(int(p[1])) == "rewarded"
            return q["counts"][int(p[2])] >= int(p[3]) or bool(q["state"] & QUEST_STATE_COMPLETE)
        if k == "level":
            me = self.me()
            return bool(me) and me.u32(UF_UNIT_LEVEL) >= int(p[1])
        if k == "item":
            return self.item_count(int(p[1])) >= int(p[2] if len(p) > 2 else 1)
        if k == "vehicle":
            return self.vehicle == (p[1] in ("on", "1"))
        if k == "map":
            return self.w.map_id == int(p[1])
        if k == "dead":
            return self.is_dead() == (p[1] == "1")
        if k == "visible":
            return any(o.alive() or o.kind == "go" for o in self.objects_of(int(p[1]), ("unit", "go")))
        if k == "gone":
            return not self.objects_of(int(p[1]), ("unit", "go"))
        raise ValueError("unknown condition %r" % spec)

    def fight_nearby(self, verb, args, stuck_s, quest=None, entries=(), radius=30.0, center=None, protect=None, avoid=()):
        """One engagement: an attacker of this character (or of `protect`), else a live unit of `entries` within
        `radius` of `center`. Returns True when something was fought. avoid=[(entry, R)]: no target standing within R yd
        of a live unit of that entry (follow avoid=)."""
        tgt = None
        # careful: never take on a far stronger unit (Gorerot 38331, 81900 hp, one-shot a level 13 warrior in the
        # 24904 battle) - keep doing the step (follow Liam, hold the spot) and let the NPC army deal with it
        att = [a for a in self.attackers() if not self.too_strong(a)]
        if protect:
            att += [o for o in self.objs.values() if o.typeid == 3 and o.alive() and o.u64(UF_UNIT_TARGET) == protect
                    and o.unit_flags & UNIT_FLAG_IN_COMBAT and o.position() and _d3(o.position(), self.w.pos) < radius
                    and not self.too_strong(o)]
        if avoid:
            att = [a for a in att if a.position() and not self.in_avoid(a.position(), avoid)]
        if att:
            tgt = self.pick_attacker(att)
        elif entries:
            c = center or self.w.pos
            cands = [o for e in entries for o in self.objects_of(e, ("unit",))
                     if o.alive() and o.attackable() and not o.tapped_by_other() and _d3(o.position(), c) <= radius]
            if cands:
                tgt = min(cands, key=lambda a: _d3(a.position(), self.w.pos))
        if tgt is None:
            return False
        log("%s: fighting %s" % (verb, tgt.short()))
        res = self.fight(verb, args, tgt.guid, stuck_s, quest)
        if res == "died":
            self.corpse_run(verb, args, quest)
        if res == "dead":
            o = self.objs.get(tgt.guid)
            log("[KILL] entry=%d guid=%d (%s)" % (o.entry if o else 0, tgt.guid, verb))
        try:
            self.w.attack_stop()
        except OSError:
            pass
        if getattr(self, "careful", False):
            self._careful_rest(verb)       # careful pull: back to full health before the next fight
        return True


def _kv_split(rest):
    kv = dict(t.split("=", 1) for t in rest if "=" in t)
    pos = [t for t in rest if "=" not in t]
    return pos, kv


def _xyz(tok):
    return tuple(float(v) for v in tok.split(","))


def _is_int(tok):
    try:
        int(tok)
        return True
    except (TypeError, ValueError):
        return False


PLAY_VERBS = ("goto-path", "goto-npc", "goto-go", "kill", "accept", "accept-go", "turnin", "turnin-go", "talk",
              "use-go", "use-item", "spellclick", "collect", "wait-quest", "vehicle-wait", "wait-map", "wait-level",
              "choose-reward", "hold", "follow", "pet-cast", "vehicle-cast", "quest-log", "inventory", "objects",
              "equip", "pet-attack", "pet-follow", "pet-stay", "vehicle-exit", "relog", "spirit-healer", "sell", "destroy", "cast-on", "buy", "react", "abandon", "repair", "rest", "probe-attack", "pet-passive", "pet-defensive", "pet-assist", "select-faction",
              "gear-up", "spec", "leave-camp", "select-nearest")


def run_play_verb(w, args_cli, verb, rest):
    """The real-player verbs. Returns False when the line is an old-style verb of the same name (group `accept`,
    bag/slot `use-item`, guid `spellclick`, GM `kill` without an entry) so run_verb handles it as before."""
    ps = w.play
    pos, kv = _kv_split(rest)
    if verb == "accept" and len(pos) < 2:
        return False
    if verb == "use-item" and len(pos) >= 2 and _is_int(pos[0]) and _is_int(pos[1]):
        return False
    if verb == "spellclick" and (not pos or not _is_int(pos[0]) or int(pos[0]) > 0xFFFFFFFF):
        return False
    if verb == "kill" and (not pos or not _is_int(pos[0])):
        return False
    if verb == "equip" and (len(pos) != 1 or not _is_int(pos[0]) or kv.get("gm") == "1"):
        return False
    args = list(rest)
    ps.cur_near = _xyz(kv["near"]) if "near" in kv else None
    # HEADLESS_CAREFUL_WITHIN=R (opt-in): the default within= for every step (0 = the old "prefer 25 yd of near=, else any" pick,
    # which spread-out targets such as the 24954 children and the 25024 paratroopers need)
    _wdef = os.environ.get("HEADLESS_CAREFUL_WITHIN", "30")
    ps.cur_within = float(kv.get("within", _wdef)) if "near" in kv else float(_wdef)
    ps.max_deaths = int(kv.get("deaths", 3))
    ps.max_dry_kills = int(kv.get("dry", 10))
    # careful pull (kill / collect from= / hold): least-crowded target, rest to full, back off from adds.
    # Also on for every verb when HEADLESS_CAREFUL=1.
    ps.careful = kv.get("careful", os.environ.get("HEADLESS_CAREFUL", "0")) not in ("0", "")
    # crowd=R: the careful pull counts the other mobs within R yd of a candidate (default 12; the Oomlot shamans that joined a
    # "lone" pull came from 23-25 yd)
    ps.careful_radius = float(kv.get("crowd", os.environ.get("HEADLESS_CAREFUL_CROWD", "12")))
    # onlyif=x,y,z,R: run the step only when the character is within R yd of that point (a waypoint walk that only applies from one
    # starting area, e.g. out of a graveyard; later iterations of a resumed block skip it)
    if "onlyif" in kv and not ps.is_dead():
        try:
            _ox, _oy, _oz, _orr = (float(v_) for v_ in kv["onlyif"].split(","))
            if w.pos is not None and _d3(w.pos, (_ox, _oy, _oz)) > _orr:
                log("%s: onlyif %s - not near, skipped" % (verb, kv["onlyif"]))
                return True
        except ValueError:
            pass
    # dead (or a ghost from an earlier session): do what a player does first - run back and take the body
    if ps.is_dead() and verb not in ("spirit-healer", "quest-log", "inventory", "objects", "relog", "wait-quest",
                                     "wait-map", "wait-level", "vehicle-wait"):
        ps.corpse_run(verb, args, quest=int(kv["quest"]) if "quest" in kv else None)
    # HEADLESS_AUTOFIGHT=1 (opt-in): like a player, fight off what is attacking before a non-combat step (walking, talking,
    # equipping, using). Without it, mobs that followed the character into a quest camp killed it during turn-ins and equips.
    if (os.environ.get("HEADLESS_AUTOFIGHT", "0") not in ("0", "") and not ps.vehicle and not ps.is_dead()
            and verb in ("goto-path", "goto-npc", "goto-go", "accept", "accept-go", "turnin", "turnin-go", "talk", "equip",
                         "use-item", "use-go", "spellclick", "cast-on", "buy", "sell", "repair", "follow")):
        for _n in range(8):
            att = ps.attackers()
            if not att or ps.is_dead():
                break
            a0 = ps.pick_attacker(att)
            log("%s: autofight - %s attacks first" % (verb, a0.short()))
            ps.fight(verb, list(rest), a0.guid, 30.0, None)
    if (ps.char_class == 3 and not ps.is_dead() and not ps.vehicle and os.environ.get("HEADLESS_PET", "1") == "1"
            and verb in ("goto-path", "goto-npc", "goto-go", "kill", "collect", "follow", "hold", "accept", "turnin", "talk")
            and not ps.controlled_guid() and 883 in ps.known_spells):
        # a hunter's pet is a tank: after a login, a death or a vehicle ride there is none out. Call Pet (883); a dead pet
        # fails the cast, then Revive Pet (982) (the pet then needs 883 again)
        try:
            for _try in range(2):
                t_p = time.time()
                w.cast(883)
                res_p = ps._cast_result(883, t_p, 6.0)
                w.pump(1.0)
                if ps.controlled_guid():
                    log("%s: hunter pet called (Call Pet 883)" % verb)
                    break
                if 982 in ps.known_spells:
                    t_p = time.time()
                    w.cast(982)
                    ps._cast_result(982, t_p, 12.0)
                    w.pump(1.0)
                    log("%s: Call Pet failed (result=%s) - cast Revive Pet 982" % (verb, res_p))
        except OSError:
            pass
    if (ps.char_class == 3 and not ps.is_dead() and verb in ("use-item", "use-go", "talk", "accept", "accept-go", "turnin",
            "turnin-go", "equip", "spellclick", "collect", "goto-path", "goto-npc", "goto-go")):
        # a hunter's Auto Shot keeps repeating at a dead target: every tick fails BAD_TARGETS (13) and the flood both
        # swamps the log and is read as the failure of the next cast (use-item cast-failed=13)
        try:
            w.cancel_auto_repeat()
        except OSError:
            pass
    stuck_s = float(kv.get("stuck", 30))
    timeout = float(kv.get("timeout", 300))
    quest = int(kv["quest"]) if "quest" in kv else None

    if verb == "goto-path":
        # goto-path x,y,z [map=M] [range=R] [timeout=S] [stuck=S]
        goal = _xyz(pos[0])
        if "map" in kv and w.map_id != int(kv["map"]):
            ps.stuck(verb, args, "wrong-map have=%s want=%s" % (w.map_id, kv["map"]))
        ps.extra_move_flags = 0x100000 if kv.get("swim", "0") not in ("0", "") else 0    # MOVEMENTFLAG_SWIMMING
        if kv.get("fly", "0") not in ("0", ""):
            ps.extra_move_flags |= 0x1000000 | 0x800000     # MOVEMENTFLAG_FLYING | CAN_FLY (a flying vehicle this client drives)
        try:
            ps.walk_to(lambda: goal, float(kv.get("range", 1.5)), verb, args, timeout=timeout, stuck_s=stuck_s)
        finally:
            ps.extra_move_flags = 0
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb in ("goto-npc", "goto-go"):
        # goto-npc ENTRY [range=R] [near=x,y,z] [wait=S] [timeout=S] / goto-go ENTRY [...]
        kinds = ("unit",) if verb == "goto-npc" else ("go",)
        o = ps.goto_object(verb, args, int(pos[0]), kinds, float(kv.get("range", 3.0)), timeout, stuck_s,
                           wait_s=float(kv.get("wait", 8)), near=_xyz(kv["near"]) if "near" in kv else None)
        ps.ok(verb, args, "guid=%d dist=%.1f" % (o.guid, _d3(o.position(), w.pos)) if o else "")
    elif verb == "kill":
        # kill ENTRY [count=N] [quest=Q] [loot=1] [timeout=S] [stuck=S]
        entry = int(pos[0])
        count = int(kv["count"]) if "count" in kv else (None if quest else 1)
        loot = kv.get("loot", "0") not in ("0", "")

        def done(kills):
            if count is not None and kills >= count:
                return True
            # until=COND (e.g. obj:Q:i:N): ends the kill when that objective is already done (a resumed run)
            if "until" in kv and ps.cond(kv["until"]):
                return True
            if quest:
                st = ps.quest_status(quest)
                if st in ("complete", "rewarded"):
                    return True
                c = ps.quest_credit.get((quest, entry))
                if c and c[0] >= c[1] > 0:
                    return True
            return False
        # seated in a vehicle: the server drops melee from a passenger (ATTACKSTOP after each swing). Step out first, or
        # STUCK in-vehicle; vehicle=1 keeps the seat (a vehicle form that kills by itself, e.g. the Weed Whacker).
        if ps.vehicle and ps.vehicle_guid and kv.get("vehicle", "0") != "1":
            log("%s: seated in vehicle %d - stepping out first" % (verb, ps.vehicle_guid))
            w.send_packet(CMSG_REQUEST_VEHICLE_EXIT, b"")
            if not ps._wait(lambda: not ps.vehicle, 8.0):
                ps.stuck(verb, args, "in-vehicle guid=%d" % (ps.vehicle_guid or 0), quest)
            w.move_guid = None
        # nopet=1: do not send the pet in (fight alone while the pet stays back, e.g. after pet-stay)
        ps.no_pet_attack = kv.get("nopet", "0") == "1"
        try:
            n = ps.kill_loop(verb, args, entry, done, loot, float(kv.get("timeout", 900)), stuck_s, quest)
        except SystemExit as e_:
            # optional=1: no target in reach / a STUCK is not fatal (e.g. clearing the tigers on a road: none may be left)
            if kv.get("optional", "0") != "1" or e_.code != 3:
                raise
            n = 0
            log("%s: optional - skipped" % verb)
        finally:
            ps.no_pet_attack = False
        ps.ok(verb, args, "kills=%d quest=%s" % (n, ps.quest_state_str(quest)))
    elif verb in ("accept", "accept-go"):
        if verb == "accept-go":
            kv["go"] = "1"
        ps.accept(verb, args, int(pos[0]), int(pos[1]), kv)
    elif verb in ("turnin", "turnin-go"):
        if verb == "turnin-go":
            kv["go"] = "1"
        ps.turnin(verb, args, int(pos[0]), int(pos[1]), kv)
    elif verb == "wait-level":
        # wait-level N [timeout=S]
        want = int(pos[0])
        if not ps._wait(lambda: ps.cond("level:%d" % want), float(kv.get("timeout", 60)), 0.5):
            me = ps.me()
            ps.stuck(verb, args, "level=%d" % (me.u32(UF_UNIT_LEVEL) if me else 0))
        ps.ok(verb, args, "level=%d" % ps.me().u32(UF_UNIT_LEVEL))
    elif verb == "hold":
        # hold x,y,z seconds=S [fight=1] [entries=a,b] [radius=R] [until=COND] [leash=R]
        spot = _xyz(pos[0])
        secs = float(kv.get("seconds", 60))
        fight_on = kv.get("fight", "1") not in ("0", "")
        entries = [int(e) for e in kv["entries"].split(",")] if "entries" in kv else []
        radius = float(kv.get("radius", 30))
        leash = float(kv.get("leash", 8))
        until = kv.get("until")
        ps.walk_to(lambda: spot, 2.0, verb, args, timeout=timeout, stuck_s=stuck_s, quest=quest)
        if getattr(ps, "careful", False):
            ps._careful_rest(verb)   # a careful hold starts at full health (it was walked into at half after a death)
        t0 = time.time()
        fought = 0
        while time.time() - t0 < secs and not (until and ps.cond(until)):
            if ps.is_dead():
                ps.corpse_run(verb, args, quest)
                if getattr(ps, "careful", False):
                    ps._careful_rest(verb)
            if fight_on and ps.fight_nearby(verb, args, stuck_s, quest, entries, radius, center=spot):
                fought += 1
                continue
            if _d3(w.pos, spot) > leash:
                ps.walk_to(lambda: spot, 2.0, verb, args, timeout=timeout, stuck_s=stuck_s, quest=quest)
            w.pump(0.5)
        ps.ok(verb, args, "fights=%d until=%s quest=%s" % (fought, until and int(ps.cond(until)), ps.quest_state_str(quest)))
    elif verb == "follow":
        # follow ENTRY [range=R] [until=COND] [timeout=S] [fight=1] [lost=S] [avoid=ENTRY:R[,ENTRY:R]]
        # avoid= : never step within R yd of a live unit of that entry; when the escort stands inside that radius,
        # hold at the nearest point outside it (Liam held the 24904 jail next to Gorerot 38331, each new follow leg walked
        # Wolfplay onto Gorerot and his Smash 71774 one-shot it)
        avoid = [(int(a.split(":")[0]), float(a.split(":")[1])) for a in kv["avoid"].split(",")] if kv.get("avoid") else []
        # sticky=1 : keep following the same unit across follow steps (and relogs); when it is out of view, walk to
        # where it was last seen instead of taking the nearest other unit of ENTRY - after Gorerot died the march leader
        # Liam walked on out of view and the next leg took the static Liam 38218 world spawn at the jail instead
        sticky = kv.get("sticky", "0") not in ("0", "")
        if not hasattr(ps, "follow_sticky"):
            ps.follow_sticky = {}
        entry = int(pos[0])
        rng = float(kv.get("range", 8))
        until = kv.get("until")
        fight_on = kv.get("fight", "1") not in ("0", "")
        tmo = float(kv.get("timeout", 1800))
        lost_s = float(kv.get("lost", 45))
        t0 = time.time()
        guid, last_seen = None, time.time()
        if sticky and entry in ps.follow_sticky:
            guid = ps.follow_sticky[entry][0]
        while not (until and ps.cond(until)):
            if time.time() - t0 > tmo:
                if until:
                    ps.stuck(verb, args, "timeout-until=%s" % until, quest)
                break
            if ps.is_dead():
                ps.corpse_run(verb, args, quest)
            o = ps.objs.get(guid) if guid else None
            if sticky and guid and (o is None or not o.position() or not o.alive()) and entry in ps.follow_sticky:
                sg, sp, st = ps.follow_sticky[entry]
                if time.time() - st < max(lost_s, 120.0):
                    if sp and w.pos and _d3(w.pos, sp) > 8.0:
                        log("%s: sticky - guid=%d out of view, walking to where it was last seen (%.1f, %.1f)" % (verb, sg, sp[0], sp[1]))
                        gp = ps.avoid_point(sp, avoid) if avoid else sp
                        ps.walk_to(lambda: gp, 4.0, verb, args, timeout=90, stuck_s=max(stuck_s, 45), quest=quest, fail="return",
                                   abort_fn=lambda: (sg in ps.objs and ps.objs[sg].position() is not None) or bool(ps.attackers()))
                    w.pump(0.5)
                    if sg in ps.objs and ps.objs[sg].position() is not None:
                        continue
                    if fight_on and ps.attackers():
                        ps.fight_nearby(verb, args, stuck_s, quest, avoid=avoid)
                    continue
                log("%s: sticky - guid=%d not seen for %.0f s, taking the nearest %d" % (verb, sg, time.time() - st, entry))
                del ps.follow_sticky[entry]
                guid, o = None, None
            if o is None or not o.position():
                o = ps.nearest(entry, ("unit",), lambda x: x.alive())
                guid = o.guid if o else None
            if sticky and o is not None and o.position():
                ps.follow_sticky[entry] = (o.guid, o.position(), time.time())
            if o is None:
                if time.time() - last_seen > lost_s:
                    ps.stuck(verb, args, "lost-entry=%d" % entry, quest)
                w.pump(0.5)
                continue
            last_seen = time.time()
            if avoid and w.pos:
                hit = ps.in_avoid(w.pos, avoid)
                if hit is not None:
                    out = ps.avoid_point(tuple(w.pos), avoid, margin=3.0)
                    log("%s: avoid - within %.0f yd of %s, stepping out to (%.1f, %.1f)" % (verb, hit[1], hit[0].short(), out[0], out[1]))
                    try:
                        w.attack_stop()
                    except OSError:
                        pass
                    ps.walk_to(lambda: out, 1.5, verb, args, timeout=20, stuck_s=8, quest=quest, fail="return")
                    w.pump(0.5)
                    continue
            # careful: an elite that turned on this character (Gorerot's 71774 smash) - pull back ~25 yd straight away
            # from it and come back to the escort once an NPC holds it again (too_strong() is False then)
            if getattr(ps, "careful", False) and fight_on:
                big = [a for a in ps.attackers() if ps.too_strong(a) and a.position()]
                if big:
                    b = min(big, key=lambda a: _d3(a.position(), w.pos))
                    bp = b.position()
                    if _d3(bp, w.pos) < 25.0:
                        vx, vy = w.pos[0] - bp[0], w.pos[1] - bp[1]
                        vl = math.hypot(vx, vy) or 1.0
                        away = (bp[0] + 27.0 * vx / vl, bp[1] + 27.0 * vy / vl, w.pos[2])
                        log("%s: %s is on this character - pulling back to (%.1f, %.1f)" % (verb, b.short(), away[0], away[1]))
                        try:
                            w.attack_stop()
                        except OSError:
                            pass
                        ps.walk_to(lambda: away, 3.0, verb, args, timeout=20, stuck_s=8, quest=quest, fail="return")
                        w.pump(1.0)
                        continue
            if fight_on and ps.fight_nearby(verb, args, stuck_s, quest, protect=guid, radius=25.0, avoid=avoid):
                if getattr(ps, "careful", False):
                    # rest between fights like a player (fight after fight killed Wolfplay), but only while the
                    # escort stands still - resting while Liam marched on left the character behind in the respawns
                    lp0 = ps.objs[guid].position() if guid in ps.objs else None
                    w.pump(1.5)
                    lp1 = ps.objs[guid].position() if guid in ps.objs else None
                    if lp0 and lp1 and _d3(lp0, lp1) < 2.0:
                        ps._careful_rest(verb, max_s=10)
                continue
            if avoid and ps.in_avoid(o.position(), avoid) is not None:
                # the escort stands inside an avoided radius: hold at the nearest point outside it
                g = guid
                goal_fn = lambda: (ps.avoid_point(ps.objs[g].position(), avoid) if g in ps.objs and ps.objs[g].position() else None)
                gp = goal_fn()
                if gp and _d3(w.pos, gp) > 4.0:
                    ps.walk_to(goal_fn, 2.0, verb, args, timeout=60, stuck_s=max(stuck_s, 45), quest=quest, fail="return",
                               abort_fn=lambda: bool(ps.attackers()) or ps.in_avoid(w.pos, avoid) is not None)
                else:
                    w.pump(0.5)
            elif _d3(w.pos, o.position()) > rng:
                g = guid
                ps.walk_to(lambda: (ps.objs[g].position() if g in ps.objs else None), rng * 0.6, verb, args,
                           timeout=60, stuck_s=max(stuck_s, 45), quest=quest, fail="return",
                           abort_fn=(lambda: bool(ps.attackers()) or (bool(avoid) and ps.in_avoid(w.pos, avoid) is not None))
                           if fight_on else ((lambda: ps.in_avoid(w.pos, avoid) is not None) if avoid else None))
            else:
                w.pump(0.5)
        ps.ok(verb, args, "until=%s quest=%s" % (until, ps.quest_state_str(quest)))
    elif verb == "pet-cast":
        # pet-cast SLOT|SPELL [target=ENTRY|at=x,y,z]  (CMSG_PET_ACTION with the button the server sent)
        pet = ps.controlled_guid()
        if not pet:
            ps.stuck(verb, args, "no-pet-or-charm")
        spell, button = ps.bar_spell(pos[0])
        tgt, at = 0, (0.0, 0.0, 0.0)
        if "target" in kv:
            o = ps.nearest(int(kv["target"]), ("unit", "go"), lambda x: x.kind == "go" or x.alive())
            if o is None:
                ps.stuck(verb, args, "target-not-visible=%s" % kv["target"], quest)
            tgt = o.guid
        elif "at" in kv:
            at = _xyz(kv["at"])
        t0 = time.time()
        ps.pet_action(pet, button, tgt, at)
        w.pump(float(kv.get("wait", 1.0)))
        ps.ok(verb, args, "spell=%d pet=%d" % (spell, pet))
    elif verb == "vehicle-cast":
        # vehicle-cast SLOT|SPELL [target=ENTRY|at=x,y,z] [aim=PITCH] [speed=S]
        # CMSG_CAST_SPELL: the server casts it from the vehicle when the vehicle has the spell. `at=` adds the
        # ground target; `aim=` (radians, default 0.785 with at=) and the missile speed derived from the distance
        # fill the 5.4.8 elevation / missile-speed fields used by trajectory spells (catapults, throwers, bombs).
        spell, _button = ps.bar_spell(pos[0])
        tgt, dest, elev, spd = None, None, None, None
        if "target" in kv:
            o = ps.nearest(int(kv["target"]), ("unit", "go"), lambda x: x.kind == "go" or x.alive(),
                           exclude=ps.used_targets.get((verb, int(kv["target"])), set()) if kv.get("fresh") == "1" else ())
            if o is not None and "cluster" in kv:
                # cluster=R: aim an area spell at the live target with the most others of that entry within R yd
                # (nearest-target aiming killed too few Alliance Sailors before the 15-minute cyclone ran out)
                cr = float(kv["cluster"])
                live = [x for x in ps.objects_of(int(kv["target"]), ("unit",)) if x.alive() and x.position()]
                if live:
                    o = max(live, key=lambda x: (sum(1 for y in live if _d3(x.position(), y.position()) <= cr),
                                                 -_d3(x.position(), w.pos)))
            if o is None and kv.get("optional", "0") == "1":
                # optional=1: nothing to shoot in view yet (fighters that fly in around a moving vehicle) is OK
                ps.ok(verb, args, "none-in-view")
                return True
            if o is None:
                ps.stuck(verb, args, "target-not-visible=%s" % kv["target"], quest)
            tgt = o.guid
            if kv.get("fresh") == "1":
                ps.used_targets.setdefault((verb, int(kv["target"])), set()).add(o.guid)
            if "aim" in kv or kv.get("ground") == "1":
                dest = o.position()
                # beyond=D: aim D yd past the target, on the line from the vehicle, so a knockback from the impact point
                # pushes the target toward the shooter instead of away (24681 War Machines knocked uphill out of sight)
                if dest and "beyond" in kv and w.pos:
                    bx, by = dest[0] - w.pos[0], dest[1] - w.pos[1]
                    bl = math.hypot(bx, by) or 1.0
                    bd = float(kv["beyond"])
                    dest = (dest[0] + bd * bx / bl, dest[1] + bd * by / bl, dest[2])
        if "at" in kv:
            dest = _xyz(kv["at"])
        if dest is not None:
            if "aim" in kv or "speed" in kv or kv.get("ground") == "1" or "at" in kv:
                elev = float(kv.get("aim", 0.785))
                src = w.pos
                x = max(1.0, _d2(src, dest))
                h = dest[2] - src[2]
                denom = 2.0 * math.cos(elev) ** 2 * (x * math.tan(elev) - h)
                spd = float(kv["speed"]) if "speed" in kv else (math.sqrt(19.2911 * x * x / denom) if denom > 0 else 30.0)
            if tgt is not None and "aim" not in kv:
                dest = None
        # like a player, turn the vehicle toward what it shoots at first. Vehicle spells with a cone
        # (e.g. the Gilneas Rebel Cannon 67279/67278, SpellTargetRestrictions ConeAngle 60) never hit anything
        # outside the vehicle's facing. Only when this client drives the vehicle (w.move_guid), so a passenger seat
        # or a plain character cast is untouched; face=0 turns it off.
        if kv.get("face", "1") not in ("0", "") and getattr(w, "move_guid", None):
            aim_p = None
            if "at" in kv:
                aim_p = _xyz(kv["at"])
            elif tgt and ps.objs.get(tgt) is not None:
                aim_p = ps.objs[tgt].position()
            if aim_p is not None:
                ps.face(aim_p)
                w.pump(0.3)
        t0 = time.time()
        w.cast(spell, tgt, dest=dest, elevation=elev, speed=spd)
        res = ps._cast_result(spell, t0, float(kv.get("wait", 0.8)))
        if res is not None and kv.get("strict", "0") == "1":
            ps.stuck(verb, args, "cast-failed result=%d(%s)" % (res, SPELL_FAILED_NAMES.get(res, "?")), quest)
        ps.ok(verb, args, "spell=%d result=%s quest=%s" % (spell, "ok" if res is None else SPELL_FAILED_NAMES.get(res, res),
                                                          ps.quest_state_str(quest)))
    elif verb == "choose-reward":
        # choose-reward [i] - answer the pending SMSG_QUESTGIVER_OFFER_REWARD (e.g. after an auto-complete quest)
        off = ps.offer
        if not off:
            ps.stuck(verb, args, "no-offer-pending")
        if not ps.choose_reward(verb, args, int(kv.get("guid", 0)) or (w.gossip or {}).get("guid", 0) or w.char_guid,
                                off["quest"], int(pos[0]) if pos else 0):
            ps.stuck(verb, args, "reward-not-confirmed", off["quest"])
    elif verb == "talk":
        # talk ENTRY [option=text-substring] [range=R] [go=1]  (go=1: a gameobject with a gossip menu, opened by using it)
        is_go = kv.get("go", "0") not in ("0", "")
        npc = ps.goto_object(verb, args, int(pos[0]), ("go",) if is_go else ("unit",), float(kv.get("range", 3.5)),
                             timeout, stuck_s)
        ps.face(npc.position())
        w.gossip = None
        t_hello = time.time()
        if is_go:
            w.gameobject_use(npc.guid)
        else:
            w.gossip_hello(npc.guid)
        if not ps._wait(lambda: w.gossip is not None or getattr(ps, "quest_window_t", 0) >= t_hello, 8.0):
            if kv.get("optional", "0") == "1":      # e.g. an event NPC that is busy (no gossip) on a resumed run
                ps.ok(verb, args, "no-gossip-menu (optional)")
                return True
            ps.stuck(verb, args, "no-gossip-menu")
        if w.gossip is None:
            # the NPC answered with a quest window only (quest list / details / request items / reward), no gossip menu
            if "option" in kv and kv.get("optional", "0") != "1":
                ps.stuck(verb, args, "quest-window-only no-option=%s" % kv["option"])
            ps.ok(verb, args, "quest-window (no gossip menu)")
            return True
        if "option" in kv:
            want = kv["option"].replace("_", " ").lower()
            opt = next((o for o in w.gossip["options"] if want in o["text"].lower()), None)
            if opt is None:
                if kv.get("optional", "0") == "1":
                    ps.ok(verb, args, "no-option-matching (optional)")
                    return True
                ps.stuck(verb, args, "no-option-matching=%s" % kv["option"])
            w.gossip_select(opt["id"], npc.guid, w.gossip["menu"], kv.get("code", ""))
            w.pump(2.0)
        ps.ok(verb, args, "menu=%s" % (w.gossip or {}).get("menu"))
    elif verb == "use-go":
        # use-go ENTRY [range=R] [loot=1] [quest=Q] [wait=S]
        entry = int(pos[0])
        go = ps.goto_object(verb, args, entry, ("go",), float(kv.get("range", 3.0)), timeout, stuck_s, quest=quest,
                            pred=lambda o: o.guid not in ps.used_go or time.time() - ps.used_go[o.guid] > 60)
        before = (ps.quest_state_str(quest), len(ps.inventory()))
        ps.use_gameobject(go, float(kv.get("wait", 2.0)))
        n = w.loot_all(1.0) if w.loot_windows and kv.get("loot", "1") not in ("0", "") else 0
        ps.ok(verb, args, "guid=%d looted=%d quest=%s changed=%d" % (
            go.guid, n, ps.quest_state_str(quest), int(before != (ps.quest_state_str(quest), len(ps.inventory())))))
    elif verb == "use-item":
        # use-item ITEM_ENTRY [on=ENTRY|at=x,y,z|self] [range=R] [quest=Q]
        item = int(pos[0])
        found = ps.find_item(item)
        if not found:
            ps.stuck(verb, args, "item-not-in-bags item=%d" % item, quest)
        bag, slot, iguid = found
        target, dest = None, None
        if "on" in kv:
            on = kv["on"]
            kinds = ("go",) if on.startswith("go:") else ("unit",) if on.startswith("npc:") else ("unit", "go")
            on_entry = int(on.split(":", 1)[1]) if ":" in on else int(on)
            bad_key = ("use-item-failed", on_entry)
            bad = ps.used_targets.setdefault(bad_key, set())
            ps.gone_ok = kv.get("fail", "stuck") == "ok"
            try:
                o = ps.goto_object(verb, args, on_entry, kinds, float(kv.get("range", 5.0)), timeout,
                                   stuck_s, quest=quest,
                                   fresh_key=None if kv.get("fresh", "1") == "0" else (verb, on_entry),
                                   # dead=1: the item is used on a corpse (25123 Blastshadow's Soulstone 73702 on his body)
                                   pred=lambda x: (x.kind == "go" or (x.alive() != (kv.get("dead", "0") == "1")))
                                   and x.guid not in bad
                                   and ("maxdz" not in kv or (x.position() and w.pos
                                                             and abs(x.position()[2] - w.pos[2]) <= float(kv["maxdz"]))))
            finally:
                ps.gone_ok = False
            if o is None:
                ps.ok(verb, args, "target-gone")
                return True
            target = o.guid
            ps.face(o.position())
        elif "at" in kv and kv["at"].split(":", 1)[0] in ("go", "npc"):
            # at=go:ENTRY / at=npc:ENTRY: a ground-targeted item thrown at the nearest such object (fresh by default)
            kind_, ent_ = kv["at"].split(":", 1)
            o = ps.goto_object(verb, args, int(ent_), ("go",) if kind_ == "go" else ("unit",), float(kv.get("range", 5.0)),
                               timeout, stuck_s, quest=quest,
                               fresh_key=None if kv.get("fresh", "1") == "0" else (verb, "at", int(ent_)))
            p_ = o.position()
            # aim a little above the object's origin: a rock/deposit model blocks the line of sight to its own base
            dest = (p_[0], p_[1], p_[2] + float(kv.get("lift", 2.0)))
            ps.face(dest)
        elif "at" in kv:
            dest = _xyz(kv["at"])
            ps.walk_to(lambda: dest, float(kv.get("range", 5.0)), verb, args, timeout=timeout, stuck_s=stuck_s, quest=quest)
        elif "self" in pos[1:] or "self" in kv:
            target = w.char_guid
        t0 = time.time()
        ps.use_item_target(bag, slot, iguid, target, dest)
        w.pump(1.5)
        fails = [(s, r) for t, s, r in ps.cast_fail if t >= t0]
        if fails and "on" in kv and target:
            ps.used_targets.setdefault(("use-item-failed", on_entry), set()).add(target)   # never pick it again
        if fails and kv.get("fail", "stuck") != "ok":
            ps.stuck(verb, args, "cast-failed result=%d(%s)" % (fails[-1][1], SPELL_FAILED_NAMES.get(fails[-1][1], "?")), quest)
        ps.ok(verb, args, "bag=%d slot=%d quest=%s%s" % (bag, slot, ps.quest_state_str(quest),
              (" cast-failed=%d" % fails[-1][1]) if fails else ""))
    elif verb == "spellclick":
        # spellclick ENTRY [range=R] [vehicle=1] [optional=1]  (optional=1: nothing to click in view is OK, e.g. fires that
        # only exist in one phase of a fight)
        if kv.get("optional", "0") == "1":
            used_sc = ps.used_targets.get((verb, int(pos[0])), set()) if kv.get("fresh", "1") != "0" else set()
            if not [o for o in ps.objects_of(int(pos[0]), ("unit",)) if o.alive() and o.guid not in used_sc]:
                ps.ok(verb, args, "none-in-view")
                return True
        npc = ps.goto_object(verb, args, int(pos[0]), ("unit",), float(kv.get("range", 3.5)), timeout, stuck_s,
                             fresh_key=None if kv.get("fresh", "1") == "0" else (verb, int(pos[0])),
                             pred=lambda o: o.alive())
        ps.face(npc.position())
        w.spellclick(npc.guid)
        if kv.get("vehicle", "0") not in ("0", ""):
            if not ps._wait(lambda: ps.vehicle, float(kv.get("wait", 10))):
                ps.stuck(verb, args, "no-vehicle-after-click")
        else:
            w.pump(1.5)
        ps.ok(verb, args, "guid=%d vehicle=%d" % (npc.guid, int(ps.vehicle)))
    elif verb == "collect":
        # collect ITEM_ENTRY count=N [from=ENTRY|go=ENTRY] [quest=Q] [timeout=S] [stuck=S]
        item = int(pos[0])
        need = int(kv.get("count", 1))
        tmo = float(kv.get("timeout", 1200))
        t0 = time.time()
        last_cnt, last_prog = ps.item_count(item), time.time()
        if "from" in kv:
            ps.kill_loop(verb, args, int(kv["from"]), lambda k: ps.item_count(item) >= need, True, tmo, stuck_s, quest,
                         progress_fn=lambda: ps.item_count(item))
        elif "go" in kv:
            ge = int(kv["go"])
            while ps.item_count(item) < need:
                if time.time() - t0 > tmo:
                    ps.stuck(verb, args, "timeout have=%d" % ps.item_count(item), quest)
                if ps.is_dead():
                    ps.corpse_run(verb, args, quest)
                c = ps.item_count(item)
                if c != last_cnt:
                    last_cnt, last_prog = c, time.time()
                elif time.time() - last_prog > stuck_s * 6:
                    ps.stuck(verb, args, "no-item-progress have=%d" % c, quest)
                # fight off what attacks first (a chest guarded by mobs: opening it under attack got the character killed,
                # and a dead character cannot see the chest, which then looked like a missing gameobject)
                for _n in range(10):
                    att = ps.attackers()
                    if not att or ps.is_dead():
                        break
                    a0 = ps.pick_attacker(att)
                    log("%s: fighting attacker %s before using the object" % (verb, a0.short()))
                    ps.fight(verb, args, a0.guid, stuck_s, quest)
                if ps.is_dead():
                    continue                      # the loop top does the corpse run
                go = ps.goto_object(verb, args, ge, ("go",), float(kv.get("range", 3.0)), tmo, stuck_s, quest=quest,
                                    wait_s=float(kv.get("wait", 15)),
                                    pred=lambda o: o.guid not in ps.used_go or time.time() - ps.used_go[o.guid] > 90)
                ps.use_gameobject(go, float(kv.get("usewait", 2.5)))
                if w.loot_windows:
                    w.loot_all(1.0)
                else:
                    ps.used_go.pop(go.guid, None)  # the use failed (e.g. the Opening cast was interrupted): try it again
                w.pump(0.5)
        else:
            ps.stuck(verb, args, "collect-needs-from-or-go")
        if ps.item_count(item) < need:
            ps.stuck(verb, args, "have=%d need=%d" % (ps.item_count(item), need), quest)
        ps.ok(verb, args, "have=%d quest=%s" % (ps.item_count(item), ps.quest_state_str(quest)))
    elif verb == "wait-quest":
        # wait-quest QUEST complete|rewarded|accepted [timeout=S]
        qid, want = int(pos[0]), (pos[1] if len(pos) > 1 else "complete")
        ok_states = {"complete": ("complete", "rewarded"), "rewarded": ("rewarded",),
                     "accepted": ("incomplete", "complete", "rewarded")}.get(want, (want,))
        if not ps._wait(lambda: ps.quest_status(qid) in ok_states, float(kv.get("timeout", 60)), 0.5):
            ps.stuck(verb, args, "quest-not-%s" % want, qid)
        ps.ok(verb, args, "state=%s" % ps.quest_state_str(qid))
    elif verb == "vehicle-wait":
        # vehicle-wait [on|off] [timeout=S]
        want_on = not (pos and pos[0].lower() == "off")
        if not ps._wait(lambda: ps.vehicle == want_on, float(kv.get("timeout", 60)), 0.25):
            ps.stuck(verb, args, "vehicle-%s-not-seen" % ("enter" if want_on else "exit"))
        ps.ok(verb, args, "vehicle=%d" % int(ps.vehicle))
    elif verb == "select-faction":
        # select-faction alliance|horde - the Pandaren neutral faction pick (quest 31450 "A New Fate"). Gossip "I'm ready to choose
        # my destiny" on Spirit of Master Shang Xi 56013 (talk 56013 option=destiny) makes the server send
        # SMSG_SHOW_NEURTRAL_PLAYER_FACTION_SELECT_UI; the client answers with CMSG_SELECT_FACTION (u32: 1 Alliance, 0 Horde).
        choice = 0 if (pos and pos[0].lower() in ("horde", "0")) else 1
        w.send_packet(CMSG_SELECT_FACTION, struct.pack("<I", choice))
        log("sent CMSG_SELECT_FACTION choice=%d (%s)" % (choice, "Alliance" if choice else "Horde"))
        w.pump(3.0)
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb == "wait-map":
        # wait-map MAP [timeout=S]
        want = int(pos[0])
        if not ps._wait(lambda: w.map_id == want, float(kv.get("timeout", 120)), 0.5):
            ps.stuck(verb, args, "map-not-reached have=%s" % w.map_id)
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb == "equip":
        # equip ITEM_ENTRY - CMSG_AUTOEQUIP_ITEM (ItemHandler.cpp: u8 slot, u8 bag) from the bag slot holding it
        item = int(pos[0])
        found = ps.find_item(item)
        if not found:
            # already worn (a resumed run): equipment slots 0-18 of PLAYER_FIELD_INV_SLOTS
            me_ = ps.me()
            worn = [es for es in range(19) if me_ is not None and me_.u64(UF_PLAYER_INV_SLOTS + 2 * es)
                    and getattr(ps.objs.get(me_.u64(UF_PLAYER_INV_SLOTS + 2 * es)), "entry", 0) == item]
            if worn:
                ps.ok(verb, args, "already-equipped slot=%d" % worn[0])
                return True
            ps.stuck(verb, args, "item-not-in-bags item=%d" % item)
        bag, slot, iguid = found
        w.send_packet(CMSG_AUTOEQUIP_ITEM, struct.pack("<BB", slot, bag))
        log("sent CMSG_AUTOEQUIP_ITEM bag=%d slot=%d item=%d" % (bag, slot, item))
        if not ps._wait(lambda: all(g != iguid for _b, _s, g, _e, _c in ps.inventory()), float(kv.get("wait", 5))):
            ps.stuck(verb, args, "equip-failed item=%d" % item)
        ps.ok(verb, args)
    elif verb in ("pet-attack", "pet-follow", "pet-stay"):
        # pet-attack [target=ENTRY] / pet-follow / pet-stay - CMSG_PET_ACTION with ACT_COMMAND (7) << 24 | command
        pet = ps.controlled_guid()
        if not pet:
            ps.stuck(verb, args, "no-pet-or-charm")
        tgt = 0
        if verb == "pet-attack":
            if "target" in kv:
                o = ps.nearest(int(kv["target"]), ("unit",), lambda x: x.alive())
                if o is None:
                    ps.stuck(verb, args, "target-not-visible=%s" % kv["target"])
                tgt = o.guid
            else:
                att = ps.attackers()
                tgt = att[0].guid if att else 0
            if not tgt:
                ps.stuck(verb, args, "no-target")
        ps.pet_action(pet, 0x07000000 | {"pet-attack": 2, "pet-follow": 1, "pet-stay": 0}[verb], tgt)
        w.pump(1.0)
        ps.ok(verb, args, "pet=%d target=%d" % (pet, tgt))
    elif verb in ("pet-passive", "pet-defensive", "pet-assist"):
        # the pet bar's reaction buttons: CMSG_PET_ACTION with ACT_REACTION (6) << 24 | REACT_PASSIVE 0 / DEFENSIVE 1 / ASSIST 3
        pet = ps.controlled_guid()
        if not pet:
            ps.stuck(verb, args, "no-pet-or-charm")
        ps.pet_action(pet, 0x06000000 | {"pet-passive": 0, "pet-defensive": 1, "pet-assist": 3}[verb], 0)
        w.pump(1.0)
        ps.ok(verb, args, "pet=%d" % pet)
    elif verb == "vehicle-exit":
        # vehicle-exit [timeout=S] - CMSG_REQUEST_VEHICLE_EXIT (VehicleHandler.cpp), then wait to be off it
        w.send_packet(CMSG_REQUEST_VEHICLE_EXIT, b"")
        log("sent CMSG_REQUEST_VEHICLE_EXIT")
        if not ps._wait(lambda: not ps.vehicle, float(kv.get("timeout", 10))):
            ps.stuck(verb, args, "still-on-vehicle")
        w.move_guid = None
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb == "cast-on":
        # cast-on SPELL ENTRY [range=R] [go=1] [wait=S] - walk into range of the nearest ENTRY, face it and cast SPELL
        spell, entry = int(pos[0]), int(pos[1])
        kinds = ("go",) if kv.get("go", "0") not in ("0", "") else ("unit",)
        rng = float(kv.get("range", 20.0))
        min_rng = float(kv.get("min", 0.0))            # e.g. Charge: min=8
        # fresh=1: never the same target twice (one cast per NPC, e.g. entertaining partygoers)
        used_co = ps.used_targets.setdefault((verb, spell, entry), set()) if kv.get("fresh", "0") == "1" else set()
        res = None
        for attempt in range(5):
            # a target already between min= and range=: cast from here (a charge needs the gap, a mob closes it)
            inwin = [x for x in ps.objects_of(entry, kinds) if (x.kind == "go" or x.alive()) and x.guid not in used_co
                     and min_rng + 1.0 <= _d3(x.position(), w.pos) <= rng]
            if inwin:
                o = min(inwin, key=lambda x: _d3(x.position(), w.pos))
            else:
                o = ps.goto_object(verb, args, entry, kinds, rng, timeout, stuck_s, quest=quest,
                                   pred=lambda x: (x.kind == "go" or x.alive()) and x.guid not in used_co)
            ps.face(o.position())
            w.set_selection(o.guid)
            log("%s: try %d target=%d hp=%d dist=%.1f" % (verb, attempt + 1, o.guid, o.health, _d3(o.position(), w.pos)))
            t0 = time.time()
            w.cast(spell, o.guid)
            res = ps._cast_result(spell, t0, float(kv.get("wait", 1.5)))
            if res is None:
                break
            if res in (SPELL_FAILED_LINE_OF_SIGHT, SPELL_FAILED_OUT_OF_RANGE):
                rng = max(min_rng + 1.0, rng / 2.0)       # closer (round the obstacle) and try again
                log("%s: %s - moving to %.0f yd and casting again" % (verb, SPELL_FAILED_NAMES.get(res), rng))
                continue
            if res == SPELL_FAILED_UNIT_NOT_INFRONT:
                w.orientation = (w.orientation + 1.0) % (2 * math.pi)
                continue
            if res == 132 and o.position():                     # TOO_CLOSE: step back out past min=
                tp = o.position()
                dx, dy = w.pos[0] - tp[0], w.pos[1] - tp[1]
                dl = math.hypot(dx, dy) or 1.0
                back = max(min_rng, 5.0) + 3.0
                spot = (tp[0] + dx / dl * back, tp[1] + dy / dl * back, w.pos[2])
                log("%s: TOO_CLOSE - stepping back to %.0f yd" % (verb, back))
                ps.walk_to(lambda: spot, 1.0, verb, args, timeout=60, stuck_s=stuck_s, fail="return")
                rng = back + 1.0
                continue
            break
        if res is not None:
            ps.stuck(verb, args, "cast-failed result=%d(%s)" % (res, SPELL_FAILED_NAMES.get(res, "?")), quest)
        if kv.get("fresh", "0") == "1":
            used_co.add(o.guid)
        ps.ok(verb, args, "target=%d quest=%s" % (o.guid, ps.quest_state_str(quest)))
    elif verb == "sell":
        # sell VENDOR_ENTRY items=a,b,... | sell VENDOR_ENTRY all [except=a,b,...]   (CMSG_SELL_ITEM, ItemHandler.cpp)
        vendor = ps.goto_object(verb, args, int(pos[0]), ("unit",), float(kv.get("range", 3.5)), timeout, stuck_s)
        ps.face(vendor.position())
        w.gossip_hello(vendor.guid)
        w.pump(1.0)
        want = set(int(x) for x in kv["items"].split(",")) if "items" in kv else None
        keep = set(int(x) for x in kv.get("except", "").split(",") if x)
        # `sell VENDOR junk` sells grey (Quality 0) items that have a sell price, from the read-only db_items facts
        junk = set()
        if "junk" in pos[1:]:
            junk = {e for e, f in db_items([e for _b, _s, _g, e, _c in ps.inventory()]).items() if f[6] == 0 and f[7] > 0}
        sold = 0
        for b, sl, g, e, c in ps.inventory():
            if (want is not None and e in want) or (want is None and "all" in pos[1:] and e not in keep) or e in junk:
                ps._sell_item(vendor.guid, g, c)
                sold += 1
        w.pump(1.0)
        ps.ok(verb, args, "sold=%d free-slots-used=%d" % (sold, len(ps.inventory())))
    elif verb == "abandon":
        # abandon QUEST: CMSG_QUESTLOG_REMOVE_QUEST with the quest's log slot (right-click > Abandon Quest)
        qid = int(pos[0])
        q = ps.quest_log().get(qid)
        if not q:
            ps.ok(verb, args, "not-in-log")
            return True
        w.send_packet(CMSG_QUESTLOG_REMOVE_QUEST, struct.pack("<B", int(q["slot"])))
        log("sent CMSG_QUESTLOG_REMOVE_QUEST quest=%d slot=%d" % (qid, q["slot"]))
        if not ps._wait(lambda: qid not in ps.quest_log(), float(kv.get("timeout", 8))):
            ps.stuck(verb, args, "still-in-log", qid)
        ps.ok(verb, args, "removed")
    elif verb == "react":
        # react SECONDS map=KEY:SPELL,KEY:SPELL... [until=COND] [once=1]: watch incoming chat (whispers, emotes, raid-boss
        # whispers) and cast the mapped spell (self/vehicle-bar cast) when a new line contains KEY (case-insensitive,
        # `_` = space). Answers each line once. Used for prompts such as "Use your Blastcrackers!" in the Kezan vault.
        secs = float(pos[0]) if pos else 60.0
        pairs = []
        for item_ in kv.get("map", "").split(","):
            if ":" in item_:
                k_, v_ = item_.rsplit(":", 1)
                pairs.append((k_.replace("_", " ").lower(), int(v_)))
        until = kv.get("until")
        t0 = time.time()
        seen_t = t0
        answered = 0
        while time.time() - t0 < secs and not (until and ps.cond(until)):
            new = [(t_, ty_, m_) for t_, ty_, m_ in getattr(w, "chat_lines", []) if t_ > seen_t]
            if new:
                seen_t = new[-1][0]
            for _t, _ty, m_ in new:
                low = m_.lower()
                hit = next((sp for k_, sp in pairs if k_ in low), None)
                if hit:
                    w.pump(float(kv.get("delay", 0.6)))
                    w.cast(hit)
                    answered += 1
                    log("react: '%s' -> cast %d" % (m_[:60], hit))
            w.pump(0.25)
        if until and not ps.cond(until):
            ps.stuck(verb, args, "until-not-met answered=%d" % answered, quest)
        ps.ok(verb, args, "answered=%d quest=%s" % (answered, ps.quest_state_str(quest)))
    elif verb == "probe-attack":
        # probe-attack ENTRY at=x,y,z [seconds=S]: walk next to the unit of ENTRY nearest to the point and try to melee it like a
        # player (CMSG_ATTACKSWING); report its flags and whether any swing landed. For "should not be attackable" checks.
        entry = int(pos[0])
        pt = _xyz(kv["at"])
        cand = [o for o in ps.objects_of(entry, ("unit",)) if o.position()]
        if not cand:
            ps.stuck(verb, args, "not-visible entry=%d" % entry)
        o = min(cand, key=lambda x: _d3(x.position(), pt))
        ps.walk_to(lambda: o.position(), 2.0, verb, args, timeout=60, stuck_s=stuck_s, fail="return")
        ps.face(o.position())
        log("[PROBE-ATTACK] target %s dist-to-point=%.1f" % (o.short(), _d3(o.position(), pt)))
        t0 = time.time()
        w.set_selection(o.guid)
        w.attack(o.guid)
        w.pump(float(kv.get("seconds", 5)))
        hits = [(t, a, v, d) for t, a, v, d in getattr(w, "melee_log", []) if t >= t0 and v == o.guid and a == w.char_guid]
        log("[PROBE-ATTACK] guid=%d unit_flags=0x%X hits=%d dmg=%d hp=%d/%d" % (o.guid, o.unit_flags, len(hits),
            sum(d for _t, _a, _v, d in hits), o.health, o.max_health))
        try:
            w.attack_stop()
        except OSError:
            pass
        ps.ok(verb, args, "guid=%d hits=%d" % (o.guid, len(hits)))
    elif verb == "leave-camp":
        # leave-camp [entries=a,b] [radius=R] [dist=D] [fights=N]: like a player who logged in (or came back to life)
        # inside a camp - when live hostile units are within R yd (default 30), walk D yd (default 35) to the side farthest
        # from all of them, even when one is already hitting, then fight what followed out there (no adds join) before the
        # next step. A level 10 warrior that logged in between two Slavedrivers 37701 and fought on the spot died in 45 s.
        entries = [int(e) for e in kv["entries"].split(",")] if "entries" in kv else []
        radius = float(kv.get("radius", 30))
        dist = float(kv.get("dist", 35))
        here = tuple(w.pos)
        foes = ps.camp_foes(here, radius, entries)
        moved = 0
        if foes:
            pt, dmin = ps.quiet_point(here, foes, rings=(dist, dist * 0.7))
            log("%s: %d hostile within %.0f yd - walking out to (%.1f, %.1f), nearest hostile there %.0f yd"
                % (verb, len(foes), radius, pt[0], pt[1], dmin))
            try:
                w.attack_stop()
            except OSError:
                pass
            ps.walk_to(lambda: pt, 3.0, verb, args, timeout=40.0, stuck_s=10.0, quest=quest,
                       abort_fn=lambda: ps.is_dead(), fail="return")
            moved = 1
        fights = 0
        for _n in range(int(kv.get("fights", 6))):
            if ps.is_dead() or not ps.attackers():
                break
            if ps.fight_nearby(verb, args, stuck_s, quest):
                fights += 1
        ps.ok(verb, args, "foes=%d moved=%d fights=%d" % (len(foes), moved, fights))
    elif verb == "rest":
        # rest [full=F] [max=S]: out of combat, sit until F (default 0.95) of max health, like a player before the next leg
        ps._careful_rest(verb, full=float(kv.get("full", 0.95)), max_s=float(kv["max"]) if "max" in kv else None)
        me_ = ps.me()
        ps.ok(verb, args, "hp=%s" % ("%d/%d" % (me_.health, me_.max_health) if me_ is not None else "?"))
    elif verb == "repair":
        # repair VENDOR_ENTRY: walk to a repair NPC, open it and repair everything (CMSG_REPAIR_ITEM with item guid 0).
        # Gear breaks after about ten deaths (durability 0): a broken weapon makes every weapon ability fail EQUIPPED_ITEM_CLASS.
        vendor = ps.goto_object(verb, args, int(pos[0]), ("unit",), float(kv.get("range", 3.5)), timeout, stuck_s)
        ps.face(vendor.position())
        w.gossip_hello(vendor.guid)
        w.pump(1.0)
        nb = int(vendor.guid).to_bytes(8, "little")
        ib = bytes(8)
        bw = BitWriter()
        for which, idx in (("i", 2), ("i", 5), ("n", 3), ("g", 0), ("n", 7), ("i", 4), ("n", 2), ("i", 0), ("i", 3),
                           ("n", 6), ("n", 1), ("n", 4), ("i", 6), ("n", 5), ("n", 0), ("i", 7), ("i", 1)):
            bw.bit(False if which == "g" else ((nb if which == "n" else ib)[idx] != 0))
        body = bytearray(bw.flush())
        for which, idx in (("i", 2), ("n", 1), ("i", 1), ("n", 4), ("n", 7), ("n", 3), ("n", 2), ("i", 7), ("n", 5),
                           ("n", 0), ("i", 5), ("i", 3), ("i", 4), ("i", 6), ("n", 6), ("i", 0)):
            v = (nb if which == "n" else ib)[idx]
            if v:
                body.append(v ^ 1)
        w.send_packet(CMSG_REPAIR_ITEM, bytes(body))
        log("sent CMSG_REPAIR_ITEM npc=%d (all items)" % vendor.guid)
        w.pump(1.5)
        ps.ok(verb, args, "npc=%d" % vendor.guid)
    elif verb == "buy":
        # buy VENDOR_ENTRY ITEM [slot=N] [count=N]: CMSG_LIST_INVENTORY (the vendor window), then CMSG_BUY_ITEM into the
        # backpack (bag guid = the player). slot = the 1-based vendor list position (npc_vendor.slot + 1 for the first items).
        item = int(pos[1])
        vendor = ps.goto_object(verb, args, int(pos[0]), ("unit",), float(kv.get("range", 3.5)), timeout, stuck_s)
        ps.face(vendor.position())
        vg = int(vendor.guid)
        w.send_packet(CMSG_LIST_INVENTORY, guid_mask_bits(vg, (6, 7, 3, 1, 2, 0, 4, 5)) + guid_byte_seq(vg, (0, 7, 1, 6, 4, 3, 5, 2)))
        w.pump(1.0)
        before = ps.item_count(item)
        vb, bb = vg.to_bytes(8, "little"), int(w.char_guid).to_bytes(8, "little")
        bw = BitWriter()
        for which, idx in (("v", 6), ("b", 6), ("b", 4), ("v", 4)):
            bw.bit((vb if which == "v" else bb)[idx] != 0)
        bw.bits(1, 2)                                     # itemType 1 = item (2 = currency)
        for which, idx in (("v", 0), ("v", 3), ("b", 3), ("v", 7), ("v", 5), ("b", 2), ("v", 1), ("b", 7), ("v", 2),
                           ("b", 1), ("b", 0), ("b", 5)):
            bw.bit((vb if which == "v" else bb)[idx] != 0)
        body = bytearray(struct.pack("<IIII", 255, int(kv.get("count", 1)), item, int(kv.get("slot", 1))) + bw.flush())
        for which, idx in (("v", 5), ("v", 0), ("b", 3), ("b", 1), ("b", 6), ("v", 2), ("v", 7), ("v", 6), ("b", 0),
                           ("b", 5), ("v", 4), ("b", 2), ("v", 3), ("b", 7), ("v", 1), ("b", 4)):
            v = (vb if which == "v" else bb)[idx]
            if v:
                body.append(v ^ 1)
        w.send_packet(CMSG_BUY_ITEM, bytes(body))
        log("sent CMSG_BUY_ITEM vendor=%d item=%d slot=%s" % (vg, item, kv.get("slot", 1)))
        if not ps._wait(lambda: ps.item_count(item) > before, float(kv.get("wait", 5))):
            ps.stuck(verb, args, "not-bought item=%d have=%d" % (item, ps.item_count(item)), quest)
        ps.ok(verb, args, "have=%d quest=%s" % (ps.item_count(item), ps.quest_state_str(quest)))
    elif verb == "gear-up":
        # gear-up - , what a player does after a turn-in: put empty bag slots to use (equip carried bags) and wear
        # bag items that are upgrades (higher item level for the slot, usable at this level), each by CMSG_AUTOEQUIP_ITEM
        # like a right-click. Item facts come from the optional read-only db_items lookup; without it this is a no-op.
        me_ = ps.me()
        level = me_.u32(UF_UNIT_LEVEL) if me_ is not None else 0
        worn = {}
        if me_ is not None:
            for es in range(23):
                g = me_.u64(UF_PLAYER_INV_SLOTS + 2 * es)
                if g:
                    worn[es] = getattr(ps.objs.get(g), "entry", 0)
        bag_items = ps.inventory()
        facts = db_items([e for e in worn.values()] + [e for _b, _s, _g, e, _c in bag_items])
        if not facts:
            ps.ok(verb, args, "no-item-facts (set MOP_MYSQL_BIN / MOP_DB_*)")
            return True
        done = []
        for b, s_, g, e, _c in bag_items:
            f = facts.get(e)
            if not f:
                continue
            cls, sub, inv, ilvl, req, cslots = f[:6]
            if req > level:
                continue
            if cls == 1 and inv == 18 and cslots > 0:            # a bag: only while a bag slot is empty
                if len([es for es in range(19, 23) if es in worn]) >= 4:
                    continue
            elif cls in (2, 4) and inv in GEAR_SLOTS:
                if cls == 2 and inv in (15, 25, 26) and sub in (2, 3, 16, 18) and (ps.char_class or 0) != 3:
                    # a bow, gun, crossbow or thrown weapon goes in the main hand in 5.4.8; only a
                    # hunter fights with one - a warrior that wore a higher-level Compact Shotgun had Mortal Strike and
                    # Heroic Strike fail EQUIPPED_ITEM_CLASS (31) and died swinging bare-handed
                    continue
                cur = [facts.get(worn.get(es, 0), (0, 0, 0, 0, 0, 0))[3] if worn.get(es) else -1 for es in GEAR_SLOTS[inv]]
                if min(cur) >= ilvl:
                    continue
            else:
                continue
            w.send_packet(CMSG_AUTOEQUIP_ITEM, struct.pack("<BB", s_, b))
            log("gear-up: CMSG_AUTOEQUIP_ITEM bag=%d slot=%d item=%d (ilvl %d)" % (b, s_, e, ilvl))
            ps._wait(lambda: all(gg != g for _b2, _s2, gg, _e2, _c2 in ps.inventory()), 3.0)
            if all(gg != g for _b2, _s2, gg, _e2, _c2 in ps.inventory()):
                done.append(e)
                me_ = ps.me()                                      # refresh what is worn for the next comparisons
                worn = {}
                for es in range(23):
                    gw = me_.u64(UF_PLAYER_INV_SLOTS + 2 * es) if me_ is not None else 0
                    if gw:
                        worn[es] = getattr(ps.objs.get(gw), "entry", 0)
                        facts.setdefault(worn[es], facts.get(worn[es], (0, 0, 0, 0, 0, 0)))
        # bags nearly full (fewer than junk-below=4 free slots) -> sell grey items at a vendor within 60 yd, if one is
        # in sight (the quest hubs have one); junk-below=0 turns it off
        sold = 0
        if ps.free_bag_slots() < int(kv.get("junk-below", 4)):
            facts = db_items([e for _b, _s, _g, e, _c in ps.inventory()])
            junk = [(g, c) for _b, _s, g, e, c in ps.inventory() if e in facts and facts[e][6] == 0 and facts[e][7] > 0]
            vendors = [o for o in ps.objs.values() if o.typeid == 3 and o.alive() and o.u32(UF_UNIT_NPC_FLAGS) & 0x80
                       and o.position() and _d3(o.position(), w.pos) <= 60.0]
            if junk and vendors:
                vend = min(vendors, key=lambda o: _d3(o.position(), w.pos))
                back = tuple(w.pos)
                ps.walk_to(lambda: vend.position(), 3.0, verb, args, timeout=60, stuck_s=15)
                ps.face(vend.position())
                w.gossip_hello(vend.guid)
                w.pump(1.0)
                for g, c in junk:
                    ps._sell_item(vend.guid, g, c)
                    sold += 1
                w.pump(1.0)
                log("gear-up: sold %d junk item(s) to %s, %d bag slots free" % (sold, vend.short(), ps.free_bag_slots()))
                ps.walk_to(lambda: back, 2.0, verb, args, timeout=60, stuck_s=15)
            elif junk:
                log("gear-up: bags nearly full (%d free) and no vendor in sight" % ps.free_bag_slots())
        ps.ok(verb, args, "equipped=%s sold=%d" % (",".join(str(x) for x in done) or "-", sold))
    elif verb == "spec":
        # spec [TAB] - choose the specialization at level 10 like a player does in the talent pane
        # (CMSG_SET_PRIMARY_TALENT_TREE); without TAB the class's damage spec. The server ignores it once one is set.
        me_ = ps.me()
        level = me_.u32(UF_UNIT_LEVEL) if me_ is not None else 0
        if level < 10:
            ps.ok(verb, args, "skipped level=%d" % level)
            return True
        tab = int(pos[0]) if pos else DEFAULT_DPS_SPEC_TAB.get(ps.char_class or 0, 0)
        before = len(ps.known_spells)
        w.send_packet(CMSG_SET_PRIMARY_TALENT_TREE, struct.pack("<I", tab))
        ps._wait(lambda: len(ps.known_spells) > before, 5.0)
        ps.ok(verb, args, "tab=%d learned=%d" % (tab, len(ps.known_spells) - before))
    elif verb == "destroy":
        # destroy ITEM_ENTRY [count=N]   (CMSG_DESTROY_ITEM: u32 count, u8 slot, u8 bag) - every stack of that entry
        item = int(pos[0])
        n = 0
        for b, sl, g, e, c in ps.inventory():
            if e == item:
                w.send_packet(CMSG_DESTROY_ITEM, struct.pack("<iBB", int(kv.get("count", c)), sl, b))
                n += 1
        w.pump(1.0)
        ps.ok(verb, args, "stacks=%d left=%d" % (n, ps.item_count(item)))
    elif verb == "spirit-healer":
        # spirit-healer [ENTRY=6491] - as a ghost, walk to the spirit healer, talk to it and accept its resurrection
        # (CMSG_GOSSIP_HELLO -> SMSG_SPIRIT_HEALER_CONFIRM -> CMSG_SPIRIT_HEALER_ACTIVATE, NPCHandler.cpp:556)
        entry = int(pos[0]) if pos else 6491
        if not ps.is_dead():
            ps.ok(verb, args, "alive")
            return True
        if not ps.is_ghost():
            w.release_spirit()
            if not ps._wait(ps.is_ghost, 20):
                ps.stuck(verb, args, "release-failed")
        shp = ps.spirit_healer_pos
        near = ps.cur_near or (tuple(shp[1:]) if shp and shp[0] == w.map_id else None)
        # near=x,y,z picks the healer standing there (within=R yd, default 60), not the one at the graveyard the ghost
        # was sent to: a player may walk to any spirit healer
        sh_pred = None
        if ps.cur_near:
            nx_, wr_ = tuple(ps.cur_near), float(kv.get("within", 60))
            sh_pred = lambda o, nx_=nx_, wr_=wr_: o.position() is not None and _d3(o.position(), nx_) < wr_
        npc = ps.goto_object(verb, args, entry, ("unit",), float(kv.get("range", 4.0)), timeout, stuck_s,
                             wait_s=float(kv.get("wait", 10)), near=near, pred=sh_pred)
        ps.face(npc.position())
        w.gossip_hello(npc.guid)
        ps._wait(lambda: ps.spirit_confirm is not None, 3.0)
        ps._guid_packet(CMSG_SPIRIT_HEALER_ACTIVATE, npc.guid, (2, 7, 6, 0, 5, 4, 1, 3), (1, 5, 6, 3, 2, 0, 7, 4))
        log("sent CMSG_SPIRIT_HEALER_ACTIVATE guid=%d" % npc.guid)
        if not ps._wait(lambda: not ps.is_dead(), float(kv.get("reswait", 10))):
            ps.stuck(verb, args, "not-resurrected")
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb == "relog":
        # relog [wait=S] - log out like a client and log the same character back in, in the same run
        relog_client(w, args_cli, float(kv.get("wait", 2)))
        ps = w.play
        ps.ok(verb, args, "pos=%s" % ps.pos_str())
    elif verb == "quest-log":
        for q, v in sorted(ps.quest_log().items()):
            log("[QUEST-LOG] id=%d slot=%d status=%s counts=%s" % (q, v["slot"], ps.quest_status(q),
                                                                   [c for c in v["counts"] if c] or 0))
        log("[QUEST-LOG] rewarded this session: %s" % (sorted(ps.quest_rewarded) or "-"))
    elif verb == "inventory":
        for b, s, g, e, c in ps.inventory():
            log("[INV] bag=%d slot=%d entry=%d count=%d guid=%d" % (b, s, e, c, g))
        me_ = ps.me()
        for es in range(19):   # worn items: equipment slots 0-18 (15 main hand, 16 off hand)
            g = me_.u64(UF_PLAYER_INV_SLOTS + 2 * es) if me_ is not None else 0
            if g:
                log("[EQUIP] slot=%d entry=%d guid=%d" % (es, getattr(ps.objs.get(g), "entry", 0), g))
    elif verb == "select-nearest":
        # select-nearest ENTRY: CMSG_SET_SELECTION on the nearest visible unit/GO of ENTRY (a GM observer's `.npc info`)
        o = ps.nearest(int(pos[0]), ("unit", "go"))
        if o is None:
            ps.ok(verb, args, "none-in-view")
            return True
        w.set_selection(o.guid)
        w.pump(0.5)
        ps.ok(verb, args, o.short())
    elif verb == "objects":
        want = int(pos[0]) if pos else None
        for o in sorted(ps.objs.values(), key=lambda o: _d3(o.position(), w.pos) if o.position() else 1e9):
            if want is None or o.entry == want:
                if o.kind in ("item",):
                    continue
                p = o.position()
                log("[OBJ] %s dist=%s" % (o.short(), "%.1f" % _d3(p, w.pos) if p else "-"))
    else:
        return False
    return True


def relog_client(w, args_cli, wait=2.0):
    """Log the character out (CMSG_LOGOUT_REQUEST), then authenticate again and log the same character in on a
    new world connection; the WorldClient object `w` is kept so the step runner keeps going."""
    guid = int(w.char_guid or 0) & 0xFFFFFFFFFFFF
    keep = {k: getattr(w, k) for k in ("char_name", "char_race", "char_class", "language", "bf_accept")}
    log("relog: logging out %s" % keep["char_name"])
    w.logout()
    try:
        w.s.close()
    except OSError:
        pass
    w.pump_sleep = None
    time.sleep(max(0.0, wait))
    K = auth_login(args_cli.host, args_cli.auth_port, args_cli.account, args_cli.password)
    nw = WorldClient(args_cli.host, args_cli.world_port, K, args_cli.account)
    nw.handshake(K, realm_id=args_cli.realm)
    nw.char_enum()
    if not nw.player_login(guid):
        log("STUCK verb=relog args=- reason=login-failed pos=- quest=-")
        raise SystemExit(3)
    auto = w.__dict__.get("auto")             # a --log-movement wrapper installed on the old object stays
    w.__dict__.clear()
    w.__dict__.update(nw.__dict__)
    if auto is not None:
        w.auto = auto
    for k, v in keep.items():
        setattr(w, k, v)
    w.play.w = w
    w.play.char_class = w.char_class
    log("relog: back in world map=%s pos=(%.1f, %.1f, %.1f)" % (w.map_id, w.pos[0], w.pos[1], w.pos[2]))
    w.pump(3)


def expand_inline_blocks(steps):
    """`repeat ... { a ; b ; c } end` on one line -> repeat / a / b / c / end lines (nesting allowed)."""
    out = []
    for line in steps:
        if "{" in line and "}" in line:
            head, rest = line.split("{", 1)
            body, tail = rest.rsplit("}", 1)
            out.append(head.strip())
            out.extend(expand_inline_blocks([b.strip() for b in _split_top(body) if b.strip()]))
            out.append(tail.strip() or "end")
        else:
            out.append(line)
    return out


def _split_top(body):
    parts, depth, cur = [], 0, ""
    for ch in body:
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
        if ch == ";" and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    parts.append(cur)
    return parts


def _step_verb(line):
    parts = line.split()
    return parts[0].lower() if parts else ""


def _block_end(steps, i):
    """Index of the `end` matching the `repeat` at steps[i]."""
    depth = 0
    for j in range(i + 1, len(steps)):
        v = _step_verb(steps[j])
        if v == "repeat":
            depth += 1
        elif v == "end":
            if depth == 0:
                return j
            depth -= 1
    log("STUCK verb=repeat args=%s reason=repeat-without-end pos=- quest=-" % steps[i].replace(" ", ","))
    raise SystemExit(3)


def exec_steps(w, args_cli, steps, state):
    """Run step lines in order. Control lines:
      repeat N | repeat until=COND [max=N] [timeout=S]  ...  end    (COND: see PlayState.cond)
      retry VERB ... until=COND [every=S] [max=N]   - run VERB, then wait up to S s for COND; again up to N times
    A STUCK inside a retried verb counts as one failed try; anywhere else it ends the run with exit code 3."""
    i = 0
    while i < len(steps):
        line = steps[i]
        verb = _step_verb(line)
        if time.time() - state["t0"] > state["budget"]:
            _session_timeout(w, state["n"] + 1, line, state["budget"])
        if verb == "repeat":
            j = _block_end(steps, i)
            body = steps[i + 1:j]
            pos, kv = _kv_split(line.split()[1:])
            count = int(pos[0]) if pos else None
            until = kv.get("until")
            mx = int(kv.get("max", count if count is not None else 1000))
            tmo = float(kv.get("timeout", 3600))
            ts, k = time.time(), 0
            log("step: %s (%d body lines)" % (line, len(body)))
            while True:
                if until and w.play.cond(until):
                    break
                if count is not None and k >= count:
                    break
                if k >= mx or time.time() - ts > tmo:
                    if until:
                        w.play.stuck("repeat", line.split()[1:], "until-not-met=%s iterations=%d" % (until, k))
                    break
                exec_steps(w, args_cli, body, state)
                k += 1
            log("OK repeat iterations=%d%s" % (k, (" until=%s" % until) if until else ""))
            i = j + 1
            continue
        if verb == "end":
            i += 1
            continue
        state["n"] += 1
        if verb == "retry":
            toks = line.split()[1:]
            own = [t for t in toks if t.split("=", 1)[0] in ("until", "every", "max")]
            inner = " ".join(t for t in toks if t not in own)
            kv = dict(t.split("=", 1) for t in own)
            until = kv.get("until")
            every = float(kv.get("every", 10))
            mx = int(kv.get("max", 10))
            log("step: %s" % line)
            done = False
            for k in range(1, mx + 1):
                try:
                    run_verb(w, args_cli, inner)
                except SystemExit as exc:
                    if exc.code != 3:
                        raise
                    log("RETRY try=%d/%d of %r failed (STUCK above); trying again" % (k, mx, inner))
                if not until:
                    done = True
                    break
                if w.play._wait(lambda: w.play.cond(until), every, 0.5):
                    done = True
                    break
            if not done:
                w.play.stuck("retry", toks, "until-not-met=%s tries=%d" % (until, mx))
            log("OK retry %s tries=%d" % (inner, k))
        else:
            state["line"] = line
            log("step: %s" % line)
            run_verb(w, args_cli, line)
        w.pump(1)
        i += 1


def _load_local_defaults():
    """Optional JSON file of local defaults: $MOP_HEADLESS_DEFAULTS, else headless.local.json next to
    this script. Keys: account, password, host, auth_port, world_port, soap_host, soap_port, soap_user,
    soap_pass, server_log. Missing or unreadable file = no local defaults."""
    path = os.environ.get("MOP_HEADLESS_DEFAULTS") or os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "headless.local.json")
    try:
        with open(path, "r", encoding="utf-8") as fh:
            data = json.load(fh)
        return data if isinstance(data, dict) else {}
    except (OSError, ValueError):
        return {}


def _default(env, key, fallback, local):
    """Command-line default for one option: environment variable, then local defaults file, then fallback."""
    value = os.environ.get(env)
    if value is not None:
        return value
    return local.get(key, fallback)


def main():
    _stall_watchdog_start()
    local = _load_local_defaults()
    ap = argparse.ArgumentParser(description="headless 5.4.8 real-client login")
    ap.add_argument("--cmd", default="test", choices=["test", "login", "play", "botchecks", "create"])
    ap.add_argument("--host", default=_default("MOP_HOST", "host", "127.0.0.1", local))
    ap.add_argument("--auth-port", type=int, default=int(_default("MOP_AUTH_PORT", "auth_port", 3724, local)))
    ap.add_argument("--world-port", type=int, default=int(_default("MOP_WORLD_PORT", "world_port", 8085, local)))
    # No built-in account or password: pass them, set MOP_ACCOUNT / MOP_PASSWORD, or use a local defaults file.
    ap.add_argument("--account", default=_default("MOP_ACCOUNT", "account", None, local))
    ap.add_argument("--password", default=_default("MOP_PASSWORD", "password", None, local))
    ap.add_argument("--char", default=os.environ.get("MOP_CHAR_GUID", "auto"),
                    help="character guid (the DB `characters.guid` counter); 'auto'/'0' picks the first enumerated character")
    ap.add_argument("--realm", type=int, default=1)
    ap.add_argument("--queue", type=int, default=0, help="battleground type id to queue")
    ap.add_argument("--accept", action="store_true", help="accept the popup and enter")
    ap.add_argument("--bf-accept", action="store_true", default=os.environ.get("MOP_BF_ACCEPT", "") == "1",
                    help="#900/#898: accept Wintergrasp/Tol Barad battlefield-manager entry (war) and queue invites")
    ap.add_argument("--hold", type=int, default=30, help="seconds to stay in world")
    ap.add_argument("--max-session", type=int, default=0,
                    help="#875: hard wall-clock limit for --step/--script sessions (0 = auto: hold + step timeouts + overhead + 60 s)")
    ap.add_argument("--trace", action="store_true", help="log every packet")
    ap.add_argument("--invite", default="",
                    help="character name to invite into the group before anything else "
                         "(issue #53: puts a bot in the player's group so the in-battleground "
                         "master/follow behaviour can be observed)")
    ap.add_argument("--duel-guid", type=int, default=0,
                    help="botchecks: send a duel request to this character guid (raw counter)")
    ap.add_argument("--events-out", default="",
                    help="botchecks: write a JSON summary of observed events to this path")
    # S9 (#206) task 2: create a second (Horde) character on the test account from
    # the char-select screen, before any character is selected/logged in - CMSG_CHAR_CREATE is
    # only valid there, so this is a separate --cmd rather than a --step (the "play" path always
    # does char_enum + player_login first).
    ap.add_argument("--create-name", default="", help="--cmd create: new character name")
    ap.add_argument("--create-race", type=int, default=0, help="--cmd create: race id (2=Orc)")
    ap.add_argument("--create-class", type=int, default=0, help="--cmd create: class id (1=Warrior)")
    ap.add_argument("--create-gender", type=int, default=0, help="--cmd create: 0=male, 1=female")
    # S1a: common CLI (issue #165) - identical on headless.py and headless335.py.
    ap.add_argument("--script", default="", help="file of newline-separated verb lines to run after login")
    ap.add_argument("--step", action="append", default=[],
                    help="one verb line to run after login; repeatable, runs in order given. "
                         "#470 start-zone verbs: "
                         "gossip-hello <npc|rawguid> (CMSG_GOSSIP_HELLO, logs options); "
                         "gossip-select <optionId|text=SUBSTR> [code=TXT] [guid=G] [menu=M] "
                         "(CMSG_GOSSIP_SELECT_OPTION, uses last gossip menu); "
                         "faction-select <horde|alliance> (CMSG_SELECT_FACTION, neutral pandaren); "
                         "gameobject-use|gameobject-report-use <rawguid|entry:spawnId> (CMSG_GAMEOBJ_USE/REPORT_USE; "
                         "entry:spawnId builds the static GO guid); "
                         "attack <npc|rawguid> / attack-stop (CMSG_ATTACKSWING / CMSG_ATTACKSTOP); "
                         "spellclick <npc|rawguid> (CMSG_SPELLCLICK). 'npc' = guid cached by the npcguid verb. "
                         "#969: pump <s>; bp-products (shop list + balance line); "
                         "bp-buy <productId> [confirms=N] [gap=S] [decline=1] [timeout=S] [tail=S] (full shop purchase).")
    ap.add_argument("--log-movement", default="",
                    help="#747: log every movement packet received for these movers as MOVEPKT lines: 'all' (every "
                         "other player), 'units' (every unit), or a comma list of names and/or DB guid counters")
    ap.add_argument("--log-movement-out", default="",
                    help="#747: append the MOVEPKT lines to this file instead of stdout (the summary goes to both)")
    # SOAP (worldserver.conf SOAP.Enabled/IP/Port) for the GM verbs; the account needs GM level 3 (administrator).
    ap.add_argument("--soap-host", default=_default("MOP_SOAP_HOST", "soap_host", "127.0.0.1", local))
    ap.add_argument("--soap-port", type=int, default=int(_default("MOP_SOAP_PORT", "soap_port", 7878, local)))
    ap.add_argument("--soap-user", default=_default("MOP_SOAP_USER", "soap_user", None, local))
    ap.add_argument("--soap-pass", default=_default("MOP_SOAP_PASSWORD", "soap_pass", None, local))
    # #209 round-6 rule (review of S9 round 5): a walk-on driver must gate every step on the
    # SERVER's own authoritative position, never on this client's self.pos - see the
    # "wait-for-server-pos" verb below.
    ap.add_argument("--server-log", default=_default("MOP_SERVER_LOG", "server_log", "Server.log", local),
                    help="Server.log path to read [PROBE] lines from for wait-for-server-pos")
    args = ap.parse_args()
    if not args.account or args.password is None:
        ap.error("no account: pass --account and --password (or set MOP_ACCOUNT / MOP_PASSWORD)")
    global TRACE, _PUMP_CLIENT
    TRACE = args.trace
    t0 = time.time()   # issue #61/perf: login-latency measurement point

    K = auth_login(args.host, args.auth_port, args.account, args.password)
    if args.cmd == "test":
        log("auth-only test complete")
        return 0

    w = WorldClient(args.host, args.world_port, K, args.account)
    if args.log_movement:
        # #747: MOVEPKT capture (see MoveLog); the summary is printed however the run ends
        atexit.register(attach_movement_log(w, args.log_movement, args.log_movement_out).summary)
    w.bf_accept = args.bf_accept
    w.handshake(K, realm_id=args.realm)

    if args.cmd == "login":
        w.pump(10)   # auth response burst; time syncs answered
        return 0
    if args.cmd == "create":
        # Char-select screen state (right after world handshake, before char_enum/player_login) -
        # the only place CMSG_CHAR_CREATE is valid.
        if not args.create_name or not args.create_race or not args.create_class:
            log("ERROR: --cmd create needs --create-name/--create-race/--create-class")
            return 1
        # a real client lists the characters first, then creates; the server answers SMSG_CHAR_CREATE after its async
        # name / account-limit / realm-limit queries (CharacterHandler.cpp HandleCharCreateOpcode)
        existing = w.char_enum()
        log("account %s has %d character(s) before create" % (args.account, len(existing)))
        w.char_create(args.create_name, args.create_race, args.create_class, args.create_gender)
        w.pump(30, stop_on=(SMSG_CHAR_CREATE,))
        code = w.char_create_result
        name = CHAR_CREATE_CODES.get(code, "NO_REPLY" if code is None else "unknown")
        print("CREATE-RESULT code=%s name=%s char=%s race=%d class=%d"
              % ("-" if code is None else code, name, args.create_name, args.create_race, args.create_class), flush=True)
        print("CREATE-DONE", flush=True)
        if code == 47:                          # CHAR_CREATE_SUCCESS
            return 0
        return 2 if code is None else 1
    # --- play ---
    chars = w.char_enum()
    for c in chars:
        log("char: guid=%d name=%s level=%d" % (c["guid"], c["name"], c["level"]))

    if args.char == "auto" or args.char == "0":
        if not chars:
            log("ERROR: --char auto but no characters were enumerated")
            return 1
        chosen = chars[0]
        log("auto-selected first character: guid=%d name=%s level=%d" %
            (chosen["guid"], chosen["name"], chosen["level"]))
        char_guid = chosen["guid"]
    else:
        char_guid = int(args.char)

    if not w.player_login(char_guid):
        log("ERROR: player login did not reach SMSG_LOGIN_VERIFY_WORLD")
        return 1
    _PUMP_CLIENT = w   # #8: SOAP calls pump this session; HEADLESS_LOGOUT=1 logs it out at exit
    print("LOGIN-OK map=%d pos=(%.1f, %.1f, %.1f)" %
          (w.map_id, w.pos[0], w.pos[1], w.pos[2]), flush=True)
    # Perf harness: time from process start of main() to a fully-loaded world session.
    print("LOGIN-MS %d" % int((time.time() - t0) * 1000), flush=True)
    w.emit("login", account=args.account, guid=char_guid, map=w.map_id)
    w.emit("pos", x="%.1f" % w.pos[0], y="%.1f" % w.pos[1], z="%.1f" % w.pos[2])
    chosen_meta = next((c for c in chars
                        if (c["guid"] & 0xFFFFFFFFFFFF) == (int(char_guid) & 0xFFFFFFFFFFFF)), None)
    if chosen_meta:
        w.char_name = chosen_meta["name"]
        w.char_race = chosen_meta.get("race")
        w.char_class = chosen_meta.get("class")
        w.play.char_class = w.char_class
        w.language = RACE_LANGUAGE.get(w.char_race, LANG_COMMON)
    w.pump(3)

    if w.map_id in BG_MAPS:
        log("character is inside battleground map %d - leaving first" % w.map_id)
        w.leave_bg()
        w.pump(30, stop_on=(SMSG_NEW_WORLD,))
        w.pump(3)
        log("now on map %s" % w.map_id)

    if args.invite:
        w.invite_to_group(args.invite)
        # The bot answers on its own AI tick via AcceptInvitationAction, which runs on the
        # "often" trigger, so give it a few ticks rather than expecting an immediate reply.
        log("invited %r, waiting 20s for the bot to accept..." % args.invite)
        w.pump(20)

    # S1a: common --script/--step verb execution (issue #165). Runs after the
    # legacy --invite/BG handling above so old runners keep working unchanged.
    steps = list(args.step)
    if args.script:
        with open(args.script, "r", encoding="utf-8") as fh:
            steps = [ln for ln in (l.split("#", 1)[0].strip() for l in fh) if ln] + steps
    if steps:
        run_steps(w, args, steps)

    if args.cmd == "botchecks":
        # Bot-checks mode: run whatever was requested, then ALWAYS write a JSON
        # summary so the runner can distinguish "check ran, expected event
        # absent" (e.g. no BG popup because bot BG joining is not implemented
        # yet) from "check never ran".
        if args.duel_guid:
            w.duel(args.duel_guid)
            # Wait for the bot's response: SMSG_DUEL_REQUESTED echoes back when
            # the opponent proposes, or SMSG_DUEL_COUNTDOWN once it accepts.
            w.pump(30, stop_on=(SMSG_DUEL_COUNTDOWN,))

        if args.queue:
            w.queue_bg(args.queue)
            log("waiting up to 180s for the battleground popup...")
            got = w.pump(180, stop_on=(SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION,))
            if not got:
                log("no popup within timeout (queue may not have filled yet)")
            elif args.accept:
                w.accept_bg()
                log("accepted, waiting for the worldport...")
                if not w.pump(60, stop_on=(SMSG_NEW_WORLD,)):
                    log("no worldport after accept")
                else:
                    w.pump(5)   # let the destination map's initial packets arrive
                    log("arrived on map %s" % w.map_id)

        summary = {
            "account": args.account,
            "char_guid": w.char_guid,
            "map": w.map_id,
            "pos": w.pos,
            "bg_slot": w.bg_slot,
            "duel_request": w.duel_request,
            "events": w.events,
        }
        out = json.dumps(summary, indent=2)
        print("BOTCHECKS-SUMMARY " + json.dumps(summary), flush=True)
        if args.events_out:
            with open(args.events_out, "w", encoding="utf-8") as fh:
                fh.write(out)
        return 0

    if args.queue:
        w.queue_bg(args.queue)
        log("waiting up to 180s for the battleground popup...")
        got = w.pump(180, stop_on=(SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION,))
        if not got:
            log("no popup within timeout (queue may not have filled yet)")
            return 1
        if args.accept:
            w.accept_bg()
            log("accepted, waiting for the worldport...")
            got = w.pump(60, stop_on=(SMSG_NEW_WORLD,))
            if not got:
                log("ERROR: no worldport after accept")
                return 1
            w.pump(5)   # let the destination map's initial packets arrive
            log("arrived on map %s" % w.map_id)

    log("holding session for %ds (Ctrl+C to stop)" % args.hold)
    w.pump(args.hold)
    log("done, map=%s" % w.map_id)

    if w.ws_log:
        log("--- battleground objective world states (%d changes) ---" % len(w.ws_log))
        for t, name, value in w.ws_log:
            log("  t=%6.1fs  %-24s = %d" % (t, name, value))
        print("OBJECTIVE-SUMMARY " + json.dumps({
            "map": w.map_id,
            "final": {WORLD_STATES[f]: v for f, v in w.world_states.items()},
            "changes": w.ws_log,
        }), flush=True)
    else:
        log("no battleground objective world states observed")
    return 0


def _graceful_logout():
    """#8 (wave27): opt-in (HEADLESS_LOGOUT=1) clean logout when the run ends, so the character does not linger
    in the world for the server's 60 s disconnect tolerance answering nothing (3 'possible cheater' lines per
    client exit). Off by default: drivers that touch the master after the client exits rely on it staying online."""
    global _PUMP_CLIENT
    w, _PUMP_CLIENT = _PUMP_CLIENT, None   # once only
    if w is None or os.environ.get("HEADLESS_LOGOUT", "0") != "1":
        return
    w.logout()


if __name__ == "__main__":
    rc = 130
    try:
        rc = main()
    except KeyboardInterrupt:
        pass
    except SystemExit as exc:
        rc = exc.code
    finally:
        _graceful_logout()
    sys.exit(rc)
