-- #988 wave 2 (merge): Western Plaguelands creature abilities from DeltaTho PR 15 file 2026_10_24_43_world.sql, ADDED to creatures that already have smart_scripts.
-- 6 creatures, 6 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 1783:1 1784:2 1815:6 1831:2 1852:4 44482:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 1783 AND `id` >= 1)
  OR (`entryorguid` = 1784 AND `id` >= 2)
  OR (`entryorguid` = 1815 AND `id` >= 6)
  OR (`entryorguid` = 1831 AND `id` >= 2)
  OR (`entryorguid` = 1852 AND `id` >= 4)
  OR (`entryorguid` = 44482 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1783, 0, 1, 0, 2, 0, 100, 0, 0, 60, 22000, 25000, 0, 11, 85521, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Vile Aegis at 60% HP'),
(1784, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 85434, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Tombkeeper''s Curse'),
(1815, 0, 6, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 85842, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Survival Instincts at 30% HP'),
(1831, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 17164, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Hunter - On Reset - Cast to summon Scarlet Hound'),
(1852, 0, 4, 0, 0, 0, 100, 0, 18000, 23000, 50000, 65000, 0, 11, 83701, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Summon Berserkers'),
(44482, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 85842, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Survival Instincts at 30% HP');
