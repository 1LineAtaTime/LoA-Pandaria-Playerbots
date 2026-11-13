-- ported from PR 15 2026_10_25_50_world.sql (#986)
-- Waiting to Exsanguinate (27045): nothing summoned Deathstalker Rane Yorick at the Abandoned Outhouse, the armoire could not be
-- used and the meeting between Lord Darius Crowley and Packleader Ivar Bloodfang never played, so Learn of Darius Crowley's Plans
-- could not be had. Ported from TrinityCore: the outhouse summons Yorick on accept or through its gossip option
-- (go_silverpine_abandoned_outhouse), Yorick leads the way to the armoire and hides (npc_silverpine_deathstalker_rane_yorick), and
-- clicking the armoire hides the player inside it for the scene, which ends with Kill Credit Yorick (npc_silverpine_armoire,
-- npc_silverpine_lord_darius_crowley_exsanguinate, npc_silverpine_packleader_ivar_bloodfang_exsanguinate).
UPDATE `gameobject_template` SET `AIName` = '', `ScriptName` = 'go_silverpine_abandoned_outhouse' WHERE `entry` = 205143;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_deathstalker_rane_yorick' WHERE `entry` = 44882;
UPDATE `creature_template` SET `AIName` = '', `unit_flags` = 33554440, `VehicleId` = 1055, `ScriptName` = 'npc_silverpine_armoire' WHERE `entry` = 44893;
UPDATE `creature_template` SET `AIName` = '', `unit_flags` = 32770, `ScriptName` = 'npc_silverpine_lord_darius_crowley_exsanguinate' WHERE `entry` = 44883;
UPDATE `creature_template` SET `AIName` = '', `unit_flags` = 32770, `VehicleId` = 1059, `ScriptName` = 'npc_silverpine_packleader_ivar_bloodfang_exsanguinate' WHERE `entry` = 44884;

-- Dangerous Intentions (27039) is started by Deathstalker Commander Belmont and only ended at the outhouse
DELETE FROM `gameobject_queststarter` WHERE `id` = 205143 AND `quest` = 27039;

-- The sniffed Yorick hiding next to the armoire: the quest summons him
DELETE FROM `creature` WHERE `guid` = 375614 AND `id` = 44882;

-- Yorick's body (seen once the quest is complete) does not wander
UPDATE `creature` SET `wander_distance` = 0, `MovementType` = 0 WHERE `guid` = 306633 AND `id` = 44899;

-- Detect: Quest Invis Zone 2 shows the body after the quest (Kill Credit Yorick updates the area auras)
DELETE FROM `spell_area` WHERE `spell` = 83739 AND `area` = 239;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(83739, 239, 27045, 0, 0, 0, 2, 1, 66, 11);

-- The outhouse option, while the quest is in progress and Yorick is not out
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11897;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11897, 0, 0, 'Yorick, you in there? I''m ready to do this! Let''s go!', 44940, 1, 1, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11897 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11897, 0, 0, 0, 9, 0, 27045, 0, 0, 0, 0, 0, '', 'Abandoned Outhouse - Show option while Waiting to Exsanguinate is in progress'),
(15, 11897, 0, 0, 0, 1, 0, 83751, 1, 0, 1, 0, 0, '', 'Abandoned Outhouse - Show option while Deathstalker Rane Yorick is not summoned');

-- The armoire in the house summons the hiding spot while the quest is in progress
UPDATE `creature_template` SET `npcflag` = 16777216 WHERE `entry` = 44894;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 44894;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(44894, 83756, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 44894 AND `SourceEntry` = 83756;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 44894, 83756, 0, 0, 47, 0, 27045, 8, 0, 0, 0, 0, '', 'Armoire - Allow spellclick while Waiting to Exsanguinate is in progress');

