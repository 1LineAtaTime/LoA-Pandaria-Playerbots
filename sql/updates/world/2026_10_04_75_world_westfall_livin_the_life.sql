-- Epic #441 (alexkulya audit rows SQ008 / WU002). Staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- Westfall: quest 26228 "Livin' the Life" (Jangolode Mine vehicle scene with Glubtok and the Shadowy Figure).
-- Source: alexkulya MoP commit 2c2de8c, sql/updates/world/2026_08_24_01_world_quest_livin'_the_life.sql
--         (needs C++ spell_westfall_quest_credit_jangolode_event from src/server/scripts/EasternKingdoms/zone_westfall.cpp, same epic).
-- Replaces LoA's older SAI for 42500/42492 (identical to alexkulya's pre-fix DB: house summoned the wrong
-- Shadowy Figure entry 34867, which has no SAI, so half of the dialogue never played; no spellclick so the
-- summoner is not seated in the house vehicle; no despawn of the actors).
-- LoA adaptations:
--  * creature_text / creature_text_locale sections dropped: LoA already has the exact same texts and emotes
--    for 42492 and 42515 (groups 0-4 / 0-5, with BroadcastTextId). The DELETE of creature 34867's texts is
--    dropped too (34867 has 10 other spawns in LoA; its texts are harmless).
--  * creature_template_addon for 42515: alexkulya bytes2=4097 -> LoA SheathState=1, PvPFlags=16 (same mapping
--    LoA's own conversion used for 13,440 rows).
--  * explicit smart_scripts/conditions column lists (LoA has extra event_param5/target_param4/ErrorTextId).

DELETE FROM `spell_script_names` WHERE `spell_id` = 79275;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(79275, 'spell_westfall_quest_credit_jangolode_event');

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 42500;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(42500, 46598, 1, 0);

DELETE FROM `spell_linked_spell` WHERE `spell_trigger` = 79262;
INSERT INTO `spell_linked_spell` (`spell_trigger`, `spell_effect`, `type`, `comment`) VALUES
(79262, 79265, 0, 'Summon Lou''s House - Summon Shadowy Figure'),
(79262, 79263, 0, 'Summon Lou''s House - Summon Glubtok');

UPDATE `creature_template` SET `unit_flags` = 520, `VehicleId` = 899, `AIName` = 'SmartAI' WHERE `entry` = 42500;
UPDATE `creature_template` SET `unit_flags` = 33587976, `AIName` = 'SmartAI' WHERE `entry` = 42515;
UPDATE `creature_template` SET `type_flags` = 0, `unit_class` = 2, `unit_flags` = 33554504, `AIName` = 'SmartAI' WHERE `entry` = 42492;

DELETE FROM `creature_template_addon` WHERE `entry` = 42515;
INSERT INTO `creature_template_addon` (`entry`, `SheathState`, `PvPFlags`, `auras`) VALUES
(42515, 1, 16, '79192');

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (42500, 42515, 42492) AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4250000, 4251500, 4251501, 4249200) AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42500, 0, 0, 0, 63, 0, 100, 0, 0, 0, 0, 0, 80, 4250000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Just Created - Start Script'),
(42500, 0, 1, 0, 28, 0, 100, 0, 0, 0, 0, 0, 11, 79274, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Passenger Removed - Cast Spell ''Force Despawn Jangolode Actors'''),
(4250000, 9, 0, 0, 0, 0, 100, 0, 9300, 9300, 0, 0, 11, 79290, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 1: Glubtok'''),
(4250000, 9, 1, 0, 0, 0, 100, 0, 6500, 6500, 0, 0, 11, 79279, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 1: Figure'''),
(4250000, 9, 2, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 11, 79292, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 2: Glubtok'''),
(4250000, 9, 3, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 11, 79283, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 2: Figure'''),
(4250000, 9, 4, 0, 0, 0, 100, 0, 16000, 16000, 0, 0, 11, 79294, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 3: Glubtok'''),
(4250000, 9, 5, 0, 0, 0, 100, 0, 4100, 4100, 0, 0, 11, 79284, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 3: Figure'''),
(4250000, 9, 6, 0, 0, 0, 100, 0, 6400, 6400, 0, 0, 11, 79297, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 4: Glubtok'''),
(4250000, 9, 7, 0, 0, 0, 100, 0, 11400, 11400, 0, 0, 11, 79287, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Cast Spell ''General Trigger 4: Figure'''),
(42515, 0, 0, 1, 63, 0, 100, 0, 0, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Just Created - Set Run (0)'),
(42515, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, -9841.68, 1400.04, 37.1295, 0, 'Shadowy Figure - On Just Created (Link) - Move To Position'),
(42515, 0, 2, 0, 8, 0, 100, 0, 79273, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79273) - Despawn'),
(42515, 0, 3, 4, 8, 0, 100, 0, 79279, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79279) - Say Text Line 0'),
(42515, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 128, 610, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79279) (Link) - Play Anim Kit (610)'),
(42515, 0, 5, 0, 8, 0, 100, 0, 79283, 0, 0, 0, 80, 4251500, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79283) - Start Script'),
(42515, 0, 6, 0, 8, 0, 100, 0, 79284, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79284) - Say Text Line 3'),
(42515, 0, 7, 0, 8, 0, 100, 0, 79287, 0, 0, 0, 80, 4251501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Spellhit (79287) - Start Script'),
(4251500, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Say Text Line 1'),
(4251500, 9, 1, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 128, 593, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Play Anim Kit (593)'),
(4251500, 9, 2, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Say Text Line 2'),
(4251500, 9, 3, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 128, 606, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Play Anim Kit (606)'),
(4251501, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Say Text Line 4'),
(4251501, 9, 1, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Say Text Line 5'),
(4251501, 9, 2, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 11, 79275, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Cast Spell ''Quest Credit: Jangolode Event'''),
(4251501, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 24222, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowy Figure - On Script - Cast Spell ''Vanish Visual'''),
(42492, 0, 0, 0, 60, 0, 100, 1, 1, 1, 0, 0, 11, 64195, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Update - Cast Spell ''Simple Teleport'' (No Repeat)'),
(42492, 0, 1, 0, 8, 0, 100, 0, 79273, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Spellhit (79273) - Despawn'),
(42492, 0, 2, 0, 8, 0, 100, 0, 79290, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Spellhit (79290) - Say Text Line 0'),
(42492, 0, 3, 0, 8, 0, 100, 0, 79292, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Spellhit (79292) - Say Text Line 1'),
(42492, 0, 4, 0, 8, 0, 100, 0, 79294, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Spellhit (79294) - Say Text Line 2'),
(42492, 0, 5, 0, 8, 0, 100, 0, 79297, 0, 0, 0, 80, 4249200, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Spellhit (79297) - Start Script'),
(4249200, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Script - Say Text Line 3'),
(4249200, 9, 1, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glubtok - On Script - Say Text Line 4');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (79273, 79290, 79292, 79294, 79297, 79279, 79283, 79284, 79287);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 79273, 0, 0, 31, 0, 3, 42515, 0, 0, 0, 0, '', 'Despawn Jangolode Actors - Target Shadowy Figure'),
(13, 1, 79273, 0, 1, 31, 0, 3, 42492, 0, 0, 0, 0, '', 'Despawn Jangolode Actors - Target Glubtok'),
(13, 1, 79290, 0, 0, 31, 0, 3, 42492, 0, 0, 0, 0, '', 'General Trigger 1: Glubtok - Target Glubtok'),
(13, 1, 79292, 0, 0, 31, 0, 3, 42492, 0, 0, 0, 0, '', 'General Trigger 2: Glubtok - Target Glubtok'),
(13, 1, 79294, 0, 0, 31, 0, 3, 42492, 0, 0, 0, 0, '', 'General Trigger 3: Glubtok - Target Glubtok'),
(13, 1, 79297, 0, 0, 31, 0, 3, 42492, 0, 0, 0, 0, '', 'General Trigger 4: Glubtok - Target Glubtok'),
(13, 1, 79279, 0, 0, 31, 0, 3, 42515, 0, 0, 0, 0, '', 'General Trigger 1: Figure - Shadowy Figure'),
(13, 1, 79283, 0, 0, 31, 0, 3, 42515, 0, 0, 0, 0, '', 'General Trigger 2: Figure - Shadowy Figure'),
(13, 1, 79284, 0, 0, 31, 0, 3, 42515, 0, 0, 0, 0, '', 'General Trigger 3: Figure - Shadowy Figure'),
(13, 1, 79287, 0, 0, 31, 0, 3, 42515, 0, 0, 0, 0, '', 'General Trigger 4: Figure - Shadowy Figure');
