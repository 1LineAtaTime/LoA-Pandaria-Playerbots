-- Vilebranch Kidnapper (14748): its AI and lines.
-- Sourced: TDB 4.4.2 and 1210 smart_scripts (Execute 7160 at the start of combat, flee for help at 15 %, a line on aggro) and creature_text
-- (broadcast texts 10003-10005, present in mop). The entry had no AI and no texts, and no per-spawn SmartAI exists for it.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=14748;
DELETE FROM `smart_scripts` WHERE `entryorguid`=14748 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(14748,0,0,0,0,0,100,1,0,0,0,0,0,11,7160,1,0,0,0,0,2,0,0,0,0,0,0,0,0,'Vilebranch Kidnapper - In Combat - Cast Execute (No Repeat)'),
(14748,0,1,0,2,0,100,1,0,15,0,0,0,25,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'Vilebranch Kidnapper - Between 0-15% Health - Flee For Assist (No Repeat)'),
(14748,0,2,0,4,0,100,0,0,0,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,0,'Vilebranch Kidnapper - On Aggro - Say Line 0');
DELETE FROM `creature_text` WHERE `CreatureID`=14748;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(14748,0,0,'Another fool falls for the trap!',12,0,100,0,0,0,0,10003,0,'Vilebranch Kidnapper'),
(14748,0,1,'Dinner has arrived!',12,0,100,0,0,0,0,10004,0,'Vilebranch Kidnapper'),
(14748,0,2,'The High Priestess will feast on your bones!',12,0,100,0,0,0,0,10005,0,'Vilebranch Kidnapper');
