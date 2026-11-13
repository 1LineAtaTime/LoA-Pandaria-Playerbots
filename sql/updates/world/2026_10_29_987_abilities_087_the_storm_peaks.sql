-- #987 wave 1 (pure additions): The Storm Peaks creature abilities, imported from DeltaTho PR 15 file 2026_10_25_01_world.sql.
-- 9 creatures, 12 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '29351,29358,29475,29483,29753,29809,30164,30422,35189';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(29351, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 7000, 9000, 11, 38621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Niffelem Frost Giant - IC - Cast Debilitating Strike'),
(29358, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 7000, 9000, 11, 61572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostworg - IC - Cast Frostbite'),
(29358, 0, 1, 0, 0, 0, 100, 0, 2000, 6000, 6000, 9000, 11, 50075, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostworg - IC - Cast Maim Flesh'),
(29475, 0, 0, 0, 9, 0, 100, 0, 0, 10, 250, 250, 11, 54537, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Improved Land Mine - Victim In Range 0-10 Yards - Cast Spell Detonation'),
(29483, 0, 0, 0, 9, 0, 100, 0, 0, 20, 1000, 1000, 11, 54559, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'K3 Perimeter Turret - Victim In Range 0-20 Yards - Cast ''Energy Surge'''),
(29753, 0, 0, 0, 0, 0, 100, 0, 3000, 3000, 7000, 7000, 11, 57833, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'IC Update - cast spell on victim'),
(29809, 0, 0, 0, 1, 0, 100, 1, 0, 10000, 0, 0, 11, 55291, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Son of Hodir - Out of Combat - Cast Periodic Club Throw'),
(30164, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 24604, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cavedweller Worg - On Aggro - Cast ''Furious Howl'' (No Repeat)'),
(30422, 0, 0, 1, 2, 0, 100, 1, 0, 10, 0, 0, 11, 56732, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On HP Update - Cast Spell'),
(30422, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 56731, 0, 0, 0, 0, 0, 21, 40, 0, 0, 0, 0, 0, 0, 'On HP Update - Cast Spell'),
(35189, 0, 0, 0, 0, 0, 100, 0, 3000, 7000, 12000, 15000, 11, 61184, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skoll - In Combat - Cast ''Pounce'''),
(35189, 0, 1, 0, 0, 0, 100, 0, 5000, 9000, 8000, 13000, 11, 61186, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skoll - In Combat - Cast ''Frozen Bite''');
