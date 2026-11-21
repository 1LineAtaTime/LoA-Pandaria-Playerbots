-- Eastern Plaguelands, Rayne at the Fungal Vale: the breadcrumb Cenarion Tenacity (27544, Urk Gagbaz at Crown Guard Tower)
-- and Rayne's gossip.
--
-- 1. Order. Sourced: comments 2011-01-01 [+10] and the wiki ("previous: A Strange Historian") have Urk offer 27544 only
--    after A Strange Historian (27386); comments 2011-06-09 [+2] ("Urk ... didn't offer this one ... She was there, and offered
--    me both quests"), 2011-03-13 and 2014-05-28 [+5] have Rayne give Postponing the Inevitable (27420) without the
--    breadcrumb, and the breadcrumb gone once 27420 is done. mop had it the other way round: 27544 offered from the start
--    and 27420 locked behind it (PrevQuestID 27544, so Player::SatisfyQuestPreviousQuest wanted 27544 rewarded). Both TDBs
--    carry the same PrevQuestID; in their core it was not a hard lock for a breadcrumb. Ours: the mechanism, as the other
--    breadcrumbs of this zone (eastwall 27532 / 27535): 27544 is offered only while 27420 was never taken
--    (condition 47, state mask 1 = none; a rewarded 27420 reads as REWARDED in Player::GetQuestStatus). A 27544 already in
--    the log stays and can still be handed in to Rayne (the ender list is not filtered by accept conditions); only her "?"
--    mark for it is not drawn once 27420 is taken (GetQuestDialogStatus skips enders whose accept conditions fail).
-- 2. Rayne's menu 7104 showed "Rayne, where should I look for savage fronds?" twice (options 0 and 1) and lacked her
--    conversation. Sourced (both TDBs): option 0 "Hello, Rayne. What brings the Cenarion Circle to the Plaguelands?" (12015)
--    -> 7123 (text 8385 = broadcast 12016) -> "Are these lands not beyond healing? They look pretty beat up." (12017) -> 7124
--    (text 8386 = broadcast 12018); option 1 the fronds line (12090) -> 7152. Menus 7123 / 7124 and texts 8385 / 8386 are free
--    in mop. The fronds line belongs to a retired repeatable (comment 2012-08-04: shown only to characters who did the old
--    quests); neither TDB conditions it, so it stays unconditioned (as TDB).

UPDATE `quest_template_addon` SET `PrevQuestID` = 27386, `NextQuestID` = 0 WHERE `ID` = 27544;
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` = 27420;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 27544;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 27544, 0, 0, 47, 0, 27420, 1, 0, 0, 0, 0, '', 'Cenarion Tenacity - only while Postponing the Inevitable was never taken');

DELETE FROM `npc_text` WHERE `ID` IN (8385, 8386);
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`)
SELECT 8385, `Text`, `Text1`, `ID`, 0, 1 FROM `broadcast_text` WHERE `ID` = 12016;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`)
SELECT 8386, `Text`, `Text1`, `ID`, 0, 1 FROM `broadcast_text` WHERE `ID` = 12018;

DELETE FROM `gossip_menu` WHERE `MenuID` IN (7123, 7124);
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES
(7123, 8385),
(7124, 8386);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (7104, 7123);
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`) VALUES
(7104, 0, 0, 'Hello, Rayne. What brings the Cenarion Circle to the Plaguelands?', 12015, 1, 1, 7123),
(7104, 1, 0, 'Rayne, where should I look for savage fronds?', 12090, 1, 1, 7152),
(7123, 0, 0, 'Are these lands not beyond healing? They look pretty beat up.', 12017, 1, 1, 7124);
