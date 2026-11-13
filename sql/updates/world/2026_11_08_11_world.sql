-- Arathi Highlands: Kovork (2603) was spawned twice at Boulderfist Outpost
-- The retail TDB 1210 and the Cataclysm Classic TDB 4.4.2 both have one Kovork, at -1177.74 -1964.52, which is
-- the sniffed spawn 160692. Remove the old copy 11658, 7 yd away (nothing references it).
DELETE FROM `creature` WHERE `guid`=11658 AND `id`=2603;
