-- The Deadmines: creature abilities
-- The heroic nightmare spirits (Glubtok, Foe Reaper 5000) never struck and the Mining Powder kegs never exploded. Port them from the
-- Cataclysm data, keeping its normal/heroic flags. The mobs no reference scripts (Mining Monkeys, Sunwing Squawker, Ol' Beaky, the
-- spirit of Helix Gearbreaker) cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '48284,49670,49681';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(48284, 0, 0, 1, 6, 0, 100, 0, 0, 0, 0, 0, 11, 90096, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell 90096 on Self // '),
(48284, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 11, 89769, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Cast spell 89769 on Self // '),
(48284, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 3, 0, 11686, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Morph to model 11686 // '),
(49670, 0, 0, 0, 0, 0, 100, 0, 4500, 4500, 3600, 3600, 11, 59304, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4500 and 4500 ms (and later repeats every 3600 and 3600 ms) - Self: Cast spell Spirit Strike (59304) on Victim // '),
(49681, 0, 0, 0, 0, 0, 100, 0, 2600, 2600, 3600, 3600, 11, 59304, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 2600 and 2600 ms (and later repeats every 3600 and 3600 ms) - Self: Cast spell Spirit Strike (59304) on Victim // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '48440,48441,48442,48450,48451,49674';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(48440, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48441, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48442, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48450, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 90913, 0, 0, 2, 'Sunwing Squawker - In Combat - Cast ''Eye Gouge'''),
(48450, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 90920, 0, 0, 2, 'Sunwing Squawker - In Combat - Cast ''Eye Peck'''),
(48451, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 90913, 0, 0, 2, 'Ol'' Beaky - In Combat - Cast ''Eye Gouge'''),
(48451, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 90920, 0, 0, 2, 'Ol'' Beaky - In Combat - Cast ''Eye Peck'''),
(49674, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 59304, 0, 0, 2, 'Helix Gearbreaker - In Combat - Cast ''Spirit Strike''');
