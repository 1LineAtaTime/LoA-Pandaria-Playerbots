-- Split periodic producers from their actual destination/target spells.
-- The original selector definition for legacy Mark of Nature25042 (absent in
-- 5.4.8) uses radius12=100 yards. The native AuraScript now performs that check
-- on periodic25041; death marker25040 must remain an ordinary marker aura.
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName)
SELECT 25041,ScriptName FROM spell_script_names WHERE spell_id=25040 AND ScriptName='spell_mark_of_nature';
DELETE FROM spell_script_names WHERE spell_id=25040 AND ScriptName='spell_mark_of_nature';
-- Producer116779 already periodically triggers both actual aura spells. Their
-- target selectors remain; recursive reapplication AuraScripts were removed.
DELETE FROM spell_script_names WHERE spell_id=116779 AND ScriptName IN ('spell_titan_gas','spell_titan_gas2');
-- Rain Dance124860 supplies native periodic missiles. Random arena positioning
-- belongs to missile124864; preserve the other native missile124867.
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName)
SELECT 124864,ScriptName FROM spell_script_names WHERE spell_id=124860 AND ScriptName='spell_brawlers_guild_rain_dance';
DELETE FROM spell_script_names WHERE spell_id=124860 AND ScriptName='spell_brawlers_guild_rain_dance';
-- The fixate aura123740 produces the actual area selection spell123742.
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName)
SELECT 123742,ScriptName FROM spell_script_names WHERE spell_id=123740 AND ScriptName='spell_dark_of_night_fixate';
DELETE FROM spell_script_names WHERE spell_id=123740 AND ScriptName='spell_dark_of_night_fixate';
-- Healing Potion120124 is not Xin's crossbow selector. Preserve its native heal.
DELETE FROM spell_script_names WHERE spell_id=120124 AND ScriptName='spell_crossbow_xin';
-- Stream of Blades119338 is a self aura; retain actual cone/area selectors.
DELETE FROM spell_script_names WHERE spell_id=119338 AND ScriptName='spell_dart';
-- Inhale122853 is damage, not a script-effect carrying a spell ID. Boss122852
-- native stack aura/Exhale scaling remains implemented in boss_zorlok.cpp.
DELETE FROM spell_script_names WHERE spell_id=122853 AND ScriptName='spell_inhale';
-- Devastating Arc117006 natively triggers116835, whose real damage/opportunity
-- handler stays bound. Its old AuraScript tried AddAura of a non-aura spell.
DELETE FROM spell_script_names WHERE spell_id=117006 AND ScriptName='spell_devastating_arc';
-- Both old Storm Unleashed aura callbacks tried AddAura123616, which is a
-- non-aura dummy spell. Preserve actual vehicle aura123599, ride script124258,
-- wind/tornado AI and summon/visual scripts; these unused callbacks cannot run.
DELETE FROM spell_script_names WHERE spell_id=123600 AND ScriptName='spell_su_dummy';
DELETE FROM spell_script_names WHERE spell_id=123616 AND ScriptName='spell_su_dumaura';
