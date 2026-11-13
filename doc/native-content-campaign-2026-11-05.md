# Native events and starting-zone activation campaign — November 5, 2026

The user authorized large documented repair/testing batches on the current test
server. This batch repairs native execution and activates reviewed complete
ports already in the repository. It does not remove missing bindings merely to
lower logs, enable commented incomplete encounters, or claim all content works.

## Native work (source7f75c1fb)

- Implement synchronous SmartAI source-type3 immediate timed position summons
  through the actual Map::ScriptsStart event path. Validate every row and the
  entire graph; reject duplicates, missing/cyclic links, unreachable linked rows,
  deferred/phase/unsupported actions and nonfinite coordinates. Invalid groups
  are rejected atomically. Evaluate conditions and authored chance. A 0..99 draw
  gives exactly50 successful samples for50%; preserve legacy zero-as-always.
- Copy event rows during execution so summon callbacks/reloads cannot invalidate
  references. Timed summons own their normal lifetime; no dangling event context
  or delayed callback is retained. Failed summons still log the event/row/entry/
  map/coordinates. Retain the existing full event_scripts sequence when one
  exists, instead of running two summoning backends or discarding extra triggers.
- Event26277 has25 independent50% worgen summons with50-second lifetimes and
  position targets. This is the supported current event group.
- Event21888 already has an authored legacy sequence with six immediate actors
  plus delayed quest trigger52235. Preserve its distinct positions/lifetimes and
  extra trigger; log the backend choice once as loading information. Its six
  overlapping SmartAI rows are not separately executed.
- Event21889 has six rows including nonzero action_param5, no matching legacy
  sequence, and remains unsupported. Do not silently ignore those parameters or
  claim that all37 previously rejected rows are now repaired. One actionable
  group diagnostic replaces per-row rejection; the entire group remains blocked.
- Register complete existing Nightmare Vine28720 handler (SCRIPT_EFFECT77,
  effect0) and verify its pollen28721 dependency in installed DBC. Register
  existing Sinestra area trigger6312, whose instance dialogue handler is present.
- Correct pillory NPC dialogue IDs to the current authored rows: Krennan0,
  Godfrey1, Genn protocol0/dosage2. Preserve all text instead of inventing lines.
- Add botscenario worldevent for identified random bots only; refuse personal
  characters, pending teleports and actors farther than400yd from authored
  positions. Count actual surrounding matching NPCs before/after the real Map
  dispatch and emit [WORLD_EVENT] trace for bounded automated spawn assertions.

## Data work

SQL28 adapts existing local ports2026_09_28_21 and2026_10_09_12 to the actual DB:

- Hot Rod34840 -> npc_hot_rod_rolling_my_homies, world friends34890/34892/34954
  -> npc_homies, summoned copies34957/34958/34959 -> npc_homies_summon.
- Enable the14071 quest accept hook/detection auras. Retire only the two exact
  Ace/Gobber on-foot auto-credit rows; preserve unrelated/custom SmartAI.
- Remove obsolete spell_klaxon duplicate only if61178's complete Wintergrasp/
  Kezan dispatcher exists. Preserve that dispatcher and the existing66299 radio.
- Krennan36331 -> npc_krennan_aranas_pillory; enable68639 release handler and
  the68630 area4786 scene rule after14222 rewarded, until14375 rewarded. Static
  Genn14375 giver spawn375757 already has phase4097 and start/turn-in relations.

SQL29 adapts nine nonzero training-chain fields from the seven reviewed starter
files. Fix the Monk letter31141 incorrectly leading into warrior26913, add the
published missing training prerequisites, and use Night Elf3116 instead of Dwarf
3106. The source's human warlock3105 -> mage26916 typo is corrected to26914;
the local value already matches, so no write is needed. Preserve four existing
tutorial prerequisites rather than copying upstream prerequisite removals.

## Tests before deployment

-18/18 CTests pass, including new immediate-world-event topology/chance cases.
  Native final build/deployment and final suite evidence are recorded below.
- Combined five-table disposable fixture passes: eight creature bindings, nine
  chain fields, two exact retired SmartAI rows, spell dispatch/release and area
  rule. All other fields/rows preserved using per-row SHA256 over binary values
  plus explicit changed-column comparisons. A second application is identical.
- Scoped manual SQL28/29 rollback restores the complete fixture to the original
  five-table snapshot. Fixture removed. No runtime rollback was performed.
- SQL29 independently passes full-row preservation and repeat stability with all
  nine old-value guards matching across14,232 addon rows.
- Before deployment, the17-case port contract suite has4 passes/13 failures,
  reproducing missing hookups; postconditions will be rerun after application.
- The fixture caught and corrected rollback generation bugs for empty strings
  and collations before any live data mutation. The generic numeric tester also
  needed case-insensitive SQL column lookup (NextQuestId versus NextQuestID).

Raw private reports are out/campaign-ports-fixture.json,
out/campaign-training-chains-fixture.json and out/campaign-ports-before.json.
Tracked assertions and manual reversal are in contrib/upstream-tests/suites/
and contrib/upstream-tests/rollback/. Manual rollback is outside auto updates.

## Missing-binding review

