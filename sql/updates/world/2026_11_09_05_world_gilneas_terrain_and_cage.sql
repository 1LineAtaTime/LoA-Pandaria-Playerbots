-- Gilneas map654 must select one terrain stage, not all three sibling maps.
-- Phase definitions select the same quest boundaries (14386 / 14466).
-- Condition29 is consumed by RebuildTerrainSwaps on every client phase update.
DELETE FROM conditions WHERE SourceTypeOrReferenceId=29 AND SourceGroup=0 AND SourceEntry IN (638,655,656) AND SourceId=0;
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment) VALUES
(29,0,638,0,0,8,0,14386,0,0,1,0,0,'','Gilneas: original terrain until Leader of the Pack rewarded'),
(29,0,655,0,0,8,0,14386,0,0,0,0,0,'','Gilneas: first terrain after Leader of the Pack'),
(29,0,655,0,0,8,0,14466,0,0,1,0,0,'','Gilneas: first terrain until Kings Observatory rewarded'),
(29,0,656,0,0,8,0,14466,0,0,0,0,0,'','Gilneas: final terrain after Kings Observatory');
-- quest_end_status is an ALLOWED-status mask, not a remove-on-status mask.
--11 = none/complete/incomplete;64 would retain these auras AFTER the end reward.
UPDATE spell_area SET quest_end_status=11 WHERE spell IN (49416,59073,72872,76642) AND area IN (4714,4755) AND quest_end IN (14159,14293,14221,14222) AND quest_end_status=64;
-- Server base-map areas can remain4714/4755 while the client overlays Duskhaven.
-- The post-Last Stand transition is zone-wide and must not depend on overlay area4786.
DELETE FROM spell_area WHERE spell=68630 AND area IN (4714,4755) AND quest_start=14222 AND quest_end=14375;
INSERT INTO spell_area (spell,area,quest_start,quest_end,aura_spell,racemask,gender,autocast,quest_start_status,quest_end_status) VALUES
(68630,4714,14222,14375,0,2097152,2,1,64,11),
(68630,4755,14222,14375,0,2097152,2,1,64,11);
