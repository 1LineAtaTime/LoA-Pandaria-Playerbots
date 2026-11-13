-- Keep the post-battle city phase through its complete quest chain:
-- 24902 -> 24903 -> 24920 -> 24678 -> 24602 -> 24679.
-- Switching after 24902 hid Lorna/King Genn before their follow-ups.
-- Switch after Knee-Deep is rewarded; bridge its Krennan turn-in between stages.
UPDATE conditions SET ConditionValue1=24678,
    Comment='Gilneas stage 10: remain until Knee-Deep is rewarded'
WHERE SourceTypeOrReferenceId=25 AND SourceGroup=4755 AND SourceEntry=10
  AND ConditionTypeOrReference=8 AND NegativeCondition=1
  AND ConditionValue1 IN (24902,24678);
UPDATE conditions SET ConditionValue1=24678,
    Comment='Gilneas stage 11: after Knee-Deep reward'
WHERE SourceTypeOrReferenceId=25 AND SourceGroup=4755 AND SourceEntry=11
  AND ConditionTypeOrReference=8 AND NegativeCondition=0
  AND ConditionValue1 IN (24902,24678);
UPDATE phase_definitions SET comment='Gilneas City: After quest 24678'
WHERE zoneId=4755 AND entry=11;
UPDATE creature SET phaseMask=(phaseMask | 2097152)
WHERE map=654 AND id=38144 AND (phaseMask & 524288)<>0;
