-- #988 wave 2 (merge): Eastern Plaguelands creature abilities from DeltaTho PR 15 file 2026_10_24_45_world.sql, ADDED to creatures that already have smart_scripts.
-- 23 creatures, 29 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 8519:8 8520:7 8521:3 8522:2 8526:2 8535:1 8538:3 8541:2 8548:1 8551:3 8556:1 8563:1 8598:2 9449:3 9450:4 9452:2 10821:1 10824:19 10825:2 10826:2 11896:2 12261:2 16184:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 8519 AND `id` >= 8)
  OR (`entryorguid` = 8520 AND `id` >= 7)
  OR (`entryorguid` = 8521 AND `id` >= 3)
  OR (`entryorguid` = 8522 AND `id` >= 2)
  OR (`entryorguid` = 8526 AND `id` >= 2)
  OR (`entryorguid` = 8535 AND `id` >= 1)
  OR (`entryorguid` = 8538 AND `id` >= 3)
  OR (`entryorguid` = 8541 AND `id` >= 2)
  OR (`entryorguid` = 8548 AND `id` >= 1)
  OR (`entryorguid` = 8551 AND `id` >= 3)
  OR (`entryorguid` = 8556 AND `id` >= 1)
  OR (`entryorguid` = 8563 AND `id` >= 1)
  OR (`entryorguid` = 8598 AND `id` >= 2)
  OR (`entryorguid` = 9449 AND `id` >= 3)
  OR (`entryorguid` = 9450 AND `id` >= 4)
  OR (`entryorguid` = 9452 AND `id` >= 2)
  OR (`entryorguid` = 10821 AND `id` >= 1)
  OR (`entryorguid` = 10824 AND `id` >= 19)
  OR (`entryorguid` = 10825 AND `id` >= 2)
  OR (`entryorguid` = 10826 AND `id` >= 2)
  OR (`entryorguid` = 11896 AND `id` >= 2)
  OR (`entryorguid` = 12261 AND `id` >= 2)
  OR (`entryorguid` = 16184 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8519, 0, 8, 0, 0, 0, 100, 0, 5000, 8000, 24000, 25000, 0, 11, 16555, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Drowning Death'),
(8520, 0, 7, 0, 0, 0, 100, 0, 5000, 8000, 24000, 25000, 0, 11, 16555, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Drowning Death'),
(8520, 0, 8, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 14099, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Mighty Blow'),
(8521, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 85873, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Flow of Gratitude on Aggro'),
(8522, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 24000, 25000, 0, 11, 16555, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Drowning Death'),
(8522, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 85873, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Flow of Gratitude on Aggro'),
(8526, 0, 2, 0, 0, 0, 100, 0, 8000, 8000, 17000, 19000, 0, 11, 11639, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Word: Pain'),
(8535, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 14000, 19000, 0, 11, 16573, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Putrid Bile'),
(8538, 0, 3, 0, 0, 0, 100, 0, 8000, 8000, 12000, 14000, 0, 11, 38240, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Chilling Touch'),
(8538, 0, 4, 0, 0, 0, 100, 0, 3000, 5000, 20000, 27000, 0, 11, 18267, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Curse of Weakness'),
(8541, 0, 2, 0, 9, 0, 100, 0, 0, 5, 25000, 25000, 0, 11, 6605, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Terrifying Screech on Close'),
(8548, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 16587, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Dark Whispers on Aggro'),
(8551, 0, 3, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 11, 51733, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Aura on Spawn'),
(8556, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 18000, 19000, 0, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Poison'),
(8563, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 17000, 18000, 0, 11, 43410, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Chop'),
(8598, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 17000, 19000, 0, 11, 90789, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Plague Bite'),
(9449, 0, 3, 0, 2, 0, 100, 0, 0, 40, 22000, 25000, 0, 11, 15586, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Heal at 40% HP'),
(9450, 0, 4, 0, 2, 0, 100, 0, 0, 40, 22000, 25000, 0, 11, 25058, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Renew at 40% HP'),
(9452, 0, 2, 0, 9, 0, 100, 0, 0, 8, 15000, 25000, 0, 11, 11975, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Arcane Explosion on Close'),
(10821, 0, 1, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 16577, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Disease Cloud on Spawn'),
(10821, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 30% HP'),
(10824, 0, 19, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 12057, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Strike'),
(10825, 0, 2, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 7020, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Stoneform at 40% HP'),
(10826, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 79901, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Blood Plague'),
(10826, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Demon Armor on Spawn'),
(10826, 0, 4, 0, 0, 0, 100, 0, 10000, 11000, 14000, 16500, 0, 11, 79902, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Plague Strike'),
(11896, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 6917, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Venom Spit'),
(12261, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 22000, 25000, 0, 11, 3427, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Infected Wound'),
(16184, 0, 2, 0, 13, 0, 100, 0, 20000, 30000, 0, 0, 0, 11, 32322, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Dark Shriek on Player Spell Cast');
