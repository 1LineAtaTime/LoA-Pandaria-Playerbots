-- #988 wave 2 (merge): Loch Modan creature abilities from DeltaTho PR 15 file 2026_10_24_18_world.sql, ADDED to creatures that already have smart_scripts.
-- 16 creatures, 32 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 1161:2 1162:3 1164:2 1165:3 1167:4 1172:2 1173:3 1174:5 1177:2 1197:3 1398:4 1399:4 1425:2 2476:1 14267:1 45369:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 1161 AND `id` >= 2)
  OR (`entryorguid` = 1162 AND `id` >= 3)
  OR (`entryorguid` = 1164 AND `id` >= 2)
  OR (`entryorguid` = 1165 AND `id` >= 3)
  OR (`entryorguid` = 1167 AND `id` >= 4)
  OR (`entryorguid` = 1172 AND `id` >= 2)
  OR (`entryorguid` = 1173 AND `id` >= 3)
  OR (`entryorguid` = 1174 AND `id` >= 5)
  OR (`entryorguid` = 1177 AND `id` >= 2)
  OR (`entryorguid` = 1197 AND `id` >= 3)
  OR (`entryorguid` = 1398 AND `id` >= 4)
  OR (`entryorguid` = 1399 AND `id` >= 4)
  OR (`entryorguid` = 1425 AND `id` >= 2)
  OR (`entryorguid` = 2476 AND `id` >= 1)
  OR (`entryorguid` = 14267 AND `id` >= 1)
  OR (`entryorguid` = 45369 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1161, 0, 2, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Trogg - In Combat - Cast ''Stone Splinter'''),
(1162, 0, 3, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Scout - In Combat - Cast ''Stone Splinter'''),
(1162, 0, 4, 0, 0, 0, 100, 0, 5000, 6000, 11000, 18000, 0, 11, 78509, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Scout - In Combat - Cast ''Torch Toss'''),
(1164, 0, 2, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Bonesnapper - In Combat - Cast ''Stone Splinter'''),
(1164, 0, 3, 0, 0, 0, 100, 0, 2000, 4500, 9000, 12000, 0, 11, 82625, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Bonesnapper - In Combat - Cast ''Bone Toss'''),
(1165, 0, 3, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Geomancer - In Combat - Cast ''Stone Splinter'''),
(1165, 0, 4, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79927, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Geomancer - Out of Combat - Cast ''Earth Shield'''),
(1165, 0, 5, 0, 0, 0, 100, 0, 8500, 8500, 10000, 10500, 0, 11, 79886, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Geomancer - In Combat - Cast ''Lava Burst'''),
(1167, 0, 4, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Digger - In Combat - Cast ''Stone Splinter'''),
(1167, 0, 5, 0, 0, 0, 100, 0, 12000, 15000, 28000, 33000, 0, 11, 80382, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Digger - In Combat - Cast ''Dirt Toss'''),
(1167, 0, 6, 0, 0, 0, 100, 0, 7000, 8000, 12000, 18000, 0, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Digger - In Combat - Cast ''Pierce Armor'''),
(1172, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 27000, 32000, 0, 11, 84863, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunnel Rat Vermin - In Combat - Cast ''Tunnel'''),
(1173, 0, 3, 0, 0, 0, 100, 0, 8000, 11000, 18000, 22000, 0, 11, 82708, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunnel Rat Scout - In Combat - Cast ''Flaming Arrow'''),
(1174, 0, 5, 0, 0, 0, 100, 0, 2000, 4500, 9000, 11000, 0, 11, 81305, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunnel Rat Geomancer - In Combat - Cast ''Rock Barrage'''),
(1177, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 27000, 32000, 0, 11, 84863, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunnel Rat Surveyor - In Combat - Cast ''Tunnel'''),
(1177, 0, 3, 0, 0, 0, 100, 0, 10000, 13000, 26000, 27000, 0, 11, 80382, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Tunnel Rat Surveyor - In Combat - Cast ''Dirt Toss'''),
(1197, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Shaman - In Combat - Cast ''Wrath'''),
(1197, 0, 4, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Shaman - In Combat - Cast ''Stone Splinter'''),
(1197, 0, 5, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79927, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonesplinter Shaman - Out of Combat - Cast ''Earth Shield'''),
(1398, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Boss Galgosh - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(1398, 0, 5, 0, 0, 0, 100, 0, 5800, 9500, 10500, 12500, 0, 11, 82625, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Boss Galgosh - In Combat - Cast ''Bone Toss'''),
(1398, 0, 6, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 80983, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Boss Galgosh - On Aggro - Cast ''Commanding Shout'' (No Repeat)'),
(1399, 0, 4, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79927, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magosh - Out of Combat - Cast ''Earth Shield'''),
(1399, 0, 5, 0, 0, 0, 100, 0, 10000, 12000, 38000, 42000, 0, 11, 39591, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magosh - In Combat - Cast ''Searing Totem'''),
(1399, 0, 6, 0, 0, 0, 100, 0, 8000, 8500, 12000, 20000, 0, 11, 13281, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Magosh - In Combat - Cast ''Earth Shock'''),
(1399, 0, 7, 0, 0, 0, 100, 0, 9000, 12000, 22000, 22000, 0, 11, 84857, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Magosh - In Combat - Cast ''Stone Splinter'''),
(1425, 0, 2, 0, 0, 0, 100, 0, 5000, 5000, 15000, 15000, 0, 11, 81252, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kubb - In Combat - Cast ''Full of Meat'''),
(2476, 0, 1, 2, 2, 0, 100, 1, 0, 25, 0, 0, 0, 11, 81173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Gosh-Haldir - Between 0-25% Health - Cast ''Frenzy'' (No Repeat)'),
(2476, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Gosh-Haldir - Between 0-25% Health - Say Line 0 (No Repeat)'),
(14267, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Emogg the Crusher - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(14267, 0, 2, 0, 0, 0, 100, 0, 8000, 11000, 22000, 28000, 0, 11, 84709, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Emogg the Crusher - In Combat - Cast ''Stomp'''),
(45369, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 13000, 0, 11, 13340, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Morick Darkbrew - In Combat - Cast ''Fire Blast''');
