-- Ruins of Gilneas: creature abilities
-- None of the creatures of the Battle for Gilneas (Forsaken Vanguards and Sharpshooters, Worgen Rebels, Rushers and Battlemages,
-- Bloodfang Scouts, Gilnean Warhounds, the 7th Legion Submariners, Lord Walden, Lord Hiram Creed) had AI, and neither the Cataclysm
-- nor the classic data scripts them. Cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '44111,45280,45292,45335,45367,45375,45376,45403,45405,45481,45483,45499,45535,45536,45537,45538,46029,57802';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(44111, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 1, 'Worgen Invader - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(45280, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 9080, 0, 0, 2, 'Forsaken Vanguard - In Combat - Cast ''Hamstring'''),
(45280, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 32736, 0, 0, 2, 'Forsaken Vanguard - In Combat - Cast ''Mortal Strike'''),
(45292, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79878, 0, 0, 2, 'Worgen Rebel - In Combat - Cast ''Bloodthirst'''),
(45292, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 12000, 15000, 11, 79881, 0, 0, 2, 'Worgen Rebel - In Combat - Cast ''Slam'''),
(45335, 0, 0, 0, 2, 0, 100, 0, 0, 40, 30000, 35000, 11, 79853, 0, 0, 1, 'Highlands Fox - Between 0-40% Health - Cast ''Evasion'''),
(45367, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 68687, 0, 0, 2, 'Gilnean Hound - Within 8-25 Range - Cast ''Leap'''),
(45375, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 75002, 0, 0, 2, 'Worgen Rusher - Within 8-25 Range - Cast ''Leaping Rush'''),
(45376, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 10000, 14000, 11, 80012, 0, 0, 2, 'Forsaken Sharpshooter - In Combat - Cast ''Chimera Shot'''),
(45376, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 79721, 0, 0, 2, 'Forsaken Sharpshooter - In Combat - Cast ''Explosive Shot'''),
(45376, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Forsaken Sharpshooter - In Combat - Cast ''Shoot'''),
(45403, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 81087, 0, 0, 2, '7th Legion Submariner - In Combat - Cast ''Puncture Wound'''),
(45403, 0, 1, 0, 9, 0, 100, 0, 8, 30, 6000, 9000, 11, 86249, 0, 0, 2, '7th Legion Submariner - Within 8-30 Range - Cast ''Throw'''),
(45405, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 79970, 0, 0, 1, 'Master Sergeant Pietro Zaren - Within 0-8 Range - Cast ''Divine Storm'''),
(45405, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 12000, 15000, 11, 79971, 0, 0, 2, 'Master Sergeant Pietro Zaren - In Combat - Cast ''Hammer of Wrath'''),
(45405, 0, 2, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 79976, 32, 0, 1, 'Master Sergeant Pietro Zaren - Out of Combat - Cast ''Retribution Aura'''),
(45481, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 60195, 0, 0, 2, 'Bloodfang Scout - In Combat - Cast ''Sinister Strike'''),
(45483, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 79857, 0, 0, 1, 'Worgen Battlemage - Within 0-8 Range - Cast ''Blast Wave'''),
(45483, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 79854, 64, 0, 2, 'Worgen Battlemage - In Combat - Cast ''Fireball'''),
(45483, 0, 2, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 79849, 32, 0, 1, 'Worgen Battlemage - Out of Combat - Cast ''Molten Armor'''),
(45499, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 86345, 0, 0, 2, 'Gilnean Warhound - Within 8-25 Range - Cast ''Leap'''),
(45535, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 75002, 0, 0, 2, 'Worgen Rusher - Within 8-25 Range - Cast ''Leaping Rush'''),
(45536, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 82669, 0, 0, 2, 'Bloodfang Scout - In Combat - Cast ''Shoot'''),
(45536, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 60195, 0, 0, 2, 'Bloodfang Scout - In Combat - Cast ''Sinister Strike'''),
(45537, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 79854, 64, 0, 2, 'Worgen Battlemage - In Combat - Cast ''Fireball'''),
(45537, 0, 1, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 79849, 32, 0, 1, 'Worgen Battlemage - Out of Combat - Cast ''Molten Armor'''),
(45538, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 86345, 0, 0, 2, 'Gilnean Warhound - Within 8-25 Range - Cast ''Leap'''),
(46029, 0, 0, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 85965, 0, 0, 1, 'Lord Walden - Between 0-40% Health - Cast ''Holy Nova'''),
(46029, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9734, 64, 0, 2, 'Lord Walden - In Combat - Cast ''Holy Smite'''),
(46029, 0, 2, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 48168, 32, 0, 1, 'Lord Walden - Out of Combat - Cast ''Inner Fire'''),
(57802, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 110055, 0, 0, 1, 'Lord Hiram Creed - Between 0-30% Health - Cast ''Berserk'' (No Repeat)'),
(57802, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 109706, 32, 0, 1, 'Lord Hiram Creed - In Combat - Cast ''Blackhowl''s Rage'''),
(57802, 0, 2, 0, 0, 0, 100, 0, 8000, 12000, 20000, 25000, 11, 109664, 0, 0, 2, 'Lord Hiram Creed - In Combat - Cast ''Blackhowl''s Will'''),
(57802, 0, 3, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 109196, 0, 0, 2, 'Lord Hiram Creed - In Combat - Cast ''Consuming Darkness''');
