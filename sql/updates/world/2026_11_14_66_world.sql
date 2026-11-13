-- Eastern Plaguelands, Heroes of Darrowshire (27388): the map areas of Davil's Libram and Redpath's Shield.
--
-- Sourced: retail 12.1 quest_poi. The area on the libram's fireplace (1866,-3187) carried the Extended Annals' objective (index 0,
-- 261171) and the one on the shield (1872,-3180) the libram's (index 1, 261172); retail has them as index 1 / 261172 (libram) and
-- index 2 / 261173 (shield), so the shield objective had no area and the tracker pointed at the house for the annals.
UPDATE `quest_poi` SET `ObjectiveIndex`=1, `QuestObjectiveId`=261172 WHERE `QuestID`=27388 AND `Idx1`=0;
UPDATE `quest_poi` SET `ObjectiveIndex`=2, `QuestObjectiveId`=261173 WHERE `QuestID`=27388 AND `Idx1`=1;
