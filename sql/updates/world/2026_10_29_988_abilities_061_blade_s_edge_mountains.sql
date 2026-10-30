-- #988 wave 2 (merge): Blade's Edge Mountains creature abilities from DeltaTho PR 15 file 2026_10_24_75_world.sql, ADDED to creatures that already have smart_scripts.
-- 21 creatures, 30 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 19945:1 19948:3 19973:2 19985:1 19988:1 19989:1 19990:1 19993:3 20600:3 20748:1 20924:3 20925:1 21326:7 21497:3 21956:1 22160:4 22181:10 22204:2 22221:1 22242:2 22289:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 19945 AND `id` >= 1)
  OR (`entryorguid` = 19948 AND `id` >= 3)
  OR (`entryorguid` = 19973 AND `id` >= 2)
  OR (`entryorguid` = 19985 AND `id` >= 1)
  OR (`entryorguid` = 19988 AND `id` >= 1)
  OR (`entryorguid` = 19989 AND `id` >= 1)
  OR (`entryorguid` = 19990 AND `id` >= 1)
  OR (`entryorguid` = 19993 AND `id` >= 3)
  OR (`entryorguid` = 20600 AND `id` >= 3)
  OR (`entryorguid` = 20748 AND `id` >= 1)
  OR (`entryorguid` = 20924 AND `id` >= 3)
  OR (`entryorguid` = 20925 AND `id` >= 1)
  OR (`entryorguid` = 21326 AND `id` >= 7)
  OR (`entryorguid` = 21497 AND `id` >= 3)
  OR (`entryorguid` = 21956 AND `id` >= 1)
  OR (`entryorguid` = 22160 AND `id` >= 4)
  OR (`entryorguid` = 22181 AND `id` >= 10)
  OR (`entryorguid` = 22204 AND `id` >= 2)
  OR (`entryorguid` = 22221 AND `id` >= 1)
  OR (`entryorguid` = 22242 AND `id` >= 2)
  OR (`entryorguid` = 22289 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(19945, 0, 1, 0, 0, 0, 100, 0, 1500, 3000, 6500, 9000, 0, 11, 9532, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lashh''an Windwalker - In Combat - Cast ''Lightning Bolt'''),
(19948, 0, 3, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 34802, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodmaul Skirmisher - Between 20-80% Health - Cast ''Kick'' (No Repeat)'),
(19973, 0, 2, 0, 0, 0, 100, 0, 6000, 15000, 18000, 28000, 0, 11, 37633, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Abyssal Flamebringer - IC - Cast Abyssal Strike'),
(19985, 0, 1, 0, 0, 0, 100, 0, 2500, 3500, 7500, 8000, 0, 11, 9532, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ruuan''ok Cloudgazer - In Combat - Cast ''Lightning Bolt'''),
(19988, 0, 1, 0, 0, 0, 100, 0, 2500, 5500, 10000, 11000, 0, 11, 37587, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishna Falconwing - In Combat - Cast Bestial Wrath'),
(19989, 0, 1, 0, 0, 0, 100, 0, 2500, 5500, 20000, 21000, 0, 11, 37589, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishna Harbinger - In Combat - Cast Shriveling Gaze'),
(19989, 0, 2, 0, 0, 0, 100, 0, 1000, 6000, 10000, 15000, 0, 11, 9532, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishna Harbinger - In Combat - Cast Lightning Bolt'),
(19990, 0, 1, 0, 0, 0, 100, 0, 2500, 5500, 10000, 11000, 0, 11, 35321, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishna Scorncrow - In Combat - Cast Gushing Wound'),
(19993, 0, 3, 0, 0, 0, 100, 0, 15000, 30000, 30000, 60000, 0, 11, 37592, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodmaul Mauler - In Combat - Cast Knockdown'),
(20600, 0, 3, 0, 0, 0, 100, 0, 3000, 6000, 8000, 12000, 0, 11, 38770, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Maggoc - In Combat - Cast ''Mortal Wound'''),
(20600, 0, 4, 0, 0, 0, 100, 0, 3000, 6000, 5000, 10000, 0, 11, 38777, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Maggoc - In Combat - Cast ''Rock Rumble'''),
(20600, 0, 5, 0, 0, 0, 100, 0, 4000, 7000, 5000, 10000, 0, 11, 42139, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Maggoc - In Combat - Cast ''Boulder'''),
(20748, 0, 1, 0, 0, 0, 100, 0, 3000, 8000, 11000, 15000, 0, 11, 5781, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Thunderlord Dire Wolf - In Combat - Cast ''Threatening Growl'''),
(20924, 0, 3, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishnath Basilisk - Between 0-15% Health - Cast ''Feign Death'''),
(20924, 0, 4, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 0, 11, 35385, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grishnath Basilisk - In Combat - Cast ''Threshalisk Charge'' (Phase 1)'),
(20925, 0, 1, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Scalded Basilisk - Between 0-15% Health - Cast ''Feign Death'''),
(20925, 0, 2, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 0, 11, 35385, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Scalded Basilisk - In Combat - Cast ''Threshalisk Charge'' (Phase 1)'),
(21326, 0, 7, 0, 0, 0, 100, 0, 10000, 12000, 35000, 40000, 0, 11, 37696, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raven''s Wood Leafbeard - IC - cast Force of Nature'),
(21497, 0, 3, 0, 0, 0, 85, 0, 15000, 15000, 30000, 30000, 0, 11, 15847, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Blackscale - In Combat - Cast ''Tail Sweep'''),
(21956, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 19000, 20000, 0, 11, 13443, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rema - In Combat - Cast ''Rend'''),
(21956, 0, 2, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 5781, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rema - Between 20-80% Health - Cast ''Threatening Growl'' (No Repeat)'),
(21956, 0, 3, 0, 2, 0, 100, 1, 10, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rema - Between 10-30% Health - Cast ''Enrage'' (No Repeat)'),
(22160, 0, 4, 0, 0, 0, 100, 0, 20000, 50000, 60000, 90000, 0, 11, 37592, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodmaul Taskmaster - In Combat - Cast Knockdown'),
(22181, 0, 10, 0, 3, 0, 100, 0, 0, 75, 20000, 20000, 0, 11, 17008, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Aether Ray - Between 0-75% Mana - Cast ''Mana Drain'''),
(22204, 0, 2, 0, 0, 0, 100, 0, 9000, 20000, 13000, 27000, 0, 11, 38356, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fear Fiend - IC - Cast Fel Flames'),
(22221, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 37628, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felstorm Overseer - On Aggro - Cast ''Fel Immolate'' (No Repeat)'),
(22221, 0, 2, 0, 0, 0, 100, 0, 2700, 3000, 6700, 7000, 0, 11, 37945, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felstorm Overseer - In Combat - Cast ''Fel Fireball'''),
(22221, 0, 3, 0, 2, 0, 100, 1, 10, 30, 0, 0, 0, 11, 37488, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felstorm Overseer - Between 10-30% Health - Cast ''Fel Flames'' (No Repeat)'),
(22242, 0, 2, 0, 0, 0, 100, 0, 2500, 4500, 12000, 15000, 0, 11, 29881, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bash''ir Spell-Thief - In Combat - Cast ''Drain Mana'''),
(22289, 0, 1, 0, 2, 0, 100, 1, 5, 20, 0, 0, 0, 11, 7139, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkflame Infernal - Between 5-20% Health - Cast ''Fel Stomp'' (No Repeat)');
