-- #987 wave 1 (pure additions): Ruins of Gilneas creature abilities, imported from DeltaTho PR 15 file 2026_10_24_21_world.sql.
-- 11 creatures, 19 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '44111,45335,45367,45403,45481,45483,45499,45536,45537,45538,57802';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(44111, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Worgen Invader - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(45335, 0, 0, 0, 2, 0, 100, 0, 0, 40, 30000, 35000, 11, 79853, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Highlands Fox - Between 0-40% Health - Cast ''Evasion'''),
(45367, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 68687, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilnean Hound - Within 8-25 Range - Cast ''Leap'''),
(45403, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 81087, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, '7th Legion Submariner - In Combat - Cast ''Puncture Wound'''),
(45403, 0, 1, 0, 9, 0, 100, 0, 8, 30, 6000, 9000, 11, 86249, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, '7th Legion Submariner - Within 8-30 Range - Cast ''Throw'''),
(45481, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 60195, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodfang Scout - In Combat - Cast ''Sinister Strike'''),
(45483, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 79857, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Worgen Battlemage - Within 0-8 Range - Cast ''Blast Wave'''),
(45483, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 79854, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Worgen Battlemage - In Combat - Cast ''Fireball'''),
(45483, 0, 2, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 79849, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Worgen Battlemage - Out of Combat - Cast ''Molten Armor'''),
(45499, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 86345, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilnean Warhound - Within 8-25 Range - Cast ''Leap'''),
(45536, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 82669, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodfang Scout - In Combat - Cast ''Shoot'''),
(45536, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 60195, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodfang Scout - In Combat - Cast ''Sinister Strike'''),
(45537, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 79854, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Worgen Battlemage - In Combat - Cast ''Fireball'''),
(45537, 0, 1, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 79849, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Worgen Battlemage - Out of Combat - Cast ''Molten Armor'''),
(45538, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 86345, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilnean Warhound - Within 8-25 Range - Cast ''Leap'''),
(57802, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 110055, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lord Hiram Creed - Between 0-30% Health - Cast ''Berserk'' (No Repeat)'),
(57802, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 109706, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lord Hiram Creed - In Combat - Cast ''Blackhowl''s Rage'''),
(57802, 0, 2, 0, 0, 0, 100, 0, 8000, 12000, 20000, 25000, 11, 109664, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lord Hiram Creed - In Combat - Cast ''Blackhowl''s Will'''),
(57802, 0, 3, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 109196, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lord Hiram Creed - In Combat - Cast ''Consuming Darkness''');
