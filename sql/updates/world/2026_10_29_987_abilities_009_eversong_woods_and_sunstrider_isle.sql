-- #987 wave 1 (pure additions): Eversong Woods and Sunstrider Isle creature abilities, imported from DeltaTho PR 15 file 2026_10_24_13_world.sql.
-- 7 creatures, 7 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '15494,15635,15649,15650,15949,15967,16855';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(15494, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 11, 11939, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Yasmine Teli/Larien - On Just Summoned - Summon Imp'),
(15635, 0, 0, 0, 2, 0, 100, 1, 10, 30, 0, 0, 11, 12160, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eversong Tender - Between 10-30% Health - Cast ''Rejuvenation'' (No Repeat)'),
(15649, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 29117, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Feral Dragonhawk Hatchling - Between 5-15% Health - Cast ''Feather Burst'' (No Repeat)'),
(15650, 0, 0, 0, 0, 0, 100, 0, 3100, 3100, 22800, 22800, 11, 29117, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crazed Dragonhawk - In Combat - Cast ''Feather Burst'''),
(15949, 0, 0, 0, 0, 0, 100, 0, 2500, 3000, 9500, 10000, 11, 6818, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thaelis the Hungerer - In Combat - Cast ''Corrupted Intellect'''),
(15967, 0, 0, 0, 2, 0, 100, 1, 20, 40, 0, 0, 11, 7994, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ether Fiend - Between 20-40% Health - Cast ''Nullify Mana'' (No Repeat)'),
(16855, 0, 0, 0, 0, 0, 100, 0, 4500, 6000, 25000, 26000, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tregla - In Combat - Cast ''Pierce Armor''');
