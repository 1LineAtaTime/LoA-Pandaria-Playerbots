-- Lost Isles 24958 'Volcanoth!': the Bootzooka (item 52043, spell 73066) must be fired at Volcanoth (38855).
-- The spell had two CONDITION_SOURCE_TYPE_SPELL rows for creature 38855: ConditionTarget 1 (the target, correct) and
-- ConditionTarget 0 (the caster must be Volcanoth), which no player can meet, so every cast failed with
-- SPELL_FAILED_CASTER_AURASTATE. Keep the target row only (the reference 5.4.8 data has no caster row).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceEntry` = 73066 AND `ConditionTarget` = 0;

-- Undo:
-- INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
--   (17,0,73066,0,0,31,0,3,38855,0,0,0,0,'',NULL);
