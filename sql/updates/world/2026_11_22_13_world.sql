-- Dun Morogh: Vagash only in his cave; Covert Operatives without a parachute.
-- Problem 1: Vagash (1388, target of Protecting the Herd, 314) has two spawns: in the cave of Amberstill Ranch (170363)
--   and on the hill west of it (170252).
-- Problem 2: three Covert Operatives (41335; spawns 170310, 170311, 170318) carry aura 77404 'Gnomish VLD Parachute'
--   (feather fall + parachute look) permanently and walk on the ground with it; the aura belongs to a landing.
-- Change: remove spawn 170252 (no addon, pool, formation, game event or linked respawn row); clear the aura on the
--   three operatives.
-- Source: SkyFire 5.4.8 world database (one Vagash spawn, in the cave at -5393 -1278; no aura on these operatives);
--   5.4.8 spell data 77404.
-- Rollback:
--   UPDATE `creature_addon` SET `auras`='77404' WHERE `guid` IN (170310,170311,170318);
--   INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
--   (170252,1388,0,1,803,1,1,0,0,1050,0,-5486.46,-1273.96,446.163,4.07587,300,0,3,0,352,0,1,0,0,0,0,0,'',0,0);
DELETE FROM `creature` WHERE `guid`=170252 AND `id`=1388;
UPDATE `creature_addon` SET `auras`=NULL WHERE `guid` IN (170310,170311,170318) AND `auras`='77404';
