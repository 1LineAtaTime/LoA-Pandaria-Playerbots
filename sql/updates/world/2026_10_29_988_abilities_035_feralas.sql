-- #988 wave 2 (merge): Feralas creature abilities from DeltaTho PR 15 file 2026_10_24_44_world.sql, ADDED to creatures that already have smart_scripts.
-- 6 creatures, 9 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 5268:1 5278:3 5286:1 5359:1 11443:16 39896:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 5268 AND `id` >= 1)
  OR (`entryorguid` = 5278 AND `id` >= 3)
  OR (`entryorguid` = 5286 AND `id` >= 1)
  OR (`entryorguid` = 5359 AND `id` >= 1)
  OR (`entryorguid` = 11443 AND `id` >= 16)
  OR (`entryorguid` = 39896 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5268, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironfur Bear - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(5268, 0, 2, 0, 2, 0, 100, 0, 0, 40, 18000, 21000, 0, 11, 81122, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironfur Bear - Between 0-40% Health - Cast ''Ironfur'''),
(5278, 0, 3, 0, 0, 0, 100, 0, 2000, 3000, 11000, 12000, 0, 11, 50273, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sprite Darter - In Combat - Cast ''Arcane Barrage'''),
(5286, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 17000, 20000, 0, 11, 3149, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Longtooth Runner - In Combat - Cast ''Furious Howl'''),
(5359, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 12000, 14000, 0, 11, 79810, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shore Strider - In Combat - Cast ''Frost Cleave'''),
(5359, 0, 2, 0, 0, 0, 100, 0, 7000, 9000, 18000, 23000, 0, 11, 10987, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shore Strider - In Combat - Cast ''Geyser'''),
(11443, 0, 16, 0, 0, 0, 100, 0, 1000, 3000, 9000, 9000, 0, 11, 20822, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gordok Ogre-Mage - IC - Cast ''Frostbolt'''),
(39896, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Feral Scar Yeti - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(39896, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 17000, 0, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Feral Scar Yeti - In Combat - Cast ''Swipe''');
