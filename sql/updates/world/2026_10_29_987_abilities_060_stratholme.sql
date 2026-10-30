-- #987 wave 1 (pure additions): Stratholme creature abilities, imported from DeltaTho PR 15 file 2026_10_24_66_world.sql.
-- 7 creatures, 13 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '10399,10441,10461,10536,10820,11082,11142';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(10399, 0, 0, 0, 1, 0, 100, 1, 0, 0, 5200, 5200, 11, 17224, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thuzadin Acolyte - Out of Combat - Cast spell (17224) on Self'),
(10399, 0, 1, 0, 1, 0, 100, 0, 5000, 5000, 5000, 5000, 11, 17237, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thuzadin Acolyte - Out of Combat Cast spell  (17237) on Self'),
(10399, 0, 2, 0, 1, 0, 100, 0, 5000, 5000, 5000, 5000, 11, 17239, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thuzadin Acolyte - Out of Combat Cast spell  (17239) on Self'),
(10399, 0, 3, 0, 1, 0, 100, 0, 5000, 5000, 5000, 5000, 11, 17240, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thuzadin Acolyte - Out of Combat Cast spell  (17240) on Self'),
(10441, 0, 0, 0, 0, 0, 100, 0, 0, 11000, 20000, 30000, 11, 16448, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plagued Rat - In Combat - Cast Black Rot'),
(10461, 0, 0, 0, 0, 0, 100, 0, 0, 9000, 10000, 30000, 11, 16460, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plagued Insect - In Combat - Cast Festering Bite'),
(10536, 0, 0, 0, 0, 0, 100, 0, 0, 11000, 20000, 30000, 11, 16449, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plagued Maggot - In Combat - Cast Maggot Slime'),
(10820, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 12471, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(10820, 0, 1, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 79934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Demon Armor on Spawn'),
(11082, 0, 0, 0, 13, 0, 100, 0, 8000, 10000, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stratholme Courier - Victim Casting - Cast Pummel'),
(11082, 0, 1, 0, 0, 0, 100, 0, 3000, 8000, 11000, 17000, 11, 15618, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stratholme Courier - In Combat - Cast Snap Kick'),
(11142, 0, 0, 0, 0, 0, 100, 0, 0, 6000, 20000, 30000, 11, 5137, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Undead Postman - In Combat - Cast Call of the Grave'),
(11142, 0, 1, 0, 0, 0, 100, 0, 2000, 9000, 11000, 19000, 11, 7713, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Undead Postman - In Combat - Cast Wailing Dead');
