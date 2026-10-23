-- #679 / #841 Zul'Gurub (map 859): spell script bindings for the mechanics the fork's scripts now run (boss_venoxis.cpp, boss_zanzil.cpp).
-- DRAFT - not applied. The PM applies it to the test `world` DB with a backup; needs a worldserver restart (spell_script_names is read at
-- startup) and the matching build (the script names below exist only from that build on).
--
-- 96466 Whispers of Hethiss (Venoxis): EFFECT_1 PERIODIC_TRIGGER_SPELL sits on Venoxis himself (TARGET_UNIT_CASTER, SpellEffect.dbc), so
--   the generic tick casts 96469 back on Venoxis; spell_venoxis_whispers_of_hethiss sends every tick to the channel target
--   (port of The-Cataclysm-Preservation-Project TrinityCore boss_high_priest_venoxis.cpp spell_venoxis_whispers_of_hethiss).
-- 96339 Zanzil's Graveyard Gas pulse: SCRIPT_EFFECT on TARGET_UNIT_DEST_AREA_ENTRY 100 yd with no `conditions` row (here or in
--   the MoP 5.4.8 data); spell_zanzil_graveyard_gas keeps only Zanzil's Toxic Gas 52062 (CPP spell_zanzil_zanzils_graveyard_gas).
-- 96335 Zanzil's Graveyard Gas damage: PERIODIC_DAMAGE_PERCENT ignores damage-taken modifiers in this core, so Toxic Torment 96328 (-90%
--   nature taken) did nothing; spell_zanzil_graveyard_gas_damage applies the nature damage-taken multiplier to each tick.
-- Before: no rows for these three spells (world, checked 2026-10-23). No `disables` row for any of them.
-- Undo: DELETE FROM `spell_script_names` WHERE `spell_id` IN (96466, 96339, 96335);
DELETE FROM `spell_script_names` WHERE `spell_id` IN (96466, 96339, 96335);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(96466, 'spell_venoxis_whispers_of_hethiss'),
(96339, 'spell_zanzil_graveyard_gas'),
(96335, 'spell_zanzil_graveyard_gas_damage');
