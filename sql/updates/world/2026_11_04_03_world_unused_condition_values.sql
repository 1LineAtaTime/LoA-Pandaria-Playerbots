-- Normalize only fields not read by Condition::Meets in this core.
-- Keep every condition, source/target, negation, and effective argument.
-- In particular do NOT clear QuestRewarded.value3 (race-filter override) or
-- NearCreature.value3 (dead/alive selector): those are real runtime arguments.
UPDATE `conditions` SET `ConditionValue2`=0
WHERE `ConditionTypeOrReference` IN (8,9,14,28) AND `ConditionValue2`<>0;
UPDATE `conditions` SET `ConditionValue3`=0
WHERE `ConditionTypeOrReference` IN (1,9,14,28) AND `ConditionValue3`<>0;
UPDATE `conditions` SET `ConditionValue1`=0,`ConditionValue2`=0,`ConditionValue3`=0
WHERE `ConditionTypeOrReference`=36
AND (`ConditionValue1`<>0 OR `ConditionValue2`<>0 OR `ConditionValue3`<>0);
