-- #987 wave 1 (pure additions): Moonglade creature abilities, imported from DeltaTho PR 15 file 2026_10_24_59_world.sql.
-- 3 creatures, 7 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '11822,38952,53563';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11822, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moonglade Warden - In Combat - Cast ''Shoot'''),
(11822, 0, 1, 0, 9, 0, 100, 0, 0, 5, 13600, 14500, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moonglade Warden - Within 0-5 Range - Cast ''Cleave'''),
(38952, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nordrassil Druid - In Combat - Cast ''Wrath'''),
(38952, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 18000, 22000, 11, 22127, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Nordrassil Druid - In Combat - Cast ''Entangling Roots'''),
(38952, 0, 2, 0, 0, 0, 100, 0, 11000, 13000, 15000, 35000, 11, 23380, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nordrassil Druid - In Combat - Cast ''Moonfire'''),
(53563, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moonglade Warden - In Combat - Cast ''Cleave'''),
(53563, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moonglade Warden - In Combat - Cast ''Shoot''');
