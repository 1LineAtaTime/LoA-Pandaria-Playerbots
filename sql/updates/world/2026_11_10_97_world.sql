-- Escorting Zen'Kiki 83819: dismiss Zen'Kiki 44904 when the aura goes (quest abandoned or handed in, zone left).
-- Ours: the spell script. 83819 summons him with effect 0 (SPELL_EFFECT_SUMMON), which isn't bound to its dummy aura (effect 2).
DELETE FROM `spell_script_names` WHERE `spell_id`=83819 AND `ScriptName`='spell_wpl_escorting_zenkiki';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (83819,'spell_wpl_escorting_zenkiki');
