-- Supersedes migration 07's overly late Knee-Deep boundary.
-- Keep Lorna's city phase until Slowing the Inevitable is accepted; its bat,
-- 6 catapult objectives and 40 invader objectives belong to phase 524288.
-- Use the quest condition types supported by PhaseMgr's live quest updates.
DELETE FROM conditions WHERE SourceTypeOrReferenceId=25 AND SourceGroup=4755 AND SourceEntry IN(10,11);
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment) VALUES
(25,4755,10,0,0,8,0,24904,0,0,0,0,0,'','City: battle rewarded'),
(25,4755,10,0,0,9,0,24920,0,0,1,0,0,'','City: bombing not active'),
(25,4755,10,0,0,28,0,24920,0,0,1,0,0,'','City: bombing not complete'),
(25,4755,10,0,0,8,0,24920,0,0,1,0,0,'','City: bombing not rewarded'),
(25,4755,11,0,0,9,0,24920,0,0,0,0,0,'','Bombing: quest active'),
(25,4755,11,0,0,8,0,24679,0,0,1,0,0,'','Bombing: before blessing reward'),
(25,4755,11,0,1,28,0,24920,0,0,0,0,0,'','Bombing: quest complete'),
(25,4755,11,0,1,8,0,24679,0,0,1,0,0,'','Bombing: before blessing reward'),
(25,4755,11,0,2,8,0,24920,0,0,0,0,0,'','Bombing: quest rewarded'),
(25,4755,11,0,2,8,0,24679,0,0,1,0,0,'','Bombing: before blessing reward');
UPDATE phase_definitions SET comment='Gilneas City: Slowing the Inevitable accepted through Patriarchs Blessing'
WHERE zoneId=4755 AND entry=11;
-- Genn offers the bombing quest in the city, receives it in the bombing phase,
-- then offers Knee-Deep. Do not change character history or widen all phases.
UPDATE creature SET phaseMask=(phaseMask | 524288)
WHERE map=654 AND id=38539 AND (phaseMask & 2097152)<>0;

-- Keep Krennan available to offer Keel Harbor immediately after blessing reward.
UPDATE creature SET phaseMask=(phaseMask | 1048576)
WHERE map=654 AND id=38144 AND (phaseMask & 524288)<>0;
