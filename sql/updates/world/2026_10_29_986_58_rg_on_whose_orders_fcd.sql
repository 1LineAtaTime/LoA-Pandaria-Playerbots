-- ported from PR 15 2026_10_27_74_world.sql (#986)
-- On Whose Orders? (27364) comes over the F.C.D. ("<The F.C.D. buzzes.> ... Find him at the Cathedral Quarter in Gilneas City and
-- take him out") once Captain Rutsak has told the player of the 7th Legion and Master Sergeant Zaren and sent them to avenge his
-- men (Vengeance for Our Soldiers, 27360); Wowhead and warcraft.wiki.gg have it start automatically. Nothing gave it, so the
-- player who turned in to Rutsak got no word of Zaren. Have the player cast Start Quest: Master Sergeant Zaren (84870) when they
-- take Vengeance for Our Soldiers from Rutsak, and again when they hand it in to him in case it was missed; its script shows the
-- quest in its window as it is accepted (spell_silverpine_start_quest_master_sergeant_zaren). The quest's prerequisites still
-- apply and the spell does nothing once the quest is taken or done. Pulling Zaren (2026_10_27_64) and Forward Commander
-- Onslaught still give it too.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 45389;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45389 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45389, 0, 0, 0, 19, 0, 100, 0, 27360, 0, 0, 0, 0, 85, 84870, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Captain Rutsak - On Quest ''Vengeance for Our Soldiers'' Taken - Invoker Cast Start Quest: Master Sergeant Zaren'),
(45389, 0, 1, 0, 20, 0, 100, 0, 27360, 0, 0, 0, 0, 85, 84870, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Captain Rutsak - On Quest ''Vengeance for Our Soldiers'' Rewarded - Invoker Cast Start Quest: Master Sergeant Zaren');

DELETE FROM `spell_script_names` WHERE `spell_id` = 84870 AND `ScriptName` = 'spell_silverpine_start_quest_master_sergeant_zaren';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84870, 'spell_silverpine_start_quest_master_sergeant_zaren');
