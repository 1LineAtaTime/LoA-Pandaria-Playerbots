# Endgame return camera and movement ownership

The user completed26706's return flight, but reported a distant camera and no
turn-in dialogue at Lorna. File-only packet logging confirmed receipt of the
real questgiver-hello for43727, rejected by the interaction check. A read-only
native snapshot showed the player still at the harbor landing point,48.48 yards
from Lorna, although the client showed the character walking beside her. The
completed quest and questgiver binding were intact. Subsequently the live
character's26706 reward record was confirmed; it is not reset or re-added.

Two control-owner cleanup defects were found: Player::SetClientControl cleared
the current mover GUID when revoking any target, and GameClient::RemoveAllowedMover
cleared the active mover when revoking a different, older unit. Both now clear
only the owner being revoked. This preserves a newer player-control grant when
the old vehicle releases its control.

The scripted wyvern now separates input from camera ownership: AI control stays
in place, while the camera viewpoint explicitly follows the moving wyvern.
On normal arrival or aborted boarding, it releases that viewpoint and finishes
by granting control back to the rider. Arrival repeats that restore after all
passenger teardown and before the world-space transfer and quest credit. The
camera is never intentionally left attached to a departing summon.

Visual Studio RelWithDebInfo/x64 build succeeded. The deployed regression passed:
the private farsight target matched the moving wyvern while boarded and was0
after landing; the fixture returned to(-1336.1,2097.88,13.87), then sent ordinary
walking packets to Lorna. The server saved(-1299.28,2128.4,5.63), returned the
real offer-reward opcode0x074f, and accepted the normal quest reward exchange.
The26706 reward record was saved, logout acknowledged, and the owned test
account, character and dependent records removed.

Runner: `contrib/headless-client/wyvern_camera_turnin_regression.py`; private
report: `out/wyvern-camera-turnin-regression.json`. The test seeds only its own
prerequisites and starts the native escape with a normal ride aura on a temporary
wyvern; no GM quest-completion/reward command is used. Its local client position
is synchronized to the acknowledged server return before ordinary walking,
without another server teleport. This verifies protocol ownership and gameplay
flow; real-client pixel framing remains separate from this camera-field check.

The native EXE/PDB checkpoint is private under
`out/backups/wyvern-control-20261112/`. No SQL migration or account/password change
is required. Temporary recovery teleport bookmarks are removed from the world
table and the native teleport cache is reloaded. No live player quest is completed
by the regression; disposable identities own all seeded/test quest state.
