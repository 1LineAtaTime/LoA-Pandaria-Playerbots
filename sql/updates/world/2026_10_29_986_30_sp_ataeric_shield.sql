-- ported from PR 15 2026_10_27_44_world.sql (#986)
-- Transdimensional Warfare: Chapter III (27518): overload Archmage Ataeric's shield by killing Arcane Fiends near him
-- Ataeric's old script ran on a timer: 50 seconds after the pull it gave the credit, whatever the player did. His
-- mop_world template has the stunned flag (retail has none), so his untriggered Arcane Shield, Dimensional Beam and Summon
-- Arcane Fiend casts all failed and he could be killed before the timer ran out, with no credit.
-- The quest objective ("Overload Archmage Ataeric with arcane power to bring down the Transdimensional Shield"), his boss
-- whisper "Destroy Arcane Fiends near Archmage Ataeric to overload the shield!" and the retail Arcane Fiend script ("On
-- Just Died - Cast Arcane Expulsion") describe the fight; the steps below are ours: he stands shielded and beaming, summons
-- an Arcane Fiend every 10 seconds, and each fiend that dies within 15 yards of him strains the shield. His lines mark the
-- five steps; at the fifth the shield breaks and he dies, which credits every player within 100 yards. The casts are
-- triggered so the stunned flag does not stop them.

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (2120,45766);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (2120,45766) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=2120000 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2120,0,0,1,25,0,100,0,0,0,0,0,0,11,87251,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Reset - Cast Arcane Shield'),
(2120,0,1,0,61,0,100,0,0,0,0,0,0,11,85373,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Reset - Cast Dimensional Beam'),
(2120,0,2,3,4,0,100,0,0,0,0,0,0,21,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Aggro - Disable Combat Movement'),
(2120,0,3,4,61,0,100,0,0,0,0,0,0,20,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Aggro - Disable Melee'),
(2120,0,4,5,61,0,100,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Aggro - Say Line 1'),
(2120,0,5,0,61,0,100,0,0,0,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Aggro - Say Line 0 (boss whisper to the player who pulled him)'),
(2120,0,6,0,0,0,100,0,1000,1000,10000,10000,0,11,87253,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - In Combat - Cast Summon Arcane Fiend'),
(2120,0,7,0,38,0,100,0,1,1,0,0,0,63,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Data Set 1 1 - Add 1 to Counter 1'),
(2120,0,8,0,77,0,100,0,1,1,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Counter 1 = 1 - Say Line 2'),
(2120,0,9,0,77,0,100,0,1,2,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Counter 1 = 2 - Say Line 3'),
(2120,0,10,0,77,0,100,0,1,3,0,0,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Counter 1 = 3 - Say Line 4'),
(2120,0,11,0,77,0,100,0,1,4,0,0,0,1,5,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Counter 1 = 4 - Say Line 5'),
(2120,0,12,0,77,0,100,0,1,5,0,0,0,80,2120000,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Counter 1 = 5 - Run Script'),
(2120,0,13,0,6,0,100,0,0,0,0,0,0,33,45769,0,0,0,0,0,17,0,100,0,0,0,0,0,0,'Archmage Ataeric - On Death - Quest Credit Transdimensional Warfare: Chapter III'),
(2120,0,14,0,7,0,100,0,0,0,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - On Evade - Despawn'),
(2120000,9,0,0,0,0,100,0,0,0,0,0,0,1,6,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - Script - Say Line 6'),
(2120000,9,1,0,0,0,100,0,3000,3000,0,0,0,28,87251,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - Script - Remove Arcane Shield'),
(2120000,9,2,0,0,0,100,0,0,0,0,0,0,28,85373,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - Script - Remove Dimensional Beam'),
(2120000,9,3,0,0,0,100,0,2000,2000,0,0,0,1,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - Script - Say Line 7'),
(2120000,9,4,0,0,0,100,0,2000,2000,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Archmage Ataeric - Script - Die'),
(45766,0,0,1,6,0,100,0,0,0,0,0,0,11,87249,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Arcane Fiend - On Death - Cast Arcane Expulsion'),
(45766,0,1,0,61,0,100,0,0,0,0,0,0,45,1,1,0,0,0,0,19,2120,15,0,0,0,0,0,0,'Arcane Fiend - On Death - Set Data 1 1 on Archmage Ataeric within 15 yards');
