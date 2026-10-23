-- Quest 14071 "Rolling with my Homies": bind the scripts ported into
-- src/server/scripts/Maelstrom/zone_kezan.cpp and retire the placeholder SAI.
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = 'npc_original_hot_rod' WHERE `entry` = 34840;
--   UPDATE `creature_template` SET `ScriptName` = 'npc_friend_gob' WHERE `entry` IN (34957, 34958, 34959);
--   UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` IN (34890, 34892, 34954);
--   UPDATE `quest_template_addon` SET `ScriptName` = '' WHERE `ID` = 14071;
--   DELETE FROM `spell_script_names` WHERE `spell_id` = 66299 AND `ScriptName` = 'spell_radio';
--   and re-insert the three smart_scripts rows deleted below:
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (34890, 0, 0, 0, 10, 0, 100, 0, 1, 40, 1000, 1000, 0, 33, 34959, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Izzy'),
--   (34892, 0, 0, 0, 10, 0, 100, 0, 1, 40, 1000, 1000, 0, 33, 34957, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Ace'),
--   (34954, 0, 0, 0, 10, 0, 100, 0, 1, 40, 1000, 1000, 0, 33, 34958, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Gobber');

-- 34840 Hot Rod: the ported vehicle AI keeps running while driven (OnCharmed override),
-- so PassengerBoarded fires and the homies board.
UPDATE `creature_template` SET `ScriptName` = 'npc_hot_rod_rolling_my_homies' WHERE `entry` = 34840;

-- 34957/34958/34959 Ace/Gobber/Izzy summoned copies: give the objective credit and ride along.
UPDATE `creature_template` SET `ScriptName` = 'npc_homies_summon' WHERE `entry` IN (34957, 34958, 34959);

-- 34890/34892/34954 Izzy/Ace/Gobber in the world: replace the OOC-LOS kill-credit SAI
-- (which credited on foot and never put anyone in the car) with npc_homies.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_homies' WHERE `entry` IN (34890, 34892, 34954);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34890, 34892, 34954) AND `source_type` = 0;

-- Quest hook: grants the three invisibility detection auras on accept.
UPDATE `quest_template_addon` SET `ScriptName` = 'quest_rolling_with_my_homies' WHERE `ID` = 14071;

-- Hot Rod radio (creature_template.spell4 = 66299), no previous binding.
DELETE FROM `spell_script_names` WHERE `spell_id` = 66299;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(66299, 'spell_radio');
