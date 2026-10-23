-- Refs #844: conditions rows of type 18 carry Cata/MoP semantics (value1 0=gameobject, 1=creature, value2=entry);
-- on this core 18 is CONDITION_TITLE. Convert to CONDITION_OBJECT_ENTRY_GUID (31): value1 = TypeID (5 gameobject, 3 unit), value2 = entry.
-- 37 rows (all SourceTypeOrReferenceId 13 = spell implicit target). Checked: 21 rows (spell 60535) point at gameobject_template
-- "Lamp Post"; the other 16 point at creature_template entries. No type-31 rows exist for these spells (no PK clash).
-- Undo: UPDATE conditions SET ConditionTypeOrReference=18, ConditionValue1=IF(ConditionValue1=5,0,1) WHERE SourceTypeOrReferenceId=13 AND ConditionTypeOrReference=31 AND SourceEntry IN (<list below>);
-- VERIFY: SELECT COUNT(*) FROM conditions WHERE ConditionTypeOrReference=18; -> 0 (and no "Title condition" lines in DBErrors).
UPDATE `conditions` SET `ConditionTypeOrReference`=31, `ConditionValue1`=5
 WHERE `SourceTypeOrReferenceId`=13 AND `ConditionTypeOrReference`=18 AND `ConditionValue1`=0 AND `SourceEntry`=60535;
UPDATE `conditions` SET `ConditionTypeOrReference`=31, `ConditionValue1`=3
 WHERE `SourceTypeOrReferenceId`=13 AND `ConditionTypeOrReference`=18 AND `ConditionValue1`=1
   AND `SourceEntry` IN (70586,74634,74670,75763,75809,76029,76036,76095,76097,76816,82224,83151,83235,83445,83466,91277);
-- PM 2026-10-21: one more type-18 row the lane missed (SourceType 17 = spell, not 13): Throw Quel'delar 70586 on the skull 37852.
UPDATE `conditions` SET `ConditionTypeOrReference`=31, `ConditionValue1`=3
 WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=70586 AND `ConditionTypeOrReference`=18 AND `ConditionValue1`=1 AND `ConditionValue2`=37852;
