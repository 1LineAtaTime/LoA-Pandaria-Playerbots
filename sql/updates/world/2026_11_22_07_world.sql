-- Ironforge, Seize the Ambassador (26118): Princess Moira Thaurissan (42129) says her line when Ambassador Slaghammer
--   (42153) is delivered.
-- Problem: the ambassador gives the quest credit when he is within 10 yards of Moira (smart_scripts 42153 id 1), but
--   Moira says nothing.
-- Change: Moira's line as creature_text 42129 group 0; a linked TALK row 42153 id 2 with Moira (closest 42129 within
--   30 yards) as the talker. The same line under 42153 group 0 exists only so that SmartAIMgr accepts the TALK row of
--   42153; the ambassador never says it.
-- Source: 5.4.8 client data, broadcast text 42044; Wowpedia 'Seize the Ambassador' (same line).
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=42153 AND `source_type`=0 AND `id`=2;
--   UPDATE `smart_scripts` SET `link`=0 WHERE `entryorguid`=42153 AND `source_type`=0 AND `id`=1;
--   DELETE FROM `creature_text` WHERE `CreatureID` IN (42129,42153) AND `GroupID`=0;
DELETE FROM `creature_text` WHERE `CreatureID` IN (42129,42153) AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(42129,0,0,'If we were in Shadowforge City, I would have you tossed into the Ring of Law for your transgressions, Ambassador. The darkest dungeons of Ironforge will have to do for now.',12,0,100,0,0,0,0,42044,0,'Moira Thaurissan - arrest of Ambassador Slaghammer (quest 26118)'),
(42153,0,0,'If we were in Shadowforge City, I would have you tossed into the Ring of Law for your transgressions, Ambassador. The darkest dungeons of Ironforge will have to do for now.',12,0,100,0,0,0,0,42044,0,'Same line: only so that SmartAIMgr accepts the TALK row of 42153 (the talker is Moira, target 19); never spoken by the ambassador');
DELETE FROM `smart_scripts` WHERE `entryorguid`=42153 AND `source_type`=0 AND `id`=2;
UPDATE `smart_scripts` SET `link`=2 WHERE `entryorguid`=42153 AND `source_type`=0 AND `id`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42153,0,2,0,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,19,42129,30,0,0,0,0,0,0,'Ambassador Slaghammer - Linked - Moira Thaurissan says her line (broadcast text 42044)');
