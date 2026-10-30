-- #988 wave 2 (merge): Icecrown creature abilities from DeltaTho PR 15 file 2026_10_25_02_world.sql, ADDED to creatures that already have smart_scripts.
-- 11 creatures, 18 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 29648:1 29769:1 29770:1 29840:1 30409:7 30922:3 31277:12 31397:1 32149:1 32262:3 33429:7

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 29648 AND `id` >= 1)
  OR (`entryorguid` = 29769 AND `id` >= 1)
  OR (`entryorguid` = 29770 AND `id` >= 1)
  OR (`entryorguid` = 29840 AND `id` >= 1)
  OR (`entryorguid` = 30409 AND `id` >= 7)
  OR (`entryorguid` = 30922 AND `id` >= 3)
  OR (`entryorguid` = 31277 AND `id` >= 12)
  OR (`entryorguid` = 31397 AND `id` >= 1)
  OR (`entryorguid` = 32149 AND `id` >= 1)
  OR (`entryorguid` = 32262 AND `id` >= 3)
  OR (`entryorguid` = 33429 AND `id` >= 7));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(29648, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 54476, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bone Gryphon - On Reset - Cast ''Blood Presence'''),
(29769, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 29266, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Vile - On Reset - Cast Permament Feign Death'),
(29769, 0, 2, 0, 9, 0, 100, 0, 0, 5, 5000, 8000, 0, 11, 6253, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Vile - On Range - Cast Back Hand'),
(29769, 0, 3, 0, 0, 0, 100, 0, 10000, 15000, 15000, 23000, 0, 11, 56646, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Vile - IC - Cast Enrage'),
(29770, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 29266, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Nightswood - On Reset - Cast Permament Feign Death'),
(29770, 0, 2, 0, 0, 0, 100, 0, 0, 8000, 13000, 18000, 0, 11, 5884, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Nightswood - IC - Cast Banshee Curse'),
(29770, 0, 3, 0, 9, 0, 100, 0, 0, 5, 4000, 7000, 0, 11, 16838, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Nightswood - On Range - Cast Banshee Banshee Shiek'),
(29770, 0, 4, 0, 1, 0, 100, 0, 0, 0, 3000, 3000, 0, 11, 55070, 0, 0, 0, 0, 0, 19, 29771, 0, 0, 0, 0, 0, 0, 0, 'Lady Nightswood - OOC - Cast Shadow Cultist: Blue Smoke Beam'),
(29840, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 29266, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'The Leaper - On Reset - Cast Permament Feign Death'),
(29840, 0, 2, 0, 9, 0, 100, 0, 0, 5, 4000, 7000, 0, 11, 60195, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'The Leaper - On Range - Cast Sinister Strike'),
(29840, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 30000, 35000, 0, 11, 60177, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'The Leaper - In Combat - Cast Hunger For Blood'),
(30409, 0, 7, 0, 9, 0, 100, 0, 0, 10, 5000, 7000, 0, 11, 60290, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Apprentice Osterkilgr - In Combat - cast Blast Wave'),
(30922, 0, 3, 0, 0, 0, 100, 0, 6000, 7000, 10000, 11000, 0, 11, 10966, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'IC Update - cast uppercut'),
(31277, 0, 12, 0, 0, 0, 100, 0, 17000, 18000, 15000, 17000, 0, 11, 61139, 64, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'IC spell cast'),
(31397, 0, 1, 0, 0, 0, 100, 0, 1000, 1000, 14000, 14000, 0, 11, 3148, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Saronite Mine Slave - IC - Cast Head Crack'),
(32149, 0, 1, 0, 0, 0, 100, 0, 3000, 5200, 9200, 11500, 0, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Fallen Hero''s Spirit - In Combat - Cast ''Strike'''),
(32262, 0, 3, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 11, 60309, 0, 0, 0, 0, 0, 19, 32264, 30, 0, 0, 0, 0, 0, 0, 'Shadow Channeler - OOC - Cast Channel Souls'),
(33429, 0, 7, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 64100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Boneguard Lieutenant - On Spawn - Defend Aura');
