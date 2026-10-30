-- #988 wave 2 (merge): Shadowmoon Valley creature abilities from DeltaTho PR 15 file 2026_10_24_77_world.sql, ADDED to creatures that already have smart_scripts.
-- 9 creatures, 16 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 21337:1 21450:1 21453:1 21503:7 21506:2 21663:1 21876:1 22017:15 22093:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 21337 AND `id` >= 1)
  OR (`entryorguid` = 21450 AND `id` >= 1)
  OR (`entryorguid` = 21453 AND `id` >= 1)
  OR (`entryorguid` = 21503 AND `id` >= 7)
  OR (`entryorguid` = 21506 AND `id` >= 2)
  OR (`entryorguid` = 21663 AND `id` >= 1)
  OR (`entryorguid` = 21876 AND `id` >= 1)
  OR (`entryorguid` = 22017 AND `id` >= 15)
  OR (`entryorguid` = 22093 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(21337, 0, 1, 0, 0, 0, 100, 0, 2500, 5000, 10000, 12500, 0, 11, 7159, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Illidari Shadowstalker - In Combat - Cast ''Backstab'''),
(21450, 0, 1, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 38254, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Skethyl Owl - Between 20-80% Health - Cast ''Festering Wound'' (No Repeat)'),
(21450, 0, 2, 0, 2, 0, 100, 1, 10, 30, 0, 0, 0, 11, 38021, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Skethyl Owl - Between 10-30% Health - Cast ''Terrifying Screech'' (No Repeat)'),
(21453, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 20000, 25000, 0, 11, 32062, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ashtongue Shaman - In Combat - Cast ''Fire Nova Totem'''),
(21453, 0, 2, 0, 2, 0, 100, 1, 5, 30, 0, 0, 0, 11, 37067, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ashtongue Shaman - Between 5-30% Health - Cast ''Bloodlust'' (No Repeat)'),
(21503, 0, 7, 0, 0, 0, 100, 0, 5000, 8000, 10000, 16000, 0, 11, 32707, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunfury Warlock - IC - Cast Incinerate'),
(21503, 0, 8, 0, 0, 0, 100, 0, 0, 3000, 3000, 3000, 0, 11, 9613, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunfury Warlock - IC - Cast Shadow Bolt'),
(21506, 0, 2, 0, 23, 0, 100, 0, 38722, 1, 5000, 5000, 0, 11, 37833, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Azaloth - On aura x1 - Cast ban'),
(21506, 0, 3, 0, 9, 0, 100, 0, 0, 5, 8000, 15000, 0, 11, 40504, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Azaloth - On Range - Cast Cleave'),
(21506, 0, 4, 0, 0, 0, 100, 0, 8000, 8000, 8000, 12000, 0, 11, 11443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Azaloth - IC - Cast Cripple'),
(21506, 0, 5, 0, 0, 0, 100, 0, 5000, 10000, 12000, 15000, 0, 11, 38741, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Azaloth - IC - Cast Rain of Fire'),
(21663, 0, 1, 0, 2, 0, 100, 1, 20, 80, 0, 0, 0, 11, 34079, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Oronu the Elder - Between 20-80% Health - Cast ''Grounding Totem'' (No Repeat)'),
(21663, 0, 2, 0, 2, 0, 100, 1, 10, 30, 0, 0, 0, 11, 11986, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Oronu the Elder - Between 10-30% Health - Cast ''Healing Wave'' (No Repeat)'),
(21876, 0, 1, 0, 1, 0, 33, 0, 8000, 21000, 8000, 21000, 0, 11, 37784, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Chain of Shadows - Out of Combat - Cast ''Enforced Submission'''),
(22017, 0, 15, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 35190, 0, 0, 0, 0, 0, 19, 20431, 20, 0, 0, 0, 0, 0, 0, 'Eclipsion Spellbinder - On Reset - Cast ''Crystal Channel'''),
(22093, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 15000, 20000, 0, 11, 32736, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Illidari Watcher - In Combat - Cast ''Mortal Strike''');
