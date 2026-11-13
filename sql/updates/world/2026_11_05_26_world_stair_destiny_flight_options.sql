-- Adapt SkyFire release26 2026_08_09_world_04/_05 to this fork's option IDs.
-- Here option0 is the scripted introductory ride and option1 is the taxi map
-- (the upstream patch uses the opposite numbering). Accept either active
-- state, not just incomplete, because the delivery item can complete the
-- objective before the player takes the flight.
UPDATE conditions SET ConditionTypeOrReference=47,ConditionValue2=10
 WHERE SourceTypeOrReferenceId=15 AND SourceGroup IN (7938,7939)
 AND SourceEntry=0 AND SourceId=0 AND ElseGroup=0 AND ConditionTypeOrReference=28
 AND ConditionTarget=0 AND ((SourceGroup=7938 AND ConditionValue1=10289) OR (SourceGroup=7939 AND ConditionValue1=10140))
 AND ConditionValue2=0 AND ConditionValue3=0 AND NegativeCondition=0
 AND ErrorType=0 AND ErrorTextId=0 AND ScriptName='';
DELETE FROM conditions
 WHERE SourceTypeOrReferenceId=15 AND SourceGroup IN (7938,7939)
 AND SourceEntry=1 AND SourceId=0 AND ElseGroup=0 AND ConditionTypeOrReference=8
 AND ConditionTarget=0 AND ((SourceGroup=7938 AND ConditionValue1=10289) OR (SourceGroup=7939 AND ConditionValue1=10140))
 AND ConditionValue2=0 AND ConditionValue3=0 AND NegativeCondition=0
 AND ErrorType=0 AND ErrorTextId=0 AND ScriptName='';
