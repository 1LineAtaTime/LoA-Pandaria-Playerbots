-- Restore exclusive post-Observatory Gilneas phases in the original world.
-- Existing phase_definitions label the quest boundaries below, but lacked all
-- SourceType25 conditions. Unconditional early masks exposed Bloodfang enemies
-- during the Forsaken battle. Overwrite+last prevents earlier masks accumulating.
-- Does not change a character's quest history or complete any objective.
DELETE FROM conditions WHERE SourceTypeOrReferenceId=25 AND SourceGroup=4755 AND SourceEntry BETWEEN 8 AND 13;
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment) VALUES
(25,4755,8,0,0,8,0,14466,0,0,0,0,0,'','Gilneas stage 8: milestone rewarded'),
(25,4755,8,0,0,8,0,24676,0,0,1,0,0,'','Gilneas stage 8: before next milestone reward'),
(25,4755,9,0,0,8,0,24676,0,0,0,0,0,'','Gilneas stage 9: milestone rewarded'),
(25,4755,9,0,0,8,0,24904,0,0,1,0,0,'','Gilneas stage 9: before next milestone reward'),
(25,4755,10,0,0,8,0,24904,0,0,0,0,0,'','Gilneas stage 10: milestone rewarded'),
(25,4755,10,0,0,8,0,24902,0,0,1,0,0,'','Gilneas stage 10: before next milestone reward'),
(25,4755,11,0,0,8,0,24902,0,0,0,0,0,'','Gilneas stage 11: milestone rewarded'),
(25,4755,11,0,0,8,0,24679,0,0,1,0,0,'','Gilneas stage 11: before next milestone reward'),
(25,4755,12,0,0,8,0,24679,0,0,0,0,0,'','Gilneas stage 12: milestone rewarded'),
(25,4755,12,0,0,8,0,24681,0,0,1,0,0,'','Gilneas stage 12: before next milestone reward'),
(25,4755,13,0,0,8,0,24681,0,0,0,0,0,'','Gilneas stage 13: milestone rewarded');
UPDATE phase_definitions SET flags=3 WHERE zoneId=4755 AND entry BETWEEN 8 AND 13;
-- Keep the battle starter and rejoin NPC available after Push Them Out is rewarded.
UPDATE creature SET phaseMask=(phaseMask | 262144) WHERE map=654 AND id IN(37783,38553) AND (phaseMask & 131072)<>0;
-- Lorna turns in the battle, then offers The Hunt for Sylvanas after its reward.
UPDATE creature SET phaseMask=(phaseMask | 2097152) WHERE map=654 AND id=38611 AND (phaseMask & 262144)<>0;
