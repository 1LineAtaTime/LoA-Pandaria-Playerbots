-- Northern Barrens, Love it or Limpet (14038): the Limpet Mine appears when the player plants it.
-- Problem: a fixed Limpet Mine spawn (34767, guid 265910, VerifiedBuild 0) stands on the hull of the Bellipotent all the
--   time, so the player sees it before planting and planting adds nothing. Plant Limpet Mine (66165) gives the credit
--   (effect 134, 34764) and sends event 21873, which has no script.
-- Change: remove the fixed spawn; event 21873 summons the Limpet Mine at the old spot for 60 s (the duration is our own
--   choice, no source). Spawn 265910 has no creature_addon, pool, formation, game event or linked respawn row.
-- Source: Spell.dbc 66165 (effects 61 event 21873 and 134); Wowhead MoP Classic npc 34767 (pins on the hull only).
-- Rollback:
--   DELETE FROM `event_scripts` WHERE `id`=21873 AND `command`=10 AND `datalong`=34767;
--   INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
--   (265910,34767,1,17,391,1,1,0,0,29402,0,-1457.18,-3970.81,-4.50941,0,300,0,0,0,42,0,0,0,0,0,0,0,'',0,0);
DELETE FROM `creature_addon` WHERE `guid`=265910;
DELETE FROM `creature` WHERE `guid`=265910 AND `id`=34767;
DELETE FROM `event_scripts` WHERE `id`=21873 AND `command`=10 AND `datalong`=34767;
INSERT INTO `event_scripts` (`id`,`delay`,`command`,`datalong`,`datalong2`,`dataint`,`x`,`y`,`z`,`o`) VALUES
(21873,0,10,34767,60000,0,-1457.18,-3970.81,-4.50941,0);
