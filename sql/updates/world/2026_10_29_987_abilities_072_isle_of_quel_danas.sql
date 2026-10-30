-- #987 wave 1 (pure additions): Isle of Quel'Danas creature abilities, imported from DeltaTho PR 15 file 2026_10_24_78_world.sql.
-- 5 creatures, 15 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '24972,25027,25028,25030,26253';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(24972, 0, 0, 0, 8, 0, 100, 1, 44997, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - On Spellhit ''Converting Sentry'' - Despawn Instant'),
(24972, 0, 1, 0, 0, 0, 100, 0, 5000, 9000, 13000, 20000, 86, 35856, 1, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - In Combat - Cross Cast ''stunn'''),
(24972, 0, 2, 0, 0, 0, 100, 0, 5500, 6700, 11200, 16700, 11, 33688, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - In Combat - Cast ''Crystal Strike'''),
(24972, 0, 3, 0, 0, 0, 100, 0, 8000, 12000, 8000, 12000, 11, 35892, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - In Combat - Cast ''Suppression'''),
(24972, 0, 4, 5, 2, 0, 100, 1, 0, 50, 0, 0, 11, 45014, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - Between 0-50% Health - Cast ''Capacitor Overload'' (No Repeat)'),
(24972, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - Between 0-50% Health - Say Line 0 (No Repeat)'),
(24972, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 75, 44986, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - Between 0-50% Health - add aura Broken Capacitor'),
(24972, 0, 7, 8, 2, 0, 100, 0, 95, 99, 2000, 2000, 28, 44994, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - Between 95% - 100% health - Remove aura ''Self repair'''),
(24972, 0, 8, 0, 61, 0, 100, 0, 0, 0, 0, 0, 28, 44986, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Erratic Sentry - Between 95% - 100% health - Remove aura ''Broken Capacitor'''),
(25027, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 12787, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frenzied Ghoul - Between 5-15% Health - Cast ''Thrash'' (No Repeat)'),
(25028, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 12787, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skeletal Ravager - Between 5-15% Health - Cast ''Thrash'' (No Repeat)'),
(25030, 0, 0, 0, 0, 0, 100, 0, 2100, 3200, 8600, 9400, 11, 15284, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wrath Enforcer - In Combat - Cast ''Cleave'''),
(26253, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 18328, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Sun Peacekeeper - Within 0-8 Range - Cast ''Incapacitating Shout'''),
(26253, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 23337, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Sun Peacekeeper - In Combat - Cast ''Shoot'''),
(26253, 0, 2, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 25821, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Sun Peacekeeper - Within 8-25 Range - Cast ''Charge''');
