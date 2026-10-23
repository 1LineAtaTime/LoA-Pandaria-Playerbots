-- Mobus (50009, rare elite whale shark, Abyssal Depths / Vashj'ir, guid 317090): ability kit, patrol,
-- water-only movement, Thrash aura (epic #441, audit rows NP001 / WU001).
--
-- LoA state: ScriptName='' AIName='' and 0 smart_scripts rows -> plain melee. The guid's creature_addon row
-- (auras NULL) overrides the template addon's aura 8876. Path 317090 (12 points) starts with two points at
-- the circle centre, so Mobus swims through the middle of the Abyssal Breach every loop.
--
-- Intended design (Wowpedia "Mobus", MediaWiki API, fetched 2026-10-04): "circling the Abyssal Breach";
-- abilities Algae 93491 (2 s cast, 80 yd, ground pool), Enrage 8599, Ram 93492 (5 s cast, 30-50 yd range,
-- frontal knockback), Wake 93494 (5 s cast, tail knockback). Wowhead mop-classic tooltips for 93491/93492/
-- 93494 give the same ranges/cast times.
-- AI: LoA's OWN pre-rebuild SmartAI for 50009, which covers all four abilities
--   (sql/old/Merged to World DB 30_12_2023/2021_12_21_01_world_sai_Full_548.sql lines 31406, 46322-46324),
--   re-inserted with LoA's current smart_scripts columns. Chosen over alexkulya's boss_mobus.cpp (d06543d),
--   which has no Enrage and would need a new C++ file + CMake/ScriptLoader edits.
-- Patrol: alexkulya d06543d sql/updates/world/2026_09_09_00_world_boss_mobus.sql - a clean 16-point circle
--   (same centre -5694.9,5372.1 and radius 300 as LoA's path) as path 3170900, start point on the circle.
-- Movement: InhabitType 2 (water only) -> creature_template_movement Ground 0 / Swim 1 / Flight 0.
-- Pool 30010 (Mobus/Akma'hat/Garr/Julak-Doom/Xariona) is identical in LoA already - not touched.
--
-- Rollback: DELETE FROM `smart_scripts` WHERE `entryorguid` = 50009 AND `source_type` = 0;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 50009; DELETE FROM `creature_template_movement`
--   WHERE `CreatureId` = 50009; restore creature 317090 / creature_addon 317090 / waypoint_data 317090 from a dump.

UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 50009;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 50009 AND `source_type` = 0;
INSERT INTO `smart_scripts`
(`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,
 `event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,
 `action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,
 `target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`)
VALUES
(50009,0,0,0,2,0,100,1,0,40,0,0,0,11,8599,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Mobus - Between 0-40% Health - Cast Enrage (No Repeat)'),
(50009,0,1,0,0,0,100,0,8000,8000,25000,25000,0,11,93491,0,0,0,0,0,4,0,0,0,0,0,0,0,0,'Mobus - In Combat - Cast Algae'),
(50009,0,2,0,9,0,100,0,30,50,15000,25000,0,11,93492,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Mobus - Within 30-50 Range - Cast Ram'),
(50009,0,3,0,0,0,100,0,5000,8000,14000,15000,0,11,93494,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Mobus - In Combat - Cast Wake');

DELETE FROM `creature_template_movement` WHERE `CreatureId` = 50009;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`) VALUES
(50009, 0, 1, 0, 0);

UPDATE `creature` SET `position_x` = -5417.732, `position_y` = 5257.314, `position_z` = -487.6926, `orientation` = 2.7324486,
  `wander_distance` = 0, `MovementType` = 2 WHERE `guid` = 317090 AND `id` = 50009;

DELETE FROM `creature_addon` WHERE `guid` = 317090;
INSERT INTO `creature_addon` (`guid`, `path_id`, `auras`) VALUES
(317090, 3170900, '8876');

DELETE FROM `waypoint_data` WHERE `id` IN (317090, 3170900);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`) VALUES
(3170900, 0, -5417.732, 5257.314, -487.6926, 0, 0, 0),
(3170900, 1, -5482.764, 5159.9873, -487.6926, 0, 0, 0),
(3170900, 2, -5580.091, 5094.955, -487.6926, 0, 0, 0),
(3170900, 3, -5694.896, 5072.119, -487.6926, 0, 0, 0),
(3170900, 4, -5809.701, 5094.955, -487.6926, 0, 0, 0),
(3170900, 5, -5907.028, 5159.9873, -487.6926, 0, 0, 0),
(3170900, 6, -5972.06, 5257.314, -487.6926, 0, 0, 0),
(3170900, 7, -5994.896, 5372.119, -487.6926, 0, 0, 0),
(3170900, 8, -5972.06, 5486.9243, -487.6926, 0, 0, 0),
(3170900, 9, -5907.0283, 5584.251, -487.6926, 0, 0, 0),
(3170900, 10, -5809.701, 5649.283, -487.6926, 0, 0, 0),
(3170900, 11, -5694.896, 5672.119, -487.6926, 0, 0, 0),
(3170900, 12, -5580.091, 5649.283, -487.6926, 0, 0, 0),
(3170900, 13, -5482.7637, 5584.251, -487.6926, 0, 0, 0),
(3170900, 14, -5417.732, 5486.924, -487.6926, 0, 0, 0),
(3170900, 15, -5394.896, 5372.119, -487.6926, 0, 0, 0);
