-- Shadowfang Keep: creature abilities
-- None of the Cataclysm trash of Shadowfang Keep (Frantic Geists, Dark Creepers, Corpse Eaters, Fetid Ghouls, the Mindless Horrors
-- and Unstable Ravagers, Spitebone skeletons, Shadowy Attendants, Ghostly Cooks) had AI, and the Deathsworn Captain and Haunted
-- Servitors lacked part of theirs. Port them from the Cataclysm data, keeping its normal/heroic flags.

SET @ENTRIES := '3872,3875,47131,47132,47134,47135,47136,47137,47143,47145,47146,47231,47232';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3872, 0, 0, 0, 1, 0, 100, 3, 0, 0, 0, 0, 11, 7165, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Deathsworn Captain - Out of Combat - Cast ''Battle Stance'''),
(3872, 0, 1, 0, 0, 0, 100, 2, 4100, 5800, 8200, 18200, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deathsworn Captain - In Combat - Cast ''Cleave'''),
(3872, 0, 2, 0, 0, 0, 100, 2, 6700, 7300, 14600, 33300, 11, 9080, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deathsworn Captain - In Combat - Cast ''Hamstring'''),
(3875, 0, 0, 0, 0, 0, 100, 0, 15000, 16000, 15000, 16000, 11, 7057, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 15000 and 16000 ms (and later repeats every 15000 and 16000 ms) - Self: Cast spell Haunting Spirits (7057) on Random hostile // '),
(3875, 0, 1, 0, 0, 0, 100, 4, 3000, 3000, 3000, 3000, 11, 91291, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 3000 and 3000 ms (and later repeats every 3000 and 3000 ms) - Self: Cast spell Drain Life (91291) on Random hostile // '),
(47131, 0, 0, 0, 0, 0, 100, 1, 100, 100, 0, 0, 11, 91077, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 100 and 100 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 91077 on Victim // '),
(47131, 0, 1, 0, 0, 0, 100, 0, 6000, 7000, 3000, 3000, 11, 91064, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 7000 ms (and later repeats every 3000 and 3000 ms) - Self: Cast spell 91064 on Victim // '),
(47132, 0, 0, 0, 0, 0, 100, 0, 6000, 7000, 5000, 5000, 11, 91522, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 7000 ms (and later repeats every 5000 and 5000 ms) - Self: Cast spell 91522 on Victim // '),
(47134, 0, 0, 1, 0, 0, 100, 0, 7000, 7000, 6000, 7000, 11, 91088, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 7000 and 7000 ms (and later repeats every 6000 and 7000 ms) - Self: Cast spell Consume Flesh (91088) on Victim // '),
(47134, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 67, 0, 1000, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' Linked - Trigger timed event #0 in 1000 - 1000 ms // '),
(47134, 0, 2, 0, 59, 0, 100, 4, 0, 0, 0, 0, 11, 91099, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'On timed event 0 triggered - Self: Cast spell Rotting Bile (91099) on Random hostile (not top) // '),
(47135, 0, 0, 0, 6, 0, 100, 4, 0, 0, 0, 0, 11, 91554, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Fetid Cloud (91554) on Self // '),
(47136, 0, 0, 1, 0, 0, 100, 0, 8000, 9000, 12000, 13000, 11, 3242, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 9000 ms (and later repeats every 12000 and 13000 ms) - Self: Cast spell 3242 on Victim // '),
(47136, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Trigger timed event 0 // '),
(47136, 0, 2, 0, 59, 0, 100, 4, 0, 0, 0, 0, 11, 91668, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On timed event 0 triggered - Self: Cast spell 91668 on Self // '),
(47137, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 8000, 9000, 11, 91213, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 6000 ms (and later repeats every 8000 and 9000 ms) - Self: Cast spell Smash (91213) on Self // '),
(47137, 0, 1, 0, 0, 0, 100, 4, 10000, 11000, 10000, 11000, 11, 91220, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 11000 ms (and later repeats every 10000 and 11000 ms) - Self: Cast spell Cowering Roar (91220) on Self // '),
(47143, 0, 0, 0, 0, 0, 100, 0, 6000, 7000, 10000, 11000, 11, 91419, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 7000 ms (and later repeats every 10000 and 11000 ms) - Self: Cast spell Bone Flurry (91419) on Random hostile // '),
(47145, 0, 0, 0, 0, 0, 100, 0, 3000, 3000, 6000, 6000, 11, 87081, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 3000 and 3000 ms (and later repeats every 6000 and 6000 ms) - Self: Cast spell 87081 on Victim // '),
(47145, 0, 1, 0, 0, 0, 100, 0, 8000, 8000, 9500, 9500, 11, 11972, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 8000 ms (and later repeats every 9500 and 9500 ms) - Self: Cast spell 11972 on Victim // '),
(47145, 0, 2, 0, 0, 0, 100, 4, 10000, 10000, 21000, 21000, 11, 91629, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 10000 ms (and later repeats every 21000 and 21000 ms) - Self: Cast spell 91629 on Self // '),
(47146, 0, 0, 0, 0, 0, 100, 0, 6000, 6000, 11000, 11000, 11, 32731, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 6000 ms (and later repeats every 11000 and 11000 ms) - Self: Cast spell 32731 on Self // '),
(47231, 0, 0, 0, 1, 0, 100, 0, 29000, 30000, 29000, 30000, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 29000 and 30000 ms (and later repeats every 29000 and 30000 ms) - None: Play emote ONESHOT_TALK(DNR) (1) // '),
(47231, 0, 1, 0, 0, 0, 100, 2, 1, 1, 3000, 3000, 11, 12739, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1 and 1 ms (and later repeats every 3000 and 3000 ms) - Self: Cast spell Shadow Bolt (12739) on Victim // '),
(47231, 0, 2, 0, 0, 0, 100, 4, 11000, 14000, 9000, 10000, 11, 91324, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 11000 and 14000 ms (and later repeats every 9000 and 10000 ms) - Self: Cast spell Shadowy Touch (91324) on Random hostile // '),
(47231, 0, 3, 0, 0, 0, 100, 4, 8000, 10000, 14000, 16000, 11, 91325, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 10000 ms (and later repeats every 14000 and 16000 ms) - Self: Cast spell Shadow Vortex (91325) on Random hostile // '),
(47232, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 1000, 3000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 7000 and 8000 ms (and later repeats every 1000 and 3000 ms) - Invoker: Cast spell Cleave (15496) to Victim // '),
(47232, 0, 1, 0, 0, 0, 100, 4, 6000, 7000, 6000, 7000, 11, 91348, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 7000 ms (and later repeats every 6000 and 7000 ms) - Invoker: Cast spell Tenderize (91348) to Victim // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '47138';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(47138, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 91677, 64, 0, 2, 'Pustulant Monstrosity - In Combat - Cast ''Pustulant Spit''');
