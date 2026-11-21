-- Eastern Plaguelands, the Plaguewood: nine more Overstuffed Golems (27 -> 36, as both TDBs), the kill-and-burn target of
-- Scourged Mass (27528, 8 corpses). 19 TDB spots had no mop golem within 60 yd.
-- Sourced: positions, heights and facings (TDB 12.1.0 / 4.4.2, the nine spots farthest from any mop golem, 77-133 yd).
-- Ours: area 2277 (the nearest mop golem's; the TDB rows have 0) and mop's own spawn style for the entry (wander 3 yd,
-- 300 s; the TDB's 120 s is TrinityCore's default, not a sniffed value).
DELETE FROM `creature` WHERE `guid` BETWEEN 3129603 AND 3129611;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`MovementType`,`curhealth`) VALUES
(3129603,45851,0,139,2277,1,1,2874.15,-3679.70,115.663,6.2617,300,3,1,1),
(3129604,45851,0,139,2277,1,1,3204.48,-3754.31,146.964,3.6031,300,3,1,1),
(3129605,45851,0,139,2277,1,1,3054.70,-3493.35,131.702,4.7454,300,3,1,1),
(3129606,45851,0,139,2277,1,1,2946.01,-3282.52,117.241,2.8010,300,3,1,1),
(3129607,45851,0,139,2277,1,1,3149.20,-3358.57,158.782,2.3562,300,3,1,1),
(3129608,45851,0,139,2277,1,1,3193.51,-3839.28,130.329,0.5532,300,3,1,1),
(3129609,45851,0,139,2277,1,1,3282.29,-3748.86,150.426,2.1368,300,3,1,1),
(3129610,45851,0,139,2277,1,1,3178.87,-3047.15,143.059,1.9277,300,3,1,1),
(3129611,45851,0,139,2277,1,1,3022.88,-3275.01,143.033,3.5682,300,3,1,1);
-- Gibbering Ghoul 8531 casts Curse of Tongues (12889, enemies within 20 yd) in combat and Diseased Flayer 8532 casts Birth
-- (26047, 3 s emerge) on respawn: both TDBs' SmartAI ids 1 and 2, same ids here. Their id 0 (the Darrowshire spirit summon)
-- belongs to the finale commit (_74), which deletes only id 0, sets the same AIName and clears ScriptName npc_ghoul_flayer;
-- these rows delete only ids 1 / 2. Until _74 clears the ScriptName the C++ class runs instead and these rows wait.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (8531,8532);
DELETE FROM `smart_scripts` WHERE `entryorguid`=8531 AND `source_type`=0 AND `id`=1;
DELETE FROM `smart_scripts` WHERE `entryorguid`=8532 AND `source_type`=0 AND `id`=2;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(8531,0,1,0,0,0,100,0,6000,12000,22000,28000,0,11,12889,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Gibbering Ghoul - In Combat - Cast Curse of Tongues'),
(8532,0,2,0,11,0,100,0,0,0,0,0,0,11,26047,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Diseased Flayer - On Respawn - Cast Birth');
