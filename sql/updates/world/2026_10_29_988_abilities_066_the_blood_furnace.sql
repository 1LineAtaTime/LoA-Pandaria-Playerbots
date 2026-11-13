-- #988 wave 2 (merge): The Blood Furnace creature abilities from DeltaTho PR 15 file 2026_10_24_80_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 7 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17371:16 17397:4 17398:2 17477:15 17626:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17371 AND `id` >= 16)
  OR (`entryorguid` = 17397 AND `id` >= 4)
  OR (`entryorguid` = 17398 AND `id` >= 2)
  OR (`entryorguid` = 17477 AND `id` >= 15)
  OR (`entryorguid` = 17626 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17371, 0, 16, 0, 0, 0, 100, 4, 2300, 16600, 21850, 43750, 0, 11, 32863, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Seed of Corruption'' (Heroic Dungeon)'),
(17371, 0, 17, 0, 1, 0, 100, 0, 12600, 13700, 21900, 24900, 0, 11, 33111, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Out of Combat - Cast ''Fel Power'''),
(17397, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 31059, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Adept - Out Of Combat - Cast Hellfire Channeling'),
(17398, 0, 2, 0, 0, 0, 100, 0, 6000, 18500, 1200, 27950, 0, 11, 22427, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nascent Fel Orc - In Combat - Cast ''Concussion Blow'''),
(17398, 0, 3, 0, 0, 0, 100, 0, 1200, 12150, 12150, 27950, 0, 11, 31900, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nascent Fel Orc - In Combat - Cast ''Stomp'''),
(17477, 0, 15, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 30540, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Out of Combat - Cast ''Summon Visual'' (No Repeat)'),
(17626, 0, 3, 0, 0, 0, 100, 0, 5000, 12000, 25000, 35000, 0, 11, 18765, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Laughing Skull Legionnaire - In Combat - Cast Sweeping Strikes');
