-- ported from PR 15 2026_10_27_47_world.sql (#986)
-- The Battlefront (area 5456): show who only arrives there later in the story
-- Sylvanas 46124 and High Warlord Cromush 46125 stand at the Battlefront after Cities in Dust (Sylvanas ends it and gives
-- Empire of Dirt), and Archmage Ataeric 46049 with his Forsaken Mages 46048 are the Ambermill magi raised after
-- Transdimensional Warfare: Chapter III. All were in the default phase and stood there from the start.
-- Phase.dbc has no phase named for the Battlefront; use two unused generic ones ("Quest Zone-Specific 22" and "23").
-- The phases are zone-wide (130), so they are seen from outside the Battlefront; these entries only stand there.
-- The Forsaken Mages no longer see the worgen in the default phase next to them, so they stand rather than fight.
--   197  Ataeric and the Forsaken Mages   Transdimensional Warfare: Chapter III rewarded
--   198  Sylvanas and Cromush             Cities in Dust completed or rewarded
UPDATE `creature` SET `phaseId`=197, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5456 AND `id` IN (46048,46049);
UPDATE `creature` SET `phaseId`=198, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5456 AND `id` IN (46124,46125);

DELETE FROM `phase_area` WHERE `AreaId`=130 AND `PhaseId` IN (197,198);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(130,197,'The Battlefront - Archmage Ataeric and the Forsaken Mages after Transdimensional Warfare: Chapter III'),
(130,198,'The Battlefront - Sylvanas and Cromush after Cities in Dust');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=130 AND `SourceGroup` IN (197,198);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,197,130,0,0,47,0,27518,64,0,0,0,0,'','The Battlefront - phase 197 once Transdimensional Warfare: Chapter III is rewarded'),
(26,198,130,0,0,47,0,27601,66,0,0,0,0,'','The Battlefront - phase 198 once Cities in Dust is completed');
