-- Eastern Plaguelands, Borelgore (The Corpulent One, 27477): he walks the trench out and back instead of standing in a
-- 3-yd circle, and loses the Strike neither TDB gives him.
-- Sourced: the corridor (33 Wowhead pins of npc 11896, converted with WorldMapArea 23: x 2882..3011, y -4271..-4666),
--   "found wandering in a large trench", "patrolling from west to east and under a bridge" (comment 2011-06-28 [+8], wiki);
--   his kit (both TDBs: Trample, Venom Spit, Virulent Poison Proc; no Strike).
-- He drops 7-8 Slabs of Carrion Worm Meat for Gathering Some Grub(s) (27456) (Wowhead comment 2012-08-25 [+1] on item
--   13853; neither TDB has the row): quest drop at 100 % (the less grindy value).
-- Ours: the line and the heights (the trench floor read from the map terrain), the out-and-back shape. Neither TDB has a path.
UPDATE `creature` SET `position_x`=2975.0,`position_y`=-4300.0,`position_z`=79.56,`orientation`=4.71,`MovementType`=2,`wander_distance`=0 WHERE `guid`=175254 AND `id`=11896;
DELETE FROM `creature_addon` WHERE `guid`=175254;
INSERT INTO `creature_addon` (`guid`,`path_id`) VALUES (175254,175254);
DELETE FROM `waypoint_data` WHERE `id`=175254;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`) VALUES
(175254,1,2975.0,-4300.0,79.56),(175254,2,2978.0,-4330.0,78.44),(175254,3,2977.0,-4360.0,75.06),(175254,4,2979.0,-4390.0,93.19),
(175254,5,2963.0,-4420.0,95.43),(175254,6,2968.0,-4450.0,87.89),(175254,7,2971.0,-4480.0,75.70),(175254,8,2977.0,-4510.0,75.86),
(175254,9,2971.0,-4540.0,74.22),(175254,10,2955.0,-4570.0,74.04),(175254,11,2941.0,-4600.0,73.64),(175254,12,2921.0,-4630.0,79.10),
(175254,13,2894.0,-4660.0,82.75),(175254,14,2921.0,-4630.0,79.10),(175254,15,2941.0,-4600.0,73.64),(175254,16,2955.0,-4570.0,74.04),
(175254,17,2971.0,-4540.0,74.22),(175254,18,2977.0,-4510.0,75.86),(175254,19,2971.0,-4480.0,75.70),(175254,20,2968.0,-4450.0,87.89),
(175254,21,2963.0,-4420.0,95.43),(175254,22,2979.0,-4390.0,93.19),(175254,23,2977.0,-4360.0,75.06),(175254,24,2978.0,-4330.0,78.44);
DELETE FROM `smart_scripts` WHERE `entryorguid`=11896 AND `source_type`=0 AND `id`=0;
DELETE FROM `creature_loot_template` WHERE `entry`=11896 AND `item`=13853;
INSERT INTO `creature_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`groupid`,`mincountOrRef`,`maxcount`) VALUES (11896,13853,-100,0,7,8);
