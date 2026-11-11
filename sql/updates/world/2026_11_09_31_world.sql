-- Revantusk Drummers (14734): stand on their platform and drum.
-- Sourced: TDB 4.4.2 and 1210 spawn the three on these spots standing (MovementType 0) and give the entry SmartAI that plays emote 36
-- every 1.2 s out of combat. mop let them wander 3 yd on a platform 30 yd up, with no AI.
UPDATE `creature` SET `MovementType`=0,`wander_distance`=0 WHERE `id`=14734 AND `guid` IN (191194,191195,191196);
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=14734;
DELETE FROM `smart_scripts` WHERE `entryorguid`=14734 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(14734,0,0,0,1,0,100,0,1200,1200,1200,1200,0,5,36,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Revantusk Drummer - OOC - Play Emote');
