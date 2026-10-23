-- Quest 29422 "Huo, the Spirit of Fire" (epic #440, issue #464): undo 2026_10_04_50_world_pandaren_huo_fan_the_flames.sql.
-- #464 was a false positive. The LoA world DB already gives the "Reignite the Spirit of Fire" credit through
-- spell_scripts: 102522 "Fan the Flames" effect 1 schedules SCRIPT_COMMAND_CAST_SPELL 109090 (+2 s), 109095 (+5 s),
-- 109105 (+8 s) and 109109 (+11 s); 109109 effect 1 triggers 109107 "Fan the Flames: Credit" (kill credit 54787).
-- The MoP 5.4.8 data has the same four rows. t440 r2/r3 proved it: objective 252007 was credited even when the
-- channel aura was removed early, i.e. without the spell_fan_the_flames AuraScript. That AuraScript duplicated the
-- credit and is removed from zone_wandering_island_north.cpp in the same change (core build).
--
-- Rollback: INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (102522, 'spell_fan_the_flames');
--           (only together with the C++ class, which this change removes)

DELETE FROM `spell_script_names` WHERE `spell_id` = 102522 AND `ScriptName` = 'spell_fan_the_flames';
