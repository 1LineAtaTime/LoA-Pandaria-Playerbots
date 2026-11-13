-- #988 wave 2 (merge): Dun Morogh, Coldridge Valley and New Tinkertown creature abilities from DeltaTho PR 15 file 2026_10_24_07_world.sql, ADDED to creatures that already have smart_scripts.
-- 11 creatures, 15 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 808:2 1115:3 1119:2 1260:3 1961:1 6123:1 8503:3 41251:2 41611:1 42291:1 43230:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 808 AND `id` >= 2)
  OR (`entryorguid` = 1115 AND `id` >= 3)
  OR (`entryorguid` = 1119 AND `id` >= 2)
  OR (`entryorguid` = 1260 AND `id` >= 3)
  OR (`entryorguid` = 1961 AND `id` >= 1)
  OR (`entryorguid` = 6123 AND `id` >= 1)
  OR (`entryorguid` = 8503 AND `id` >= 3)
  OR (`entryorguid` = 41251 AND `id` >= 2)
  OR (`entryorguid` = 41611 AND `id` >= 1)
  OR (`entryorguid` = 42291 AND `id` >= 1)
  OR (`entryorguid` = 43230 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(808, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 15000, 0, 11, 79895, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grik''nir the Cold - In Combat - Cast ''Frost Strike'''),
(1115, 0, 3, 0, 0, 0, 100, 0, 3500, 4500, 12000, 13000, 0, 11, 69851, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rockjaw Skullthumper - In Combat - Cast ''Club'''),
(1119, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 18501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hammerspine - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(1260, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 18501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Great Father Arctikus - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(1260, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77808, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Great Father Arctikus - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(1961, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Mangeclaw - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(6123, 0, 1, 0, 0, 0, 100, 0, 3000, 4500, 11500, 17500, 0, 11, 14873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Spy - In Combat - Cast ''Sinister Strike'''),
(6123, 0, 2, 0, 0, 0, 100, 0, 8000, 11000, 27000, 32000, 0, 11, 3583, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Spy - In Combat - Cast ''Deadly Poison'''),
(8503, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 20793, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gibblewilt - In Combat - Cast ''Fireball'''),
(41251, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 38557, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Builder - In Combat - Cast ''Throw'''),
(41251, 0, 3, 0, 9, 0, 100, 0, 0, 5, 18000, 19000, 0, 11, 58461, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Builder - Within 0-5 Range - Cast ''Sunder Armor'''),
(41611, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 240000, 240000, 0, 11, 8258, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Amberstill Mountaineer - In Combat - Cast ''Devotion Aura'''),
(41611, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 14000, 18000, 0, 11, 81460, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Amberstill Mountaineer - In Combat - Cast ''Fierce Strike'''),
(42291, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 84152, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Crushcog Sentry-Bot - On Aggro - Cast ''Wailing Siren'' (No Repeat)'),
(43230, 0, 2, 0, 0, 0, 100, 0, 4500, 6500, 18000, 19000, 0, 11, 84148, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Crushcog Technician - In Combat - Cast ''Irradiation Gun''');
