-- #696 Hour of Twilight, Archbishop Benedictus: Thrall's Water Shell protects from the Wave of Virtue.
-- Wowpedia "Archbishop Benedictus (tactics)" (MediaWiki API 2026-10-22): "The Water Shell protects allies from the Wave of Virtue";
-- Wowhead tooltip API 103744 Water Shell: "protects allies from the Wave of Virtue and increases damage done by 100%".
-- The wave damage spells 103684 (Wave of Virtue) / 103781 (Wave of Twilight) are DEST_CASTER + UNIT_DEST_AREA_ENEMY 15 yd
-- (SpellEffect.dbc), so they hit every player near the wave, shelled or not. spell_benedictus_wave_damage
-- (boss_archbishop_benedictus.cpp) drops the players carrying the shell pulse 103744 and those beyond the wave's 6 yd reach.
-- Phase 2 (Wave of Twilight) has no shell (Thrall is held by Engulfing Twilight), the filter only keeps the 6 yd reach there.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (103684, 103781) AND `ScriptName` = 'spell_benedictus_wave_damage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(103684, 'spell_benedictus_wave_damage'),
(103781, 'spell_benedictus_wave_damage');
