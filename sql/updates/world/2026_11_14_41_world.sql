-- Eastern Plaguelands, Counter-Plague Research (27531) and A Fate Worse Than Butchery (27533) without their breadcrumbs;
-- 27531's berries and map areas.
-- Sourced:
--   The Plaguewood Tower (27532) and Just a Little Touched (27535) are breadcrumbs: Judkins offers 27531 to anyone who walks
--   up, and 27532 is no longer offered once 27531 is taken or done (Wowhead comments 2012-02-25 [+8], 2012-08-04 on 27532);
--   "if you have 27533 you will not be able to pick up" 27535 (comment 2011-03-03 [+11]). Both TDBs list the breadcrumbs as
--   prerequisites instead; decisions R4 rules for the comments (on the user's question list).
--   4-7 Rotberries a bush (both TDBs, loot 34719).
--   The map areas by objective as retail, matched by position (tdb-diff section 6): the foot's one-point area is the
--   foot's own position (205558), the others follow retail's indices.
-- Source type 19 alone gates both the offer and the "!" over the giver (Player::CanTakeQuest, GetQuestDialogStatus);
-- type 20 is unused in this core, and the DELETE still clears any such row.
-- Ours: none. The quest NextQuestID links go with the prerequisites (a quest whose NextQuestID points at X is a required
-- previous quest of X in this core); RewardNextQuest stays, so a breadcrumb still offers its target at turn-in.
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID` IN (27531,27533);
UPDATE `quest_template_addon` SET `NextQuestID`=0 WHERE `ID` IN (27532,27535);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` IN (19,20) AND `SourceEntry` IN (27532,27535);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`Comment`) VALUES
(19,0,27532,0,0,47,0,27531,1,0,0,'The Plaguewood Tower: only while Counter-Plague Research was never taken'),
(19,0,27535,0,0,47,0,27533,1,0,0,'Just a Little Touched: only while A Fate Worse Than Butchery was never taken');
UPDATE `gameobject_loot_template` SET `mincountOrRef`=4,`maxcount`=7 WHERE `entry`=205559 AND `item`=61364;
UPDATE `quest_poi` SET `ObjectiveIndex`=0,`QuestObjectiveId`=266670 WHERE `QuestID`=27531 AND `Idx1`=1;
UPDATE `quest_poi` SET `ObjectiveIndex`=1,`QuestObjectiveId`=266671 WHERE `QuestID`=27531 AND `Idx1` IN (2,3);
UPDATE `quest_poi` SET `ObjectiveIndex`=2,`QuestObjectiveId`=266672 WHERE `QuestID`=27531 AND `Idx1`=4;
