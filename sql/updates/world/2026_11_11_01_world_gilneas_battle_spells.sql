-- Pair with zone_gilneas.cpp artillery destination and friendly militia filters.
-- Reference: Arkania/ArkCORE-NG a7304c3075bf8ee7eb5bdf45f31cda01c8626b52,
-- src/server/scripts/EasternKingdoms/Gilneas/zone_gilneas_city2.cpp.
INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT 72051,'spell_gilneas_artillery_trajectory' WHERE NOT EXISTS
 (SELECT 1 FROM spell_script_names WHERE spell_id=72051 AND ScriptName='spell_gilneas_artillery_trajectory');
INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT 72013,'spell_gilneas_artillery_trajectory' WHERE NOT EXISTS
 (SELECT 1 FROM spell_script_names WHERE spell_id=72013 AND ScriptName='spell_gilneas_artillery_trajectory');
INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT 71388,'spell_gilneas_patriots_inspiration' WHERE NOT EXISTS
 (SELECT 1 FROM spell_script_names WHERE spell_id=71388 AND ScriptName='spell_gilneas_patriots_inspiration');
-- Both inspiration aura effects select nearby Gilnean Militia, not arbitrary units.
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=71388;
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment)
VALUES (13,3,71388,0,0,31,0,3,38221,0,0,0,0,'','Patriots rapier: inspire Gilnean Militia');

-- Keep quest artillery available after a driver leaves; do not hide it for two hours.
UPDATE creature SET spawntimesecs=30,spawntimesecs_max=30 WHERE map=654 AND id IN (38377,38424) AND (spawntimesecs>30 OR spawntimesecs_max>30);
