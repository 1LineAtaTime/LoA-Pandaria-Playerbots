-- Deathknell: Deathguard abilities
-- Deathguard Elite and Deathguard Randolph had no AI and only melee'd. Port their abilities from the Cataclysm data. Both already
-- have Invisibility and Stealth Detection as a template aura, so its cast on reset is left out.

SET @ENTRIES := '1736,7980';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(1736, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 2000, 3000, 11, 95826, 64, 2, 'Deathguard Randolph - In Combat - Cast ''Shoot'''),
(1736, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 8, 2, 0, 1, 'Deathguard Randolph - On Reset - Set Reactstate Aggressive'),
(7980, 0, 0, 0, 0, 0, 100, 0, 5000, 7000, 7000, 10000, 11, 14516, 0, 2, 'Deathguard Elite - In Combat - Cast ''Strike'''),
(7980, 0, 1, 0, 9, 0, 100, 0, 0, 5, 12000, 16000, 11, 9080, 0, 2, 'Deathguard Elite - Within 0-5 Range - Cast ''Hamstring''');
