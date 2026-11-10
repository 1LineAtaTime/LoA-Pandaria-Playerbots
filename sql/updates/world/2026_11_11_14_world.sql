-- Western Plaguelands, Zen'Kiki, the Druid (26953), A Different Approach (26954), Zen'Kiki and the Cultists (26955): Zen'Kiki can
-- be asked back, from Adrine Towhide 44456 or from Zen'Kiki 44863 in the Menders' Stead.
--
--  * Sourced: both TDBs (4.4.2 and 12.1.0): Adrine Towhide's menu 12722 option 0 "Is Zen'Kiki available for some more combat
--    practice?" (BroadcastText 50822, no reply menu) and camp Zen'Kiki 44863's gossip menu 11899 (texts 16694 "'Ey, mon! You come
--    to help Zen'Kiki out?" and 16711 "Thanks for helpin' me out, mon! No more shapeshifts for me... I'm gonna work on my
--    healing!"; option 0 "Yes, Zen'Kiki.  Let's go run some combat practice.", BroadcastText 44983). Wowhead comments: after a
--    logout, a flight or a battleground "just go back to TOWHEAD and ask if Zen Kiki is available for more training", "ask Adrine
--    Towhide for his help and he is back", "talk to him in the camp, and he will start following you again".
--  * Ours: what the options do (remove the quest's Escorting Zen'Kiki aura, which dismisses the old summon through
--    spell_wpl_escorting_zenkiki, then cast it again so its summon effect runs), when they show (one of the three quests in
--    progress, the same state spell_area gives the aura in), and which of 11899's texts shows when (16711 once 26955 is handed
--    in; the TDBs have both texts and no condition).
--  * Why: Escorting Zen'Kiki 83819 / 83456 is saved at logout and its summon is not, and spell_area never casts an aura the player
--    still has, so after a login the player had no Zen'Kiki: 26954's hawks can't be pulled without him, and 26955's bears are only
--    healed by him.
-- The rows of one event run in id order in one pass (smart_scripts is loaded ORDER BY id): the old Zen'Kiki goes, then the new one
-- comes. Separate rows, not a link chain: a linked row whose conditions fail would stop the rest.

DELETE FROM `gossip_menu_option` WHERE (`MenuID` = 12722 AND `OptionID` = 0) OR (`MenuID` = 11899 AND `OptionID` = 0);
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12722,0,0,'Is Zen''Kiki available for some more combat practice?',50822,1,1,0,0,0,0,NULL,0,0),
(11899,0,0,'Yes, Zen''Kiki.  Let''s go run some combat practice.',44983,1,1,0,0,0,0,NULL,0,0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` IN (12722, 11899) AND `SourceEntry` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 14 AND `SourceGroup` = 11899 AND `SourceEntry` IN (16694, 16711);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` IN (44456, 44863) AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,12722,0,0,0,47,0,26953,8,0,0,0,0,'','Adrine Towhide - Zen''Kiki option while Zen''Kiki, the Druid is in progress'),
(15,12722,0,0,1,47,0,26954,8,0,0,0,0,'','Adrine Towhide - Zen''Kiki option while A Different Approach is in progress'),
(15,12722,0,0,2,47,0,26955,8,0,0,0,0,'','Adrine Towhide - Zen''Kiki option while Zen''Kiki and the Cultists is in progress'),
(15,11899,0,0,0,47,0,26953,8,0,0,0,0,'','Zen''Kiki - practice option while Zen''Kiki, the Druid is in progress'),
(15,11899,0,0,1,47,0,26954,8,0,0,0,0,'','Zen''Kiki - practice option while A Different Approach is in progress'),
(15,11899,0,0,2,47,0,26955,8,0,0,0,0,'','Zen''Kiki - practice option while Zen''Kiki and the Cultists is in progress'),
(14,11899,16694,0,0,8,0,26955,0,0,1,0,0,'','Zen''Kiki - greeting until Zen''Kiki and the Cultists is handed in'),
(14,11899,16711,0,0,8,0,26955,0,0,0,0,0,'','Zen''Kiki - healer text once Zen''Kiki and the Cultists is handed in'),
(22,3,44456,0,0,47,0,26953,8,0,0,0,0,'','Adrine Towhide - recast Escorting Zen''Kiki 83819 for Zen''Kiki, the Druid'),
(22,3,44456,0,1,47,0,26955,8,0,0,0,0,'','Adrine Towhide - recast Escorting Zen''Kiki 83819 for Zen''Kiki and the Cultists'),
(22,4,44456,0,0,47,0,26954,8,0,0,0,0,'','Adrine Towhide - recast Escorting Zen''Kiki 83456 for A Different Approach'),
(22,3,44863,0,0,47,0,26953,8,0,0,0,0,'','Zen''Kiki - recast Escorting Zen''Kiki 83819 for Zen''Kiki, the Druid'),
(22,3,44863,0,1,47,0,26955,8,0,0,0,0,'','Zen''Kiki - recast Escorting Zen''Kiki 83819 for Zen''Kiki and the Cultists'),
(22,4,44863,0,0,47,0,26954,8,0,0,0,0,'','Zen''Kiki - recast Escorting Zen''Kiki 83456 for A Different Approach');

UPDATE `creature_template` SET `gossip_menu_id` = 11899, `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 44863;
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 44456;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44456, 44863) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(44456,0,0,0,62,0,100,0,12722,0,0,0,0,28,83819,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Adrine Towhide - On Gossip Option 0 Selected - Remove Escorting Zen''Kiki 83819 (dismisses Zen''Kiki 44904)'),
(44456,0,1,0,62,0,100,0,12722,0,0,0,0,28,83456,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Adrine Towhide - On Gossip Option 0 Selected - Remove Escorting Zen''Kiki 83456 (dismisses Zen''Kiki 44269)'),
(44456,0,2,0,62,0,100,0,12722,0,0,0,0,85,83819,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Adrine Towhide - On Gossip Option 0 Selected - Invoker Cast Escorting Zen''Kiki 83819 (26953 or 26955 in progress)'),
(44456,0,3,0,62,0,100,0,12722,0,0,0,0,85,83456,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Adrine Towhide - On Gossip Option 0 Selected - Invoker Cast Escorting Zen''Kiki 83456 (26954 in progress)'),
(44456,0,4,0,62,0,100,0,12722,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Adrine Towhide - On Gossip Option 0 Selected - Close Gossip'),
(44863,0,0,0,62,0,100,0,11899,0,0,0,0,28,83819,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Zen''Kiki - On Gossip Option 0 Selected - Remove Escorting Zen''Kiki 83819 (dismisses Zen''Kiki 44904)'),
(44863,0,1,0,62,0,100,0,11899,0,0,0,0,28,83456,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Zen''Kiki - On Gossip Option 0 Selected - Remove Escorting Zen''Kiki 83456 (dismisses Zen''Kiki 44269)'),
(44863,0,2,0,62,0,100,0,11899,0,0,0,0,85,83819,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Zen''Kiki - On Gossip Option 0 Selected - Invoker Cast Escorting Zen''Kiki 83819 (26953 or 26955 in progress)'),
(44863,0,3,0,62,0,100,0,11899,0,0,0,0,85,83456,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Zen''Kiki - On Gossip Option 0 Selected - Invoker Cast Escorting Zen''Kiki 83456 (26954 in progress)'),
(44863,0,4,0,62,0,100,0,11899,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Zen''Kiki - On Gossip Option 0 Selected - Close Gossip');
