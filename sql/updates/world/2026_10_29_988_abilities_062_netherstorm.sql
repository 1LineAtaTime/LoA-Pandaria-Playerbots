-- #988 wave 2 (merge): Netherstorm creature abilities from DeltaTho PR 15 file 2026_10_24_76_world.sql, ADDED to creatures that already have smart_scripts.
-- 14 creatures, 29 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 18850:3 18853:3 19466:1 19541:1 20102:2 20132:1 20134:14 20139:15 20248:5 20409:1 20607:2 20673:1 20769:1 20928:4

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 18850 AND `id` >= 3)
  OR (`entryorguid` = 18853 AND `id` >= 3)
  OR (`entryorguid` = 19466 AND `id` >= 1)
  OR (`entryorguid` = 19541 AND `id` >= 1)
  OR (`entryorguid` = 20102 AND `id` >= 2)
  OR (`entryorguid` = 20132 AND `id` >= 1)
  OR (`entryorguid` = 20134 AND `id` >= 14)
  OR (`entryorguid` = 20139 AND `id` >= 15)
  OR (`entryorguid` = 20248 AND `id` >= 5)
  OR (`entryorguid` = 20409 AND `id` >= 1)
  OR (`entryorguid` = 20607 AND `id` >= 2)
  OR (`entryorguid` = 20673 AND `id` >= 1)
  OR (`entryorguid` = 20769 AND `id` >= 1)
  OR (`entryorguid` = 20928 AND `id` >= 4));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18850, 0, 3, 0, 0, 0, 100, 0, 1000, 2000, 30000, 30000, 0, 11, 35877, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunfury Guardsman - In Combat - Cast Mark of the Sunfury'),
(18853, 0, 3, 0, 0, 0, 100, 0, 1000, 2000, 30000, 30000, 0, 11, 35877, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunfury Bloodwarder - In Combat - Cast Mark of the Sunfury'),
(19466, 0, 1, 0, 0, 0, 100, 0, 0, 3500, 15000, 21000, 0, 11, 29426, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Exarch Orelis - In Combat - Cast ''Heroic Strike'''),
(19466, 0, 2, 0, 0, 0, 100, 0, 10000, 15000, 30000, 30000, 0, 11, 16509, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Exarch Orelis - In Combat - Cast ''Rend'''),
(19466, 0, 3, 0, 0, 0, 100, 0, 7500, 21000, 20000, 45000, 0, 11, 13730, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Exarch Orelis - In Combat - Cast ''Demoralizing Shout'''),
(19541, 0, 1, 0, 9, 0, 100, 0, 5, 30, 3600, 3600, 0, 11, 36246, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Netherstorm Agent - Within 5-30 Range - Cast ''Shoot Tech Gun'''),
(20102, 0, 2, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65526, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Midsummer'''),
(20102, 0, 3, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65522, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Winter Veil'''),
(20102, 0, 4, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65524, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Lunar Festival'''),
(20102, 0, 5, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65523, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Default'''),
(20102, 0, 6, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65525, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Hallow''s End'''),
(20102, 0, 7, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65511, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Brewfest'''),
(20102, 0, 8, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65527, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Spirit of Competition'''),
(20102, 0, 9, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65528, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Pirates'' Day'''),
(20102, 0, 10, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 65529, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Commoner - On Respawn - Cast ''Gossip NPC Appearance - Day of the Dead (DotD)'''),
(20132, 0, 1, 0, 0, 0, 100, 0, 2500, 7500, 15000, 25000, 0, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Socrethar - In Combat - Cast ''Cleave'''),
(20132, 0, 2, 0, 0, 0, 100, 0, 10000, 15000, 40000, 60000, 0, 11, 37538, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Socrethar - In Combat - Cast ''Anti-Magic Shield'''),
(20132, 0, 3, 0, 0, 0, 100, 0, 17000, 24000, 10000, 35000, 0, 11, 28448, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Socrethar - In Combat - Cast ''Shadow Bolt Volley'''),
(20132, 0, 4, 0, 0, 0, 100, 0, 30000, 40000, 30000, 40000, 0, 11, 37540, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Socrethar - In Combat - Cast ''Fireball Barrage'''),
(20132, 0, 5, 0, 0, 0, 100, 0, 30000, 45000, 35000, 50000, 0, 11, 37537, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Socrethar - In Combat - Cast ''Backlash'''),
(20134, 0, 14, 0, 1, 0, 50, 0, 10000, 20000, 15000, 30000, 0, 11, 34397, 0, 0, 0, 0, 0, 19, 19421, 30, 0, 0, 0, 0, 0, 0, '20134 - Ooc - Cast Red Beam'),
(20139, 0, 15, 0, 1, 0, 50, 0, 10000, 20000, 15000, 30000, 0, 11, 34397, 0, 0, 0, 0, 0, 19, 19421, 30, 0, 0, 0, 0, 0, 0, '20139 - Ooc - Cast Red Beam'),
(20248, 0, 5, 0, 1, 0, 50, 0, 10000, 20000, 15000, 30000, 0, 11, 34397, 0, 0, 0, 0, 0, 19, 19421, 30, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 10000 and 20000 ms (and later repeats every 15000 and 30000 ms) (50% chance) - Self: Cast spell Red Beam (34397) on Closest alive creature Netherstorm Crystal Target (19421) in 30 yards. 20248 - Ooc - Cast Red Beam'),
(20409, 0, 1, 0, 9, 0, 100, 0, 10, 35, 0, 0, 0, 11, 36099, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kirin''Var Apprentice - Within 10-35 Range - Cast ''Throw Hammer'''),
(20607, 0, 2, 0, 2, 0, 30, 0, 0, 15, 0, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Craghide Basilisk - Between 0-15% Health - Cast ''Feign Death'''),
(20673, 0, 1, 0, 0, 0, 100, 0, 2000, 5000, 4000, 6000, 0, 11, 36594, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Swiftwing Shredder - IC - Cast Lightning Breath'),
(20769, 0, 1, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 11, 35515, 0, 0, 0, 0, 0, 9, 20454, 0, 200, 0, 0, 0, 0, 0, 'Salaadin''s Energy Ball - Out of Combat - Cast ''Salaadin''s Tesla'''),
(20928, 0, 4, 0, 0, 0, 100, 0, 1000, 1000, 7000, 9000, 0, 11, 37179, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironspine Forgelord - In Combat - Cast ''Hammer Slam'''),
(20928, 0, 5, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 33962, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ironspine Forgelord - On Aggro - Cast ''Toughen'' (No Repeat)');
