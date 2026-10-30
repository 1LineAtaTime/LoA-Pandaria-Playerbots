-- #963 (2026-10-28, run v11 DBErrors.log): three SmartAI rows that the loader or FillScript drops.
--
-- 1) 16937 Dreghood Geomancer / 16938 Dreghood Brute -> 19477 Fleeing Dreghood Warrior (963d / 963i / 963k).
--    REAL CAUSE of "Entry 16937 ... Event 4, Link Event 5 not found or invalid" (v11) and "Event 5, Link Event 6" (v9, v10):
--    the load-time check SmartAIMgr::IsTextValid (SmartScriptMgr.cpp:1288-1328) looks the Say row up under the SCRIPT's entry
--    (16937) with a self target, and 16937 has no creature_text group 0, so the loader drops the row:
--    DBErrors.log line 45 "SmartAIMgr: Entry 16937 SourceType 0 Event 5 Action 1 using non-existent Text id 0, skipped."
--    The missing row is the "link not found". EVADE (963d) and COMBAT_STOP (963i) never reset the SmartAI: mEvents is only
--    filled in FillScript and never cleared, and UpdateEntry (Creature.cpp:509-612) does not re-create the AI. 16938 passed only
--    because the brute has its own group 0 aggro lines.
--    FIX (AC data shape, acore_world smart_scripts 17058 id 5): the Taskmaster says the line through a CREATURE_RANGE target
--    (19477 within 20 yd), which the loader does not check against the script entry, and the first fleeing warrior speaks
--    one of 19477's three lines, like AC. Order is guaranteed: SET_DATA runs the brutes' chain synchronously
--    (SmartAI::SetData -> ProcessEventsFor), so the brutes are already 19477 when the linked Say runs. The brute/geomancer chain
--    loses its Say row: 2 template, 3 faction 15, 4 emote 470, 5 random movement 10, 6 despawn 10 s, 7 combat stop (963k end).
-- 2) 8035 Dark Iron Land Mine: all five rows carry event_flags 3 (not repeatable + normal dungeon only). The mines are also
--    summoned in the open world by 8978 Thauris Balgarr (Burning Steppes, spell 11802), where FillScript keeps no event:
--    "Entry 8035 has events but no events added to list because of instance flags." Dungeon bit dropped (event_flags 1),
--    as 963g did for 5649; AC's 8035 rows carry no difficulty flag either.
-- 3) 8978 Thauris Balgarr row 0 (Cast Shoot) links to row 5, which does not exist (SFDB 548 caster-template row 4 -> 5
--    renumbered to 0 without its link): "Event 0, Link Event 5 not found or invalid". It was also an event 9 Range 0-0 that
--    can hardly fire. Row taken from AC acore_world 8978 id 0: In Combat 0-0, repeat 2300-3900, Cast Shoot (combat move), no link.
--
-- Idempotent. UNDO:
--   1) DELETE FROM smart_scripts WHERE entryorguid=17058 AND source_type=0 AND id=6; UPDATE smart_scripts SET link=0 WHERE
--      entryorguid=17058 AND source_type=0 AND id=5; re-run 2026_10_28_963d + 963i + 963k for 16937/16938 ids 2-8.
--   2) UPDATE smart_scripts SET event_flags=3 WHERE entryorguid=8035 AND source_type=0 AND id BETWEEN 0 AND 4;
--   3) UPDATE smart_scripts SET link=5,event_type=9,event_param3=21700,event_param4=35200 WHERE entryorguid=8978 AND source_type=0 AND id=0;

-- 1a) Brute / geomancer chain without the Say row.
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (16937,16938) AND `source_type`=0 AND `id` BETWEEN 5 AND 8;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(16938,0,5,6,61,0,100,0,0,0,0,0,0,89,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Start Random Movement 10'),
(16938,0,6,7,61,0,100,0,0,0,0,0,0,41,10000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Despawn In 10000 ms'),
(16938,0,7,0,61,0,100,0,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Combat Stop'),
(16937,0,5,6,61,0,100,0,0,0,0,0,0,89,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Start Random Movement 10'),
(16937,0,6,7,61,0,100,0,0,0,0,0,0,41,10000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Despawn In 10000 ms'),
(16937,0,7,0,61,0,100,0,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Combat Stop');

-- 1b) Illidari Taskmaster: after freeing the geomancers (row 5, the last Set Data), one Fleeing Dreghood Warrior says line 0.
UPDATE `smart_scripts` SET `link`=6 WHERE `entryorguid`=17058 AND `source_type`=0 AND `id`=5;
DELETE FROM `smart_scripts` WHERE `entryorguid`=17058 AND `source_type`=0 AND `id`=6;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(17058,0,6,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,9,19477,0,20,0,0,0,0,0,'Illidari Taskmaster - Linked - Fleeing Dreghood Warrior (within 20 yd) Say Line 0');

-- 2) Dark Iron Land Mine: keep "not repeatable", drop "normal dungeon only".
UPDATE `smart_scripts` SET `event_flags`=1 WHERE `entryorguid`=8035 AND `source_type`=0 AND `id` BETWEEN 0 AND 4 AND `event_flags`=3;

-- 3) Thauris Balgarr: AC row 0.
UPDATE `smart_scripts` SET `link`=0, `event_type`=0, `event_param1`=0, `event_param2`=0, `event_param3`=2300, `event_param4`=3900,
  `comment`='Thauris Balgarr - In Combat CMC - Cast ''Shoot'''
  WHERE `entryorguid`=8978 AND `source_type`=0 AND `id`=0;
