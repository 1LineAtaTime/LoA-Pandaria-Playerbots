-- Refs #540 (closes the Westfall half; #485 was closed 2026-10-09)
-- Remove the temporary [J441] diagnostic binding that 2026_10_04_97_world_westfall_jangolode_trigger_diag.sql added:
-- the 8 "Livin' the Life" General Trigger spells (Shadowy Figure 79279/79283/79284/79287, Glubtok 79290/79292/79294/79297)
-- were bound to spell_westfall_jangolode_general_trigger, a logging-only SpellScript. The class is deleted from
-- src/server/scripts/EasternKingdoms/zone_westfall.cpp in the same build (together with the LogCredit hook on 79275), so
-- a leftover spell_script_names row would only make the loader report an unknown script name. The rows are the whole
-- of what _97 inserted; nothing else (the smart_scripts rewrite is _98, the 79275 binding is _75) is touched.
-- Rollback: re-run 2026_10_04_97_world_westfall_jangolode_trigger_diag.sql (it needs the C++ class back).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_westfall_jangolode_general_trigger';
