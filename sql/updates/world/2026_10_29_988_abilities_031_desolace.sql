-- #988 wave 2 (merge): Desolace creature abilities from DeltaTho PR 15 file 2026_10_24_40_world.sql, ADDED to creatures that already have smart_scripts.
-- 18 creatures, 32 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 4663:3 4664:1 4665:3 4666:1 4667:3 4668:3 4670:3 4671:3 4672:2 4673:2 4674:3 4675:8 4681:1 4728:1 4729:1 5760:2 35412:2 36094:4

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 4663 AND `id` >= 3)
  OR (`entryorguid` = 4664 AND `id` >= 1)
  OR (`entryorguid` = 4665 AND `id` >= 3)
  OR (`entryorguid` = 4666 AND `id` >= 1)
  OR (`entryorguid` = 4667 AND `id` >= 3)
  OR (`entryorguid` = 4668 AND `id` >= 3)
  OR (`entryorguid` = 4670 AND `id` >= 3)
  OR (`entryorguid` = 4671 AND `id` >= 3)
  OR (`entryorguid` = 4672 AND `id` >= 2)
  OR (`entryorguid` = 4673 AND `id` >= 2)
  OR (`entryorguid` = 4674 AND `id` >= 3)
  OR (`entryorguid` = 4675 AND `id` >= 8)
  OR (`entryorguid` = 4681 AND `id` >= 1)
  OR (`entryorguid` = 4728 AND `id` >= 1)
  OR (`entryorguid` = 4729 AND `id` >= 1)
  OR (`entryorguid` = 5760 AND `id` >= 2)
  OR (`entryorguid` = 35412 AND `id` >= 2)
  OR (`entryorguid` = 36094 AND `id` >= 4));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4663, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77721, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Augur - In Combat - Cast ''Shadow Weave'''),
(4663, 0, 4, 0, 9, 0, 100, 0, 0, 8, 20000, 24000, 0, 11, 77718, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Augur - Within 0-8 Range - Cast ''Shadowflame'''),
(4663, 0, 5, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Augur - In Combat - Cast ''Felblood'''),
(4664, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Reaver - In Combat - Cast ''Felblood'''),
(4664, 0, 2, 0, 9, 0, 100, 0, 0, 5, 15000, 22000, 0, 11, 77670, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Reaver - Within 0-5 Range - Cast ''Fire Whirl'''),
(4665, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77703, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Adept - In Combat - Cast ''Magma Burst'''),
(4665, 0, 4, 0, 9, 0, 100, 0, 0, 8, 21000, 22000, 0, 11, 77695, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Adept - Within 0-8 Range - Cast ''Dragon''s Breath'''),
(4665, 0, 5, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Adept - In Combat - Cast ''Felblood'''),
(4666, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Felsworn - In Combat - Cast ''Felblood'''),
(4667, 0, 3, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 11, 74919, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Shadowmage - Out of Combat - Cast ''Shadow Channeling'' (No Repeat)'),
(4667, 0, 4, 0, 0, 0, 100, 0, 2000, 3000, 60000, 65000, 0, 11, 77645, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Shadowmage - In Combat - Cast ''Felblood'''),
(4667, 0, 5, 0, 0, 0, 100, 0, 6000, 7000, 15000, 21000, 0, 11, 77722, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Shadowmage - In Combat - Cast ''Void Whip'''),
(4668, 0, 3, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Summoner - In Combat - Cast ''Felblood'''),
(4670, 0, 3, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77806, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Rogue - On Respawn - Cast ''Stealth'' (No Repeat)'),
(4671, 0, 3, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 1784, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Trickster - On Respawn - Cast ''Stealth'' (No Repeat)'),
(4671, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77457, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Trickster - On Aggro - Cast ''Dazzling Lights'' (No Repeat)'),
(4671, 0, 5, 0, 2, 0, 100, 0, 0, 45, 25000, 33000, 0, 11, 77438, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Trickster - Between 0-45% Health - Cast ''Vanish'''),
(4672, 0, 2, 0, 0, 0, 100, 0, 3000, 4500, 18000, 22000, 0, 11, 77469, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Felsworn - In Combat - Cast ''Taint of Sargeras'''),
(4673, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Betrayer - In Combat - Cast ''Shoot'''),
(4674, 0, 3, 0, 2, 0, 100, 0, 0, 30, 45000, 51000, 0, 11, 77471, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Shadowstalker - Between 0-30% Health - Cast ''Shadow Shield'''),
(4675, 0, 8, 0, 0, 0, 100, 0, 3000, 4500, 21000, 25000, 0, 11, 77425, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Hellcaller - In Combat - Cast ''Immolate'''),
(4675, 0, 9, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 77426, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hatefury Hellcaller - Out of Combat - Cast ''Summon Burning Minion'' (No Repeat)'),
(4681, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 60000, 65000, 0, 11, 77645, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter - In Combat - Cast ''Felblood'''),
(4728, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 18000, 25000, 0, 11, 77503, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gritjaw Basilisk - In Combat - Cast ''Petrifying Stare'''),
(4729, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 18000, 25000, 0, 11, 77503, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hulking Gritjaw Basilisk - In Combat - Cast ''Petrifying Stare'''),
(5760, 0, 2, 0, 0, 0, 100, 0, 4000, 8000, 22000, 27000, 0, 11, 68545, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Azrethoc - In Combat - Cast ''Lord Azrethoc''s Bellow'''),
(5760, 0, 3, 0, 0, 0, 100, 0, 2000, 3000, 18000, 25000, 0, 11, 68548, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Azrethoc - In Combat - Cast ''Shadow Torch'''),
(5760, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 68541, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Azrethoc - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(35412, 0, 2, 0, 0, 0, 100, 0, 2000, 3500, 12000, 14000, 0, 11, 15611, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rejuvenated Thunder Lizard - In Combat - Cast ''Lizard Bolt'''),
(35412, 0, 3, 0, 0, 0, 100, 0, 6000, 8000, 24000, 27000, 0, 11, 77601, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rejuvenated Thunder Lizard - In Combat - Cast ''Lightning Nova'''),
(36094, 0, 4, 0, 0, 0, 100, 0, 4000, 5000, 18000, 22000, 0, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Enraged Kodo - In Combat - Cast ''Trample'''),
(36094, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 18501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Enraged Kodo - Between 0-30% Health - Cast ''Enrage'' (No Repeat)');
