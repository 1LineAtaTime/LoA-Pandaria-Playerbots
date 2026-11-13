-- #988 wave 2 (merge): Howling Fjord creature abilities from DeltaTho PR 15 file 2026_10_24_96_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 7 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 24014:1 24018:1 24271:5 24677:1 24797:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 24014 AND `id` >= 1)
  OR (`entryorguid` = 24018 AND `id` >= 1)
  OR (`entryorguid` = 24271 AND `id` >= 5)
  OR (`entryorguid` = 24677 AND `id` >= 1)
  OR (`entryorguid` = 24797 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(24014, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Necrolord - In Combat - Cast ''Shadow Bolt'''),
(24014, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 25000, 35000, 0, 11, 18267, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Necrolord - In Combat - Cast ''Curse of Weakness'''),
(24014, 0, 3, 0, 0, 0, 100, 0, 8000, 12000, 8000, 12000, 0, 11, 17173, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Necrolord - In Combat - Cast ''Drain Life'''),
(24018, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 3000, 5000, 0, 11, 32862, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Necro Overlord Mezhen - IC - cast Drain soul'),
(24271, 0, 5, 0, 0, 0, 100, 0, 5000, 5000, 7000, 10000, 0, 11, 49729, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'IRONGolem - IC - Cast'),
(24677, 0, 1, 0, 0, 0, 100, 0, 2000, 6000, 6000, 9000, 0, 11, 50075, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Spearfang Worg - In Combat - Cast ''Maim Flesh'''),
(24797, 0, 3, 0, 0, 0, 100, 0, 4000, 6000, 6000, 16000, 0, 11, 50169, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Reef Cow - In Combat - Cast ''Flipper Thwack''');
