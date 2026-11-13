-- #988 wave 2 (merge): Hellfire Peninsula creature abilities from DeltaTho PR 15 file 2026_10_24_71_world.sql, ADDED to creatures that already have smart_scripts.
-- 13 creatures, 15 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 16880:2 16915:1 16929:1 16967:1 16968:1 16974:1 16975:1 16977:15 16978:3 19350:2 19414:1 22259:1 22267:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 16880 AND `id` >= 2)
  OR (`entryorguid` = 16915 AND `id` >= 1)
  OR (`entryorguid` = 16929 AND `id` >= 1)
  OR (`entryorguid` = 16967 AND `id` >= 1)
  OR (`entryorguid` = 16968 AND `id` >= 1)
  OR (`entryorguid` = 16974 AND `id` >= 1)
  OR (`entryorguid` = 16975 AND `id` >= 1)
  OR (`entryorguid` = 16977 AND `id` >= 15)
  OR (`entryorguid` = 16978 AND `id` >= 3)
  OR (`entryorguid` = 19350 AND `id` >= 2)
  OR (`entryorguid` = 19414 AND `id` >= 1)
  OR (`entryorguid` = 22259 AND `id` >= 1)
  OR (`entryorguid` = 22267 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16880, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 33908, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hulking Helboar - On Reset - Cast ''Burning Spikes'''),
(16915, 0, 1, 0, 1, 0, 100, 0, 45000, 90000, 45000, 90000, 0, 11, 34396, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foreman Razelcraz - OOC - Cast Zap'),
(16929, 0, 1, 0, 2, 0, 100, 1, 0, 60, 0, 0, 0, 11, 33896, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonescythe Alpha - Between 0-60% Health - Cast ''Desperate Defense'' (No Repeat)'),
(16967, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 11, 32720, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Haal''eshi Talonguard - On Aggro - Cast ''Sprint'''),
(16968, 0, 1, 0, 0, 0, 100, 0, 1000, 6000, 8000, 11000, 0, 11, 32738, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunneler - In Combat - Cast ''Bore'''),
(16968, 0, 2, 0, 9, 0, 100, 0, 4, 50, 2000, 3500, 0, 11, 31747, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunneler - Within 4-50 Range - Cast ''Poison'''),
(16974, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 34234, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rogue Voidwalker - On Just Died - Cast ''Collapse'''),
(16975, 0, 1, 0, 2, 0, 100, 1, 0, 50, 0, 0, 0, 11, 33916, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Uncontrolled Voidwalker - Between 0-50% Health - Cast ''Void Drain'' (No Repeat)'),
(16977, 0, 15, 0, 1, 0, 100, 0, 317000, 317000, 300000, 300000, 0, 11, 29459, 0, 0, 0, 0, 0, 10, 86056, 0, 0, 0, 0, 0, 0, 0, 'Arch Mage Xintor - OOC - Cast ''Pyroblast'''),
(16978, 0, 3, 0, 0, 0, 50, 0, 4000, 7000, 5000, 8000, 0, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Commander Thalvos - In Combat - Cast Strike'),
(19350, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 22696, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thornfang Venomspitter - On Respawn - Cast ''Thorns'''),
(19414, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 13000, 16000, 0, 11, 33960, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Guard - In Combat - Cast ''Counterstrike'''),
(22259, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 38891, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Wardling - On Death - Cast Magic Sucker Device (Despawn Visual)'),
(22267, 0, 2, 0, 1, 0, 100, 0, 0, 0, 3000, 3000, 0, 11, 38719, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magic Sucker Device Buttress - IC - Cast Magic Sucker Device Buttress Appearance'),
(22267, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3000, 3000, 0, 11, 38919, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magic Sucker Device Buttress - OOC - Cast Magic Sucker Device Buttress Appearance');
