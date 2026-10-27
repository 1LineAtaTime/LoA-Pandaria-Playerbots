-- To Forsaken Forward Command (27290): Sylvanas at the Sepulcher says "Arthura will escort you to Forward Command. Tell me when
-- you are ready and I will send you on your way." but had nothing to tell her, so the player had to find their own way into
-- Gilneas. 5.4.8 has the flight: Flight of the Val'kyr (84695) summons Arthura (45318, vehicle 1100) beside her and seats the
-- player through its ride aura (84691, which also gives the Gilneas Act I phase). No sniff has the gossip option, its text or the
-- flight path; the option's text is ours. It shows only on the Sepulcher's Sylvanas (the same entry stands at Forsaken High
-- Command) while the quest is in the log: it has no objectives, so it is complete as soon as it is accepted.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_arthura_forward_command' WHERE `entry` = 45318;

DELETE FROM `spell_script_names` WHERE `spell_id` = 84695 AND `ScriptName` = 'spell_silverpine_flight_of_the_valkyr_forward_command';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84695, 'spell_silverpine_flight_of_the_valkyr_forward_command');

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11823 AND `OptionID` = 1;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11823, 1, 0, 'I am ready to go to Forsaken Forward Command.', 0, 1, 1, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11823 AND `SourceEntry` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11823, 1, 0, 0, 47, 0, 27290, 10, 0, 0, 0, 0, '', 'Lady Sylvanas Windrunner - show the Forward Command option while To Forsaken Forward Command is in the log'),
(15, 11823, 1, 0, 0, 31, 1, 3, 44365, 306012, 0, 0, 0, '', 'Lady Sylvanas Windrunner - Forward Command option only on the Sylvanas at the Sepulcher');
