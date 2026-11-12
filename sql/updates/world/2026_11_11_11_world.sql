-- Western Plaguelands, Ace in the Hole (27089): the quest map areas of two objectives were swapped.
--
-- mop had "Frightened Provincial Minuteman slain and converted" (objective 0, id 265247) on area 3, the point at Lurid's body
-- (1346,-1273), and "Hunt Thassarian" (objective 2, id 265249) on area 2, the Andorhal town polygon.
--  * Sourced: retail TDB 1210 quest_poi has 265247 on the town polygon and 265249 (flags 7) on the point (1346,-1273). The
--    polygons of areas 1 and 2 are identical, so only the objective pairs change; the points and flags already match.
-- quest_poi is loaded at startup and cached by the client: restart, then relog.

UPDATE `quest_poi` SET `ObjectiveIndex` = 0, `QuestObjectiveId` = 265247 WHERE `QuestID` = 27089 AND `Idx1` = 2;
UPDATE `quest_poi` SET `ObjectiveIndex` = 2, `QuestObjectiveId` = 265249 WHERE `QuestID` = 27089 AND `Idx1` = 3;
