-- Eastern Plaguelands, Pamela's Doll (27384) and Marauders of Darrowshire (27389): an area for the objective itself.
--
-- Sourced: retail 12.1 quest_poi has an area for the doll objective (259299, an outline of Darrowshire) and for the Resonating Skulls
-- objective (261216, the Fungal Vale); mop has only the item-slot areas (ObjectiveIndex 26-28) of the parts and the Fetid Skull.
-- Comment 2013-09-10 [+33] on 27389: "the pink area is not coming up". Points as retail (12 and 6), flags as retail.
DELETE FROM `quest_poi_points` WHERE (`QuestID`=27384 AND `Idx1`=7) OR (`QuestID`=27389 AND `Idx1`=2);
DELETE FROM `quest_poi` WHERE (`QuestID`=27384 AND `Idx1`=7) OR (`QuestID`=27389 AND `Idx1`=2);
INSERT INTO `quest_poi` (`QuestID`,`Idx1`,`ObjectiveIndex`,`QuestObjectiveId`,`MapID`,`WorldMapAreaId`,`Floor`,`Priority`,`Flags`,`VerifiedBuild`) VALUES
(27384,7,0,259299,0,23,0,0,2,0),
(27389,2,0,261216,0,23,0,0,0,0);
INSERT INTO `quest_poi_points` (`QuestID`,`BlobIndex`,`Idx1`,`Idx2`,`X`,`Y`,`VerifiedBuild`) VALUES
(27384,0,7,0,1409,-3735,0),(27384,0,7,1,1420,-3735,0),(27384,0,7,2,1426,-3735,0),(27384,0,7,3,1434,-3735,0),
(27384,0,7,4,1495,-3729,0),(27384,0,7,5,1498,-3720,0),(27384,0,7,6,1492,-3714,0),(27384,0,7,7,1471,-3693,0),
(27384,0,7,8,1431,-3660,0),(27384,0,7,9,1418,-3649,0),(27384,0,7,10,1409,-3646,0),(27384,0,7,11,1399,-3646,0),
(27389,0,2,0,2480,-3846,0),(27389,0,2,1,2535,-3790,0),(27389,0,2,2,2548,-3752,0),(27389,0,2,3,2500,-3678,0),
(27389,0,2,4,2417,-3617,0),(27389,0,2,5,2213,-3491,0);
