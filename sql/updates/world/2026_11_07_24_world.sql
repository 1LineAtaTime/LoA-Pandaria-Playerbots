-- Durnholde: Infernus's Rush of Flame 75025 (needs the C++ spell_durnholde_rush_of_flame, which only acts for Infernus 49266; twelve other NPCs' copies are unchanged).
-- Sourced: tooltip, the 20 yard radius, 75024 (DBC, Wowhead spell 75025), "Rush of Flame + Conflagration combo" closing the gap (Wowhead npc_49266 comment). Ours: 5 patches along the charge.
DELETE FROM `spell_script_names` WHERE `spell_id`=75025 AND `ScriptName`='spell_durnholde_rush_of_flame';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (75025,'spell_durnholde_rush_of_flame');
