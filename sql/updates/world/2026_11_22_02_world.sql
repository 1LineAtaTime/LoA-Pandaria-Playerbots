-- Northern Barrens, Verog the Dervish (3395, quest 851): only Wyneth's death calls him, and he fights.
-- Problem: Kolkar Pack Runner (3274), Kolkar Marauder (3275) and Kolkar Bloodcharger (3397) still have the old 3% death
--   summon ("I am slain! Summon Verog!") that puts Verog at his tent; his path starts at the foot of the hill, so he
--   runs down and back up and evades. The waypoint start of his path (smart_scripts 3395 id 2) sets react state 0, so he
--   stays passive and never attacks, also when Wyneth (34846) summons him at the foot of the hill.
-- Change: remove the Kolkar summon rows and their death text; Start Waypoint keeps Verog aggressive (action_param6 2).
-- Source: Warcraft Wiki, Verog the Dervish (quest 851): "Kill Wyneth ... Verog appears and attacks the adventurer".
-- Rollback:
--   UPDATE `smart_scripts` SET `action_param6`=0 WHERE `entryorguid`=3395 AND `source_type`=0 AND `id`=2 AND `action_type`=53;
--   INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
--   (3274,0,0,'I am slain!  Summon Verog!',14,0,100,0,0,0,0,1017,0,'Kolkar Pack Runner'),
--   (3275,0,0,'I am slain! Summon Verog!',12,0,100,0,0,0,0,1017,0,'death Text'),
--   (3397,0,0,'I am slain!  Summon Verog!',14,0,100,0,0,0,0,1017,0,'Kolkar Bloodcharger');
--   INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
--   (3274,0,1,2,6,0,3,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kolkar Pack Runner - On Just Died - Say Line 0'),
--   (3274,0,2,0,61,0,100,0,0,0,0,0,0,12,3395,4,60000,0,0,0,8,0,0,0,0,-1209.65,-2738.38,102.646,4.99352,'Kolkar Pack Runner - On Just Died - Summon Creature ''Verog the Dervish'''),
--   (3275,0,1,2,6,0,3,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kolkar Marauder - On Just Died - Say Line 0'),
--   (3275,0,2,0,61,0,100,0,0,0,0,0,0,12,3395,4,60000,0,0,0,8,0,0,0,0,-1209.65,-2738.38,102.646,4.99352,'Kolkar Marauder - On Just Died - Summon Creature ''Verog the Dervish'''),
--   (3397,0,2,3,6,0,3,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kolkar Bloodcharger - On Just Died - Say Line 0'),
--   (3397,0,3,0,61,0,100,0,0,0,0,0,0,12,3395,4,60000,0,0,0,8,0,0,0,0,-1209.65,-2738.38,102.646,4.99352,'Kolkar Bloodcharger - On Just Died - Summon Creature ''Verog the Dervish''');
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid` IN (3274,3275) AND `id` IN (1,2)) OR (`entryorguid`=3397 AND `id` IN (2,3)));
DELETE FROM `creature_text` WHERE `CreatureID` IN (3274,3275,3397) AND `GroupID`=0;
UPDATE `smart_scripts` SET `action_param6`=2 WHERE `entryorguid`=3395 AND `source_type`=0 AND `id`=2 AND `action_type`=53;
