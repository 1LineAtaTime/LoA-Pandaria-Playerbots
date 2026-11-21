-- Western Plaguelands, the Horde's second battle for Andorhal: The Battle Resumes! gates it, and its quests follow in order.
-- Sourced (Wowhead MoP Classic; neither TDB links 27083, 27085, 27089 or 27144):
--  * The Battle Resumes! 27083 after the first battle and the trip to the Menders' Stead: comments on 27083 ("I was able to take this
--    quest immediately after completing The Mender's Stead", 2010) and 27089 ("You get sent off ... Victory, For Now, then get a return to
--    andorhal quest The Battle Resumes!", 2010); on 27085 "sent back to Andorhal and this quest become[s available]", and the 2014 walk
--    through ending "head to the Mender's Stead ... Damion Steel, who will give you the quest The Battle Resumes!".
--  * The Farmers' Militia 27084, Supporting the Troops 27085 and Ashes to Ashes 27086 after 27083: the comment above on 27085 and the quest
--    order posted on 27083, 27087, 27089 ("The Battle Resumes!, The Farmers' Militia / Ashes to Ashes, Lindsay Ravensun Revealed, Ace in
--    the Hole, Andorhal Once and For All, The Reckoning").
--  * Lindsay Ravensun, Revealed 27087 needs both 27084 and 27086: Wowhead's series ("1. The Farmers' Militia, Ashes to Ashes 2. Lindsay
--    Ravensun, Revealed") and the posted order. Both TDBs: ExclusiveGroup 27084, which in this core would let a player hold only one of
--    the two.
--  * Ace in the Hole 27089 after 27087 ("Leads to Ace in the Hole", 2010, and the posted order); The Reckoning 27144 after Andorhal, Once
--    and For All 27090 ("After you complete Andorhal, Once and For All, she'll offer you it", 2010).
--  * The Battle Resumes! 27083 needs Combat Training 26938 (PrevQuestID) and The Menders' Stead 26952 (condition): the comment on 27083
--    "You need to be lvl 38. And need to have completed Combat Training and The Menders' Stead"; the comment on 27085 ("you will need
--    to travel to the Bulwark and complete quests at Felstone Field before you are sent back to Andorhal"); 26938's reward text ("When
--    Koltira gives his word, I will bring these val'kyr into Andorhal"). 26938 comes after Victory, For Now 26926 through the Bulwark's
--    chain. Combat Training is completable with the val'kyr script (Scripts/WesternPlaguelands, Felstone Field).
-- Ours: the group sign -27084, so both are needed. Until now Derrington, Damion Steel, Koltira, Jearl and Sylvanas offered the whole second battle at level 38,
-- during the first.
DELETE FROM `quest_template_addon` WHERE `ID` IN (27083,27085,27144);
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES
(27083,26938),
(27085,27083),
(27144,27090);
UPDATE `quest_template_addon` SET `PrevQuestID`=27083, `NextQuestID`=27087, `ExclusiveGroup`=-27084 WHERE `ID` IN (27084,27086);
UPDATE `quest_template_addon` SET `PrevQuestID`=27087 WHERE `ID`=27089;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=27083 AND `ConditionTypeOrReference`=8 AND `ConditionValue1`=26952;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,27083,0,0,8,0,26952,0,0,0,0,0,'','The Battle Resumes! (Horde) needs The Menders'' Stead rewarded');
