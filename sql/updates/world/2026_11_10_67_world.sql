-- Western Plaguelands, Hearthglen: a Hearthglen Trainee can be duelled again after it respawns (It's About Time! 27156).
-- The trainee's SmartAI (gossip 11943) sets faction 14 to fight; at 1 % health it adds UNIT_FLAG_IMMUNE_TO_PC (256), gives the credit and
-- despawns. The core respawns a despawned spawn as the same object and does not reset its unit flags (Creature::Respawn and the
-- JUST_RESPAWNED state set npcflags, not unit flags), so a respawned trainee stayed immune to players and could not be fought: with 16
-- trainees and 7 duels per quest, the quest ran dry until a restart. The faction is already restored on respawn.
-- Ours: on respawn the trainee removes the flag (new id 13; ids 0-12 are in use, and no spawn has its own script).
DELETE FROM `smart_scripts` WHERE `entryorguid`=45162 AND `source_type`=0 AND `id`=13;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45162,0,13,0,11,0,100,0,0,0,0,0,0,19,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Respawn - Remove Unit Flag Immune To PC');
