-- The Wandering Isle: creature abilities
-- The escaped Horde crew and their leaders (Delora Lionheart, Korga Strongmane, Makael Bay) had no AI; the zone is MoP-only, so there
-- is no Cataclysm or classic data, but their sniffed spell lists are in creature_template spell1-8. Cast those. The click-to-heal
-- Horde Druids and Alliance Priests and the Huojin Monk quest giver are left out.

SET @ENTRIES := '60554,60851,60852,60853,60854';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(60554, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 128510, 0, 0, 2, 'Escaped Horde Crewman - Within 0-5 Range - Cast ''Double Thrust'''),
(60851, 0, 0, 0, 0, 0, 100, 0, 5000, 7000, 14000, 17000, 11, 128440, 0, 0, 2, 'Delora Lionheart - In Combat - Cast ''Piercing Strikes'''),
(60851, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 128513, 0, 0, 2, 'Delora Lionheart - In Combat - Cast ''Strike'''),
(60852, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 128515, 64, 0, 2, 'Korga Strongmane - In Combat - Cast ''Lightning Bolt'''),
(60853, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 128526, 0, 0, 2, 'Makael Bay - Within 0-5 Range - Cast ''Saw Blade'''),
(60853, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 107115, 0, 0, 2, 'Makael Bay - In Combat - Cast ''Throw Dynamite'''),
(60854, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 128510, 0, 0, 2, 'Escaped Horde Crewman - Within 0-5 Range - Cast ''Double Thrust''');
