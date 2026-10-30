-- #987 wave 1 (pure additions): Vashj'ir creature abilities, imported from DeltaTho PR 15 file 2026_10_25_15_world.sql.
-- 4 creatures, 8 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '40649,42424,50052,50575';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40649, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 35201, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Paralytic Poison'),
(42424, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 57790, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stygian Bounty Deckhand - In Combat - Cast ''Bloodthirst'''),
(42424, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 61490, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stygian Bounty Deckhand - Within 8-25 Range - Cast ''Intercept'''),
(50052, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79414, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burgy Blackheart - In Combat - Cast ''Brittle Touch'''),
(50052, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 75380, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burgy Blackheart - In Combat - Cast ''Parrrley!'''),
(50052, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 75361, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burgy Blackheart - In Combat - Cast ''Swashbuckling Slice'''),
(50575, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 57790, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Iron Reaper Deckhand - In Combat - Cast ''Bloodthirst'''),
(50575, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 61490, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Iron Reaper Deckhand - Within 8-25 Range - Cast ''Intercept''');
