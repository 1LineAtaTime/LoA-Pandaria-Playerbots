-- #726: Scarlet Halls (1001) was disabled on both difficulties and Scholomance (1007) heroic was blocked
-- (disables flags 2 + access_requirement level_min 99). The MoP 5.4.8 data has no
-- disables row for either map and level_min 90 for Scholomance heroic. Player::Satisfy (Player.cpp:20759 level,
-- :20775 disables) refused entry. Port the MoP 5.4.8 values. DisableMgr and access_requirement load at startup.

DELETE FROM `disables` WHERE `sourceType` = 2 AND `entry` IN (1001, 1007);
UPDATE `access_requirement` SET `level_min` = 90 WHERE `mapId` = 1007 AND `difficulty` = 'DUNGEON_HEROIC';
