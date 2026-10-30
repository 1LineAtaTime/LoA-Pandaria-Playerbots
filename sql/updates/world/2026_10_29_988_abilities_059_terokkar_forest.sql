-- #988 wave 2 (merge): Terokkar Forest creature abilities from DeltaTho PR 15 file 2026_10_24_73_world.sql, ADDED to creatures that already have smart_scripts.
-- 10 creatures, 19 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17088:2 18482:1 18670:3 21651:1 21787:1 22199:3 23066:1 23067:1 23068:1 23219:7

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17088 AND `id` >= 2)
  OR (`entryorguid` = 18482 AND `id` >= 1)
  OR (`entryorguid` = 18670 AND `id` >= 3)
  OR (`entryorguid` = 21651 AND `id` >= 1)
  OR (`entryorguid` = 21787 AND `id` >= 1)
  OR (`entryorguid` = 22199 AND `id` >= 3)
  OR (`entryorguid` = 23066 AND `id` >= 1)
  OR (`entryorguid` = 23067 AND `id` >= 1)
  OR (`entryorguid` = 23068 AND `id` >= 1)
  OR (`entryorguid` = 23219 AND `id` >= 7));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17088, 0, 2, 0, 0, 0, 100, 0, 6000, 15000, 21000, 35000, 0, 11, 32940, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Summoner - In Combat - Cast ''Shadow Rush'''),
(18482, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 9000, 12000, 0, 11, 12548, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Empoor - In Combat - Cast ''Frost Shock'''),
(18670, 0, 3, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironjaw - In Combat - Cast ''Shoot'''),
(18670, 0, 4, 0, 9, 0, 100, 0, 0, 5, 15000, 17000, 0, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironjaw - Within 0-5 Range - Cast ''Cleave'''),
(21651, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 42247, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Skettis Reaver - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(21651, 0, 2, 0, 0, 0, 100, 0, 2500, 5000, 10000, 15000, 0, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Skettis Reaver - In Combat - Cast ''Cleave'''),
(21651, 0, 3, 0, 2, 0, 100, 1, 5, 30, 0, 0, 0, 11, 3019, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Skettis Reaver - Between 5-30% Health - Cast ''Frenzy'' (No Repeat)'),
(21787, 0, 1, 0, 0, 0, 100, 0, 4000, 8000, 7000, 16000, 0, 11, 11639, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Skettis High Priest - In Combat - Cast ''Shadow Word: Pain'''),
(21787, 0, 2, 0, 0, 0, 100, 0, 2300, 3000, 8000, 12000, 0, 11, 9734, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Skettis High Priest - In Combat - Cast ''Holy Smite'''),
(22199, 0, 3, 0, 0, 0, 100, 0, 27000, 32000, 27000, 32000, 0, 11, 21909, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Slaag - In Combat - Cast ''Dust Field'''),
(23066, 0, 1, 0, 0, 0, 100, 0, 10000, 15000, 10000, 15000, 0, 11, 17843, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Ishaal - In Combat - Cast Flash Heal'),
(23066, 0, 2, 0, 0, 0, 100, 0, 4000, 5000, 10000, 15000, 0, 11, 12160, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Ishaal - In Combat - Cast Rejuvenation'),
(23066, 0, 3, 0, 0, 0, 100, 0, 1000, 3000, 10000, 15000, 0, 11, 15654, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Ishaal - In Combat - Cast Shadow Word: Pain'),
(23067, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 8000, 12000, 0, 11, 17165, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Skizzik - In Combat - Cast Mind Flay'),
(23067, 0, 2, 0, 0, 0, 100, 0, 10000, 15000, 15000, 25000, 0, 11, 22884, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Skizzik - In Combat - Cast Psychic Scream'),
(23067, 0, 3, 0, 0, 0, 100, 0, 7000, 8000, 7000, 10000, 0, 11, 32712, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Skizzik - In Combat - Cast Shadow Nova'),
(23068, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 10000, 15000, 0, 11, 15652, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Zellek - In Combat - Cast Head Smash'),
(23068, 0, 2, 0, 0, 0, 100, 0, 10000, 15000, 18000, 25000, 0, 11, 17173, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talonpriest Zellek - In Combat - Cast Drain Life'),
(23219, 0, 7, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 11, 37417, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Blackwind Warp Chaser - On Aggro - Cast ''Warp Charge'' (No Repeat) (Normal Dungeon)');
