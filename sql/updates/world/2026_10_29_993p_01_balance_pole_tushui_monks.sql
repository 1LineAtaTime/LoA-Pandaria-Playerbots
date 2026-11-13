-- Refs #993. 29663 "The Lesson of the Balanced Rock": objective "Defeat Tushui Monks" x6 (creature 55019).
-- Cause (data): no Tushui Monk spawns (55019 or 65468) in world, the reference 5.4.8 world DB, fresh world DBs or acore (WotLK).
-- Source: the 14 sniffed Tushui Monk 65468 spawns on the Balance Poles of the Pool of the Reflection (map 860, zone 5736, area 5826)
-- from destroller22/RenegadosWoW (a 5.4.8 core) sql/FSociety/world/2018_06_09_00_Creature [65468] Tushui Monk.sql, guids
-- 21000637-21000859 there. Our 65468 template already has KillCredit1 55019 and the same SAI as 55019 (credit at 1% hp,
-- faction 35, Throw Rock), so these spawns give the objective as on retail. Replaces the lane's first 8 hand-placed 55019 rows.
-- Rollback: DELETE FROM creature WHERE guid BETWEEN 4049001 AND 4049014;
DELETE FROM `creature` WHERE `guid` BETWEEN 4049001 AND 4049014;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(4049001,65468,860,5736,5826,1,1,0,0,902.495,3179.36,108.355,5.21853,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049002,65468,860,5736,5826,1,1,0,0,919.953,3205.84,108.355,0.0698132,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049003,65468,860,5736,5826,1,1,0,0,895.915,3245.04,115.898,2.96706,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049004,65468,860,5736,5826,1,1,0,0,924.604,3252.7,115.898,3.66519,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049005,65468,860,5736,5826,1,1,0,0,904.156,3264.56,115.898,0.314159,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049006,65468,860,5736,5826,1,1,0,0,917.606,3268.73,115.898,3.26377,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049007,65468,860,5736,5826,1,1,0,0,916.924,3292.9,116.366,0.0763623,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049008,65468,860,5736,5826,1,1,0,0,910.7,3314.94,116.605,4.86313,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049009,65468,860,5736,5826,1,1,0,0,915.382,3370.43,116.605,2.705,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049010,65468,860,5736,5826,1,1,0,0,953.771,3310.21,116.605,2.83129,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049011,65468,860,5736,5826,1,1,0,0,963.062,3354.55,116.605,2.28661,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049012,65468,860,5736,5826,1,1,0,0,970.226,3330.39,116.687,5.90745,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049013,65468,860,5736,5826,1,1,0,0,996.877,3305.76,116.605,0.0828024,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(4049014,65468,860,5736,5826,1,1,0,0,977.889,3291.9,116.874,0.163576,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0);
