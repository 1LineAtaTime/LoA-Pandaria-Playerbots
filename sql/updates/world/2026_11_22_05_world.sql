-- Azuremyst Isle, The Emitter (9312): the turn-in starts the hologram scene.
-- Problem: Technician Zhanaa (17071) has the timed action list 1707100 (activate the emitter 181286, summon the Image of
--   Technician Dyvuun 16550 for 39 s, Dyvuun talks, Zhanaa answers) and the image has its three lines, but no event
--   runs the list: nothing happens at the turn-in.
-- Change: On Quest Reward 9312 -> Call Timed Action List 1707100 (invoker = the player).
-- Source: reward text of quest 9312 ("I'll just activate this last crystal ... someone is appearing!").
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=17071 AND `source_type`=0 AND `id`=1;
DELETE FROM `smart_scripts` WHERE `entryorguid`=17071 AND `source_type`=0 AND `id`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(17071,0,1,0,20,0,100,0,9312,0,0,0,0,80,1707100,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Technician Zhanaa - On Quest ''The Emitter'' Rewarded - Run Script');
