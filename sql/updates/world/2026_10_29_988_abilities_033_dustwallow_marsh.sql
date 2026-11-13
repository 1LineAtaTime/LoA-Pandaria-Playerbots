-- #988 wave 2 (merge): Dustwallow Marsh creature abilities from DeltaTho PR 15 file 2026_10_24_42_world.sql, ADDED to creatures that already have smart_scripts.
-- 8 creatures, 11 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 4344:1 4345:1 4359:1 4393:2 4394:2 4967:2 4979:10 23595:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 4344 AND `id` >= 1)
  OR (`entryorguid` = 4345 AND `id` >= 1)
  OR (`entryorguid` = 4359 AND `id` >= 1)
  OR (`entryorguid` = 4393 AND `id` >= 2)
  OR (`entryorguid` = 4394 AND `id` >= 2)
  OR (`entryorguid` = 4967 AND `id` >= 2)
  OR (`entryorguid` = 4979 AND `id` >= 10)
  OR (`entryorguid` = 23595 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4344, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 0, 11, 90171, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mottled Drywallow Crocolisk - In Combat - Cast ''Jagged Tooth Snap'''),
(4345, 0, 1, 0, 0, 0, 100, 0, 3000, 3000, 45000, 50000, 0, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Drywallow Daggermaw - In Combat - Cast ''Pierce Armor'''),
(4359, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 10277, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mirefin Murloc - In Combat - Cast ''Throw'''),
(4359, 0, 2, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 8656, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mirefin Murloc - Out of Combat - Cast ''Summon Crawler'' (No Repeat)'),
(4393, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 17000, 21000, 0, 11, 73135, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Acidic Swamp Ooze - In Combat - Cast ''Old Friends: Quest Accept & Bind'''),
(4394, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 17000, 21000, 0, 11, 73135, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bubbling Swamp Ooze - In Combat - Cast ''Old Friends: Quest Accept & Bind'''),
(4967, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 1800000, 1800000, 0, 11, 7120, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Archmage Tervosh - Out of Combat - Cast ''Proudmoore''s Defense'''),
(4979, 0, 10, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Theramore Guard - In Combat - Cast ''Shoot'''),
(4979, 0, 11, 0, 2, 0, 100, 0, 0, 50, 12000, 13000, 0, 11, 12169, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Theramore Guard - Between 0-50% Health - Cast ''Shield Block'''),
(4979, 0, 12, 0, 0, 0, 100, 0, 3000, 4000, 12000, 15000, 0, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Theramore Guard - In Combat - Cast ''Revenge'''),
(23595, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 12000, 13000, 0, 11, 32910, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Grimtotem Earthbinder - In Combat - Cast ''Windfury''');
