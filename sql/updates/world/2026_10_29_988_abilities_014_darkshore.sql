-- #988 wave 2 (merge): Darkshore creature abilities from DeltaTho PR 15 file 2026_10_24_19_world.sql, ADDED to creatures that already have smart_scripts.
-- 12 creatures, 20 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2071:3 2165:3 2237:3 32890:1 32999:1 33043:9 33044:9 33262:4 33277:4 34318:3 34369:1 34415:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2071 AND `id` >= 3)
  OR (`entryorguid` = 2165 AND `id` >= 3)
  OR (`entryorguid` = 2237 AND `id` >= 3)
  OR (`entryorguid` = 32890 AND `id` >= 1)
  OR (`entryorguid` = 32999 AND `id` >= 1)
  OR (`entryorguid` = 33043 AND `id` >= 9)
  OR (`entryorguid` = 33044 AND `id` >= 9)
  OR (`entryorguid` = 33262 AND `id` >= 4)
  OR (`entryorguid` = 33277 AND `id` >= 4)
  OR (`entryorguid` = 34318 AND `id` >= 3)
  OR (`entryorguid` = 34369 AND `id` >= 1)
  OR (`entryorguid` = 34415 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2071, 0, 3, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 8594, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Moonstalker Matriarch - Out of Combat - Cast ''Summon Moonstalker Runt'' (No Repeat)'),
(2071, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Moonstalker Matriarch - On Aggro - Cast ''Sprint'' (No Repeat)'),
(2165, 0, 3, 0, 0, 0, 100, 0, 5000, 5000, 7000, 10000, 0, 11, 3242, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grizzled Thistle Bear - In Combat - Cast ''Ravage'''),
(2165, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Grizzled Thistle Bear - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(2237, 0, 3, 0, 0, 0, 100, 0, 4000, 4500, 12000, 17000, 0, 11, 75008, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Moonstalker Sire - In Combat - Cast ''Pounce'''),
(2237, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Moonstalker Sire - On Aggro - Cast ''Sprint'' (No Repeat)'),
(32890, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 14000, 22000, 0, 11, 39207, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Enraged Tidal Spirit - In Combat - Cast ''Water Spout'''),
(32999, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75025, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Unbound Fire Elemental - On Aggro - Cast ''Rush of Flame'' (No Repeat)'),
(32999, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 9000, 15000, 0, 11, 13729, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Unbound Fire Elemental - In Combat - Cast ''Flame Shock'''),
(33043, 0, 9, 0, 0, 0, 100, 0, 2000, 4500, 30000, 35000, 0, 11, 75014, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Maddened Blackwood - In Combat - Cast ''Howl of Madness'''),
(33044, 0, 9, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Blackwood - In Combat - Cast ''Wrath'''),
(33044, 0, 10, 0, 0, 0, 100, 0, 4000, 5800, 12000, 14700, 0, 11, 15039, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Blackwood - In Combat - Cast ''Flame Shock'''),
(33262, 0, 4, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 86249, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Greymist Tidehunter - In Combat - Cast ''Throw'''),
(33277, 0, 4, 0, 9, 0, 100, 0, 0, 5, 25000, 29000, 0, 11, 3427, 0, 0, 0, 0, 0, 2, 32, 0, 0, 0, 0, 0, 0, 0, 'Greymist Warrior - Within 0-5 Range - Cast ''Infected Wound'''),
(33277, 0, 5, 0, 0, 0, 100, 0, 5000, 8000, 17000, 21000, 0, 11, 74972, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Greymist Warrior - In Combat - Cast ''Greymist'''),
(34318, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Whitetail Stag - On Aggro - Cast ''Charge'' (No Repeat)'),
(34318, 0, 4, 0, 0, 0, 100, 0, 2000, 4500, 12000, 15000, 0, 11, 31274, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Whitetail Stag - In Combat - Cast ''Knockdown'''),
(34369, 0, 1, 0, 0, 0, 100, 0, 3000, 4500, 17000, 23000, 0, 11, 75004, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hermit Tide Crawler - In Combat - Cast ''Pinch'''),
(34415, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9672, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkscale Priestess - In Combat - Cast ''Frostbolt'''),
(34415, 0, 2, 0, 9, 0, 100, 0, 0, 8, 11000, 16000, 0, 11, 75062, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darkscale Priestess - Within 0-8 Range - Cast ''Frost Nova''');
