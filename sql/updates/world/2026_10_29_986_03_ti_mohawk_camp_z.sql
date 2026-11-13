-- ported from PR 15 2026_10_23_02_world.sql (#986)
-- Night Elf Mohawk camp in Tirisfal Glades: put the pre-Cataclysm spawns back on the lowered terrain
UPDATE `creature` SET `position_z` = 34.299 WHERE `guid` = 62020; -- Night Elf Mohawk
UPDATE `creature` SET `position_z` = 33.745 WHERE `guid` = 83023; -- Mighty Frostsaber
UPDATE `gameobject` SET `position_z` = 34.596 WHERE `guid` = 9035; -- Night Elf Stool
UPDATE `gameobject` SET `position_z` = 34.000 WHERE `guid` = 9792; -- Night Elf Tent 02 x.8, sunk into the slope so its downhill edge touches the ground
