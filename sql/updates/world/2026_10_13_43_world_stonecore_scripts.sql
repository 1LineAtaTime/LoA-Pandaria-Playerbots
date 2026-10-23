-- Refs #701. The Stonecore, build 41 (needs the build: boss_slabhide.cpp, boss_ozruk.cpp, boss_high_priestess_azil.cpp).
-- Slabhide heroic Crystal Storm: 92265 (cast by the boss AI after the landing) -> aura 92301 -> 92300, the 0.5 s damage.
--   spell_slabhide_crystal_storm takes the players standing behind a landed stalactite (gameobject 204337) out of 92300's
--   targets ("Slabhide must have line of sight", Wowpedia "Slabhide" 2026-10-13).
-- Ozruk heroic Rupture: the Rupture Controller 49597 (summoned by the heroic Ground Slam through 95669) gets
--   npc_ozruk_rupture_controller, which lays the three lines of Rupture 92381 spikes (Wowpedia "Ozruk" 2026-10-13).
-- Azil Force Grip 79351 / Seismic Shard 79021: scripted in the boss AI, no rows needed (Seismic Shard NPC 42355 exists).
DELETE FROM `spell_script_names` WHERE `spell_id` = 92300 AND `ScriptName` = 'spell_slabhide_crystal_storm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (92300, 'spell_slabhide_crystal_storm');

UPDATE `creature_template` SET `ScriptName` = 'npc_ozruk_rupture_controller' WHERE `entry` = 49597 AND `ScriptName` = '';
