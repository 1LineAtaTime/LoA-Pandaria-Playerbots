-- #988 wave 2 (merge): Teldrassil and Shadowglen creature abilities from DeltaTho PR 15 file 2026_10_24_08_world.sql, ADDED to creatures that already have smart_scripts.
-- 9 creatures, 9 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2002:4 2003:4 2004:4 2005:4 2008:1 2011:1 2015:1 2030:2 7234:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2002 AND `id` >= 4)
  OR (`entryorguid` = 2003 AND `id` >= 4)
  OR (`entryorguid` = 2004 AND `id` >= 4)
  OR (`entryorguid` = 2005 AND `id` >= 4)
  OR (`entryorguid` = 2008 AND `id` >= 1)
  OR (`entryorguid` = 2011 AND `id` >= 1)
  OR (`entryorguid` = 2015 AND `id` >= 1)
  OR (`entryorguid` = 2030 AND `id` >= 2)
  OR (`entryorguid` = 7234 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2002, 0, 4, 0, 0, 0, 100, 0, 2000, 4000, 15000, 16000, 0, 11, 6950, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rascal Sprite - IC - Cast Faerie Fire'),
(2003, 0, 4, 0, 0, 0, 100, 0, 2000, 4000, 3500, 4000, 0, 11, 9613, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadow Sprite - IC - Cast Shadow Bolt'),
(2004, 0, 4, 0, 0, 0, 100, 0, 2000, 4000, 12000, 13000, 0, 11, 5514, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Sprite - IC - Cast Darken Vision'),
(2005, 0, 4, 0, 0, 0, 80, 0, 1000, 1000, 10000, 10000, 0, 11, 5515, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Vicious Grell - In Combat - Cast ''Savagery'' (No Repeat)'),
(2008, 0, 1, 0, 9, 0, 100, 0, 0, 5, 7000, 12000, 0, 11, 41056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Warrior - Within 0-5 Range - Cast ''Strike'''),
(2011, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 11000, 0, 11, 17227, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gnarlpine Augur - IC - Cast Curse of Weakness'),
(2015, 0, 1, 0, 9, 0, 100, 0, 0, 5, 6000, 9000, 0, 11, 6958, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 'Bloodfeather Harpy - Within 0-5 Range - Cast ''Blood Leech'''),
(2030, 0, 2, 0, 2, 0, 80, 0, 0, 30, 15000, 20000, 0, 11, 84036, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Elder Timberling - Between 0-30% Health - Cast ''Healing Wave'''),
(7234, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 5000, 7000, 0, 11, 78921, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ferocitas the Dream Eater - IC - Cast Dream Lash');
