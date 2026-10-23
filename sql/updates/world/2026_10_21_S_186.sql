-- Refs #186 (lane S audit 2026-10-21, build-83 DBErrors).
-- 1) the 14 TBC heroic access_requirement rows still name the old heroic keys; the loader already strips key requirements, so clearing
--    them is behaviour-neutral and silences the errors.
-- 2) guild_rewards 69209/69210 hold achievements='0' (parsed as achievement id 0).
-- Undo: UPDATE access_requirement SET item=30637,item2=30622 WHERE mapId IN (540,542,543) AND difficulty='DUNGEON_HEROIC';
--       ... item=30623 (545,546,547), 30634 (552,553,554), 30633 (555,556,557,558), 30635 (560);
--       UPDATE guild_rewards SET achievements='0' WHERE entry IN (69209,69210);
UPDATE `access_requirement` SET `item`=0, `item2`=0 WHERE `difficulty`='DUNGEON_HEROIC' AND `mapId` IN (540,542,543,545,546,547,552,553,554,555,556,557,558,560);
UPDATE `guild_rewards` SET `achievements`='' WHERE `entry` IN (69209,69210) AND `achievements`='0';