-- Armoire Camera on Crowley and on Bloodfang
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (83763, 83764);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83763, 0, 0, 31, 0, 3, 44883, 0, 0, 0, 0, '', 'Armoire Camera - Target Lord Darius Crowley'),
(13, 1, 83764, 0, 0, 31, 0, 3, 44884, 0, 0, 0, 0, '', 'Armoire Camera - Target Packleader Ivar Bloodfang');

DELETE FROM `spell_script_names` WHERE `spell_id` = 83788;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83788, 'spell_silverpine_hide_in_armoire');

DELETE FROM `creature_text` WHERE `CreatureID` IN (44882, 44883, 44884);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44882, 0, 0, 'We haven''t got much time. Crowley will be arriving shortly. Let''s go!', 12, 0, 100, 5, 0, 0, 0, 44951, 5, 'Deathstalker Rane Yorick - Waiting to Exsanguinate'),
(44882, 1, 0, 'THERE! Hide in the armoire! I''ll hide in the shadows next to you.', 12, 0, 100, 25, 0, 0, 0, 44955, 5, 'Deathstalker Rane Yorick - Waiting to Exsanguinate'),
(44882, 2, 0, 'I live... and die... for the Banshee Queen.', 12, 0, 100, 0, 0, 0, 0, 44971, 5, 'Deathstalker Rane Yorick - Waiting to Exsanguinate'),
(44883, 0, 0, 'Have you given any more thought to my proposal, Ivar?', 12, 0, 100, 6, 0, 0, 0, 44957, 5, 'Lord Darius Crowley - Waiting to Exsanguinate'),
(44883, 1, 0, 'If we''re to win this war we will need your help. Our packs must unite! The Forsaken will destroy us otherwise.', 12, 0, 100, 396, 0, 0, 0, 44960, 5, 'Lord Darius Crowley - Waiting to Exsanguinate'),
(44883, 2, 0, 'You are the alpha male, Ivar. The rest of the ferals in Silverpine will do as you command.', 12, 0, 100, 397, 0, 0, 0, 44961, 5, 'Lord Darius Crowley - Waiting to Exsanguinate'),
(44883, 3, 0, 'So you will help?', 12, 0, 100, 6, 0, 0, 0, 44965, 5, 'Lord Darius Crowley - Waiting to Exsanguinate'),
(44884, 0, 0, 'Why trust you now, Crowley? You abandoned us. Left us to die.', 12, 0, 100, 6, 0, 0, 0, 44959, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 1, 0, 'I don''t care about your war, Crowley.', 12, 0, 100, 274, 0, 0, 0, 44962, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 2, 0, 'But...', 12, 0, 100, 1, 0, 0, 0, 44963, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 3, 0, 'I have seen firsthand what the Forsaken are capable of doing. It is true. They hunt us... slaughter the defenseless.', 12, 0, 100, 1, 0, 0, 0, 44964, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 4, 0, 'Aye, I will gather my pack... we...', 12, 0, 100, 1, 0, 0, 0, 44966, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 5, 0, '%s sniffs at the air.', 16, 0, 100, 479, 0, 0, 0, 44967, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 6, 0, 'It would appear that we are being watched, Crowley. You have grown soft... Likely Greymane''s fault.', 12, 0, 100, 1, 0, 0, 0, 44968, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 7, 0, 'Treacherous little pup!', 12, 0, 100, 15, 0, 0, 0, 44969, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 8, 0, 'What say you now, spy?', 12, 0, 100, 0, 0, 0, 0, 44970, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 9, 0, 'I was hoping you''d say that...', 12, 0, 100, 0, 0, 0, 0, 44972, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate'),
(44884, 10, 0, 'I will prepare the pack. It will take some time to gather them all, but we will join... for now.', 12, 0, 100, 1, 0, 0, 0, 44973, 5, 'Packleader Ivar Bloodfang - Waiting to Exsanguinate');

-- Yorick up to the armoire and into the shadows next to it; Crowley and Bloodfang in, around Yorick and out (Bloodfang runs out)
DELETE FROM `waypoint_data` WHERE `id` IN (448820, 448821, 448830, 448831, 448840, 448841, 448842, 448843);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`, `action`, `action_chance`, `wpguid`) VALUES
(448820, 0, 1300.27, 1190.11, 52.6001, 0, 0, 0, 0, 100, 0),
(448820, 1, 1288.52, 1196.86, 52.8501, 0, 0, 0, 0, 100, 0),
(448820, 2, 1287.52, 1205.11, 53.1001, 0, 0, 0, 0, 100, 0),
(448820, 3, 1294.27, 1207.36, 54.1001, 0, 0, 0, 0, 100, 0),
(448820, 4, 1299.27, 1207.86, 54.1001, 0, 0, 0, 0, 100, 0),
(448820, 5, 1304.27, 1210.36, 54.1001, 0, 0, 0, 0, 100, 0),
(448820, 6, 1304.77, 1217.86, 54.1001, 0, 0, 0, 0, 100, 0),
(448820, 7, 1298.52, 1221.36, 54.1001, 0, 0, 0, 0, 100, 0),
(448820, 8, 1298.27, 1218.86, 55.1001, 0, 0, 0, 0, 100, 0),
(448820, 9, 1298.02, 1216.61, 56.3501, 0, 0, 0, 0, 100, 0),
(448820, 10, 1297.27, 1212.86, 58.6001, 0, 0, 0, 0, 100, 0),
(448820, 11, 1298.27, 1206.86, 58.6001, 0, 0, 0, 0, 100, 0),
(448820, 12, 1307.27, 1206.11, 58.6001, 0, 0, 0, 0, 100, 0),
(448820, 13, 1311.27, 1205.86, 58.6001, 0, 0, 0, 0, 100, 0),
(448820, 14, 1312.67, 1208.86, 58.5123, 0, 0, 0, 0, 100, 0),
(448821, 0, 1313.435, 1210.425, 58.7561, 0, 0, 0, 0, 100, 0),
(448821, 1, 1313.7, 1211.99, 58.4999, 0, 0, 0, 0, 100, 0),
(448830, 0, 1299.36, 1206.64, 58.5706, 0, 0, 0, 0, 100, 0),
(448830, 1, 1300.36, 1206.64, 58.5706, 0, 0, 0, 0, 100, 0),
(448830, 2, 1302.33, 1206.44, 58.499, 0, 0, 0, 0, 100, 0),
(448830, 3, 1305.32, 1206.43, 58.5126, 0, 0, 0, 0, 100, 0),
(448830, 4, 1313.48, 1206.09, 58.5119, 0, 0, 0, 0, 100, 0),
(448831, 0, 1310.28, 1206.0249, 58.76205, 0, 0, 0, 0, 100, 0),
(448831, 1, 1305.58, 1206.46, 58.5122, 0, 0, 0, 0, 100, 0),
(448840, 0, 1296.2, 1210.54, 58.5533, 0, 0, 0, 0, 100, 0),
(448840, 1, 1297.2, 1210.54, 58.5533, 0, 0, 0, 0, 100, 0),
(448840, 2, 1302.31, 1206.61, 58.4984, 0, 0, 0, 0, 100, 0),
(448840, 3, 1308.4, 1206.28, 58.5109, 0, 0, 0, 0, 100, 0),
(448841, 0, 1312.585, 1209.135, 58.7603, 0, 0, 0, 0, 100, 0),
(448841, 1, 1313.18, 1210.32, 58.5093, 0, 0, 0, 0, 100, 0),
(448842, 0, 1311.25, 1208.875, 58.7602, 0, 0, 0, 0, 100, 0),
(448842, 1, 1309.32, 1206.43, 58.5111, 0, 0, 0, 0, 100, 0),
(448843, 0, 1304.2949, 1207.105, 58.760803, 0, 0, 1, 0, 100, 0),
(448843, 1, 1299.0449, 1209.605, 58.760803, 0, 0, 1, 0, 100, 0),
(448843, 2, 1297.27, 1212.28, 58.5105, 0, 0, 1, 0, 100, 0);
