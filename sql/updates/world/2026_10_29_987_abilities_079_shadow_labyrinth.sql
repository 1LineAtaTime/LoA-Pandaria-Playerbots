-- #987 wave 1 (pure additions): Shadow Labyrinth creature abilities, imported from DeltaTho PR 15 file 2026_10_24_92_world.sql.
-- 1 creatures, 4 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '18636';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18636, 0, 0, 0, 10, 0, 100, 1, 0, 8, 4000, 8000, 11, 30986, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cabal Assassin - Out of Combat LoS - Cast ''Cheap Shot'''),
(18636, 0, 1, 0, 67, 0, 100, 0, 4500, 6500, 0, 0, 11, 30992, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cabal Assassin - On Behind Target - Cast ''Backstab'''),
(18636, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 11, 30981, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cabal Assassin - In Combat - Cast ''Crippling Poison'''),
(18636, 0, 3, 0, 0, 0, 100, 0, 8000, 11000, 22000, 25000, 11, 36974, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cabal Assassin - In Combat - Cast ''Wound Poison''');
