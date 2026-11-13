# Large AI ownership batch — November 8

## Scope

Activate existing SmartAI for 32 creature templates whose database native script
names are explicitly unregistered in the running build. Those templates have 85
SmartAI database event rows. They previously could not execute through SmartAI
because ownership selected an unavailable native handler. The guarded SQL changes
only AIName/ScriptName on these exact templates and retains every event row.

The batch covers Pandaria combat/quest NPCs, selected older dungeon creatures and
outdoor actors. The adjacent JSON lists every entry, prior handler and event count.
This is an available-behavior fallback: it does not recreate missing native boss
phases or quest scenes. Native parity remains open for all affected entries.
Registered native handlers and custom BattlePay, auction/playerbot policy code are
not replaced. It is SQL-only; no executable rebuild was necessary.

## Audit correction

The engine accepts both SmartAI and SmartVehicleAI in SmartScriptMgr.cpp. The QA
query accepted only SmartAI, causing 28 vehicle templates to be incorrectly flagged.
The query now matches the engine. All 28 vehicle AI selections were preserved and
checked explicitly. Do not count these 28 as repaired gameplay NPCs: they are audit
false positives, separate from the 32 actual ownership repairs.

## Backup and checks

creature_template and smart_scripts were dumped before mutation to ignored
out/qa-ai-batch-20261108/before.sql; its SHA-256 is in the JSON report. A repeated
patch application changed no affected-table checksum.

All 33 ownership/vehicle assertions and 408 existing starter/Gilneas/quest contracts
passed, along with 12 framework tests. Worldserver shut down normally and restarted
through the visible export launcher; authserver remained up. Logging thresholds
were not changed. At the initial post-restart sample, SQL signatures fell from
1,306 to 1,222 (84 absent and zero new). This is a bounded runtime/startup comparison,
not proof of full encounter behavior. Full client regression evidence follows.

## Remaining implementation backlog

The current startup names 876 unregistered native scripts across script types.
The saved remaining-native-script-bindings-2026-11-08.json maps 621 creature templates
to missing handlers; 356 have spawns and 103 are spawned quest-giver templates. Quest
accept/turn-in can still work through generic handlers, so this does not mean
all of those quests are impossible. It identifies missing custom behavior that
cannot be supplied just by changing AIName. Prioritize area/quest-chain ports using
that inventory and verify source/data compatibility.

The three remaining structural audit categories are missing creature/item/
gameobject objective targets. Eight broader gameplay capability groups remain
untested. No missing objective was fabricated or removed to get a passing result.

## Final regression result

All 15 supported client scenarios passed, all 15 temporary accounts/characters were
removed, and the comparator found no regressions. Eight gameplay groups remain
untested. The QA command returns exit 1 for the three remaining failed content
audits, not a supported client-scenario failure. The SQL fallback handlers have
not each received an encounter/quest-specific live acceptance test.
