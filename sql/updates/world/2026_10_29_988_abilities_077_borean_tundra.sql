-- #988 wave 2 (merge): Borean Tundra creature abilities from DeltaTho PR 15 file 2026_10_24_95_world.sql, ADDED to creatures that already have smart_scripts.
-- 16 creatures, 17 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 24957:4 25428:2 25465:2 25479:1 25584:6 25596:3 25601:3 25613:1 25678:1 25712:2 25716:1 25719:10 25720:10 25722:9 25789:1 26452:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 24957 AND `id` >= 4)
  OR (`entryorguid` = 25428 AND `id` >= 2)
  OR (`entryorguid` = 25465 AND `id` >= 2)
  OR (`entryorguid` = 25479 AND `id` >= 1)
  OR (`entryorguid` = 25584 AND `id` >= 6)
  OR (`entryorguid` = 25596 AND `id` >= 3)
  OR (`entryorguid` = 25601 AND `id` >= 3)
  OR (`entryorguid` = 25613 AND `id` >= 1)
  OR (`entryorguid` = 25678 AND `id` >= 1)
  OR (`entryorguid` = 25712 AND `id` >= 2)
  OR (`entryorguid` = 25716 AND `id` >= 1)
  OR (`entryorguid` = 25719 AND `id` >= 10)
  OR (`entryorguid` = 25720 AND `id` >= 10)
  OR (`entryorguid` = 25722 AND `id` >= 9)
  OR (`entryorguid` = 25789 AND `id` >= 1)
  OR (`entryorguid` = 26452 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(24957, 0, 4, 0, 1, 0, 100, 0, 5000, 10000, 10000, 15000, 0, 11, 45864, 0, 0, 0, 0, 0, 19, 23837, 20, 0, 0, 0, 0, 0, 0, 'Cult Plaguebringer - Out of Combat - Cast ''Soul Missile'''),
(25428, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 45575, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magmoth Shaman - On Reset - Cast ''Magmoth Fire Totem'''),
(25465, 0, 2, 0, 1, 0, 100, 0, 0, 0, 2000, 10000, 0, 11, 50319, 0, 0, 0, 0, 0, 21, 70, 0, 0, 0, 0, 0, 0, 0, 'Kel''Thuzad - Out of Combat - Cast ''Soul Flay'''),
(25479, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 11000, 0, 11, 49816, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kvaldir Mistweaver - In Combat - Cast ''Mist of Strangulation'''),
(25584, 0, 6, 0, 1, 0, 100, 0, 5000, 5000, 10000, 10000, 0, 11, 45777, 0, 0, 0, 0, 0, 11, 25594, 19, 0, 0, 0, 0, 0, 0, 'Inquisitor Salrand - Out of Combat - Cast ''Salrand''s Beam'''),
(25596, 0, 3, 0, 0, 0, 100, 0, 10000, 12000, 20000, 22000, 0, 11, 45876, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Infected Kodo Beast - In Combat - Cast ''Stampede'''),
(25601, 0, 3, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 46482, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Prince Valanar - On Reset - Cast ''Naxxanar Beam 2'''),
(25613, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 11000, 0, 11, 49816, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Skadir Mistweaver - In Combat - Cast ''Mist of Strangulation'''),
(25678, 0, 1, 0, 1, 0, 100, 0, 3000, 3000, 30000, 30000, 0, 11, 45872, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Doctor Razorgin - OOC - Cast Defibrillate'),
(25712, 0, 2, 0, 1, 0, 100, 0, 100, 100, 45000, 45000, 0, 11, 39550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Warbringer Goredrak - Out of Combat - Cast ''Arcane Channeling'''),
(25716, 0, 1, 0, 1, 0, 100, 0, 100, 100, 45000, 45000, 0, 11, 39550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Cerulean - Out of Combat - Cast ''Arcane Channeling'''),
(25719, 0, 10, 0, 1, 0, 100, 1, 100, 100, 0, 0, 0, 11, 39550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Coldarra Spellbinder - Out of Combat - Cast Arcane Channeling'),
(25720, 0, 10, 0, 1, 0, 100, 0, 100, 100, 45000, 45000, 0, 11, 39550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inquisitor Caleras - Out of Combat - Cast ''Arcane Channeling'''),
(25722, 0, 9, 0, 1, 0, 100, 1, 100, 100, 0, 0, 0, 11, 39550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Coldarra Spellweaver - Out of Combat - Cast Arcane Channeling'),
(25789, 0, 1, 0, 0, 0, 100, 0, 0, 1000, 10000, 12000, 0, 11, 50413, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Gammothra the Tormentor - In Combat - Cast ''Magnataur Charge'''),
(25789, 0, 2, 0, 0, 0, 100, 0, 3000, 5000, 6000, 10000, 0, 11, 50410, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gammothra the Tormentor - In Combat - Cast ''Tusk Strike'''),
(26452, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 7000, 9000, 0, 11, 19643, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Leviroth - In Combat - Cast ''Mortal Strike''');
