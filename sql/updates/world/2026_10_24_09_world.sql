-- Azuremyst Isle and Ammen Vale: creature abilities
-- Surveyor Candress cast Read Scroll (9487) where her script meant Fireball, Blood Elf Bandits used a warrior's Heroic Strike and
-- Stormwind Marines never shot. Port their abilities from the Cataclysm data; the Bandits keep their model fix.

SET @ENTRIES := '16522,17591,20556';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16522, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9053, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Surveyor Candress - In Combat - Cast ''Fireball'''),
(16522, 0, 1, 0, 4, 0, 61, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Surveyor Candress - On Aggro - Say Line 0 (No Repeat)'),
(16522, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 31515, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Surveyor Candress - On Reset - Cast ''Red Beam'''),
(17591, 0, 0, 0, 4, 0, 75, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - On Aggro - Say Line 0'),
(17591, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 3, 0, 17115, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - On Respawn - Sen ModelId 17115 (TEMP HACK!, 17116 can cause crashes!)'),
(17591, 0, 2, 0, 25, 0, 100, 1, 0, 0, 0, 0, 75, 5916, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - On Reset - Add Aura ''Shadowstalker Stealth'' (No Repeat)'),
(17591, 0, 3, 0, 9, 0, 100, 0, 0, 5, 5000, 8000, 11, 14873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - Within 0-5 Range - Cast ''Sinister Strike'''),
(17591, 0, 4, 0, 67, 0, 100, 0, 9000, 14000, 0, 0, 11, 7159, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - On Behind Target - Cast ''Backstab'''),
(17591, 0, 5, 0, 0, 0, 100, 0, 9000, 14000, 16000, 21000, 11, 15691, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blood Elf Bandit - In Combat - Cast ''Eviscerate'''),
(20556, 0, 0, 0, 9, 0, 100, 0, 0, 5, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stormwind Marine - Within 0-5 Range - Cast ''Revenge'''),
(20556, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 9000, 14000, 11, 12169, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stormwind Marine - In Combat - Cast ''Shield Block'''),
(20556, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2000, 4000, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stormwind Marine - In Combat - Cast ''Shoot''');
