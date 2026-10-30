-- #1001: Unexpected Results (8488) the AC way. AC drives Apprentice Mirveda 15402 with SmartAI (acore_world.smart_scripts
-- 15402 + action lists 1540200-1540202): on accept she turns faction 232, drops IMMUNE_TO_NPC, enters phase 1 and summons
-- Gharsul + 2 Angershade that attack her (summon action_param5 attackScriptOwner); out of combat again in phase 1 she gives
-- the group credit and resets. Our C++ npc_apprentice_mirveda and the summons' waypoint rows are replaced 1:1 by AC's rows.
-- AC event_flags 512 (WHILE_CHARMED) is DIFFICULTY_RAID40 in our core, so it is written as 0 here.
-- Needs the SmartAI attackScriptOwner + SET_EVENT_FLAG_RESET (211) + invoker-less CALL_GROUPEVENTHAPPENS port in the same commit.
UPDATE `creature_template` SET `AIName`='SmartAI', `ScriptName`='' WHERE `entry`=15402;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (15402,15958,15656);
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid` IN (1540200,1540201,1540202);
DELETE FROM `waypoints` WHERE `entry` IN (15656,15958);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(15402,0,0,0,20,0,100,0,8487,0,0,0,0,80,1540200,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - On Quest ''Corrupted Soil'' Finished - Run Script'),
(15402,0,1,2,19,0,100,0,8488,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Apprentice Mirveda - On Quest ''Unexpected Results'' Taken - Store Targetlist'),
(15402,0,2,0,61,0,100,0,0,0,0,0,0,80,1540201,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - On Quest ''Unexpected Results'' Taken - Run Script'),
(15402,0,3,0,0,0,100,0,900,900,3000,4000,0,11,20811,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Apprentice Mirveda - In Combat - Cast ''Fireball'''),
(15402,0,4,0,6,1,100,0,0,0,0,0,0,6,8488,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Apprentice Mirveda - On Just Died - Fail Quest ''Unexpected Results'' (Phase 1)'),
(15402,0,5,0,11,0,100,0,0,0,0,0,0,18,512,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - On Respawn - Set Flags Immune To NPC''s'),
(15402,0,6,7,1,1,100,0,1000,1000,1000,1000,0,80,1540202,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Out of Combat - Run Script (Phase 1)'),
(15402,0,7,0,61,0,100,0,0,0,0,0,0,26,8488,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Apprentice Mirveda - Out of Combat - Quest Credit ''Unexpected Results'' (Phase 1)'),
(15402,0,8,0,37,0,100,0,0,0,0,0,0,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'Apprentice Mirveda - On Initialize - Flag reset 0'),
(15656,0,0,1,2,0,100,1,0,30,0,0,0,11,8599,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Angershade - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(15656,0,1,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Angershade - Between 0-30% Health - Say Line 0 (No Repeat)'),
(1540200,9,0,0,0,0,100,0,0,0,0,0,0,81,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Npc Flag '),
(1540200,9,1,0,0,0,100,0,10000,10000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Say Line 0'),
(1540200,9,2,0,0,0,100,0,0,0,0,0,0,81,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Npc Flags Questgiver'),
(1540201,9,0,0,0,0,100,0,0,0,0,0,0,81,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Npc Flag '),
(1540201,9,1,0,0,0,100,0,0,0,0,0,0,2,232,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Faction 232'),
(1540201,9,2,0,0,0,100,0,0,0,0,0,0,19,512,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Remove Flags Immune To NPC''s'),
(1540201,9,3,0,0,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Event Phase 1'),
(1540201,9,4,0,0,0,100,0,0,0,0,0,0,12,15958,4,15000,0,1,0,8,0,0,0,0,8750.1,-7129.7,35.2976,3.8041,'Apprentice Mirveda - Actionlist - Summon Creature ''Gharsul the Remorseless'''),
(1540201,9,5,0,0,0,100,0,0,0,0,0,0,12,15656,4,15000,0,1,0,8,0,0,0,0,8753.61,-7133.15,35,3.8576,'Apprentice Mirveda - Actionlist - Summon Creature ''Angershade'''),
(1540201,9,6,0,0,0,100,0,0,0,0,0,0,12,15656,4,15000,0,1,0,8,0,0,0,0,8747.14,-7125.71,35.848,3.8576,'Apprentice Mirveda - Actionlist - Summon Creature ''Angershade'''),
(1540202,9,0,0,0,0,100,0,0,0,0,0,0,81,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Npc Flags Questgiver'),
(1540202,9,1,0,0,0,100,0,0,0,0,0,0,2,1604,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Faction 1604'),
(1540202,9,2,0,0,0,100,0,0,0,0,0,0,18,512,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Flags Immune To NPC''s'),
(1540202,9,3,0,0,0,100,0,0,0,0,0,0,22,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Apprentice Mirveda - Actionlist - Set Event Phase 0');
