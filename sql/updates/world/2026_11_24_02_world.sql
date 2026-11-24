-- Northern Barrens, Brak Blusterpipe: the "I'm ready to teleport. I think." gossip option.
-- Problem: the option (menu 10548, option 0) has no condition, so any player can choose it without the quest and be teleported
--   to Wenikee Boltbucket.
-- Change: the option is only offered while Wenikee Boltbucket (29026) is in progress or complete (CONDITION_QUESTSTATE, mask 2 | 8).
--   CONDITION_QUESTTAKEN is not used because Wenikee Boltbucket hands over the Samophlange when it is accepted, so the quest is
--   already complete at that moment and the option would never show.
-- Source: quest text of Wenikee Boltbucket (29026, "Talk to Brak to teleport directly to her"); Wowhead MoP Classic quest 29026.
-- Rollback:
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10548 AND `SourceEntry`=0 AND `ConditionTypeOrReference`=47 AND `ConditionValue1`=29026;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10548 AND `SourceEntry`=0 AND `ConditionTypeOrReference`=47;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 10548, 0, 0, 0, 47, 0, 29026, 10, 0, 0, 0, 0, '', 'Brak Blusterpipe - teleport only with Wenikee Boltbucket in progress or complete');
