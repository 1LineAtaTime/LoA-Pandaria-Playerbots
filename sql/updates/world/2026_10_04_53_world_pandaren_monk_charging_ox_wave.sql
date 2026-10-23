-- Monk talent Charging Ox Wave (epic #440 Monk audit). Port of upstream Legends-of-Azeroth commit cdff4343 (#463,
-- 2026-10-02, sql/updates/world/2026_10_01_00_world.sql). spell_monk_charging_ox_wave narrows the cone targets to
-- the ones in the ox's path and copies them to the stun effect. It was bound to 125084 (the visual, TARGET 47, no cone
-- targets); the test server logs at boot:
--   Spell `125084` Effect `Index: EFFECT_0 Target: 129` of script `spell_monk_charging_ox_wave` did not match dbc
--   effect data - handler bound to hook `OnObjectAreaTargetSelect` of SpellScript won't be executed
-- so the stun 119392 hit every enemy in the 30 yd cone instead of the line. 119392 effects 0 and 1 use
-- TARGET_UNIT_CONE_ENEMY_129 (verified in SpellEffect.dbc of the test server data). spell_monk_charging_ox_wave_visual
-- stays on 125084. SQL-only.
--
-- Rollback:
--   DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_charging_ox_wave';
--   INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (125084, 'spell_monk_charging_ox_wave');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_charging_ox_wave';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(119392, 'spell_monk_charging_ox_wave');
