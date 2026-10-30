-- #988 wave 2 (merge): The Mechanar creature abilities from DeltaTho PR 15 file 2026_10_24_89_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 7 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 19167:3 19231:1 19712:3 19713:1 20059:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 19167 AND `id` >= 3)
  OR (`entryorguid` = 19231 AND `id` >= 1)
  OR (`entryorguid` = 19712 AND `id` >= 3)
  OR (`entryorguid` = 19713 AND `id` >= 1)
  OR (`entryorguid` = 20059 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(19167, 0, 3, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 35191, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Slayer - On Spawn - Cast ''Arcane Explosion Proc'''),
(19231, 0, 1, 0, 0, 0, 100, 0, 11100, 19300, 22900, 28900, 0, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Crusher - In Combat - Cast ''Glob of Machine Fluid'''),
(19712, 0, 3, 0, 0, 0, 100, 0, 13300, 37400, 21700, 33800, 0, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Driller - In Combat - Cast ''Glob of Machine Fluid'''),
(19713, 0, 1, 0, 0, 0, 100, 0, 15200, 27700, 21700, 36800, 0, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Wrecker - In Combat - Cast ''Glob of Machine Fluid'''),
(20059, 0, 3, 0, 0, 0, 100, 0, 12100, 19300, 10800, 25300, 0, 11, 35243, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Starfire'''),
(20059, 0, 4, 5, 0, 0, 100, 0, 14100, 18900, 63200, 68100, 0, 11, 35251, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Summon Arcane Golem'''),
(20059, 0, 5, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 35260, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Summon Arcane Golem''');
