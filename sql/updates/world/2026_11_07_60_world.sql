-- First Blood 26022 is offered without a breadcrumb and comes before The Forsaken Trollbane 26023 and The Traitor Orc 26025.
-- Sourced: the order. warcraft.wiki.gg's First Blood infobox (Next: 26023, 26025), 26023's (Previous: First Blood) and the Arathi
-- zone progression; TDB 442 and 1210 have no row for it and Wowhead's series doesn't link them, so making it a hard gate is our inference.
-- The breadcrumb group 28572 and RewardNextQuest 26022 on 28572/28619 stay.
-- Ours: dropping NextQuestID 26022 from the two Warchief's Command quests, which both TDBs have (with it this core makes one of them
-- a hard requirement). The wiki's zone page calls them breadcrumbs, and a Wowhead comment on 28572 says they are "unavailable as soon
-- as you start any chain in the zone": the source-19 rows close them once First Blood is taken (mask 1: status none). Source 19 also
-- gates the ender's mark, so else group 1 keeps one already in the log (mask 10) open, and Alina still shows its turn-in.
UPDATE `quest_template_addon` SET `NextQuestID`=0 WHERE `ID` IN (28572,28619);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (28572,28619) AND `ConditionTypeOrReference`=47 AND `ConditionValue1` IN (26022,28572,28619);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,28572,0,0,47,0,26022,1,0,0,0,0,'','Warchief''s Command: Arathi Highlands! only before First Blood is taken'),
(19,0,28619,0,0,47,0,26022,1,0,0,0,0,'','Warchief''s Command: Arathi Highlands! only before First Blood is taken'),
(19,0,28572,0,1,47,0,28572,10,0,0,0,0,'','Warchief''s Command: Arathi Highlands! or while it is in the log'),
(19,0,28619,0,1,47,0,28619,10,0,0,0,0,'','Warchief''s Command: Arathi Highlands! or while it is in the log');
UPDATE `quest_template_addon` SET `PrevQuestID`=26022 WHERE `ID`=26023;
DELETE FROM `quest_template_addon` WHERE `ID`=26025;
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES (26025,26022);
