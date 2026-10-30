-- ported from PR 15 2026_10_25_48_world.sql (#986)
-- The Warchief Cometh (26965): nothing played Garrosh's arrival or gave its credit, so the quest could not be completed. Grand
-- Executor Mortuus now plays the scene when it is accepted (npc_silverpine_grand_executor_mortuus, ported from TrinityCore, with
-- its texts, paths and Raise Forsaken setup). It summons Garrosh, his guards and Cromush, so the copies sniffed mid-scene go.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_grand_executor_mortuus' WHERE `entry` = 44615;
-- The Fallen Humans lie dead until Agatha raises them: the sniffed unit_flags2 has the feign death flag (1), mop's lacked it
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_fallen_human', `unit_flags2` = 2049 WHERE `entry` IN (44592, 44593);

DELETE FROM `creature` WHERE `guid` BETWEEN 307077 AND 307094;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 307077 AND 307094;
DELETE FROM `waypoint_data` WHERE `id` = 307093;

DELETE FROM `creature_text` WHERE `CreatureID` IN (44365, 44629, 44640);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44365, 0, 0, 'Where is that ogre-headed buffoon?', 12, 0, 100, 6, 0, 20459, 0, 44695, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 1, 0, 'Ah, speak of the devil...', 12, 0, 100, 1, 0, 20460, 0, 44696, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 2, 0, 'Warchief, so glad you could make it.', 12, 0, 100, 1, 0, 20461, 0, 44701, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 3, 0, 'With the death of the Lich King, many of the more intelligent Scourge became... unemployed. Those ''fiends,'' as you so delicately put it, are called val''kyr. They are under my command now...', 12, 0, 100, 0, 0, 20462, 0, 44702, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 4, 0, '...and they are part of the reason that I asked to see you.', 12, 0, 100, 1, 0, 20463, 0, 44703, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 5, 0, 'Very well, Warchief. I have solved the plight of the Forsaken!', 12, 0, 100, 5, 0, 20464, 0, 44705, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 6, 0, 'As a race, we Forsaken are unable to procreate.', 12, 0, 100, 274, 0, 20465, 0, 44706, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 7, 0, 'With the aid of the val''kyr, we are now able to take the corpses of the fallen and create new Forsaken.', 12, 0, 100, 0, 0, 20466, 0, 44707, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 8, 0, 'Agatha, show the Warchief!', 12, 0, 100, 5, 0, 20467, 0, 44709, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 9, 0, 'Warchief, without these new Forsaken my people would die out... Our hold upon Gilneas and northern Lordaeron would crumble.', 12, 0, 100, 0, 0, 20468, 0, 44715, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44365, 10, 0, 'Isn''t it obvious, Warchief? I serve the Horde.', 12, 0, 100, 66, 0, 20469, 0, 44718, 0, 'Lady Sylvanas Windrunner - The Warchief Cometh'),
(44629, 0, 0, 'This better be important, Sylvanas. You know how I detest this place and its foul stench. Why have you called for me?', 12, 0, 100, 0, 0, 20496, 0, 44699, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 1, 0, 'And more importantly, what are those Scourge fiends doing here?', 12, 0, 100, 25, 0, 20497, 0, 44700, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 2, 0, 'Get on with it, Sylvanas.', 12, 0, 100, 1, 0, 20498, 0, 44704, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 3, 0, 'What you have done here, Sylvanas... it goes against the laws of nature. Disgusting is the only word I have to describe it.', 12, 0, 100, 0, 0, 20499, 0, 44714, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 4, 0, 'Have you given any thought to what this means, Sylvanas?', 12, 0, 100, 6, 0, 20500, 0, 44716, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 5, 0, 'What difference is there between you and the Lich King now?', 12, 0, 100, 6, 0, 20501, 0, 44717, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 6, 0, 'Watch your clever mouth.', 12, 0, 100, 397, 0, 20502, 0, 44719, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 7, 0, 'Cromush, you stay behind and make sure the Banshee Queen is well "guarded." I will be expecting a full report when next we meet.', 12, 0, 100, 0, 0, 20503, 0, 44720, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44629, 8, 0, 'Remember, Sylvanas, eventually we all have to stand before our maker and face judgment. Your day may come sooner than others...', 12, 0, 100, 0, 0, 20504, 0, 44721, 0, 'Garrosh Hellscream - The Warchief Cometh'),
(44640, 0, 0, 'ABBERATION!', 12, 0, 100, 5, 0, 0, 0, 44713, 0, 'High Warlord Cromush - The Warchief Cometh'),
(44640, 1, 0, 'As you command, Warchief.', 12, 0, 100, 66, 0, 0, 0, 44738, 0, 'High Warlord Cromush - The Warchief Cometh');

-- Garrosh walks back to the portal, Cromush walks in from the Apothecarium
DELETE FROM `waypoint_data` WHERE `id` IN (446290, 446402);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`, `action`, `action_chance`, `wpguid`) VALUES
(446290, 0, 1377.69, 1048.67, 53.4456, 0, 0, 0, 0, 100, 0),
(446290, 1, 1373.69, 1052.17, 53.4456, 0, 0, 0, 0, 100, 0),
(446290, 2, 1365.23, 1058.61, 53.0524, 0, 0, 0, 0, 100, 0),
(446402, 0, 1412.05, 1089.42, 60.4771, 0, 0, 1, 0, 100, 0),
(446402, 1, 1405.38, 1095.16, 60.4774, 0, 0, 1, 0, 100, 0),
(446402, 2, 1392.62, 1093.5, 56.4067, 0, 0, 1, 0, 100, 0),
(446402, 3, 1380.65, 1083.27, 52.6221, 0, 0, 1, 0, 100, 0),
(446402, 4, 1372.86, 1062, 53.0398, 0, 0, 1, 0, 100, 0),
(446402, 5, 1376.69, 1048.61, 53.3362, 0, 0, 1, 0, 100, 0),
(446402, 6, 1375.32, 1046.38, 53.2336, 0, 0, 1, 0, 100, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (83173, 83149);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83173, 'spell_silverpine_raise_forsaken_83173'),
(83149, 'spell_silverpine_forsaken_trooper_masterscript_high_command');

-- Raise Forsaken only takes the Fallen Humans
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 83173;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83173, 0, 0, 31, 0, 3, 44592, 0, 0, 0, 0, '', 'Raise Forsaken - Target Fallen Human (Male)'),
(13, 1, 83173, 0, 1, 31, 0, 3, 44593, 0, 0, 0, 0, '', 'Raise Forsaken - Target Fallen Human (Female)');
