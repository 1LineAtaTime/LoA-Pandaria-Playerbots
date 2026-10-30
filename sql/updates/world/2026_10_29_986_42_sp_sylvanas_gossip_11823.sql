-- ported from PR 15 2026_10_27_56_world.sql (#986)
-- Gossip menu 11823 is shared by Sylvanas at Forsaken High Command and the Sepulcher (44365) and both Sylvanas of the Forsaken
-- Front (45525, 45617). The Cities in Dust option (start the march) only checked the quest, so every one of them offered it to a
-- player on Cities in Dust, though only 45617's script starts the march. Show it only on 45617.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11823 AND `SourceEntry` = 0 AND `ConditionTypeOrReference` = 31;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11823, 0, 0, 0, 31, 1, 3, 45617, 0, 0, 0, 0, '', 'Lady Sylvanas Windrunner - Cities in Dust option only on the Sylvanas after Rise, Godfrey');
