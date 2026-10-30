-- #988 wave 2 (merge): Dragonblight creature abilities from DeltaTho PR 15 file 2026_10_24_97_world.sql, ADDED to creatures that already have smart_scripts.
-- 9 creatures, 9 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 26402:2 26467:4 26926:2 26942:11 27406:5 27539:12 27564:15 27725:1 27749:15

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 26402 AND `id` >= 2)
  OR (`entryorguid` = 26467 AND `id` >= 4)
  OR (`entryorguid` = 26926 AND `id` >= 2)
  OR (`entryorguid` = 26942 AND `id` >= 11)
  OR (`entryorguid` = 27406 AND `id` >= 5)
  OR (`entryorguid` = 27539 AND `id` >= 12)
  OR (`entryorguid` = 27564 AND `id` >= 15)
  OR (`entryorguid` = 27725 AND `id` >= 1)
  OR (`entryorguid` = 27749 AND `id` >= 15));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26402, 0, 2, 0, 1, 0, 100, 0, 2000, 3000, 8000, 16000, 0, 11, 46977, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Anub''ar Ambusher - Out of Combat - Cast ''Digging through Snow'''),
(26467, 0, 4, 0, 0, 0, 100, 0, 4000, 6000, 19000, 21000, 0, 11, 32738, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Jormungar Tunneler  - In Combat - Cast Bore'),
(26926, 0, 2, 0, 2, 0, 100, 0, 0, 50, 13000, 19000, 0, 11, 51873, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Duke Vallenhal - Between 0-50% Health - Cast ''Bloodworm'''),
(26942, 0, 11, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 11, 45491, 0, 0, 0, 0, 0, 19, 27180, 10, 0, 0, 0, 0, 0, 0, 'Decrepit Necromancer - Out of Combat - Cast Necrotic Purple Beam'),
(27406, 0, 5, 0, 0, 0, 100, 0, 2000, 2000, 28000, 28000, 0, 11, 50713, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Onslaught Footman - IC - Unrelenting Onslaught'),
(27539, 0, 12, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 11, 49292, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On OOC Update - Cast Ruby Corrupt'),
(27564, 0, 15, 0, 1, 0, 100, 0, 0, 5000, 0, 10000, 0, 11, 49329, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Alliance Conscript - OOC - Cast Summon Frigid Ghoul Attacker'),
(27725, 0, 1, 0, 1, 0, 100, 0, 0, 5000, 5000, 15000, 0, 11, 49068, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ruby Guardian - OOC - Cast Summon Frigid Ghoul'),
(27749, 0, 15, 0, 1, 0, 100, 0, 0, 5000, 0, 10000, 0, 11, 49329, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Conscript - OOC - Cast Summon Frigid Ghoul Attacker');
