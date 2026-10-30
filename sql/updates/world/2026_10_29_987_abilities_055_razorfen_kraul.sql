-- #987 wave 1 (pure additions): Razorfen Kraul creature abilities, imported from DeltaTho PR 15 file 2026_10_24_60_world.sql.
-- 9 creatures, 9 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '4425,4435,4437,4511,4512,4514,4538,4541,6168';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4425, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 6000, 9000, 11, 12553, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Blind Hunter - In Combat - Cast Shock'),
(4435, 0, 0, 0, 0, 0, 100, 0, 2000, 12000, 30000, 70000, 11, 9128, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Razorfen Warrior - In Combat - Cast Battle Shout'),
(4437, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 7000, 15000, 11, 6533, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Razorfen Warden - In Combat - Cast Net'),
(4511, 0, 0, 0, 0, 0, 100, 0, 0, 0, 10000, 15000, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Agam''ar - In Combat - Cast Rushing Charge'),
(4512, 0, 0, 0, 0, 0, 100, 0, 3000, 10000, 30000, 35000, 11, 8267, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rotting Agam''ar - In Combat - Cast Cursed Blood'),
(4514, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Raging Agam''ar - Between Health 0-40% - Cast Frenzy'),
(4538, 0, 0, 0, 0, 0, 100, 0, 1000, 6000, 7000, 13000, 11, 12553, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Kraul Bat - In Combat - Cast Shock'),
(4541, 0, 0, 0, 0, 0, 100, 0, 2000, 7000, 20000, 35000, 11, 8282, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Blood of Agamaggan - In Combat - Cast Curse of Blood'),
(6168, 0, 0, 0, 0, 0, 100, 0, 0, 500, 3000, 3000, 11, 9532, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Roogug - In Combat - Cast Lightning Bolt');
