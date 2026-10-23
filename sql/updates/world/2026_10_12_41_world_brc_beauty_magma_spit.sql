-- Refs #699. Blackrock Caverns, Beauty: bind Magma Spit 76031 to spell_beauty_magma_spit (new AuraScript in
-- boss_beauty.cpp, needs the build). Wowhead Cata 76031: "Explodes when removed." On removal (expiry or dispel, not
-- death) the victim launches the four Magma Spit missiles 76058 / 76072 / 76074 / 76076 -> Magma Blast 76070.
-- No row for 76031 in the test world DB before this.
DELETE FROM `spell_script_names` WHERE `spell_id` = 76031 AND `ScriptName` = 'spell_beauty_magma_spit';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (76031, 'spell_beauty_magma_spit');
