-- Epic #441 (alexkulya audit rows CR017 / SQ004). Staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- Greenstone Village scenario: bind Belligerent Blossom (61183) to its C++ AI.
--
-- src/server/scripts/Pandaria/Scenarios/GreenstoneVillage/greenstone_village.cpp registers
-- class npc_greenstone_belligerent_blossom ("// Belligerent Blossom 61183") under the
-- ScriptName 'npc_greenstone_terror' (pillager-type AI: cosmetic wood strikes, aggressive
-- react state, Pollen Puff 118819 every 3-8 s). No creature_template row uses that name:
-- the 2023 DB-errors fix (sql/archive .../2023_1_18_00_fix_db_error.sql) wrote the bind to
-- the non-existent entry 611590 (typo for 61183), so the class was never used and 61183 fell
-- back to a single SmartAI row (Pollen Puff every 11 s).
--
-- Source: alexkulya-mop-audit row SQ004 (CR017), re-verified against the LoA world DB
-- and LoA source on 2026-09-23.

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_greenstone_terror' WHERE `entry` = 61183;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 61183 AND `source_type` = 0;
