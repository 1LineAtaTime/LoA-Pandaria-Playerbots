-- #987 wave 1 (pure additions): Wintergrasp creature abilities, imported from DeltaTho PR 15 file 2026_10_25_03_world.sql.
-- 6 creatures, 9 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '30739,30740,30842,30849,30872,30875';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(30739, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 42648, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Warsong Champion - Out of Combat - Cast ''Sleeping Sleep'''),
(30739, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Warsong Champion - In Combat - Cast ''Strike'''),
(30740, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 42648, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Valiance Expedition Champion - Out of Combat - Cast ''Sleeping Sleep'''),
(30740, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Valiance Expedition Champion - In Combat - Cast ''Strike'''),
(30842, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 12000, 24000, 11, 38240, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wandering Shadow - In Combat - Cast ''Chilling Touch'''),
(30842, 0, 1, 0, 0, 0, 50, 0, 25000, 30000, 25000, 30000, 11, 18267, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wandering Shadow - In Combat - Cast ''Curse of Weakness'''),
(30849, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 43543, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Chilled Earth Elemental - In Combat - Cast ''Throw Ice'''),
(30872, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 12000, 20000, 11, 51131, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadow Revenant - In Combat - Cast ''Strangulate'''),
(30875, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 56620, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Tempest Revenant - In Combat - Cast ''Seething Flames''');
