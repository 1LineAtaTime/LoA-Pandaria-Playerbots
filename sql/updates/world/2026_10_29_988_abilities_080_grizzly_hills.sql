-- #988 wave 2 (merge): Grizzly Hills creature abilities from DeltaTho PR 15 file 2026_10_24_98_world.sql, ADDED to creatures that already have smart_scripts.
-- 4 creatures, 7 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 26261:1 26417:2 27259:13 27959:10

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 26261 AND `id` >= 1)
  OR (`entryorguid` = 26417 AND `id` >= 2)
  OR (`entryorguid` = 27259 AND `id` >= 13)
  OR (`entryorguid` = 27959 AND `id` >= 10));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26261, 0, 1, 0, 1, 0, 100, 0, 2000, 5000, 4000, 6000, 0, 11, 46815, 0, 0, 0, 0, 0, 19, 26264, 85, 0, 0, 0, 0, 0, 0, 'Grizzly Hills Giant - Out of Combat - Cast ''Toss Boulder'''),
(26417, 0, 2, 0, 0, 0, 100, 1, 10000, 10000, 0, 0, 0, 11, 49717, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'IC - cast spell'),
(26417, 0, 3, 0, 0, 0, 100, 0, 5000, 6000, 15000, 16000, 0, 11, 55196, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'IC - cast spell'),
(27259, 0, 13, 0, 1, 0, 100, 0, 5000, 8000, 5000, 8000, 0, 11, 48213, 0, 0, 0, 0, 0, 19, 26785, 25, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - Out of Combat - Cast ''Dragonflayer Crone Fire'''),
(27959, 0, 10, 0, 2, 0, 100, 1, 0, 50, 0, 0, 0, 11, 50222, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - Between 0-50% Health - Cast ''The Cleansing: Your Inner Turmoil`s Whisper to Controller - On Health 50%'' (No Repeat)'),
(27959, 0, 11, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 50223, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Cast ''The Cleansing: Your Inner Turmoil`s Whisper to Controller - On Death'''),
(27959, 0, 12, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 50238, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Cast ''The Cleansing: Your Inner Turmoil`s On Death Cast on Master''');
