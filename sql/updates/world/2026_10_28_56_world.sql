-- The Forsaken Front: the raised Godfrey, Walden and Ashbury step aside when they go with the player.
-- Lord Godfrey 45622, Lord Walden 45623 and Baron Ashbury 45624 stand at the Forsaken Front from Rise, Godfrey on (phase
-- 278, 2026_10_27_41). Taking the Battlefront hands them to the player as companions (45878-45880, Battlefront Summon
-- Master Aura), so from then on the player saw each of them twice. They do not come back: after On Her Majesty's Secret
-- Service the trio has its Cities in Dust actors, and Godfrey turns on the Forsaken there. Phase 191 "Quest Zone-Specific
-- 16" (unused generic phase; the assignment is ours): from Rise, Godfrey completed until Taking the Battlefront is rewarded.
UPDATE `creature` SET `phaseId`=191, `phaseGroup`=0 WHERE `guid` IN (306537,306532,306539) AND `id` IN (45622,45623,45624);

DELETE FROM `phase_area` WHERE `AreaId`=130 AND `PhaseId`=191;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(130,191,'The Forsaken Front - raised Godfrey, Walden and Ashbury before they go with the player');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=130 AND `SourceGroup`=191;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,191,130,0,0,47,0,27472,66,0,0,0,0,'','The Forsaken Front - phase 191 once Rise, Godfrey is completed'),
(26,191,130,0,0,47,0,27542,43,0,0,0,0,'','The Forsaken Front - phase 191 until Taking the Battlefront is rewarded');
