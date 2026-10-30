-- ported from PR 15 2026_10_27_64_world.sql (#986)
-- On Whose Orders? (27364) starts by itself (Wowhead, warcraft.wiki.gg) when the player takes on Master Sergeant Pietro Zaren,
-- who carries the orders: 5.4.8 has Start Quest: Master Sergeant Zaren (84870), which nothing cast, and both world databases have
-- the quest auto-accepted and auto-completed (flags 589824), here 0. Have the player cast it when Zaren is pulled (the trigger is
-- ours); the quest's prerequisites still apply. Forward Commander Onslaught still offers it, as on retail.
UPDATE `quest_template` SET `Flags` = 589824 WHERE `ID` = 27364;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45405 AND `source_type` = 0 AND `id` = 3;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45405, 0, 3, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 85, 84870, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Master Sergeant Pietro Zaren - On Aggro - Invoker Cast Start Quest: Master Sergeant Zaren');
