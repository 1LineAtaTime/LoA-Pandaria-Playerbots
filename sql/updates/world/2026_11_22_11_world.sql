-- Gavin Gnarltree (225, Duskwood): add the missing "On Respawn - Start Waypoint" SmartAI row so he walks his route.
-- Problem: smart_scripts has six rows for entry 225 (ids 1 to 6: pause and emote at waypoints 1, 6 and 10), but the row
--   that starts the walk, id 0, is missing. The template uses SmartAI, the spawn has MovementType 0 and the route
--   (waypoints entry 225, 10 points) exists, but nothing ever starts it, so he stands still.
-- Change: insert smart_scripts 225/0/0: event On Respawn (11), action Start Waypoint (53) on path 225, repeating,
--   walking. No other row, route or spawn is changed.
-- Source: the sniff-based Cataclysm Preservation world database has the same seven rows (ids 0 to 6), an identical
--   10-point route and the same AIName and spawn MovementType 0.
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=225 AND `source_type`=0 AND `id`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=225 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(225,0,0,0,11,0,100,0,0,0,0,0,0,53,0,225,1,0,0,0,1,0,0,0,0,0,0,0,0,'Gavin Gnarltree - On Respawn - Start Waypoint');
