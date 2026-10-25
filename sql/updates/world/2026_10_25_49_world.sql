-- Iterating Upon Success (26998): Bat Handler Maggotbreath's "take a bat" option did nothing, so neither the bat credit nor the
-- murloc kills could be had. The option now summons a Forsaken Bat (npc_silverpine_bat_handler_maggotbreath, ported from
-- TrinityCore) that flies its rider to Lordamere Lake, circles it with Blight Concoction on the action bar and flies back to Forsaken
-- High Command once the 50 Vile Fin are slain or the rider picks Go Home (npc_silverpine_forsaken_bat, spell_silverpine_go_home).
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_bat_handler_maggotbreath' WHERE `entry` = 44825;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_forsaken_bat' WHERE `entry` = 44821;

-- The bat option only while the quest is in progress (the flight master option stays). Its npcflag was 0, which hides it.
UPDATE `gossip_menu_option` SET `OptionNpcflag` = 1 WHERE `MenuID` = 11892 AND `OptionID` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11892 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11892, 0, 0, 0, 9, 0, 26998, 0, 0, 0, 0, 0, '', 'Bat Handler Maggotbreath - Show bat option while Iterating Upon Success is in progress');

-- Blight Concoction only hits the Vile Fin Oracles and Tidehunters (the Tidehunters give Oracle kill credit)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 83575;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83575, 0, 1, 31, 0, 3, 1908, 0, 0, 0, 0, '', 'Blight Concoction - Target Vile Fin Oracle'),
(13, 1, 83575, 0, 2, 31, 0, 3, 1768, 0, 0, 0, 0, '', 'Blight Concoction - Target Vile Fin Tidehunter');

DELETE FROM `spell_script_names` WHERE `spell_id` = 83594;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83594, 'spell_silverpine_go_home');

DELETE FROM `creature_text` WHERE `CreatureID` IN (44825, 44821);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44825, 0, 0, 'You better bring that bat back in one piece, $n!', 12, 0, 100, 25, 0, 0, 0, 44906, 5, 'Bat Handler Maggotbreath - Iterating Upon Success'),
(44821, 0, 0, 'Eliminate all murlocs by using the Blight Concoction!$B$B|r|TInterface\\Icons\\INV_ALCHEMY_POTION_05.BLP:24|t', 42, 0, 100, 0, 0, 0, 0, 44902, 5, 'Forsaken Bat to Player'),
(44821, 1, 0, 'Returning to Forsaken High Command!$B$B|r|TInterface\\Icons\\ACHIEVEMENT_ZONE_SILVERPINE_01.BLP:24|t', 42, 0, 100, 0, 0, 0, 0, 44903, 5, 'Forsaken Bat to Player');

-- Forsaken High Command to the lake, around the lake (repeated), and back
DELETE FROM `waypoint_data` WHERE `id` IN (448210, 448211, 448212);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`, `action`, `action_chance`, `wpguid`) VALUES
(448210, 0, 1374.36, 1029.12, 67.8815, 0, 0, 1, 0, 100, 0),
(448210, 1, 1289.08, 1013.22, 67.8815, 0, 0, 1, 0, 100, 0),
(448210, 2, 1224, 997.236, 59.9371, 0, 0, 1, 0, 100, 0),
(448210, 3, 1178.72, 957.238, 47.2426, 0, 0, 1, 0, 100, 0),
(448210, 4, 1156.5, 852.318, 42.7704, 0, 0, 1, 0, 100, 0),
(448210, 5, 1158.1, 725.983, 42.7704, 0, 0, 1, 0, 100, 0),
(448210, 6, 1187.67, 575.212, 46.7704, 0, 0, 1, 0, 100, 0),
(448210, 7, 1202.33, 472.97, 65.4648, 0, 0, 1, 0, 100, 0),
(448211, 0, 1214.8, 414.792, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 1, 1221.35, 323.231, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 2, 1189.93, 281.372, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 3, 1100.24, 212.868, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 4, 1015.84, 188.21, 63.0412, 0, 0, 1, 0, 100, 0),
(448211, 5, 939.417, 163.743, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 6, 874.302, 114.092, 61.0968, 0, 0, 1, 0, 100, 0),
(448211, 7, 831.491, 59.3177, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 8, 770.872, 70.7726, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 9, 759.814, 109.738, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 10, 803.745, 181.882, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 11, 840.674, 202.097, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 12, 846.651, 232.344, 56.5968, 0, 0, 1, 0, 100, 0),
(448211, 13, 805.031, 261.606, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 14, 759.528, 265.283, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 15, 693.755, 282.512, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 16, 658.036, 357.233, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 17, 639.776, 433.974, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 18, 676.128, 470.752, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 19, 706.495, 454.023, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 20, 713.505, 380.88, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 21, 726.146, 352.188, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 22, 758.299, 321.101, 59.9579, 0, 0, 1, 0, 100, 0),
(448211, 23, 797.684, 295.753, 58.1801, 0, 0, 1, 0, 100, 0),
(448211, 24, 882.036, 252.597, 52.1245, 0, 0, 1, 0, 100, 0),
(448211, 25, 929.667, 225.609, 52.1245, 0, 0, 1, 0, 100, 0),
(448211, 26, 987.259, 219.582, 52.1245, 0, 0, 1, 0, 100, 0),
(448211, 27, 1030.56, 276.193, 52.1245, 0, 0, 1, 0, 100, 0),
(448211, 28, 1058.35, 308.229, 49.3745, 0, 0, 1, 0, 100, 0),
(448211, 29, 1121.81, 372.755, 56.4579, 0, 0, 1, 0, 100, 0),
(448211, 30, 1167.09, 438.865, 58.4857, 0, 0, 1, 0, 100, 0),
(448211, 31, 1197.37, 451.757, 64.569, 0, 0, 1, 0, 100, 0),
(448212, 0, 931.068, 352.101, 81.027, 0, 0, 1, 0, 100, 0),
(448212, 1, 952.535, 571.493, 108.303, 0, 0, 1, 0, 100, 0),
(448212, 2, 970.028, 664.392, 110.83, 0, 0, 1, 0, 100, 0),
(448212, 3, 1031.97, 754.155, 111.441, 0, 0, 1, 0, 100, 0),
(448212, 4, 1234.44, 880.16, 97.9414, 0, 0, 1, 0, 100, 0),
(448212, 5, 1345.66, 970.281, 75.1914, 0, 0, 1, 0, 100, 0),
(448212, 6, 1415.9, 1018.08, 55.3025, 0, 0, 1, 0, 100, 0);
