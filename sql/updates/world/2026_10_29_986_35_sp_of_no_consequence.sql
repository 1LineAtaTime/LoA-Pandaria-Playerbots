-- ported from PR 15 2026_10_27_49_world.sql (#986)
-- Of No Consequence (27547): the Cowering Troopers' spellclick cast Squirrel Scrubber Vehicle Aura (90102), which sets a
-- vehicle id on the player who clicks; their script gave the credit instead. Use the quest's own spell, Quest Credit:
-- Of No Consequence (85580, kill credit 45910 on the clicker), and keep the script for the line and the despawn.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=45910;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(45910,85580,2,0);
DELETE FROM `smart_scripts` WHERE `entryorguid`=45910 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45910,0,0,0,11,0,100,0,0,0,0,0,0,18,768,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Cowering Trooper - On Respawn - Set Immune Flags'),
(45910,0,1,2,73,0,100,0,0,0,0,0,0,83,16777216,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Cowering Trooper - On Spellclick - Remove Spellclick Flag'),
(45910,0,2,3,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Cowering Trooper - On Spellclick - Say Line 0'),
(45910,0,3,0,61,0,100,0,0,0,0,0,0,41,2000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Cowering Trooper - On Spellclick - Despawn');
