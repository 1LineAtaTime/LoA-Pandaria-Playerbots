-- City guards: rebind creature_template.ScriptName to the renamed guard classes (epic #441, audit rows
-- CR001 / CR071 / CR072 / SQ003).
--
-- Commit d6be9cee ("Mordenize Combat System part 2 (#427)", 2026-09-08) rewrote
-- src/server/scripts/World/guards.cpp and renamed the registered scripts (guards.cpp:252-253):
--   guard_generic                                  -> npc_guard_generic
--   guard_shattrath_aldor / guard_shattrath_scryer -> npc_guard_shattrath_faction
-- It shipped no DB update, so 18 + 2 creature_template rows (Stormwind City Guard 68, Orgrimmar Grunt 3296,
-- Ironforge Guard 5595, ..., Aldor Vindicator 18549, Scryer Arcane Guardian 18568) still point at names
-- that are no longer registered and load with no AI (no self-buffs, no combat spells, no emote replies,
-- no Shattrath banish).
-- The two UPDATEs are byte-identical to TrinityCore's own migration for the same rename:
--   TrinityCore/TrinityCore sql/old/9.x/world/20101_2021_07_04/2021_02_08_05_world_2017_11_22_03_world.sql
-- Stormwind City Patroller (1976): npc_guard_generic handles it explicitly (NPC_STORMWIND_CITY_PATROLLER,
-- ReceiveEmote), and MoP 5.4.8 binds it to the guard script, but LoA has AIName='SmartAI' with zero
-- smart_scripts rows (an empty AI). Bound to npc_guard_generic, guarded so a later SAI port is not overwritten.
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = 'guard_generic' WHERE `ScriptName` = 'npc_guard_generic' AND `entry` <> 1976;
--   UPDATE `creature_template` SET `ScriptName` = 'guard_shattrath_aldor' WHERE `entry` = 18549;
--   UPDATE `creature_template` SET `ScriptName` = 'guard_shattrath_scryer' WHERE `entry` = 18568;
--   UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 1976;

UPDATE `creature_template` SET `ScriptName` = 'npc_guard_generic'           WHERE `ScriptName` = 'guard_generic';
UPDATE `creature_template` SET `ScriptName` = 'npc_guard_shattrath_faction' WHERE `ScriptName` IN ('guard_shattrath_aldor', 'guard_shattrath_scryer');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_guard_generic'
WHERE `entry` = 1976 AND `ScriptName` = ''
  AND NOT EXISTS (SELECT 1 FROM (SELECT `entryorguid` FROM `smart_scripts` WHERE `entryorguid` = 1976 AND `source_type` = 0) AS s);
