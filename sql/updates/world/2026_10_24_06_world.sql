-- Elwynn Forest and Northshire: creature abilities
-- Gug Fatcandle, Tekton and the Elwynn Guard had no AI, Northshire Guards never shot, and Cutpurse and Bodyguard ran classic rows
-- copied from the Defias Cutpurse and Defias Bodyguard. Port their abilities from the Cataclysm data. The aggro texts are kept.

SET @ENTRIES := '62,94,1642,6866,48873,51519';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(62, 0, 0, 0, 1, 0, 100, 0, 0, 0, 10000, 10000, 11, 12544, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gug Fatcandle - OOC - Cast ''Frost Armor'''),
(62, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3600, 4000, 11, 20793, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gug Fatcandle - In Combat CMC - Cast ''Fireball'''),
(94, 0, 0, 0, 67, 0, 100, 0, 3900, 6900, 0, 0, 11, 37685, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cutpurse - On Behind Target - Cast ''Backstab'''),
(94, 0, 1, 0, 4, 0, 35, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cutpurse - On Aggro - Say Line 0 (No Repeat)'),
(1642, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 18950, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Northshire Guard - On Respawn - Cast ''Invisibility and Stealth Detection'''),
(1642, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northshire Guard - In Combat - Cast ''Shoot'''),
(6866, 0, 0, 0, 4, 0, 40, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Defias Bodyguard - On Aggro - Say Line 0 (No Repeat)'),
(6866, 0, 1, 0, 9, 0, 100, 0, 0, 8, 15800, 18300, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bodyguard - Within 0-8 Range - Cast ''Sprint'''),
(6866, 0, 2, 0, 0, 0, 100, 0, 5000, 7800, 16800, 17900, 11, 60842, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bodyguard - In Combat - Cast ''Expose Armor'''),
(48873, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 91121, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Tekton - On Aggro - Cast ''Tektonic Boom'' (No Repeat)'),
(51519, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Elwynn Guard - In Combat - Cast ''Shoot''');
