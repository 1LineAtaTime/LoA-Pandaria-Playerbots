-- Western Plaguelands: Gahrron's Withering Cauldron's map area clears once the key is looted.
-- Sourced: TDB 1210 `quest_poi` for 27053: the key area (its Idx1 1, mop's Idx1 0; TDB's Idx1 0 is the turn-in) is (ObjectiveIndex 1,
-- QuestObjectiveId 265871), the Gahrron's Withering Cauldron Key 13196 objective. mop still had the Cataclysm form (4, 0: item slot 0,
-- the Empty Gahrron's Withering Bottle the quest provides), which the 5.4.8 client never clears. The area's point (1679, -2278) is
-- Cauldron Lord Soulwrath's spawn, who drops the key.
UPDATE `quest_poi` SET `ObjectiveIndex`=1, `QuestObjectiveId`=265871 WHERE `QuestID`=27053 AND `Idx1`=0;
