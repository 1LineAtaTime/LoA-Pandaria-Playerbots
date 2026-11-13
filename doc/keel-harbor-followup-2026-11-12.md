# Keel Harbor encounter and merchant follow-up

Migration `2026_11_12_02_world_keel_harbor_encounter_vendor.sql` repairs three
data defects missed by the earlier isolated vehicle/weapon checks.

* Playable glaive41599 was level1, Health_mod3,126 HP. Its parked version38150
  is level12, Health_mod6.17284. The playable vehicle now copies that chassis's
  level, health and armor multipliers. Live maximum health is1525. A normal
  quest infantryman attacked the controlled fixture vehicle for eight seconds:
  health1525 to1348, still boarded and alive; a repeat ended at1244 HP.
  No immunity was added.
* The encounter's200 infantry37916, nine catapults37921 and three Wolfmaw
  Outriders37938 had7200-second respawn delays. All212 authored map654,
  phase1048576 spawns now use60 seconds plus the core's normal corpse delay.
  Spawn counts are preserved; three Outriders can supply the eight required
  kills over repeated waves. Existing persistent waits for these exact spawn
  GUIDs were capped at now+60 seconds while worldserver was stopped. No other
  realm respawn or quest state was changed. A complete timed kill/respawn cycle
  was not measured in this check.
* Marie Allen's38783 (Keel Harbor) and43558 copies had vendor/repair flags and
  a merchant gossip option but no npc_vendor rows. Her36779 and38853 versions
  have23 general-goods rows. Missing copies now inherit36779's stock without
  overwriting custom items. A real gossip selection exposed the browse option
  and received the populated inventory packet.

Endgame26706 was not missing its questgiver binding. The original character
has24681 incomplete and no reward record for it;26706 now correctly requires
rewarded24681. After abandoning a prematurely accepted Endgame, complete and
turn in They Have Allies, But So Do We first. A disposable fixture with that
prerequisite rewarded received Lorna43727's quest offer through the real
questgiver-hello opcode and accepted26706 normally. The QA runner now explicitly
requires that offer response as well as persisted acceptance.
No user quest was auto-completed, re-added or reset.

No interactive mailbox gameobject exists in the queried Keel Harbor neighborhood
(map654, x-1450..-1200, y2000..2250). Existing mailbox objects are elsewhere in
Gilneas. This does not prove every decorative mailbox model's intended behavior;
the unconfirmed nearby mailbox was left unchanged.

Private rollback: `out/backups/keel-harbor-followup-20261112/` contains the
pre-change creature, creature_template and npc_vendor dump, creature_respawn
dump, and exact affected pending wait rows. Several world tables are MyISAM;
stop worldserver and restore the relevant backup for rollback. Do not distribute
these private dumps. On another database prefix, derive pending respawn GUIDs
from that world's scoped encounter spawns; do not blindly apply unrelated GUIDs.

Runner: `contrib/headless-client/keel_harbor_regression.py`; private report:
`out/keel-harbor-followup-regression.json`. It creates one disposable identity,
uses a private-phase enemy for durability, and seeds prerequisites only on that
fixture for quest eligibility. Full normal combat, all objectives and turn-in
remain distinct from this bounded check. The repair is data-only: the existing
matching native EXE/PDB was retained and worldserver restarted to load templates,
inventory and respawn changes. Warnings/errors remain enabled.
