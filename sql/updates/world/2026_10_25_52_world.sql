-- Rise, Forsaken (27097) and No Escape (27099): nothing summoned Agatha, raised the Hillsbrad Refugees for the Forsaken raised
-- credit or played the meeting in Fenris Keep that gives the Human Leaders found credit. Ported from TrinityCore: Sylvanas and
-- Bond of the Val'kyr give the player Agatha until No Escape is turned in, she raises the refugees the player kills and, on No
-- Escape, leads the player into the meeting (areatrigger 6230), played by the Fenris Keep Camera, then flies them back.

-- Lady Sylvanas Windrunner gives and dismisses Agatha
UPDATE `creature_template` SET `ScriptName` = 'npc_silverpine_sylvanas_windrunner_high_command' WHERE `entry` = 44365;

-- Bond of the Val'kyr, from Rise, Forsaken until No Escape is turned in
DELETE FROM `spell_script_names` WHERE `spell_id` = 83979;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83979, 'spell_silverpine_bond_of_the_valkyr');

DELETE FROM `spell_area` WHERE `spell` = 83979;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(83979, 130, 27097, 27099, 0, 0, 2, 1, 74, 11);

-- Agatha Broadcast reaches only Agatha
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 83978;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83978, 0, 0, 31, 0, 3, 44951, 0, 0, 0, 0, '', 'Agatha Broadcast - Target Agatha');

-- Agatha: the player's guardian; the sniffed copy goes, since Summon Agatha brings her
UPDATE `creature_template` SET `unit_class` = 2, `HoverHeight` = 2.2, `AIName` = '', `ScriptName` = 'npc_silverpine_agatha_fenris_isle' WHERE `entry` = 44951;

DELETE FROM `creature_template_addon` WHERE `entry` = 44951;
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(44951, 0, 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 3, '85451');

DELETE FROM `creature` WHERE `guid` = 375615;
DELETE FROM `creature_addon` WHERE `guid` = 375615;

DELETE FROM `creature_text` WHERE `CreatureID` = 44951;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44951, 0, 0, 'Death is only the beginning.', 12, 0, 100, 0, 0, 0, 0, 45090, 5, 'Agatha - Broadcast'),
(44951, 0, 1, 'Different master, same insatiable thirst for power.', 12, 0, 100, 0, 0, 0, 0, 45093, 5, 'Agatha - Broadcast'),
(44951, 0, 2, 'Do you feel it, $n? The darkness surrounds us.', 12, 0, 100, 0, 0, 0, 0, 45097, 5, 'Agatha - Broadcast'),
(44951, 0, 3, 'I long for the frozen wastes of Northrend.', 12, 0, 100, 0, 0, 0, 0, 45096, 5, 'Agatha - Broadcast'),
(44951, 0, 4, 'I sense doom in your future, $n.', 12, 0, 100, 0, 0, 0, 0, 45094, 5, 'Agatha - Broadcast'),
(44951, 0, 5, 'Life is meaningless. It is in the afterlife that we are truly tested.', 12, 0, 100, 0, 0, 0, 0, 45098, 5, 'Agatha - Broadcast'),
(44951, 0, 6, 'The warmth of this place sickens me.', 12, 0, 100, 0, 0, 0, 0, 45095, 5, 'Agatha - Broadcast'),
(44951, 0, 7, 'Through me the Banshee Queen sees all...', 12, 0, 100, 0, 0, 0, 0, 45099, 5, 'Agatha - Broadcast'),
(44951, 0, 8, 'Waste no time, $n. We mustn''t keep the Banshee Queen waiting.', 12, 0, 100, 0, 0, 0, 0, 45092, 5, 'Agatha - Broadcast'),
(44951, 1, 0, 'Bow before your new master!', 14, 0, 100, 0, 0, 0, 0, 45114, 5, 'Agatha - Rise, Forsaken'),
(44951, 1, 1, 'Rise and destroy our enemies!', 14, 0, 100, 0, 0, 0, 0, 45115, 5, 'Agatha - Rise, Forsaken'),
(44951, 1, 2, 'Rise, $n! Become Forsaken!', 14, 0, 100, 0, 0, 0, 0, 45091, 5, 'Agatha - Rise, Forsaken'),
(44951, 2, 0, 'This way, $n. We will take them by surprise.', 12, 0, 100, 1, 0, 0, 0, 45193, 5, 'Agatha - No Escape'),
(44951, 3, 0, 'Run...', 12, 0, 100, 457, 0, 0, 0, 45184, 5, 'Agatha - No Escape'),
(44951, 4, 0, 'RUN!', 12, 0, 100, 457, 0, 0, 0, 45185, 5, 'Agatha - No Escape');

