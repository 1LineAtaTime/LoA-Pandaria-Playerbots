-- Brewmaster Blanche (59569) had no spawn, so "Blanche's Boomer Brew" (30567, live in 5.4.8, she is its only giver and ender) could
-- not be started. Position from the SkyFire 5.4.8 database (ProjectSkyfire/database R23.004), which matches Wowhead's
-- Thunderpaw Overlook location in the Jade Forest (about 37, 30); suggested by quietpatcher.
-- Undo: DELETE FROM `creature` WHERE `guid` = 4049101;
DELETE FROM `creature` WHERE `guid` = 4049101;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(4049101,59569,870,5785,0,1,1,0,0,2215.19,-1166.03,448.864,4.83031,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0);

-- Apothecary Karlov (29346) was a patch 3.0.2 pre-event NPC in Orgrimmar and was removed later (Wowpedia: Apothecary Karlov,
-- Desperate Research). The holiday Commoners still start "Desperate Research" (12783 troll, 12811 orc), which leads to the
-- "Orgrimmar" quests (12785, 12812) that only Karlov ends, so a player could pick up a chain that can never be finished.
-- Undo: DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` IN (12783, 12785, 12811, 12812);
DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` IN (12783, 12785, 12811, 12812);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,12783,0,'','','Deprecated quest: Desperate Research (troll), ends at removed Apothecary Karlov'),
(1,12785,0,'','','Deprecated quest: Orgrimmar (troll), ender Apothecary Karlov removed'),
(1,12811,0,'','','Deprecated quest: Desperate Research (orc), ends at removed Apothecary Karlov'),
(1,12812,0,'','','Deprecated quest: Orgrimmar (orc), ender Apothecary Karlov removed');
