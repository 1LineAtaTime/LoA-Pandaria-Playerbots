-- Western Plaguelands, Combat Training (26938), I Ain't Sayin' You a Gourd-Digger... (26956), A New Era for the Plaguelands
-- (26999): seven quest map areas sat on another objective than the one whose mobs are there.
--
--  * Sourced: quest_poi of TDB 4.4.2 (Cataclysm Classic) and 12.1.0 (retail). Their point sets are the same as mop's (same Idx1,
--    same points); only the objective of each area differs. mop's spawns agree with the TDBs: the Opengut Behemoth 45098 stands on
--    26938's single point in the barn (1791,-1252) and the troopers roam the 12-point field; Corn Stalkers / Pome Wraiths /
--    Thrashing Pumpkins on 26956's areas 1 / 2 / 3; Hulking Plaguebears (60762) around (1947,-2070), Diseased Wolves (60763) at
--    (1921,-2078) and (2015,-1527). mop had 26938's two swapped, 26956's three rotated and gave two wolf areas to Plague Lurker.
-- quest_poi is loaded at startup and cached by the client: restart, then relog.

UPDATE `quest_poi` SET `ObjectiveIndex` = 1, `QuestObjectiveId` = 253690 WHERE `QuestID` = 26938 AND `Idx1` = 0;
UPDATE `quest_poi` SET `ObjectiveIndex` = 0, `QuestObjectiveId` = 253689 WHERE `QuestID` = 26938 AND `Idx1` = 1;
UPDATE `quest_poi` SET `ObjectiveIndex` = 0, `QuestObjectiveId` = 267035 WHERE `QuestID` = 26956 AND `Idx1` = 1;
UPDATE `quest_poi` SET `ObjectiveIndex` = 1, `QuestObjectiveId` = 267036 WHERE `QuestID` = 26956 AND `Idx1` = 2;
UPDATE `quest_poi` SET `ObjectiveIndex` = 2, `QuestObjectiveId` = 267037 WHERE `QuestID` = 26956 AND `Idx1` = 3;
UPDATE `quest_poi` SET `ObjectiveIndex` = 2, `QuestObjectiveId` = 264902 WHERE `QuestID` = 26999 AND `Idx1` = 3;
UPDATE `quest_poi` SET `ObjectiveIndex` = 3, `QuestObjectiveId` = 264903 WHERE `QuestID` = 26999 AND `Idx1` IN (4, 5);
