-- Urda 2851 (Hammerfall flight master): "Take me to Revantusk Village." flies a player who holds Revantusk Village (26430) to Revantusk.
-- Sourced: the option (menu 11684, option 1, text as TDB 442/1210 and TCPP; broadcast text 43044 in mop); the quest text "Speak with Urda if you wish to fly to Revantusk";
-- the 5.4.8 spell 80568 "Taxi - Hammerfall to Revantusk" (SEND_TAXI, path 484, node 17 -> 76). Ours: the gate (26430 in the log, incomplete or complete, mask 10; no TDB has a condition)
-- and the SmartAI that casts 80568, gated by the same condition on the gossip-select row (the gossip handler does not check that the option was shown). SEND_TAXI does not need the destination node known; the normal fare of path 484 is charged.
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11684 AND `OptionID` = 1;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(11684,1,0,'Take me to Revantusk Village.',43044,1,1,0,0,0,0,NULL,0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11684 AND `SourceEntry` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11684,1,0,0,47,0,26430,10,0,0,0,0,'','Urda - Show Revantusk option while Revantusk Village is in the log');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2851 AND `source_type` = 0 AND `id` IN (10,11);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2851,0,10,11,62,0,100,0,11684,1,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Urda - On Gossip Option 1 Selected - Close Gossip'),
(2851,0,11,0,61,0,100,0,0,0,0,0,0,85,80568,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Urda - Linked - Invoker Cast ''Taxi - Hammerfall to Revantusk''');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 11 AND `SourceEntry` = 2851 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,11,2851,0,0,47,0,26430,10,0,0,0,0,'','Urda - Revantusk flight only while Revantusk Village is in the log');