Review inventory: native-binding-review-2026-11-05.csv, covering all958 baseline
missing names. Six have reviewed repairs in this batch. Five names disappear
from fresh missing-binding diagnostics; npc_friend_gob is only partially repaired:
three custom SmartAI clones934957/934958/934959 still refer to it. Do not bind
those different entries to canonical homie code without reviewing their role.
Another33 have an exact
local source token but are not proven complete/registered. For919, no exact token
was found. Name absence alone cannot rule out an implementation under an alias;
source-token presence likewise does not prove functioning code. Preserve open
states rather than automatically registering comment blocks or deleting rows.

Known deferred examples include disabled Firelands/legendary chains and the
Stay of Execution handler, whose arithmetic/effect behavior needs review before
registration. The55 distinct missing SmartAI speaker/group requests were checked
against both retained LoA and SkyFire data. The only original matches were three
explicit-other-speaker cases, including Turkey Hunter dialogue on a stale GUID;
they are not safe self-dialogue repairs. No dialogue was manufactured/copied onto
wrong actors to hide those diagnostics.

## Required remaining acceptance

- [x] Final Visual Studio build, backed-up SQL28/29/native deployment and fresh
  log scan; verify both remote refs and existing matching EXE/PDB staging.
- [x] Run the actual cannon world-event path using a reserved bot, bounded spawn
  count and recorded actor setup/return request. This is a mechanism test, not a
  full quest completion or statistical proof from one random draw.
- [ ] Natural14071 accept/drive/horn/friend pickup/seat/credit/abandon/retry.
- [ ] Natural14222 ->14375 arrival/scene/quest/reward/free movement/relog.
- [ ] Starter class training accept/credit/reward/next-quest matrix, including
  Monk31141 and Night Elf3116, with no forced-completion shortcut.
- [ ] Sinestra dialogue in its actual instance and Nightmare Vine gather/pollen
  behavior; registration/DBC checks are not those gameplay tests.
- [ ] Implement/adapt21889's unhandled semantics, remaining native bindings,
  SmartAI/text/loot/conditions/vehicles and map571/empty navigation request.
- [ ] Companion flight/teleport/instance persistence, group support/loot/silence,
  city distribution/spacing and flying NPCs; auction/mail/BattlePay endurance.
- [ ] Sanitized Windows/Linux clean installation and long bot-load/crash soak.

## Deployment and measured results —18:54–18:59 local

Visual Studio rebuilt worldserver successfully (5 succeeded/0 failed in the final
incremental build); final18/18 CTests pass. The final native code is7f75c1fb;
later8821a21a adds the paired migrations, tests and report. An embedded dirty
marker can reflect test-tool edits during compilation; matching artifacts below
identify the actual deployed pair. No native code was edited after that build.

Worldserver59704 stopped through its supported shutdown command. Full backup:
`out/native-quest-campaign-before-20261105-185405.sql`,23,725,090 bytes, SHA256
`805e2247c341a1f6d4ffae6923a1b154db399497ac431dea1afedce528c1ef39`.
The test bot's complete characters row was separately backed up in
`out/campaign-test-bot-before-20261105-185405.sql`, SHA256
`2eea3fee86ef9fc1893a7e47f5267d133c531520657f56d1d6f9527fc62e7694`.
Private dumps/configurations remain ignored and outside exports/Git.

The actual backup was restored into a disposable schema and upgraded/reversed
again. All original row hashes across57,597 creature templates,14,232 quest
addons,44,362 SmartAI rows,5,223 spell bindings and1,381 area rules return exactly
after rollback. Fixture removed; no runtime rollback performed. Report:
out/campaign-backup-recovery.json. Before reversing a deployed batch, stop the
server, retain a fresh backup, review later edits and use the paired manual
rollback or restore these five tables. Restore the previous EXE/PDB together if
reverting the native change; Git alone cannot undo the database.

Prior EXE/PDB/logs: out/runtime-before-final-nov5-20261105-185407.

| Artifact | SHA256 |
| --- | --- |
|worldserver.exe|e582351508cec7b02e0a4257c3f008af9095c152e840cf8529d5c1d5bb1e71a8|
|worldserver.pdb|6d956a9c81c11dbfb2ab20ac3ba2890eca37fcda849a5abab2e179e7ba5953d3|

Built/exported hashes match. Visible launcher started worldserver28648 at18:54:53,
initialization19sec, listening8085. Auth61228 remains on3724. No auth, personal
character, inventory, mail, wallet or BattlePay catalog migration was performed.
The reserved bot login/movement test is separate, recorded normal test activity.

Data suites:17/17 existing-port checks,34/34 chain checks,306/306 prior training
checks:357/357 total. No new Nightmare Vine/pillory hook validation error. Fresh
missing-script diagnostics953, compared with958 previously. New event diagnostics:
no generic unsupported source_type3 errors; one explicit unsupported21889 group
remains (six underlying rows). Legacy21888 backend choice is recorded once as
loading information. Other SmartAI/speaker/loot/condition/vehicle errors remain.

The actual event test at18:55:49 uses reserved RNDBOT actor Orgek, not a personal
character: Map0, event26277, before0/after17, created17 (accepted bounds1..25).
The tool requests return to the actor's saved location after the sample; it does
not claim an exact restored character state or full quest completion. The bot
continues normal rotation afterward (later saved on map571, away from test site).
Raw report: out/campaign-world-event-live.json. Authenticated shop/client UI and
full natural Goblin/Worgen quest playthroughs were not exercised by this test.

No crash observed in this short post-restart sample. This is not long-duration
load certification. No log severity/categories, navigation files or Detour code
were changed. Current map571 failure did not reproduce in this sample.
