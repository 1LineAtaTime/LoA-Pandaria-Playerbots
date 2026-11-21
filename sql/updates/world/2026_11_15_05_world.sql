-- Eastern Plaguelands, Guardians of Stratholme (27525): the map shows a turn-in only where Tarenar Sunstrike stands.
--
-- Reported in game: with Gidwin's Fate Revealed 27526 and Journey's End 27527 rewarded and 27525 complete, the map marked the
-- Plaguewood fork and the slaughterhouse as turn-ins although Tarenar Sunstrike 45729 is at neither. By the staging of
-- 2026_11_14_01 (spell_area detection auras on invisible spawns) only one Tarenar is visible to a player at a time:
--   fork 3129400 (aura 49414, seen with 49416 in area 2277) while 27526 is none or incomplete;
--   slaughterhouse 3129401 (aura 49415, seen with 49417 in area 2277) while 27526 is complete or rewarded and 27527 is not
--   rewarded;
--   Light's Hope Chapel 369660 (aura 60921, seen with 60922 in area 2268) once 27527 is rewarded.
-- Needs the core's condition source type 31 (CONDITION_SOURCE_TYPE_QUEST_POI_BLOB: SourceGroup quest, SourceEntry Idx1),
-- which leaves a blob out of the POI answer unless the player meets its conditions.
--
-- Sourced: the chapel point: TDB 12.1 quest_poi_points (27525, blob 7, UiMapID 23) 2296,-5342. 12.1 gates its three turn-in
--   blobs (fork, slaughterhouse 3007,-3438, chapel) on player conditions.
-- Ours: the conditions, written to match the spell_area rows above rather than the 12.1 PlayerCondition ids (not in this
--   client's data); the chapel blob's other columns are those of the fork blob (Idx1 2), WorldMapAreaId 23 covers the chapel.
--
-- One blob in every reachable state (27527 needs 27526 rewarded):
--   27526 none / incomplete             -> fork
--   27526 complete, 27527 none          -> slaughterhouse
--   27526 rewarded, 27527 not rewarded  -> slaughterhouse
--   27526 rewarded, 27527 rewarded      -> chapel

DELETE FROM `quest_poi` WHERE `QuestID` = 27525 AND `Idx1` = 4;
INSERT INTO `quest_poi` (`QuestID`,`Idx1`,`ObjectiveIndex`,`QuestObjectiveId`,`MapID`,`WorldMapAreaId`,`Floor`,`Priority`,`Flags`,`VerifiedBuild`) VALUES
(27525,4,-1,0,0,23,0,0,7,0);

DELETE FROM `quest_poi_points` WHERE `QuestID` = 27525 AND `Idx1` = 4;
INSERT INTO `quest_poi_points` (`QuestID`,`BlobIndex`,`Idx1`,`Idx2`,`X`,`Y`,`VerifiedBuild`) VALUES
(27525,4,4,0,2296,-5342,0);

-- 47 = CONDITION_QUESTSTATE (value2 mask: 1 none, 2 complete, 8 incomplete, 64 rewarded), 8 = CONDITION_QUESTREWARDED
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 31 AND `SourceGroup` = 27525;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(31,27525,2,0,0,47,0,27526,9,0,0,0,0,'','Guardians of Stratholme - turn-in at the Plaguewood fork while Gidwin''s Fate Revealed is none or incomplete'),
(31,27525,3,0,0,47,0,27526,66,0,0,0,0,'','Guardians of Stratholme - turn-in in the slaughterhouse once Gidwin''s Fate Revealed is complete or rewarded'),
(31,27525,3,0,0,8,0,27527,0,0,1,0,0,'','Guardians of Stratholme - turn-in in the slaughterhouse until Journey''s End is rewarded'),
(31,27525,4,0,0,8,0,27527,0,0,0,0,0,'','Guardians of Stratholme - turn-in at Light''s Hope Chapel once Journey''s End is rewarded');
