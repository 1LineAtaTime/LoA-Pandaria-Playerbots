-- #680 Lost City of the Tol'vir (map 755): High Prophet Barim's Plague of Ages jumps, bound to the filter script added in
-- boss_high_prophet_barim.cpp (spell_barim_plague_of_ages, port of The-Cataclysm-Preservation-Project TrinityCore 4.3.4
-- RegisterSpellScriptWithArgs "spell_barim_plague_of_ages_first" / "_second").
-- DRAFT - not applied. The PM applies it to the test `world` DB with a backup. It needs a worldserver restart (spell_script_names
-- is read at startup) and the matching build (the script names exist only from that build on).
--
-- 82622 (Barim) FORCE_CASTs 82637 on its victim, and 82637 FORCE_CASTs 82640 (5.4.8 SpellEffect.dbc eff2). 82637 and 82640 are
-- TARGET_SRC_CASTER + TARGET_UNIT_SRC_AREA_ALLY, 30 yd, with no target cap. Without these rows every ally within 30 yd is hit at
-- each jump (1 + N + N*N plagues per cast); with them each jump picks one random ally without an earlier stage.
-- Before: no spell_script_names rows for 82622 / 82637 / 82640 (world, checked 2026-10-23).
-- Undo: DELETE FROM `spell_script_names` WHERE `spell_id` IN (82637, 82640);
DELETE FROM `spell_script_names` WHERE `spell_id` IN (82637, 82640);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(82637, 'spell_barim_plague_of_ages_first'),
(82640, 'spell_barim_plague_of_ages_second');
