-- #987 wave 1 (pure additions): The Shattered Halls creature abilities, imported from DeltaTho PR 15 file 2026_10_24_93_world.sql.
-- 2 creatures, 4 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '16700,17357';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16700, 0, 0, 0, 0, 0, 100, 0, 1500, 5000, 240000, 240000, 11, 30472, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - In Combat - Cast ''Aura of Discipline'''),
(16700, 0, 1, 0, 13, 0, 100, 0, 10000, 15000, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - On Victim Casting - Cast ''Pummel'''),
(16700, 0, 2, 0, 38, 0, 100, 0, 1, 1, 60000, 60000, 11, 30485, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - On Data Set 1 1 (Friendly Dies Nearby) - Cast ''Enrage'''),
(17357, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 3800, 6500, 11, 30494, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Creeping Oozeling - In Combat - Cast ''Sticky Ooze''');
