-- #987 wave 1 (pure additions): Un'Goro Crater creature abilities, imported from DeltaTho PR 15 file 2026_10_24_52_world.sql.
-- 11 creatures, 14 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '6506,6514,6521,6556,6560,38254,38305,38307,38329,38708,51525';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(6506, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ravasaur Runner - On Aggro - Cast ''Rushing Charge'''),
(6514, 0, 0, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 12555, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Un''Goro Gorilla - Between 30-60% Health - Cast ''Pummel'' (No Repeat)'),
(6521, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 11350, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Living Blaze - On Aggro - Cast ''Fire Shield'''),
(6556, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 14133, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Muculent Ooze - On Reset - Cast ''Muculent Fever Proc'''),
(6560, 0, 0, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 5810, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stone Guardian - Between 30-60% Health - Cast ''Stone Skin'' (No Repeat)'),
(38254, 0, 0, 0, 0, 0, 100, 0, 5000, 9000, 19000, 27000, 11, 83383, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Steaming Fury - In Combat - Cast ''<Spell not found!>'''),
(38305, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 71791, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gorishi Fledgling Colossus - Within 0-8 Range - Cast ''Poison Explosion'''),
(38305, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 71792, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gorishi Fledgling Colossus - In Combat - Cast ''Poison Spit'''),
(38307, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gormashh - Within 0-8 Range - Cast ''Trample'''),
(38329, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 83386, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Durrin Direshovel - In Combat - Cast ''Heave'''),
(38329, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11971, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Durrin Direshovel - In Combat - Cast ''Sunder Armor'''),
(38708, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 44531, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Devilsaur Queen - In Combat - Cast ''Dash'''),
(51525, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshal Expeditions Mercenary - In Combat - Cast ''Cleave'''),
(51525, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshal Expeditions Mercenary - In Combat - Cast ''Shoot''');
