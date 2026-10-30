-- #988 wave 2 (merge): Blackrock Depths creature abilities from DeltaTho PR 15 file 2026_10_24_68_world.sql, ADDED to creatures that already have smart_scripts.
-- 4 creatures, 4 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 8921:2 8933:2 9024:3 9029:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 8921 AND `id` >= 2)
  OR (`entryorguid` = 8933 AND `id` >= 2)
  OR (`entryorguid` = 9024 AND `id` >= 3)
  OR (`entryorguid` = 9029 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8921, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 8279, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodhound - On Reset - Cast Stealth Detection'),
(8933, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 4000, 8000, 0, 11, 15656, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Cave Creeper - In Combat - Cast Poisonous Stab'),
(9024, 0, 3, 0, 0, 0, 100, 0, 2000, 7000, 30000, 45000, 0, 11, 15041, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Pyromancer Loregrain - In Combat - Cast Fire Ward'),
(9029, 0, 3, 0, 0, 0, 100, 0, 5000, 6000, 15000, 16000, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Eviscerator - In Combat - Cast Anti-Magic Shield');
