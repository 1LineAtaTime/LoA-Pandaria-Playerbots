-- #988 wave 2 (merge): Felwood creature abilities from DeltaTho PR 15 file 2026_10_24_49_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 9 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 8956:2 9516:2 48154:1 48259:3 48331:9

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 8956 AND `id` >= 2)
  OR (`entryorguid` = 9516 AND `id` >= 2)
  OR (`entryorguid` = 48154 AND `id` >= 1)
  OR (`entryorguid` = 48259 AND `id` >= 3)
  OR (`entryorguid` = 48331 AND `id` >= 9));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8956, 0, 2, 0, 9, 0, 100, 0, 0, 5, 22000, 25000, 0, 11, 84867, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Angerclaw Bear - Within 0-5 Range - Cast ''Sundering Swipe'''),
(9516, 0, 2, 0, 9, 0, 100, 0, 0, 5, 12000, 21000, 0, 11, 16247, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Banehollow - Within 0-5 Range - Cast ''Curse of Thorns'''),
(9516, 0, 3, 0, 9, 0, 100, 0, 0, 15, 8000, 13000, 0, 11, 17399, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Banehollow - Within 0-15 Range - Cast ''Shadow Shock'''),
(9516, 0, 4, 0, 0, 0, 100, 0, 4000, 5000, 19000, 27000, 0, 11, 37624, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Banehollow - In Combat - Cast ''Carrion Swarm'''),
(9516, 0, 5, 0, 0, 0, 100, 0, 2000, 11000, 12000, 22000, 0, 11, 92713, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Banehollow - In Combat - Cast ''Mind Blast'''),
(48154, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 86073, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Jadefire Shifter - On Aggro - Cast ''Flamethrower'' (No Repeat)'),
(48259, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32735, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Irontree Shredder - On Aggro - Cast ''Saw Blade'' (No Repeat)'),
(48331, 0, 9, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 36099, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Panicking Worker - On Aggro - Cast ''Throw Hammer'' (No Repeat)'),
(48331, 0, 10, 0, 2, 0, 100, 0, 0, 30, 15000, 17000, 0, 11, 30615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Panicking Worker - Between 0-30% Health - Cast ''Fear''');
