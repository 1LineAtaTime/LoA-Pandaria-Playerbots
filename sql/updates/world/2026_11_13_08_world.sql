-- Western Plaguelands, Andorhal after the second battle (phase 191, 2026_11_13_07_world.sql): Andorhal Deathguards hold the
-- western, formerly Alliance part of the town, and the bodies of Alexis Torry, Foreman Milos and Benjamin Henderson lie on the
-- path by the inn beside Lurid's.
--
-- Sourced:
--   Wowhead comments: Alexis Torry 47719 (2011-04-01 [+7]) "she doesn't make it out after the Horde overrun Andorhal. Her body is
--     on the path by the inn, next to that of Foreman Milos and Benjamin Henderson"; Lurid 45129 (2011-04-22 [+6]) "Lurid's
--     corpse, among other Alliance corpses, will be found near the Inn"; Thassarian 44453 (2011-03-22 [+10]) "Andorhal is taken
--     by the Forsaken. All NPCs are dead". warcraft.wiki.gg (Andorhal): Alliance civilians found dead in their part of the city,
--     Andorhal Deathguards patrolling it.
--   Positions: TDB 4.4.2 (12.1.0 has the same rows). Its Andorhal batch 327891-327898 holds Lurid's corpse 327891 with a second
--     copy of each of the three townsfolk away from their stalls: 327893 Alexis Torry (1395.39, -1250.97), 327897 Foreman Milos
--     (1408.33, -1284.47), 327898 Benjamin Henderson (1390.83, -1318.68). Alexis Torry's only Wowhead pin (38.8, 68.6) is that
--     spot (1400, -1252), not her stall, and the comment's author saw her only after the chain; Benjamin Henderson's second pin
--     (40.2, 68.8) = (1394, -1312) is 7 yd from 327898. The sniff's Deathguards 327857, 327933 and 327934 stand inside the
--     Alliance camp (327933 3 yd from Benjamin Henderson's stall, 327934 4 yd from Foreman Milos's) and 328034 in the first
--     battle's Alliance camp.
-- Ours:
--   That these spots are the after-battle state. The TDB batches mix states: 327891-327898 also holds the living Lurid and
--     Lang Loosegrip on their first-battle camp spots, 327933/327934 stand beside the living townsfolk's stalls, and 328034 sits
--     with 328030-328033 in the first battle's Alliance camp. The positions are sniffed; which state they belong to is our
--     inference (bodies away from the stalls, Alexis Torry's lone pin, Deathguards that cannot stand in a living Alliance camp).
--     mop's battle Deathguards 212914, 212925 and 213032 are the sniff's 327933, 327934 and 327857 (7-13 yd off, wander 3); in
--     phase 261 they stood among Thassarian's camp (212925 1 yd from Foreman Milos, 21 yd from Lang Loosegrip) and could pull
--     Alliance players handing in quests there. They move to the sniffed spots in phase 191; the battle keeps 63 Deathguards
--     (Supporting the Troops 27198 needs 15). 328034 is a new spawn (3127500).
--   The bodies: the townsfolk's own entries in phase 180 (Lurid's corpse: Horde from Ace in the Hole 27089 taken, Alliance from
--     Alas, Andorhal 27206 handed in), lying with Permanent Feign Death 29266 like Lurid's corpse, models as their living
--     spawns. A per-spawn SmartAI clears their vendor and repair flags on every reset (the core restores the template's npcflag on
--     respawn), so a body opens no vendor window and shows no tracking icon. The living spawns keep no rows (entry SmartAI with
--     no entry rows does nothing).
--   Respawn: 300 s like the living townsfolk and Lurid's corpse; the Deathguards keep their 60 s and 3 yd wander.

-- Deathguards holding the town
UPDATE `creature` SET `phaseId`=191,`phaseGroup`=0,`position_x`=1364.20,`position_y`=-1310.48,`position_z`=54.41,`orientation`=0.192 WHERE `guid`=212914 AND `id`=44447; -- TDB 327933
UPDATE `creature` SET `phaseId`=191,`phaseGroup`=0,`position_x`=1396.50,`position_y`=-1282.66,`position_z`=60.28,`orientation`=1.941 WHERE `guid`=212925 AND `id`=44447; -- TDB 327934
UPDATE `creature` SET `phaseId`=191,`phaseGroup`=0,`position_x`=1334.17,`position_y`=-1257.06,`position_z`=59.50,`orientation`=1.694 WHERE `guid`=213032 AND `id`=44447; -- TDB 327857

DELETE FROM `creature` WHERE `guid` BETWEEN 3127500 AND 3127503;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 3127500 AND 3127503;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3127500,44447,0,28,193,1,1,191,0,33872,0,1328.41,-1365.09,47.63,3.147,60,0,3,0,1226,1236,1,0,0,0,0,0,'',0,0), -- TDB 4.4.2 guid 328034
(3127501,47719,0,28,193,1,1,180,0,35787,0,1395.39,-1250.97,61.57,1.484,300,0,0,0,3678,3849,0,0,0,0,0,0,'',0,0), -- Alexis Torry's body, TDB 327893
(3127502,47721,0,28,193,1,1,180,0,35790,0,1408.33,-1284.47,61.55,1.763,300,0,0,0,4185,0,0,0,0,0,0,0,'',0,0), -- Foreman Milos's body, TDB 327897
(3127503,47717,0,28,193,1,1,180,0,35785,0,1390.83,-1318.68,58.40,1.414,300,0,0,0,4008,0,0,0,0,0,0,0,'',0,0); -- Benjamin Henderson's body, TDB 327898
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3127501,0,0,0,0,0,0,1,0,0,0,0,0,0,'29266'),
(3127502,0,0,0,0,0,0,1,0,0,0,0,0,0,'29266'),
(3127503,0,0,0,0,0,0,1,0,0,0,0,0,0,'29266');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (47717,47719,47721);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-3127501,-3127502,-3127503) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-3127501,0,0,0,25,0,100,0,0,0,0,0,0,81,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Alexis Torry (body by the inn) - On Reset - Clear npcflags'),
(-3127502,0,0,0,25,0,100,0,0,0,0,0,0,81,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Foreman Milos (body by the inn) - On Reset - Clear npcflags'),
(-3127503,0,0,0,25,0,100,0,0,0,0,0,0,81,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Benjamin Henderson (body by the inn) - On Reset - Clear npcflags');
