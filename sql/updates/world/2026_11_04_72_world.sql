-- Green Living (28325): Used Blight Canisters (207089/207090) in the Ruins of Southshore as sniffed
-- (TDB 442.26081 and retail 1210.26091 agree: 16 spawns, rotation 0 0 0 1, 120 s respawn, animprogress 255)

-- Unsniffed spawns (nearest sniffed spot 24.7-59.1 yd away); 158279 was the doubled twin of 158278
DELETE FROM `gameobject` WHERE `guid` IN (157982, 157983, 157984, 158278, 158279, 158338) AND `id` IN (207089, 207090);

-- Spawns already on a sniffed spot: sniffed entry, position, rotation and respawn
UPDATE `gameobject` SET `id` = 207089, `position_x` = -762.714, `position_y` = -474.837, `position_z` = 18.2452 WHERE `guid` = 158322;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -776.064, `position_y` = -523.719, `position_z` = 20.1766 WHERE `guid` = 158293;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -771.457, `position_y` = -553.736, `position_z` = 18.404  WHERE `guid` = 158341;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -760.432, `position_y` = -562.814, `position_z` = 16.1275 WHERE `guid` = 158246;
UPDATE `gameobject` SET `id` = 207089, `position_x` = -685.425, `position_y` = -540.155, `position_z` = 28.2432 WHERE `guid` = 158321;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -732.559, `position_y` = -580.214, `position_z` = 23.0586 WHERE `guid` = 158340;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -745.984, `position_y` = -650.325, `position_z` = 14.6754 WHERE `guid` = 158329;
UPDATE `gameobject` SET `id` = 207090, `position_x` = -810.432, `position_y` = -663.031, `position_z` = 10.5714 WHERE `guid` = 157960;
UPDATE `gameobject` SET `id` = 207089, `position_x` = -805.939, `position_y` = -565.267, `position_z` = 13.2933 WHERE `guid` = 157976;
UPDATE `gameobject` SET `id` = 207089, `position_x` = -819.767, `position_y` = -656.128, `position_z` = 11.2554 WHERE `guid` = 157979;
UPDATE `gameobject` SET `map` = 0, `zoneId` = 267, `areaId` = 271, `spawnMask` = 1, `phaseMask` = 1, `phaseId` = 0, `phaseGroup` = 0,
  `orientation` = 0, `rotation0` = 0, `rotation1` = 0, `rotation2` = 0, `rotation3` = 1, `spawntimesecs` = 120, `animprogress` = 255, `state` = 1
  WHERE `guid` IN (158322, 158293, 158341, 158246, 158321, 158340, 158329, 157960, 157976, 157979) AND `id` IN (207089, 207090);

-- Sniffed spots that had no spawn
DELETE FROM `gameobject` WHERE `guid` BETWEEN 4100005 AND 4100010;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(4100005, 207090, 0, 267, 271, 1, 1, -717.09,  -484.318, 23.8712, 0, 0, 0, 0, 1, 120, 255, 1),
(4100006, 207090, 0, 267, 271, 1, 1, -754.589, -541.533, 17.935,  0, 0, 0, 0, 1, 120, 255, 1),
(4100007, 207090, 0, 267, 271, 1, 1, -774.925, -571.363, 19.8878, 0, 0, 0, 0, 1, 120, 255, 1),
(4100008, 207089, 0, 267, 271, 1, 1, -754.667, -587.733, 17.6687, 0, 0, 0, 0, 1, 120, 255, 1),
(4100009, 207090, 0, 267, 271, 1, 1, -764.009, -608.061, 14.5294, 0, 0, 0, 0, 1, 120, 255, 1),
(4100010, 207089, 0, 267, 271, 1, 1, -879.174, -504.109, 11.54,   0, 0, 0, 0, 1, 120, 255, 1);
