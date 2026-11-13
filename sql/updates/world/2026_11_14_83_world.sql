-- Eastern Plaguelands, Victory From Within (27612): the two map areas were on each other's portal, so opening the Hall of Arms
-- portal cleared the Library Wing's area and the other way round.
-- Area Idx1 1 is the point 1745,-5456 = Argent Portal 205876, whose use credits 46102 = objective 263644 (index 0, "Open portal
-- within Hall of Arms"); area Idx1 2 is 1796,-5426 = portal 205877, credit 46103 = objective 263645 (index 1, Library Wing).
-- Sourced: TDB 12.1.0 quest_poi has exactly these pairs.
UPDATE `quest_poi` SET `ObjectiveIndex` = 0, `QuestObjectiveId` = 263644 WHERE `QuestID` = 27612 AND `Idx1` = 1;
UPDATE `quest_poi` SET `ObjectiveIndex` = 1, `QuestObjectiveId` = 263645 WHERE `QuestID` = 27612 AND `Idx1` = 2;
