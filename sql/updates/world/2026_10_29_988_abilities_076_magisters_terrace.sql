-- #988 wave 2 (merge): Magisters' Terrace creature abilities from DeltaTho PR 15 file 2026_10_24_94_world.sql, ADDED to creatures that already have smart_scripts.
-- 7 creatures, 7 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 24683:22 24684:6 24685:19 24686:6 24687:6 24688:2 24689:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 24683 AND `id` >= 22)
  OR (`entryorguid` = 24684 AND `id` >= 6)
  OR (`entryorguid` = 24685 AND `id` >= 19)
  OR (`entryorguid` = 24686 AND `id` >= 6)
  OR (`entryorguid` = 24687 AND `id` >= 6)
  OR (`entryorguid` = 24688 AND `id` >= 2)
  OR (`entryorguid` = 24689 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(24683, 0, 22, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44574, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunblade Mage Guard - Out of Combat - Cast Fel Energy Cosmetic'),
(24684, 0, 6, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44574, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunblade Blood Knight - Out of Combat - Cast Fel Energy Cosmetic'),
(24685, 0, 19, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44574, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunblade Magister - Out of Combat - Cast Fel Energy Cosmetic'),
(24686, 0, 6, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44574, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunblade Warlock - Out of Combat - Cast Fel Energy Cosmetic'),
(24687, 0, 6, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44574, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunblade Physician - Out of Combat - Cast Fel Energy Cosmetic'),
(24688, 0, 2, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44374, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wretched Skulker - Out of Combat - Cast Spell Fel Crystal Cosmetic'),
(24689, 0, 2, 0, 1, 0, 100, 1, 2000, 2000, 0, 0, 0, 11, 44374, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wretched Bruiser - Out of Combat - Cast Spell Fel Crystal Cosmetic');
