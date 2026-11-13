-- #988 wave 2 (merge): Dire Maul creature abilities from DeltaTho PR 15 file 2026_10_24_63_world.sql, ADDED to creatures that already have smart_scripts.
-- 14 creatures, 15 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 11441:6 11448:16 11452:2 11455:16 11465:1 11473:6 11486:6 11487:6 11488:4 13021:1 13197:12 13285:2 14327:14 14354:22

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 11441 AND `id` >= 6)
  OR (`entryorguid` = 11448 AND `id` >= 16)
  OR (`entryorguid` = 11452 AND `id` >= 2)
  OR (`entryorguid` = 11455 AND `id` >= 16)
  OR (`entryorguid` = 11465 AND `id` >= 1)
  OR (`entryorguid` = 11473 AND `id` >= 6)
  OR (`entryorguid` = 11486 AND `id` >= 6)
  OR (`entryorguid` = 11487 AND `id` >= 6)
  OR (`entryorguid` = 11488 AND `id` >= 4)
  OR (`entryorguid` = 13021 AND `id` >= 1)
  OR (`entryorguid` = 13197 AND `id` >= 12)
  OR (`entryorguid` = 13285 AND `id` >= 2)
  OR (`entryorguid` = 14327 AND `id` >= 14)
  OR (`entryorguid` = 14354 AND `id` >= 22));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11441, 0, 6, 0, 0, 0, 100, 0, 8000, 17000, 14000, 21000, 0, 11, 6253, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gordok Brute - In Combat - Cast Backhand'),
(11448, 0, 16, 0, 1, 0, 100, 1, 10000, 10000, 0, 0, 0, 11, 21157, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Gordok Warlock - Out of Combat - Cast Dark Channeling'),
(11452, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 13299, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wildspawn Rogue - On Reset - Cast Poison Proc'),
(11455, 0, 16, 0, 0, 0, 100, 0, 0, 2000, 120000, 120000, 0, 11, 22418, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wildspawn Felsworn - In Combat - Cast Chaotic Focus'),
(11465, 0, 1, 0, 0, 0, 100, 0, 4000, 15000, 10000, 25000, 0, 11, 16727, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warpwood Stomper - In Combat - Cast War Stomp'),
(11473, 0, 6, 0, 0, 0, 100, 0, 4000, 11000, 14000, 25000, 0, 11, 17831, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Eldreth Spectre - In Combat - Cast Call of the Graves'),
(11486, 0, 6, 0, 25, 0, 100, 65537, 0, 0, 0, 0, 0, 11, 674, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Prince Tortheldrin - On Reset - Cast Dual Wield'),
(11487, 0, 6, 0, 1, 0, 100, 1, 10000, 10000, 0, 0, 0, 11, 21157, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tendris Warpwood - Out of Combat - Cast Dark Channeling'),
(11488, 0, 4, 0, 0, 0, 100, 0, 0, 500, 2000, 2000, 0, 11, 16100, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Illyanna Ravenoak - In Combat - Cast Shoot'),
(11488, 0, 5, 0, 0, 0, 100, 0, 3000, 12000, 10000, 15000, 0, 11, 20735, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Illyanna Ravenoak - In Combat - Cast Multi-Shot'),
(13021, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 11000, 15000, 0, 11, 15550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warpwood Crusher - In Combat - Cast Trample'),
(13197, 0, 12, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 35187, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fel Lash - On Reset - Cast Arcane Explosion Proc'),
(13285, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 16428, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Death Lash - On Reset - Cast Virulent Poison Proc'),
(14327, 0, 14, 0, 0, 0, 100, 0, 11000, 11000, 30000, 30000, 0, 11, 22710, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lethtendris - In Combat - Cast Enlarge'),
(14354, 0, 22, 0, 7, 0, 100, 0, 0, 0, 0, 0, 0, 11, 22735, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Pusillin - On Evade - Cast Spirit of Runn Tum');
