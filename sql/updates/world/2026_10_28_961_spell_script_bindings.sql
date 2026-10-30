-- Issue #961: boot-time "did not match dbc effect data" errors for spell scripts.
-- Every row below is a spell_script_names binding whose spell (5.4.8 SpellEffect.dbc + SpellMgr corrections)
-- has no effect/target/aura the script's hooks can attach to, so the handlers never ran. Each is either moved to
-- the spell that really carries the effect, or removed (the binding was inert). The code side of the other
-- mismatches is in src/server/scripts (see the issue comment).

-- Tranquility (Symbiosis): heal is 113278 (comment in spell_common.cpp), 113277 is the periodic trigger aura.
UPDATE `spell_script_names` SET `spell_id` = 113278 WHERE `spell_id` = 113277 AND `ScriptName` = 'spell_common_smart_heal_raid_25';

-- 116267 Incanter's Absorption is a damage-done buff, not an absorb (the script is the WotLK absorb trigger).
DELETE FROM `spell_script_names` WHERE `spell_id` = 116267 AND `ScriptName` = 'spell_mage_incanters_absorbtion_absorb';

-- 116779 Titan Gas is the periodic trigger aura (comments say it triggers 116803 / 118327, which keep their rows).
DELETE FROM `spell_script_names` WHERE `spell_id` = 116779 AND `ScriptName` IN ('spell_titan_gas', 'spell_titan_gas2');

-- Xin the Weaponmaster: 119311 is Stream of Blades damage (spell_dart_damage already bound), 119338 is the aura-only
-- Stream of Blades; 120142 is the crossbow Dart (random enemy in SRC_AREA_ENEMY) = spell_crossbow_xin
-- (SPELL_DART_DAMAGE in the script), 120124 is Healing Potion.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (119311, 119338, 120142) AND `ScriptName` = 'spell_dart';
UPDATE `spell_script_names` SET `spell_id` = 120142 WHERE `spell_id` = 120124 AND `ScriptName` = 'spell_crossbow_xin';

-- Shadopan Apparitions: 111698 is only the periodic trigger aura; 112060 keeps the absorb calc.
DELETE FROM `spell_script_names` WHERE `spell_id` = 111698 AND `ScriptName` = 'spell_shadopan_apparitions';

-- Rimok Bombard: the player filter belongs to the triggered spell 120202 (new loader), 120559 keeps the aura script.
DELETE FROM `spell_script_names` WHERE `spell_id` = 120202 AND `ScriptName` = 'spell_rimok_saboteur_bombard_trigger';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (120202, 'spell_rimok_saboteur_bombard_trigger');

-- Tsulong: 123740 only triggers 123742; the nearest-target fixate has nothing to select on it.
DELETE FROM `spell_script_names` WHERE `spell_id` = 123740 AND `ScriptName` = 'spell_dark_of_night_fixate';

-- Brawler's Guild Rain Dance: the random destination (TARGET_DEST_CASTER_RANDOM) is on the triggered summon 124864.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (124860, 124864) AND `ScriptName` = 'spell_brawlers_guild_rain_dance';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (124864, 'spell_brawlers_guild_rain_dance');

-- Shared Torment 145655 only targets the caster; the SRC_AREA_ENTRY filter had nothing to attach to.
DELETE FROM `spell_script_names` WHERE `spell_id` = 145655 AND `ScriptName` = 'spell_fallen_protectors_shared_torment';

-- 77827 Tail Lash: the Lightning Discharge script clears targets for any id outside the four discharge spells.
DELETE FROM `spell_script_names` WHERE `spell_id` = 77827 AND `ScriptName` = 'spell_onyxia_lightning_discharge';

-- 26013 is Deserter, not Crane Wings (144385 keeps the row).
DELETE FROM `spell_script_names` WHERE `spell_id` = 26013 AND `ScriptName` = 'spell_timeless_isle_crane_wings';

-- 25040 Mark of Nature is a dummy aura on self; the script targets a spell (25042) that is absent from the 5.4.8 data.
DELETE FROM `spell_script_names` WHERE `spell_id` = 25040 AND `ScriptName` = 'spell_mark_of_nature';

-- 70739 Geist Alarm has SEND_EVENT where 70740 has the APPLY_AURA the handler hooks; 70740 keeps the row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 70739 AND `ScriptName` = 'spell_icc_geist_alarm';

-- 85123 Siege Cannon is a trajectory summon with no SRC_AREA_ENTRY target.
DELETE FROM `spell_script_names` WHERE `spell_id` = 85123 AND `ScriptName` = 'spell_siege_cannon';
