-- #989 (Refs #989): Deadmines creature abilities (SmartAI), the PR's 2026_10_24_26 minus the C++ double-ups
-- Source: DeltaTho playerbots-fork PR 15, 2026_10_24_26. Review: (not shipped) (Detail: Deadmines, Appendix C).
-- DROPPED vs the PR (our C++ already does it):
--   48284 Mining Powder: instance_deadmines.cpp:476 OnUnitDeath casts SPELL_EXPLODE_MINE 89769; the PR's death -> 90096 + 89769 + morph would explode it twice.
--   49674 nightmare Helix Gearbreaker: instance_deadmines.cpp:1110-1115 (#826) casts Spirit Strike 59304 every 3.6 s.
-- KEPT: 49670 nightmare Glubtok and 49681 nightmare Foe Reaper 5000 Spirit Strike (59304; no C++ cast, instance_deadmines.cpp only spawns them
--   and clears pacified), Mining Monkeys 48440-48442 Throw 91038, Sunwing Squawker 48450 / Ol' Beaky 48451 Eye Gouge 90913 + Eye Peck 90920
--   (the reference 5.4.8 world DB spell1-3 of the same entries).
-- Design check 2026-10-29: Wowhead tooltip API: spell 59304 Spirit Strike (melee, stacking physical-damage-taken debuff, 10 s), 91038 Throw
--   (35 yd, 1.5 s cast), 90913 Eye Gouge, 90920 Eye Peck (-50% hit chance 5 s); NPC names 49670 Glubtok, 49681 Foe Reaper 5000, 48440 Mining
--   Monkey, 48450 Sunwing Squawker, 48451 Ol' Beaky. Timers as in TrinityCore 4.3.4 for 49670/49681 (4500/3600, 2600/3600).
-- Idempotent: DELETE then INSERT per entry.

SET @ENTRIES := '49670,49681';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(49670, 0, 0, 0, 0, 0, 100, 0, 4500, 4500, 3600, 3600, 11, 59304, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4500 and 4500 ms (and later repeats every 3600 and 3600 ms) - Self: Cast spell Spirit Strike (59304) on Victim // '),
(49681, 0, 0, 0, 0, 0, 100, 0, 2600, 2600, 3600, 3600, 11, 59304, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 2600 and 2600 ms (and later repeats every 3600 and 3600 ms) - Self: Cast spell Spirit Strike (59304) on Victim // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '48440,48441,48442,48450,48451';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(48440, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48441, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48442, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 91038, 0, 0, 2, 'Mining Monkey - In Combat - Cast ''Throw'''),
(48450, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 90913, 0, 0, 2, 'Sunwing Squawker - In Combat - Cast ''Eye Gouge'''),
(48450, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 90920, 0, 0, 2, 'Sunwing Squawker - In Combat - Cast ''Eye Peck'''),
(48451, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 90913, 0, 0, 2, 'Ol'' Beaky - In Combat - Cast ''Eye Gouge'''),
(48451, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 90920, 0, 0, 2, 'Ol'' Beaky - In Combat - Cast ''Eye Peck''');
