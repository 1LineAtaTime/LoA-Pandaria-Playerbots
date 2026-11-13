# Release-26 quest and Worgen repair batch — November 6, 2026

The user authorized broad documented patches on the current test deployment.
This batch uses 79 files from the 296-file SkyFire release-26 archive pinned at
505f6e63a20c3ea75dfebb17f58e891f4e5fb44c. It includes all data statements from44
files and a reviewed subset from35. The remaining217 files are outside THIS
batch, not automatically broken, unreviewed or unapplied by earlier campaigns.
The per-source ledger lists original hashes, upstream URLs and excluded portions.
This is not a claim that every published SkyFire fix is now installed.

## Published data adaptations

SQL33: `2026_11_06_33_world_release26_quests_ai.sql`.
The assembler validates every original hash before selecting source statements.
458 original statements contribute377 final quest-field values and starter AI.

- Map old quest Details/Objectives/Title to active QuestDescription,
  LogDescription/LogTitle. Reward and request text belongs in quest_offer_reward
  and quest_request_items, not the unused legacy quest_template text columns.
- Map prerequisite/next-quest and skill requirements to quest_template_addon;
  preserve unmodified emotes, rewards, class/race masks and unrelated fields.
- Retain SQL27/29's existing training numeric/gate corrections. Correct the
  published Night Elf Simple Sigil3106 reference to3116 before importing text.
- Populate both entry and CreatureID for authored creature speech. Restrict
  SmartAI replacement to the actor AND source type, preserving unrelated lists.
- Fix the published Frostmane Novice row1 self-link to link2, which actually
  reaches the linked frostbolt/phase actions. The final link topology is checked.
- Restore Grik'nir, Wayward Fire Elemental, Frostmane Novice/Blade/Whelp and
  Wendigo combat data; trapped-miner, frozen-mountaineer/Icy Tomb, bomber-fire,
  cage/demolitionist and mechano-tank interactions; two airfield charter routes.
- The first engine load exposed unsupported legacy action46 on Captured
  Demolitionist4264500. Translate its authored two-yard forward move into this
  engine's existing relative action114, target_y=2. SQL35 upgrades early SQL33
  deployments; the assembler now generates the correct row directly. No
  unsupported action is deleted or replaced with an empty success.

Independent input checks found all11 referenced cast/hit spells, both taxi routes
2201/2207 and display11686 in installed client/server data. Both charter NPCs
already have spellclick flags and spell52096 bindings. No generic script alias,
new navigation format, logger change or account/shop data import is involved.

## ExampleCharacter / Bloodfang local repair

The added user report identified rogue ExampleCharacter, level4, on map654 near
(-1675.04,1338.85,15.1349), quest14272 Eviscerate. The objective requires learning
2098 and practice credit44175. Bloodfang35118's assigned npc_bloodfang_worgen
handler was missing locally; no handler or matching credit existed in this fork.
The inspected pinned SkyFire Gilneas source does not supply that missing handler.
The following is a LOCAL repair, not a claim of copying a published exact fix.

- Register the missing native handler. Only an actual Eviscerate hit from a rogue
  with the incomplete quest awards practice credit. No character quest is forced
  complete and no inventory/password/account record is changed.
- While that rogue still needs practice, cap otherwise lethal damage at1HP so a
  combo-point builder/autoattack cannot remove the practice target first. After
  credit/quest completion, normal player damage can kill it. Other players can
  kill normally. Non-player ambient attackers cannot reduce it below half health.
- Ambient NPC combat uses AttackStartNoMove; fighting players still chases. This
  prevents the ambient guard fight from dragging multiple worgen into one pile.
- The template's level2/3 health is normal55/71; it is not globally inflated.
  Three actual close overlapping spawn pairs were identified. SQL34 removes only
  higher GUID221484/221504/911770, with exact entry/map/phase/position guards and
  retained counterpart checks. No addon/event/pool/formation/linked-respawn
  references were found. Retained GUIDs220893/221482/911724 are unchanged.
- Later-phase35229 has262 spawns, including67 saved at1HP. That separate phase's
  scripted/cinematic population is NOT blanket-deleted or rebalanced from this
  first-phase report. Remaining population/health review stays on the checklist.

ExampleCharacter's client acceptance remains pending: user was away from the PC. Compilation,
script loading and data checks do not establish a natural completed quest.

## Validation, deployment and recovery

