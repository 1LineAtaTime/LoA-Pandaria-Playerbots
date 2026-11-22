-- Ashenvale, Rescue the Fallen (13613): a rescued Wounded Mor'shan Defender (33266) speaks a line.
-- Problem: the defender gets up silently; a video of the quest shows each rescued defender saying "Death to the
--   Alliance!" or "For the Horde!".
-- Change: creature_text 33266 group 0 (the four broadcast texts of this group, random) and a linked Talk in the SmartAI
--   chain after the salve (id 1 -> id 3 -> id 2). Health and cast time stay as they are.
-- Source: 5.4.8 broadcast texts 33419-33422; YouTube 'Tauren Quest 127: Rescue the Fallen' (7sadJmjlBs4, 0:25, 0:45).
-- Rollback:
--   UPDATE `smart_scripts` SET `link`=2 WHERE `entryorguid`=33266 AND `source_type`=0 AND `id`=1;
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=33266 AND `source_type`=0 AND `id`=3;
--   DELETE FROM `creature_text` WHERE `CreatureID`=33266 AND `GroupID`=0;
DELETE FROM `creature_text` WHERE `CreatureID`=33266 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(33266,0,0,'Blood and thunder, $r!',12,0,100,0,0,0,0,33419,0,'Wounded Mor''shan Defender - rescued'),
(33266,0,1,'For the Horde!',12,0,100,0,0,0,0,33420,0,'Wounded Mor''shan Defender - rescued'),
(33266,0,2,'Death to the Alliance!',12,0,100,0,0,0,0,33421,0,'Wounded Mor''shan Defender - rescued'),
(33266,0,3,'Many thanks, $c!',12,0,100,0,0,0,0,33422,0,'Wounded Mor''shan Defender - rescued');
DELETE FROM `smart_scripts` WHERE `entryorguid`=33266 AND `source_type`=0 AND `id`=3;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(33266,0,3,2,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Wounded Mor''shan Defender - Linked - Say Line 0 to the rescuer');
UPDATE `smart_scripts` SET `link`=3 WHERE `entryorguid`=33266 AND `source_type`=0 AND `id`=1;
