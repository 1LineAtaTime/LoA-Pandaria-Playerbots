-- #988 wave 2 (merge): Nagrand creature abilities from DeltaTho PR 15 file 2026_10_24_74_world.sql, ADDED to creatures that already have smart_scripts.
-- 11 creatures, 16 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17160:1 18069:19 18202:5 18203:4 18207:4 18261:3 18297:1 18391:1 18688:3 19067:1 19210:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17160 AND `id` >= 1)
  OR (`entryorguid` = 18069 AND `id` >= 19)
  OR (`entryorguid` = 18202 AND `id` >= 5)
  OR (`entryorguid` = 18203 AND `id` >= 4)
  OR (`entryorguid` = 18207 AND `id` >= 4)
  OR (`entryorguid` = 18261 AND `id` >= 3)
  OR (`entryorguid` = 18297 AND `id` >= 1)
  OR (`entryorguid` = 18391 AND `id` >= 1)
  OR (`entryorguid` = 18688 AND `id` >= 3)
  OR (`entryorguid` = 19067 AND `id` >= 1)
  OR (`entryorguid` = 19210 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17160, 0, 1, 0, 9, 0, 100, 0, 8, 40, 10000, 15000, 0, 11, 31705, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Living Cyclone - On Range - Cast Magnetic Pull'),
(18069, 0, 19, 0, 0, 0, 100, 0, 3400, 6400, 13200, 26400, 0, 11, 18975, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mogor - In Combat - Cast ''Summon Ice Totem'''),
(18202, 0, 5, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32133, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Putrifier - On Aggro - Cast ''Corrupted Earth'' (No Repeat)'),
(18202, 0, 6, 0, 0, 0, 100, 0, 2500, 4000, 9500, 12000, 0, 11, 32132, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Putrifier - In Combat - Cast ''Tainted Chain Lightning'''),
(18202, 0, 7, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 32134, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Putrifier - Between 20-80% Health - Cast ''Putrid Cloud'' (No Repeat)'),
(18203, 0, 4, 0, 0, 0, 100, 0, 5000, 10000, 15000, 20000, 0, 11, 11971, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Raider - In Combat - Cast ''Sunder Armor'''),
(18203, 0, 5, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Raider - Between 20-80% Health - Cast ''Cleave'' (No Repeat)'),
(18207, 0, 4, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 3391, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkblood Scavenger - Between 20-80% Health - Cast ''Thrash'' (No Repeat)'),
(18261, 0, 3, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32281, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lantresor of the Blade - Out of Combat - Cast ''Burning Blade Blademaster'' (Phase 1) (No Repeat)'),
(18297, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 11000, 13000, 0, 11, 30981, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gankly Rottenfist - In Combat - Cast ''Crippling Poison'''),
(18297, 0, 2, 0, 0, 0, 100, 0, 9000, 11000, 15000, 17000, 0, 11, 41176, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gankly Rottenfist - In Combat - Cast ''Shadowstep'''),
(18297, 0, 3, 0, 0, 0, 100, 0, 3000, 5000, 9000, 11000, 0, 11, 15581, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gankly Rottenfist - In Combat - Cast ''Sinister Strike'''),
(18391, 0, 1, 0, 0, 0, 100, 0, 4500, 8500, 3600, 14500, 0, 11, 32000, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Giselda the Crone - In Combat - Cast ''Mind Sear'''),
(18688, 0, 3, 0, 0, 0, 100, 0, 6000, 10000, 25000, 30000, 0, 11, 31293, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'consuming shadow'),
(19067, 0, 1, 0, 1, 0, 100, 0, 3000, 3000, 10000, 10000, 0, 11, 33535, 0, 0, 0, 0, 0, 19, 19212, 100, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 3000 and 3000 ms (and later repeats every 10000 and 10000 ms) - Self: Cast spell ''Fel Energy Beam'' (33535) on closest alive creature ''Fel Cannon: Hate Target'' (19212) in 100 yards'),
(19210, 0, 1, 0, 1, 0, 100, 0, 3000, 3000, 10000, 10000, 0, 11, 33535, 0, 0, 0, 0, 0, 19, 19211, 100, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 3000 and 3000 ms (and later repeats every 10000 and 10000 ms) - Self: Cast spell ''Fel Energy Beam'' (33535) on closest alive creature ''Fel Cannon: Fear Target'' (19211) in 100 yards');
