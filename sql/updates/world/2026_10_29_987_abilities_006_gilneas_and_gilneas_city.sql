-- #987 wave 1 (pure additions): Gilneas and Gilneas City creature abilities, imported from DeltaTho PR 15 file 2026_10_24_10_world.sql.
-- 17 creatures, 24 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '35504,36057,36207,36396,36488,36671,36813,37692,37733,37735,37757,38022,38192,38210,38616,44459,44469';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35504, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2000, 2000, 11, 67595, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 0 and 0 ms (and later repeats every 2000 and 2000 ms) - Self: Cast spell 67595 on Victim // '),
(35504, 0, 1, 0, 1, 0, 100, 0, 2000, 2000, 2000, 2000, 49, 0, 0, 0, 0, 0, 0, 11, 35505, 20, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 2000 and 2000 ms (and later repeats every 2000 and 2000 ms) - Self: Attack Creature Bloodfang Ripper (35505) in 20 yd // '),
(36057, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northgate Rebel - In Combat - Cast ''Shoot'''),
(36057, 0, 1, 0, 9, 0, 100, 0, 0, 5, 12000, 14500, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northgate Rebel - Within 0-5 Range - Cast ''Cleave'''),
(36207, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On just summoned - Self: Talk 0 // '),
(36207, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Attack Owner/Summoner // '),
(36207, 0, 2, 0, 67, 0, 100, 0, 9000, 12000, 0, 0, 11, 75360, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When victim is behind me (wait 9000 - 12000 ms before next event trigger) - Self: Cast spell 75360 on Victim // Forsaken Assassin - On Behind Target - Cast ''Backstab'''),
(36396, 0, 0, 0, 0, 0, 100, 0, 3500, 4500, 14000, 20000, 11, 75361, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Sailor - In Combat - Cast ''Swashbuckling Slice'''),
(36488, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 18000, 27000, 11, 75395, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Castaway - In Combat - Cast ''Planked'''),
(36671, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 17000, 22000, 11, 75388, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Scout - In Combat - Cast ''Rusty Cut'''),
(36813, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 18500, 21500, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vilebrood Skitterer - In Combat - Cast ''Poison'''),
(37692, 0, 0, 0, 0, 0, 100, 0, 2500, 4500, 12000, 13000, 11, 57846, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Infantry - In Combat - Cast ''Heroic Strike'''),
(37733, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 15000, 19000, 11, 75359, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lord Walden - In Combat - Cast ''Seasoned Brandy'''),
(37735, 0, 0, 1, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Baron Ashbury - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(37735, 0, 1, 0, 61, 0, 100, 1, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Baron Ashbury - Between 0-30% Health - Say Line 0 (No Repeat)'),
(37757, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Howling Banshee - In Combat - Cast ''Shadow Bolt'''),
(37757, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 25000, 27000, 11, 75438, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Howling Banshee - In Combat - Cast ''Banshee Screech'''),
(38022, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Veteran Dark Ranger - In Combat - Cast ''Shoot'''),
(38022, 0, 1, 0, 0, 0, 100, 0, 8500, 12000, 18500, 24000, 11, 75439, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Veteran Dark Ranger - In Combat - Cast ''Black Shot'''),
(38192, 0, 0, 0, 0, 0, 100, 0, 2500, 4500, 12000, 13000, 11, 57846, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Infantry - In Combat - Cast ''Heroic Strike'''),
(38210, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Crossbowman - In Combat - Cast ''Shoot'''),
(38616, 0, 0, 0, 0, 0, 100, 0, 2500, 4500, 12000, 13000, 11, 57846, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Infantry - In Combat - Cast ''Heroic Strike'''),
(44459, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 13236, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Cast spell 13236 on Self // '),
(44469, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 45104, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Cast spell 45104 on Self // ');
