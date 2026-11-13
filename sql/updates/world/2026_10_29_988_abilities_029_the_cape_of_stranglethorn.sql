-- #988 wave 2 (merge): The Cape of Stranglethorn creature abilities from DeltaTho PR 15 file 2026_10_24_38_world.sql, ADDED to creatures that already have smart_scripts.
-- 13 creatures, 22 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 689:2 1493:2 1562:3 1907:1 2521:1 2522:2 2545:7 4457:2 4505:1 43376:12 43377:9 43659:8 43660:8

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 689 AND `id` >= 2)
  OR (`entryorguid` = 1493 AND `id` >= 2)
  OR (`entryorguid` = 1562 AND `id` >= 3)
  OR (`entryorguid` = 1907 AND `id` >= 1)
  OR (`entryorguid` = 2521 AND `id` >= 1)
  OR (`entryorguid` = 2522 AND `id` >= 2)
  OR (`entryorguid` = 2545 AND `id` >= 7)
  OR (`entryorguid` = 4457 AND `id` >= 2)
  OR (`entryorguid` = 4505 AND `id` >= 1)
  OR (`entryorguid` = 43376 AND `id` >= 12)
  OR (`entryorguid` = 43377 AND `id` >= 9)
  OR (`entryorguid` = 43659 AND `id` >= 8)
  OR (`entryorguid` = 43660 AND `id` >= 8));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(689, 0, 2, 0, 0, 0, 75, 0, 5000, 5000, 16000, 16000, 0, 11, 3635, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Crystal Spine Basilisk - In Combat - Cast ''Crystal Gaze'' (No Repeat) (Normal Dungeon)'),
(1493, 0, 2, 0, 9, 0, 100, 0, 0, 10, 15000, 18000, 0, 11, 81026, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sea Splash on Close'),
(1562, 0, 3, 0, 9, 0, 100, 0, 0, 8, 15000, 25000, 0, 11, 75062, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Frost Nova on Close'),
(1907, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 15000, 18000, 0, 11, 13445, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(1907, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(2521, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 83378, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Stomp'),
(2521, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 81219, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout on Aggro'),
(2522, 0, 2, 0, 1, 0, 100, 1, 1000, 1500, 0, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Jaguero Stalker - Out of Combat - Cast ''Sneak'' (No Repeat)'),
(2545, 0, 7, 0, 0, 0, 100, 0, 2700, 4600, 8700, 9600, 0, 11, 14873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, '"Pretty Boy" Duncan - In Combat - Cast ''Sinister Strike'''),
(4457, 0, 2, 0, 0, 0, 100, 0, 1000, 2000, 19900, 30000, 0, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkgill Forager - In Combat - Cast ''Rend'''),
(4457, 0, 3, 0, 0, 0, 100, 0, 2600, 2600, 3700, 9100, 0, 11, 744, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Murkgill Forager - In Combat - Cast ''Poison'''),
(4505, 0, 1, 0, 0, 0, 100, 0, 4000, 5500, 12000, 15000, 0, 11, 37998, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Piercing Strike'),
(4505, 0, 2, 0, 0, 0, 100, 0, 9000, 12000, 20000, 25000, 0, 11, 11977, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(43376, 0, 12, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 0, 11, 38558, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Throw'),
(43376, 0, 13, 0, 0, 0, 100, 0, 5000, 5000, 12000, 13000, 0, 11, 54962, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Ticking Bomb'),
(43377, 0, 9, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 83017, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Insect Swarm'),
(43377, 0, 10, 0, 0, 0, 100, 0, 0, 0, 3500, 4700, 0, 11, 78907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Starfire'),
(43377, 0, 11, 0, 0, 0, 100, 0, 10000, 10000, 15000, 18000, 0, 11, 79823, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Starfall'),
(43659, 0, 8, 0, 0, 0, 100, 0, 5000, 6000, 12000, 15000, 0, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Cleave'),
(43659, 0, 9, 0, 0, 0, 100, 0, 10000, 10000, 18000, 19000, 0, 11, 3583, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Deadly Poison'),
(43660, 0, 8, 0, 9, 0, 100, 0, 0, 10, 19000, 22000, 0, 11, 13730, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Demoralizing Shout on Close'),
(43660, 0, 9, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 34159, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Hulk at 30% HP');