-- Agatha flies the player from Fenris Keep back to Forsaken High Command
DELETE FROM `waypoint_data` WHERE `id` = 449510;
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`, `action`, `action_chance`, `wpguid`) VALUES
(449510, 0, 978.75, 682.571, 74.9817, 0, 0, 1, 0, 100, 0),
(449510, 1, 979.691, 672.536, 74.9817, 0, 0, 1, 0, 100, 0),
(449510, 2, 984.573, 670.793, 75.4262, 0, 0, 1, 0, 100, 0),
(449510, 3, 997.752, 671.819, 69.7039, 0, 0, 1, 0, 100, 0),
(449510, 4, 998.248, 683.655, 70.2039, 0, 0, 1, 0, 100, 0),
(449510, 5, 1012.42, 684.455, 65.1761, 0, 0, 1, 0, 100, 0),
(449510, 6, 1011.69, 695.212, 60.9817, 0, 0, 1, 0, 100, 0),
(449510, 7, 996.165, 694.927, 60.9817, 0, 0, 1, 0, 100, 0),
(449510, 8, 995.677, 709.804, 60.9817, 0, 0, 1, 0, 100, 0),
(449510, 9, 982.948, 711.526, 60.9817, 0, 0, 1, 0, 100, 0),
(449510, 10, 983.938, 689.569, 60.2873, 0, 0, 1, 0, 100, 0),
(449510, 11, 961.075, 688.958, 60.2873, 0, 0, 1, 0, 100, 0),
(449510, 12, 933.366, 687.901, 70.0651, 0, 0, 1, 0, 100, 0),
(449510, 13, 871.441, 700.913, 91.8428, 0, 1200, 1, 0, 100, 0),
(449510, 14, 843.257, 730.168, 99.0837, 0, 0, 1, 0, 100, 0),
(449510, 15, 891.191, 859.995, 99.0837, 0, 0, 1, 0, 100, 0),
(449510, 16, 1167.72, 992.175, 99.0837, 0, 0, 1, 0, 100, 0),
(449510, 17, 1343, 1029.57, 62.7226, 0, 0, 1, 0, 100, 0),
(449510, 18, 1367.74, 1034.35, 54.1671, 0, 0, 1, 0, 100, 0);

-- Hillsbrad Refugees are raised by Agatha when the player kills them
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_hillsbrad_refugee' WHERE `entry` IN (44954, 44966);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44954, 44966) AND `source_type` = 0;

DELETE FROM `spell_script_names` WHERE `spell_id` = 83997;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83997, 'spell_silverpine_forsaken_trooper_masterscript_fenris_isle');

-- Forsaken Troopers (raised refugees)
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_forsaken_trooper_fenris_isle' WHERE `entry` BETWEEN 44958 AND 44965;

-- The spawned troopers are raised refugees caught by the sniffer (they stand on refugee spawn points, and the template is immune to
-- all, as a raised one is): they only exist for a few seconds after Agatha raises a refugee
DELETE FROM `creature` WHERE `guid` IN (307389, 307390, 307391, 307392, 307393, 307394, 307395, 307398, 307399, 307400, 307402, 307404,
307405, 307406, 307407, 312154, 312155, 312156, 312157, 312158, 312159) AND `id` BETWEEN 44958 AND 44965;

DELETE FROM `creature_text` WHERE `CreatureID` BETWEEN 44958 AND 44965;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44958, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44958, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44958, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44958, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44958, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44959, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44959, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44959, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44959, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44959, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44960, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44960, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44960, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44960, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44960, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44961, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44961, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44961, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44961, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44961, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44962, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44962, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44962, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44962, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44962, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44963, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44963, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44963, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44963, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44963, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44964, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44964, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44964, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44964, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44964, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised'),
(44965, 0, 0, 'I am Forsaken.', 12, 0, 100, 16, 0, 0, 0, 45106, 0, 'Forsaken Trooper - Raised'),
(44965, 0, 1, 'For the glory of the Dark Lady!', 12, 0, 100, 16, 0, 0, 0, 45107, 0, 'Forsaken Trooper - Raised'),
(44965, 0, 2, 'Wha... What has happened?', 12, 0, 100, 16, 0, 0, 0, 45108, 0, 'Forsaken Trooper - Raised'),
(44965, 0, 3, 'I am eternal... I am death.', 12, 0, 100, 16, 0, 0, 0, 45109, 0, 'Forsaken Trooper - Raised'),
(44965, 0, 4, 'I LIVE!', 12, 0, 100, 16, 0, 0, 0, 45111, 0, 'Forsaken Trooper - Raised');

-- Worgen Sentry
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_worgen_sentry' WHERE `entry` = 44987;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 44987 AND `source_type` = 0;

DELETE FROM `creature_text` WHERE `CreatureID` = 44987;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44987, 0, 0, 'Hurry, my lord! The enemy is upon us!', 14, 0, 100, 0, 0, 0, 0, 45128, 0, 'Worgen Sentry - Death'),
(44987, 0, 1, 'They come!', 14, 0, 100, 0, 0, 0, 0, 45127, 0, 'Worgen Sentry - Death');

-- No Escape starts on entering Fenris Keep with Agatha
DELETE FROM `areatrigger_scripts` WHERE `entry` = 6230;
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES
(6230, 'at_silverpine_no_escape');

-- The Fenris Keep Camera plays the meeting; its actors
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_fenris_keep_camera' WHERE `entry` = 45003;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_fenris_keep_actor' WHERE `entry` IN (44989, 44990, 44993, 44994, 44995, 44996, 44997, 45002);

DELETE FROM `creature_text` WHERE `CreatureID` IN (44989, 44996);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44989, 0, 0, 'The Forsaken have broken through your defenses, magistrate.', 12, 0, 100, 396, 0, 0, 0, 45161, 0, 'Lord Darius Crowley - No Escape'),
(44989, 1, 0, 'You are out of time.', 12, 0, 100, 274, 0, 0, 0, 45162, 0, 'Lord Darius Crowley - No Escape'),
(44989, 2, 0, 'A decision must be made!', 12, 0, 100, 5, 0, 0, 0, 45163, 0, 'Lord Darius Crowley - No Escape'),
(44989, 3, 0, 'Die in battle and be raised as a servant of the Forsaken or...', 12, 0, 100, 396, 0, 0, 0, 45164, 0, 'Lord Darius Crowley - No Escape'),
(44989, 4, 0, 'Drink in my blood and be reborn as worgen, immune to the depravity of the Forsaken.', 12, 0, 100, 5, 0, 0, 0, 45165, 0, 'Lord Darius Crowley - No Escape'),
(44989, 5, 0, 'Help us destroy the Forsaken and retake Lordaeron for the Alliance!', 12, 0, 100, 5, 0, 0, 0, 45166, 0, 'Lord Darius Crowley - No Escape'),
(44996, 0, 0, 'We would rather die than be turned into worgen, but seeing as how even death provides no relief from the atrocities of this war...', 12, 0, 100, 1, 0, 0, 0, 45167, 0, 'Magistrate Henry Maleb - No Escape'),
(44996, 1, 0, 'We choose vengeance!', 12, 0, 100, 5, 0, 0, 0, 45168, 0, 'Magistrate Henry Maleb - No Escape');
