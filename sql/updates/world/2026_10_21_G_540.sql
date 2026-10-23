-- #540 chore: remove the Westfall Jangolode trigger diagnostic binding (#485 is closed).
-- 2026_10_04_97_world_westfall_jangolode_trigger_diag.sql bound eight General Trigger spells to
-- spell_westfall_jangolode_general_trigger; the script itself is already gone from zone_westfall.cpp.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_westfall_jangolode_general_trigger';
