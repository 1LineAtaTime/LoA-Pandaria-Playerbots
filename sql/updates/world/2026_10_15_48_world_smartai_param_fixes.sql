-- Refs #183 (SmartAI data errors, part 2: broken link / min-max parameters)
-- All fixes are exact-key UPDATEs; source of truth is named per row. Idempotent (each WHERE includes the old broken value).
--
-- A) "SmartScript::ProcessAction: Entry N SourceType 0, Event E, Link Event L not found or invalid, skipped." (SmartScript.cpp:2535, runtime, repeats per trigger)
--    A linked event must exist AND be event_type 61 (LINK). 4 entries, 19 lines in the test log.
--    6497 Astor Hadren (2 lines): row 4 is event_type 64 here but 61 (LINK) in the MoP 5.4.8 data (and in the row that links to it) -> restore 61.
--    26434 Frostpaw Trapper (3 lines): row 0 links to id 2 (an Aggro event); AC has link 1 (the 'Stop Attacking' LINK row) -> link 1.
--    27788 Injured 7th Legion Soldier (10 lines): row 3 (On Evade) links to id 5 (an On-Death event); identical in the MoP 5.4.8 data, no AC row,
--          the link target does not exist as a LINK -> link 0 (the row's own cast still runs; only the bogus follow-up is dropped).
--    67099 Succula (4 lines): row 0 links to id 1 (an On-Reset event, not a LINK); identical in the MoP 5.4.8 data -> link 0.
UPDATE `smart_scripts` SET `event_type`=61 WHERE `entryorguid`=6497 AND `source_type`=0 AND `id`=4 AND `link`=0 AND `event_type`=64;
UPDATE `smart_scripts` SET `link`=1 WHERE `entryorguid`=26434 AND `source_type`=0 AND `id`=0 AND `link`=2;
UPDATE `smart_scripts` SET `link`=0 WHERE `entryorguid`=27788 AND `source_type`=0 AND `id`=3 AND `link`=5;
UPDATE `smart_scripts` SET `link`=0 WHERE `entryorguid`=67099 AND `source_type`=0 AND `id`=0 AND `link`=1;
--
-- B) "SmartAIMgr: Entry N SourceType S Event E Action A uses min/max params wrong (X/Y), skipped." (SmartScriptMgr IsMinMaxValid) - 5 lines, 4 are fixable:
--    17214 Anchorite Fateema id 0, 17215 Daedal id 2 (event 20 'quest reward'): cooldownMin=2 / cooldownMax=0 here, 0/0 in AC (same quest ids 9463/9473) -> event_param2=0.
--    32423 Matthias Lehner id 0 (event 20, quest 13398): cooldownMin=1 / Max=0 here, 0/0 in AC -> event_param2=0.
--    25073 Darkspine Siren id 4 (event 2 'health pct'): 'Between 15-0% Health' has min 15 > max 0; AC's equivalent row (id 13/14) is min 0 / max 15 -> swap.
--    (3304100 Aetherion actionlist id 0 is debris: event 75 with comment '0'; left alone, 1 line.)
UPDATE `smart_scripts` SET `event_param2`=0 WHERE `entryorguid`=17214 AND `source_type`=0 AND `id`=0 AND `event_type`=20 AND `event_param2`=2;
UPDATE `smart_scripts` SET `event_param2`=0 WHERE `entryorguid`=17215 AND `source_type`=0 AND `id`=2 AND `event_type`=20 AND `event_param2`=2;
UPDATE `smart_scripts` SET `event_param2`=0 WHERE `entryorguid`=32423 AND `source_type`=0 AND `id`=0 AND `event_type`=20 AND `event_param2`=1;
UPDATE `smart_scripts` SET `event_param1`=0, `event_param2`=15 WHERE `entryorguid`=25073 AND `source_type`=0 AND `id`=4 AND `event_type`=2 AND `event_param1`=15 AND `event_param2`=0;
--
-- ROWS: 8 UPDATEs on smart_scripts (4 link, 4 min/max). 
-- VERIFY: SELECT entryorguid,id,link,event_type,event_param1,event_param2 FROM smart_scripts WHERE (entryorguid,source_type,id) IN ((6497,0,4),(26434,0,0),(27788,0,3),(67099,0,0),(17214,0,0),(17215,0,2),(32423,0,0),(25073,0,4));
-- EXPECTED DBErrors.log DROP: 19 ProcessAction link lines (runtime, per trigger) + 4 min/max boot lines (17214, 17215, 32423, 25073).
-- UNDO: reverse each SET (6497 id4 event_type 61->64; 26434 id0 link 1->2; 27788 id3 link 0->5; 67099 id0 link 0->1; 17214/17215 param2 0->2; 32423 param2 0->1; 25073 id4 params 0,15 -> 15,0).
