-- Epic #441 (alexkulya audit rows SQ009 / WU013). Staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- Westfall: Overloaded Harvest Golem (42381) out-of-combat "Unbound Energy" (79084) visual between golems.
-- Source: alexkulya MoP commit 1650909, sql/updates/world/2026_08_23_01_world_overloaded_harvest_golem.sql
--         (needs C++ spell_westfall_unbound_energy from src/server/scripts/EasternKingdoms/zone_westfall.cpp, same epic).
-- LoA adaptations: none needed for data (LoA 42381 already AIName=SmartAI with only row id 0, identical to
-- alexkulya's pre-fix DB); explicit column lists so LoA's extra event_param5/target_param4 take defaults.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 42381 AND `source_type` = 0 AND `id` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42381, 0, 1, 0, 1, 0, 100, 0, 3000, 6000, 3500, 6000, 11, 79084, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Overloaded Harvest Golem - OOC - Cast Spell ''Unbound Energy''');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 79084;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 79084, 0, 0, 31, 0, 3, 42381, 0, 0, 0, 0, '', 'Unbound Energy - Target Overloaded Harvest Golem');

DELETE FROM `spell_script_names` WHERE `spell_id` = 79084;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(79084, 'spell_westfall_unbound_energy');
