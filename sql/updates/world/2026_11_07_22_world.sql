-- Durnholde: Teracula's Rupture Line 89932 (needs the C++ spell_durnholde_rupture_line, which only acts for Teracula 49265).
-- Sourced: tooltip and missile chain (DBC, Wowhead spell 89932), "takes 2 seconds to erupt", can be dodged (Wowhead 28644 comment). Ours: the line's shape.
DELETE FROM `spell_script_names` WHERE `spell_id`=89932 AND `ScriptName`='spell_durnholde_rupture_line';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (89932,'spell_durnholde_rupture_line');
