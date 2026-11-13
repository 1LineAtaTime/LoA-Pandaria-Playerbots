-- #987 wave 1 (pure additions): Teldrassil and Shadowglen creature abilities, imported from DeltaTho PR 15 file 2026_10_24_08_world.sql.
-- 10 creatures, 17 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '1986,1994,1998,2001,2009,2151,7235,14429,34517,34530';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1986, 0, 0, 0, 0, 0, 100, 0, 3000, 7000, 3000, 7000, 11, 6751, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Webwood Spider - In Combat - Cast ''Weak Poison'''),
(1994, 0, 0, 0, 0, 0, 100, 0, 4000, 9000, 4000, 9000, 11, 11918, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Githyiss the Vile - In Combat - Cast ''Poison'''),
(1998, 0, 0, 0, 0, 0, 100, 0, 2300, 2300, 17600, 17600, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Webwood Lurker - In Combat - Cast ''Poison'''),
(2001, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 11, 6752, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Giant Webwood Spider - On Respawn - Cast ''Weak Poison Proc'''),
(2001, 0, 1, 0, 0, 0, 100, 0, 6000, 6000, 12800, 12800, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Giant Webwood Spider - In Combat - Cast ''Poison'''),
(2001, 0, 2, 0, 0, 0, 100, 0, 7500, 7500, 15000, 15000, 11, 745, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Giant Webwood Spider - In Combat - Cast ''Web'''),
(2009, 0, 0, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Shaman - Between 0-15% Health - Flee For Assist (No Repeat)'),
(2009, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 12000, 13000, 11, 15039, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Shaman - IC - Cast Flame Shock'),
(2009, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 13000, 16000, 11, 11986, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Shaman - IC - Cast Healing Wave'),
(2151, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 18561, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moon Priestess Amara - In Combat - Cast ''Shoot'''),
(7235, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Mystic - In Combat - Cast ''Wrath'''),
(7235, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Mystic - Between 0-15% Health - Flee For Assist (No Repeat)'),
(14429, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 4000, 5000, 11, 19319, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grimmaw - IC - Cast Vicious Bite'),
(34517, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 5000, 7000, 11, 65837, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ally of Nature - IC  - Cast Vine Lash'),
(34517, 0, 1, 0, 0, 0, 100, 0, 8000, 10000, 9000, 12000, 11, 65605, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ally of Nature - IC  - Cast Bursting Seed'),
(34517, 0, 2, 0, 0, 0, 100, 0, 4000, 5000, 8000, 12000, 11, 34745, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ally of Nature - IC  - Cast Shoot Thorns'),
(34530, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 5000, 7000, 11, 62245, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ally of Nature - IC  - Cast Vine Lash');
