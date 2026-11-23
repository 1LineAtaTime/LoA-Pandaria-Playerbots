-- Northern Barrens, Ratchet: Chief Engineer Foote's "<Club him unconscious.>" gossip option.
-- Problem: the option has no condition, so any player can choose it without the quest and receive the Ship Schematics
--   again and again, also after the quest objective is done.
-- Change: the option (menu 10556, option 0) is only offered while Club Foote (14034) is taken and not yet complete
--   (CONDITION_QUESTTAKEN). The quest text says to get the Ship Schematics from Foote, which completes it.
-- Source: quest text of Club Foote (14034); Wowhead MoP Classic quest 14034.
-- Rollback:
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10556 AND `SourceEntry`=0 AND `ConditionTypeOrReference`=9 AND `ConditionValue1`=14034;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10556 AND `SourceEntry`=0 AND `ConditionTypeOrReference`=9;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 10556, 0, 0, 0, 9, 0, 14034, 0, 0, 0, 0, 0, '', 'Chief Engineer Foote - club him only with Club Foote taken');
