-- Ongo'longo's Revenge (26367 Horde, 26515 Alliance): Ongo'longo breaks free at low health instead of dying
-- Sourced: the objective "Ongo'longo Enraged" is the kill credit of 42815; Wowhead comments: "bring him down to 1HP (it won't
-- drop any lower) and you'll get credit", "We don't kill him, just hit him till he breaks free and runs off"; his two yells
-- (broadcast 42750 "PUNY TROLLS NOT STOP ONGOLONGO!", 42749 "NO MORE! ONGOLONGO GO HOME!", quoted as his on npc 42815's page);
-- spell 79875 "Ongolongo Smash!" (knock back, 5.4.8 DBC).
-- Ours: which yell goes with the pull and which with the break, the 1 % threshold, the smash on his victim, the credit (the
-- tapper's group and every player on his threat list), and the exit: he stops fighting, cannot be attacked, and is gone after
-- 2 s (SmartAI hides him then and removes him 5 s later; he respawns after his 60 s). A reset makes him attackable again.
UPDATE `creature_template` SET `AIName`='SmartAI', `ScriptName`='' WHERE `entry`=42815;

DELETE FROM `creature_text` WHERE `CreatureID`=42815;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(42815,0,0,'PUNY TROLLS NOT STOP ONGOLONGO!',14,0,100,0,0,0,0,42750,0,'Ongo''longo - On Aggro'),
(42815,1,0,'NO MORE! ONGOLONGO GO HOME!',14,0,100,0,0,0,0,42749,0,'Ongo''longo - Breaks free');

DELETE FROM `smart_scripts` WHERE `entryorguid`=42815 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42815,0,0,1,25,0,100,0,0,0,0,0,0,42,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - On Reset - Set Invincibility 1 HP'),
(42815,0,1,2,61,0,100,0,0,0,0,0,0,8,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set React Aggressive'),
(42815,0,2,0,61,0,100,0,0,0,0,0,0,19,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Remove Flag Immune To Players'),
(42815,0,3,0,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - On Aggro - Say Line 0'),
(42815,0,4,5,2,0,100,1,0,1,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Between 0-1% Health - Say Line 1 (No Repeat)'),
(42815,0,5,6,61,0,100,0,0,0,0,0,0,11,79875,2,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Cast ''Ongolongo Smash!'' on Victim'),
(42815,0,6,7,61,0,100,0,0,0,0,0,0,33,42815,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Quest Credit ''Ongo''longo Enraged'' (loot recipient and group)'),
(42815,0,7,8,61,0,100,0,0,0,0,0,0,33,42815,0,0,0,0,0,24,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Quest Credit ''Ongo''longo Enraged'' (threat list)'),
(42815,0,8,9,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set React Passive'),
(42815,0,9,10,61,0,100,0,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Combat Stop'),
(42815,0,10,11,61,0,100,0,0,0,0,0,0,18,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set Flag Immune To Players'),
(42815,0,11,0,61,0,100,0,0,0,0,0,0,41,2000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Despawn In 2 s');
