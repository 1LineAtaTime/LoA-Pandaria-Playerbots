-- #988 wave 2 (merge): Zangarmarsh creature abilities from DeltaTho PR 15 file 2026_10_24_72_world.sql, ADDED to creatures that already have smart_scripts.
-- 12 creatures, 14 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17877:26 18088:4 18130:2 18131:2 18285:2 19349:1 19706:1 19729:1 19730:2 20196:1 20279:2 20280:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17877 AND `id` >= 26)
  OR (`entryorguid` = 18088 AND `id` >= 4)
  OR (`entryorguid` = 18130 AND `id` >= 2)
  OR (`entryorguid` = 18131 AND `id` >= 2)
  OR (`entryorguid` = 18285 AND `id` >= 2)
  OR (`entryorguid` = 19349 AND `id` >= 1)
  OR (`entryorguid` = 19706 AND `id` >= 1)
  OR (`entryorguid` = 19729 AND `id` >= 1)
  OR (`entryorguid` = 19730 AND `id` >= 2)
  OR (`entryorguid` = 20196 AND `id` >= 1)
  OR (`entryorguid` = 20279 AND `id` >= 2)
  OR (`entryorguid` = 20280 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17877, 0, 26, 0, 0, 0, 100, 0, 11000, 12000, 15000, 25000, 0, 11, 31964, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fhwoor - In Combat - Cast Thundershock'),
(18088, 0, 4, 0, 0, 0, 100, 0, 6000, 8000, 16000, 22000, 0, 11, 33245, 35, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodscale Enchantress - In Combat - Cast ''Ice Barrier'''),
(18130, 0, 2, 0, 0, 0, 100, 0, 3000, 3000, 3000, 6000, 0, 11, 33860, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshfang Ripper - In Combat - Cast ''Arcane Explosion'''),
(18131, 0, 2, 0, 3, 0, 100, 0, 25, 100, 3600, 3600, 0, 11, 35334, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshfang Slicer - Between 25-100% Mana - Cast ''Nether Shock'''),
(18285, 0, 2, 0, 0, 0, 100, 0, 10000, 12000, 10000, 12000, 0, 11, 35335, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, '''Count'' Ungula - In Combat - Cast ''Nether Leech'''),
(19349, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 22696, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thornfang Ravager - On Respawn - Cast ''Thorns'''),
(19706, 0, 1, 0, 2, 0, 50, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Marshrock Threshalisk - Between 0-15% Health - Cast ''Feign Death'''),
(19729, 0, 1, 0, 2, 0, 60, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironspine Threshalisk - Between 0-15% Health - Cast ''Feign Death'''),
(19730, 0, 2, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironspine Gazer - Between 0-15% Health - Cast ''Feign Death'''),
(20196, 0, 1, 0, 0, 0, 100, 0, 10000, 12000, 10000, 12000, 0, 11, 35335, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodthirsty Marshfang - In Combat - Cast ''Nether Leech'''),
(20279, 0, 2, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ragestone Threshalisk - Between 0-15% Health - Cast ''Feign Death'''),
(20279, 0, 3, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 0, 11, 35385, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ragestone Threshalisk - In Combat - Cast ''Threshalisk Charge'' (Phase 1)'),
(20280, 0, 3, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ragestone Trampler - Between 0-15% Health - Cast ''Feign Death'''),
(20280, 0, 4, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 0, 11, 35385, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ragestone Trampler - In Combat - Cast ''Threshalisk Charge'' (Phase 1)');
