-- Eastern Plaguelands, Guardians of Stratholme (27525): the map also marks the Tarenar in the slaughterhouse as its turn-in.
--
-- Reported in game: after Gidwin is rescued the quest cannot be handed in, Tarenar is gone. By the staging of 2026_11_14_01 the
-- Tarenar Sunstrike 45729 at the Plaguewood fork (3129400) leaves on the credit of Gidwin's Fate Revealed 27526, and from then
-- until Journey's End 27527 is rewarded the only Tarenar stands beside Gidwin's table in the slaughterhouse (3129401), as on
-- retail ("Tarenar is in the Ziggurat where the cut scene happened. Run back in and turn the quest in", Wowhead 2011-01-21).
-- The quest's only turn-in blob pointed at the fork, empty from that credit on.
--
-- Sourced: the point: TDB 12.1 quest_poi_points (27525, blob 9) 3007,-3438. The 4.4.2 TDB has the fork blob only.
-- Ours: the blob has no condition (12.1 gates it on a player condition this client data does not have), so a complete quest
--   shows both turn-in areas; the other columns are those of the fork blob (Idx1 2).

DELETE FROM `quest_poi` WHERE `QuestID` = 27525 AND `Idx1` = 3;
INSERT INTO `quest_poi` (`QuestID`,`Idx1`,`ObjectiveIndex`,`QuestObjectiveId`,`MapID`,`WorldMapAreaId`,`Floor`,`Priority`,`Flags`,`VerifiedBuild`) VALUES
(27525,3,-1,0,0,23,0,0,7,0);

DELETE FROM `quest_poi_points` WHERE `QuestID` = 27525 AND `Idx1` = 3;
INSERT INTO `quest_poi_points` (`QuestID`,`BlobIndex`,`Idx1`,`Idx2`,`X`,`Y`,`VerifiedBuild`) VALUES
(27525,3,3,0,3007,-3438,0);
