-- Western Plaguelands: Spirit Healers at the Chillwind Camp and Central graveyards.
-- Sourced: `game_graveyard_zone` sends ghosts in zone 28 to WorldSafeLocs 509 (Chillwind Camp, Alliance) and 1286 (Central GY), but no
-- Spirit Healer 6491 stood at either; the other three graveyards have one. Both TDBs (4.4.2 and 1210) spawn 6491 at both: guids 259335
-- (902.236, -1517, 55.037) and 259341 (1849.51, -2142.72, 68.1751), next to the WorldSafeLocs points (908.3, -1520.3) and (1839.8, -2149.7).
-- Ours: the new guids, and mop's other WPL healers' values for the rest (phaseMask 65535, 600 s respawn, 4120 health). The terrain at both
-- spots matches the TDB's z. A Pilgrim's Bounty Wild Turkey (event 41) stands on each spot; they are single event spawns, not doubles, and
-- stay.
DELETE FROM `creature` WHERE `guid` IN (3127200,3127201);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3127200,6491,0,28,3197,1,65535,0,0,0,0,902.236,-1517,55.037,4.744,600,0,0,0,4120,0,0,0,0,0,0,0,'',0,0),
(3127201,6491,0,28,28,1,65535,0,0,0,0,1849.51,-2142.72,68.1751,3.99197,600,0,0,0,4120,0,0,0,0,0,0,0,'',0,0);
