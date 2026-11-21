-- Western Plaguelands, the Menders' Stead: one Menders' Stead breadcrumb per character, and Zen'Kiki and the Cultists after the cauldron.
-- Sourced:
--  * The Menders' Stead 26952 (Horde, Garush), 27175 (Alliance, Pureheart) and 27233 (Alliance, Thurman Grant): ExclusiveGroup 26952 on all
--    three in both TDBs (4.4.2 and 1210); mop had 0, so an Alliance character could take and hand in both 27175 and 27233. NextQuestID 26999
--    stays; a positive group still lets A New Era for the Plaguelands count any one of them.
--  * Zen'Kiki and the Cultists 26955 after A Different Approach 26954 and Return to the Stead 27057: Wowhead MoP Classic comments ("Once
--    you're back, there will be a quest to investigate the cauldron at Gahrron's Withering - Once that quest is complete, this one will open
--    up", 2010; "you get a quest that sends you back to the Stead ... Quick run to the cauldron, and then the quest opens up", 2011; a list
--    of quests done before it that starts with Zen'Kiki's two and ends with 27053 and 27057) and warcraft.wiki.gg's infobox, as reported by
--    the auditor. Neither TDB links 26955; mop had no row, so Adrine offered it at once.
--  * Gahrron's Withering Cauldron 27053 (Kaartish) after An Audience with the Highlord 27002 or Memories from a Lost Past 27017: its
--    Wowhead comments ("I had a prerequisite for this either An Audience with the Highlord or The Long Trip Home"; "Memories from a
--    Lost Past quest comes directly before this one"; "I had to complete an Audience with the Highlord as well"; "Audience with the
--    highlord worked for me"), and the comment above that puts the cauldron after Hearthglen. 27002 ends the Northridge spider chain
--    (27000-27002), which the Hearthglen Mustang script makes completable. Neither TDB links 27053; it had no gate, so the whole
--    cauldron line was open from the first visit.
-- Ours: 26955 gets PrevQuestID 26954 and a condition on 27057 rewarded; 27053 gets a condition on 27002 or 27017 rewarded (either one,
-- since the comments name both).
UPDATE `quest_template_addon` SET `ExclusiveGroup`=26952 WHERE `ID` IN (26952,27175,27233);
DELETE FROM `quest_template_addon` WHERE `ID`=26955;
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES
(26955,26954);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=26955 AND `ConditionTypeOrReference`=8 AND `ConditionValue1`=27057;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,26955,0,0,8,0,27057,0,0,0,0,0,'','Zen''Kiki and the Cultists needs Return to the Stead rewarded');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=27053 AND `ConditionTypeOrReference`=8 AND `ConditionValue1` IN (27002,27017);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,27053,0,0,8,0,27002,0,0,0,0,0,'','Gahrron''s Withering Cauldron needs An Audience with the Highlord rewarded'),
(19,0,27053,0,1,8,0,27017,0,0,0,0,0,'','Gahrron''s Withering Cauldron needs Memories from a Lost Past rewarded');
