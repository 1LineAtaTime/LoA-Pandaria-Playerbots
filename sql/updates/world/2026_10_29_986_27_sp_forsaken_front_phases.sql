-- ported from PR 15 2026_10_27_41_world.sql (#986)
-- The Forsaken Front (area 5387): phase the three stacked camps
-- Every camp NPC was spawned three times on the same spots, all in the default phase: the camp before The Great Escape,
-- the camp around Godfrey's body (Sylvanas 45525 ends The Great Escape and gives Rise, Godfrey) and the camp after Rise,
-- Godfrey (Sylvanas 45617 gives the rest of the zone). Each camp uses its own entries.
-- The retail sniff (TDB 1210) has only the first camp, in the default phase; the other two stand on its very spots with
-- their own entries, so each camp gives way to the next. Phase.dbc names 277, 278, 284 and 289 "The Forsaken Front"; no
-- 5.4.8 spell grants them and no sniff shows which camp used which, so the assignment below is ours. The named NPCs around Godfrey's body get their own phase so they
-- can make way for the personal actors of Rise, Godfrey while that quest is in progress.
--   289  camp before The Great Escape          The Great Escape not taken
--   277  army around Godfrey's body             The Great Escape taken, completed or rewarded, Rise, Godfrey not completed or rewarded
--   284  Sylvanas, Godfrey and the others       The Great Escape taken, completed or rewarded, Rise, Godfrey not taken (or failed)
--   278  camp after Rise, Godfrey               Rise, Godfrey completed or rewarded
-- The bat handler, the vendor and the critters stay in the default phase. The phases are zone-wide (130), so the camp is
-- seen from outside the area; the phased entries only stand at the Forsaken Front.

UPDATE `creature` SET `phaseId`=289, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5387 AND `id` IN (44632,44911,45198,46058,46483);
-- two of the first camp's catapults and abominations stand just outside the area
UPDATE `creature` SET `phaseId`=289, `phaseGroup`=0 WHERE (`guid`,`id`) IN ((186945,45198),(187901,46058));
UPDATE `creature` SET `phaseId`=277, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5387 AND `id` IN (45587,45588,45589,45594,45600,45601,45602,45603,45604);
UPDATE `creature` SET `phaseId`=284, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5387 AND `id` IN (45525,45576,45577,45578,45583,45584,45585,45593,45598,45599);
UPDATE `creature` SET `phaseId`=278, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=5387 AND `id` BETWEEN 45617 AND 45639;

DELETE FROM `phase_area` WHERE `AreaId`=130 AND `PhaseId` IN (277,278,284,289);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(130,289,'The Forsaken Front - camp before The Great Escape'),
(130,277,'The Forsaken Front - army around Godfrey''s body'),
(130,284,'The Forsaken Front - Sylvanas and Godfrey''s body before Rise, Godfrey'),
(130,278,'The Forsaken Front - camp after Rise, Godfrey');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=130 AND `SourceGroup` IN (277,278,284,289);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,289,130,0,0,47,0,27438,1,0,0,0,0,'','The Forsaken Front - phase 289 while The Great Escape is not taken'),
(26,277,130,0,0,47,0,27438,74,0,0,0,0,'','The Forsaken Front - phase 277 once The Great Escape is taken'),
(26,277,130,0,0,47,0,27472,41,0,0,0,0,'','The Forsaken Front - phase 277 until Rise, Godfrey is completed'),
(26,284,130,0,0,47,0,27438,74,0,0,0,0,'','The Forsaken Front - phase 284 once The Great Escape is taken'),
(26,284,130,0,0,47,0,27472,33,0,0,0,0,'','The Forsaken Front - phase 284 while Rise, Godfrey is not taken'),
(26,278,130,0,0,47,0,27472,66,0,0,0,0,'','The Forsaken Front - phase 278 once Rise, Godfrey is completed');
