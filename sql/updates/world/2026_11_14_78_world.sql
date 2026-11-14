-- Eastern Plaguelands, Journey's End (27527): Gidwin's Hearthstone 61379 (Gidwin's Hearthstone 85609, 10 s cast, teleport to
-- TARGET_DEST_DB) takes the player into Light's Hope Chapel. It had no destination: the cast ended, nothing happened and the
-- stone was used up.
--
--  * Sourced: the destination, TDB 4.4.2 and 12.1 spell_target_position (map 0, 2294.56, -5334.66, 90.7523, build 25937);
--    "teleports you into Light's Hope Chapel, facing the dwarf" (item comments 2010-12-18, 2011-05-09 reply).
--  * Ours: the facing (the TDBs have none): 1.45, towards the caravan outside (2303, -5268), where Fiona takes the quest
--    (decisions R1; her copy is the caravan's). The chapel Gidwin of the comment is staged only from 27527 rewarded, so he is not
--    there while the stone is used.
--  * Owner: decisions D3 (this scope); the caravan scope's draft drops its copy.

DELETE FROM `spell_target_position` WHERE `id` = 85609;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(85609,0,0,2294.56,-5334.66,90.7523,1.45);
