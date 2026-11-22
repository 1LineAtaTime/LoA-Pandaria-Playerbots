-- Northern Barrens, Nugget Slugs (29027): Tool Buckets at the Burning Blade farm.
-- Problem: the quest asks for 10 Nugget Slugs "from buckets around the Burning Blade farm to the southwest", but all 29
--   Tool Buckets (161752, the only loot source of item 11143) stand at the Venture Co. sites; none at the farm.
-- Change: seven buckets on the Wowhead MoP Classic map pins of object 161752 in the farm cluster (zone 17,
--   41.8-43.0 / 27.4-32.6), converted to world coordinates with the 5.4.8 WorldMapArea.dbc; heights measured in game
--   (the point at 561.4 -2251.4 is under a tent roof, ground 92.04). Existing buckets are not touched.
-- Source: quest text 29027; Wowhead MoP Classic object 161752 (map pins); WorldMapArea.dbc.
-- Rollback:
--   DELETE FROM `gameobject` WHERE `guid` BETWEEN 4310001 AND 4310007 AND `id`=161752;
DELETE FROM `gameobject` WHERE `guid` BETWEEN 4310001 AND 4310007;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`VerifiedBuild`) VALUES
(4310001,161752,1,17,0,1,1,0,0,607.4,-2199.7,91.67,0,0,0,0,1,300,100,1,'',0),
(4310002,161752,1,17,0,1,1,0,0,576.8,-2211.2,91.67,0,0,0,0,1,300,100,1,'',0),
(4310003,161752,1,17,0,1,1,0,0,561.4,-2251.4,92.04,0,0,0,0,1,300,100,1,'',0),
(4310004,161752,1,17,0,1,1,0,0,760.7,-2257.1,92.17,0,0,0,0,1,300,100,1,'',0),
(4310005,161752,1,17,0,1,1,0,0,753,-2257.1,91.87,0,0,0,0,1,300,100,1,'',0),
(4310006,161752,1,17,0,1,1,0,0,630.4,-2268.6,91.67,0,0,0,0,1,300,100,1,'',0),
(4310007,161752,1,17,0,1,1,0,0,584.4,-2268.6,92.12,0,0,0,0,1,300,100,1,'',0);
