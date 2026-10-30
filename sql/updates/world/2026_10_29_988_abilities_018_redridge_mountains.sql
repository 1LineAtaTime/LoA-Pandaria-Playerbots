-- #988 wave 2 (merge): Redridge Mountains creature abilities from DeltaTho PR 15 file 2026_10_24_25_world.sql, ADDED to creatures that already have smart_scripts.
-- 15 creatures, 20 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 334:3 423:2 428:1 431:2 486:5 568:4 578:2 579:4 580:4 703:1 711:4 712:1 947:2 4462:5 14271:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 334 AND `id` >= 3)
  OR (`entryorguid` = 423 AND `id` >= 2)
  OR (`entryorguid` = 428 AND `id` >= 1)
  OR (`entryorguid` = 431 AND `id` >= 2)
  OR (`entryorguid` = 486 AND `id` >= 5)
  OR (`entryorguid` = 568 AND `id` >= 4)
  OR (`entryorguid` = 578 AND `id` >= 2)
  OR (`entryorguid` = 579 AND `id` >= 4)
  OR (`entryorguid` = 580 AND `id` >= 4)
  OR (`entryorguid` = 703 AND `id` >= 1)
  OR (`entryorguid` = 711 AND `id` >= 4)
  OR (`entryorguid` = 712 AND `id` >= 1)
  OR (`entryorguid` = 947 AND `id` >= 2)
  OR (`entryorguid` = 4462 AND `id` >= 5)
  OR (`entryorguid` = 14271 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(334, 0, 3, 0, 0, 0, 100, 0, 5000, 5000, 12000, 13000, 0, 11, 79901, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Blood Plague'),
(334, 0, 4, 0, 0, 0, 100, 0, 4000, 4000, 10000, 15000, 0, 11, 79903, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Death Coil'),
(334, 0, 5, 0, 0, 0, 100, 0, 10000, 12000, 10000, 22000, 0, 11, 79902, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Plague Strike'),
(423, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 16000, 20000, 0, 11, 3427, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Infected Wound'),
(428, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 3427, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Infected Wound'),
(431, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 30% HP'),
(486, 0, 5, 0, 0, 0, 100, 0, 6000, 8000, 18000, 22000, 0, 11, 79893, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Bloodworm'),
(486, 0, 6, 0, 0, 0, 100, 0, 4000, 4000, 10000, 18000, 0, 11, 79887, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Death and Decay'),
(486, 0, 7, 0, 9, 0, 100, 0, 10, 30, 15000, 25000, 0, 11, 79894, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Death Grip on Close'),
(568, 0, 4, 0, 0, 0, 100, 0, 5000, 8000, 15000, 15000, 0, 11, 11971, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(578, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 13000, 15000, 0, 11, 7357, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murloc Scout - In Combat - Cast ''Poisonous Stab'''),
(579, 0, 4, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sneak on Spawn'),
(580, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 0, 11, 80512, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Fling Filth'),
(703, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 32736, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Mortal Strike'),
(703, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Rushing Charge on Aggro'),
(711, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(712, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Uppercut'),
(947, 0, 2, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sneak on Spawn'),
(4462, 0, 5, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 80673, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Camouflage on Spawn'),
(14271, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 30% HP');
