-- #988 wave 2 (merge): The Botanica creature abilities from DeltaTho PR 15 file 2026_10_24_90_world.sql, ADDED to creatures that already have smart_scripts.
-- 6 creatures, 8 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17994:1 18405:2 18422:2 19507:4 19509:4 19511:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17994 AND `id` >= 1)
  OR (`entryorguid` = 18405 AND `id` >= 2)
  OR (`entryorguid` = 18422 AND `id` >= 2)
  OR (`entryorguid` = 19507 AND `id` >= 4)
  OR (`entryorguid` = 19509 AND `id` >= 4)
  OR (`entryorguid` = 19511 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17994, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 22000, 25000, 0, 11, 31567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Deterrence'''),
(17994, 0, 2, 0, 0, 0, 100, 0, 8000, 10000, 15000, 20000, 0, 11, 34879, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Multi-Shot'''),
(17994, 0, 3, 0, 0, 0, 100, 0, 8500, 10900, 14150, 21850, 0, 11, 32908, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Wing Clip'''),
(18405, 0, 2, 0, 0, 0, 100, 0, 7600, 16400, 15700, 25300, 0, 11, 34793, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tempest-Forge Peacekeeper - In Combat - Cast ''Arcane Blast'''),
(18422, 0, 2, 0, 0, 0, 100, 0, 8500, 20600, 15800, 20600, 0, 11, 34350, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Botanist - In Combat - Cast ''Nature`s Rage'''),
(19507, 0, 4, 0, 0, 0, 100, 0, 21850, 25500, 35000, 35000, 0, 11, 34247, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Gene-Splicer - In Combat - Cast ''Summon Lasher Beast'''),
(19509, 0, 4, 0, 0, 0, 100, 0, 6050, 19500, 7300, 23100, 0, 11, 34640, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Harvester - In Combat - Cast ''Wilting Touch'''),
(19511, 0, 2, 0, 0, 0, 100, 0, 13400, 20600, 20600, 29000, 0, 11, 30621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - In Combat - Cast ''Kidney Shot''');