A complete eight-table dump was restored into a disposable schema. Final fixture
passes377 expected active-field checks, unrelated-field preservation and repeat
stability; fixture removed. Changes: creature_template5, creature_text28,
quest_offer_reward144, quest_request_items36, quest_template42,
quest_template_addon22, smart_scripts72; gameobject_template unchanged. These are
row changes, not349 newly playtested quests. An initial preservation attempt
reported quest26918; the instrumented repeat did not reproduce the discrepancy
and passed. No live migration ran before that successful fixture. Keep this
observation available if a future preservation check fails.

Initial backup: out/release26-quests-before-20261106-014552.sql,52,233,333 bytes,
SHA256 `2dd826d4ce8f691aa5e7f712697fadfe3b50dca193cb25c66e0196f524e8eec1`.
Final deployment backup includes creature plus those eight tables:
out/release26-bloodfang-before-20261106-042805.sql,86,694,268 bytes,
SHA256 `383e99ef072a0cf10044fd6183b72b8435b4c67b3e29c7ceec4d89677a7d0000`.
Dumps and credential-bearing configs remain private under ignored out/.

Visual Studio final incremental build:3 succeeded,0 failed,51.994 seconds.
An earlier compile used the old getClass API; corrected to this fork's GetClass
before deployment. All19 native CTests pass. Live SQL33/34 contracts:835/835;
the final suite also records the movement alignment, removed/retained spawns and
native binding. These are data checks, not full quest or flight playthroughs.

| Artifact | SHA256 |
| --- | --- |
| worldserver.exe | 98191c7615bffdcad524c40653743b68ba78e8b8fa2ce9ca84c674dac4e4f148 |
| worldserver.pdb | 5a63d861492d3b1d4384c3a6568578b9ea2314ffeef7aacbf5287973be833225 |

Built/exported bytes match. Previous executable/symbols/logs are archived under
out/runtime-before-release26-bloodfang-20261106-042823. The user reopened the old
server during preparation; it was left running until final artifacts were ready,
then stopped normally for the fresh nine-table backup and deployment. No Ctrl+C
batch prompt or hidden replacement server. Authserver stays running.

The first boot removed the missing Bloodfang binding but exposed the one action46
backend mismatch. SQL35 and supported smart_scripts reload remove that validation
failure. Earlier logs are preserved; final startup comparison follows below.
Existing unrelated content errors remain open and are not filtered out.

For recovery: stop worldserver, preserve a fresh backup of later edits, restore
all nine recorded tables into the configured world schema from the final backup,
and restore the old EXE/PDB pair together. This also reverts subsequent edits to
those covered tables. Git revert alone does not restore database state. Use a
separate inspected subset of the backup for a narrower recovery. The native
Bloodfang handler is harmless with pre-batch spawn data, but reverting to the old
executable restores its previously missing binding.

## Reproduction

```powershell
python contrib/upstream-tests/assemble_release26_quest_batch.py --patches out/skyfire-quest-research/release26-sql --output sql/updates/world/2026_11_06_33_world_release26_quests_ai.sql --ledger doc/release26-quest-batch-ledger-2026-11-06.json --desired contrib/upstream-tests/suites/release26-quest-fields.json
python contrib/upstream-tests/test_release26_quest_batch.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --backup out/release26-quests-before-20261106-014552.sql --report out/release26-quest-fixture.json
python contrib/upstream-tests/run_world_contracts.py --config exports/worldserver.conf --client out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe --suite contrib/upstream-tests/suites/release26-quests.json --report out/release26-quest-contracts.json
```

The fixture executes only this trusted reviewed SQL and the operator's backup as
administrator in a disposable world schema; it is not an untrusted-SQL sandbox.
Remaining source statements retain explicit reasons in the ledger. Native
training-credit support for other races/classes, broader Gilneas progression,
spawn density and natural rescue/flight recovery remain acceptance/backlog work.


## Final runtime observation

Worldserver56660 reopened visibly at04:34:03 local; auth61228 remains running.
Final839/839 data contracts pass. Fresh error records: 1,423 versus 1,460 in the
pre-batch runtime, with zero new exact error signatures. The missing Bloodfang
handler and action46/4264500 mismatch are absent. Fresh Server.log contains zero
MMap warnings. Remaining content errors are still recorded, not suppressed.
This short observation is not natural quest completion or a long crash soak.
