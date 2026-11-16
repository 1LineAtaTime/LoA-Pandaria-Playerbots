-- The Pride of Kezan (25066): the Gnomeregan Stealth Fighters (39039) fly long waypoint loops out over the
-- sea. Creatures only update while a player is near their grid cells, so each fighter flew out of the
-- active area and froze at a cell edge, 150+ yd from the bomber's patrol area; the player in the bomber
-- found almost nothing to shoot. Keep them active so they stay on their paths.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 39039 AND `AIName` = '' AND `ScriptName` = '';
DELETE FROM `smart_scripts` WHERE `entryorguid` = 39039 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(39039,0,0,0,37,0,100,0,0,0,0,0,48,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Gnomeregan Stealth Fighter - On AI Init - Set Active'),
(39039,0,1,0,11,0,100,0,0,0,0,0,48,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Gnomeregan Stealth Fighter - On Respawn - Set Active');
