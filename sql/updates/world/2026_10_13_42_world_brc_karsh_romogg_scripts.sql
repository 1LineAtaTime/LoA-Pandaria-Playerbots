-- Refs #699. Blackrock Caverns, build 41 (needs the build: boss_karsh_steelbender.cpp, boss_romogg_bonecrusher.cpp).
-- Karsh: Heat Wave 75851 damage is multiplied by the stacks of Superheated Quicksilver Armor (Wowpedia "Karsh Steelbender"),
--   spell_karsh_heat_wave. Lava Spout 76007 / Bound Flames need no row (cast by boss_karsh_steelbender AI / the 75846
--   AuraScript, which is already bound).
-- Rom'ogg: Quake 75272 effect 1 (script effect on every enemy, bp 75372 = Quake NPC summon) is turned into the ground crack
--   by spell_romogg_quake; the Quake NPC 40401 (ground crack, no AI before) ruptures 3 s later (Ground Rupture 75347)
--   through npc_romogg_quake. Wowpedia "Rom'ogg Bonecrusher" (2026-10-13).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (75851, 75272) AND `ScriptName` IN ('spell_karsh_heat_wave', 'spell_romogg_quake');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(75851, 'spell_karsh_heat_wave'),
(75272, 'spell_romogg_quake');

UPDATE `creature_template` SET `ScriptName` = 'npc_romogg_quake' WHERE `entry` = 40401 AND `ScriptName` = '';
