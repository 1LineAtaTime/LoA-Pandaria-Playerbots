-- Breaking the Keystone (26042): the Keystone 2688 summons Thenan 2763 when the quest is handed in, and Thenan attacks that player; Thenan gets Thundering Stomp.
-- Sourced: the summon (entry, spot, timed-or-dead 60 s) from TDB 442/1210 smart_scripts 2688; the event from warcraft.wiki.gg and Wowhead comments ("When turned in an elemental lvl 29 spawns");
-- Thundering Stomp 80835 (0-8 yd, every 15-25 s) from TDB 442/1210 smart_scripts 2763 and Wowhead's Abilities tab.
-- Sourced too: event_phase_mask 0 (both TDBs; mop had 1, which a GameObject script, always in phase 0, never matches in this core's IsInPhase).
-- Ours, as this core reads the row: quest 26042 instead of the retired Classic quest 652 (also wrong in both TDBs); action_param4 attackInvoker = 1, so Thenan attacks the
-- player who handed in (the TDBs have param4 0 and param5 4; param5 is attackScriptOwner here and does nothing for a GameObject owner).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2688 AND `source_type` = 1 AND `id` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2688,1,0,0,20,0,100,0,26042,0,0,0,0,12,2763,1,60000,1,0,0,8,0,0,0,0,-1517.45,-2181.3,17.3127,2.72271,'Keystone - On Quest ''Breaking the Keystone'' Rewarded - Summon Creature ''Thenan'' (attacks the player)');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2763 AND `source_type` = 0 AND `id` = 1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2763,0,1,0,9,0,100,0,0,8,15000,25000,0,11,80835,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Thenan - Within 0-8 Range - Cast ''Thundering Stomp''');
