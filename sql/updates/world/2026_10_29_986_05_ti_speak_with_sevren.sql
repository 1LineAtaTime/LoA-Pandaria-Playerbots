-- ported from PR 15 2026_10_24_01_world.sql (#986)
-- Speak with Sevren (25005): Coleman Farthing offered it straight away, but it hands in the Agamand remains collected in Deaths in
-- the Family (25029), as the pre-Cataclysm 355 followed 354
UPDATE `quest_template_addon` SET `PrevQuestID` = 25029 WHERE `ID` = 25005;
-- #986 addition (not in the PR): Wowpedia lists "Speak with Sevren" as following both The Mills Overrun (25004) and Deaths in the
-- Family (25029); the PR only sets PrevQuestID 25029, so also require 25004 to be rewarded (SFDB 5.4.8 has no prerequisite at all)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 25005 AND `ConditionTypeOrReference` = 8 AND `ConditionValue1` = 25004;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 25005, 0, 0, 8, 0, 25004, 0, 0, 0, 0, 0, '', 'Speak with Sevren (25005) requires The Mills Overrun (25004) rewarded');
