-- Death Knight Soul Reaper (114866 blood, 130735 frost, 130736 unholy): spell_dk_soul_reaper now hooks effect 1's
-- SPELL_AURA_PERIODIC_DUMMY, so it is bound to the three player spells again. It comes off 69409, the Lich King's Soul
-- Reaper in Icecrown Citadel, which keeps its own spell_the_lich_king_soul_reaper: the death-knight script made the boss
-- gain Soul Reaper's haste when its target died.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_soul_reaper' AND `spell_id` IN (69409, 114866, 130735, 130736);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(114866, 'spell_dk_soul_reaper'),
(130735, 'spell_dk_soul_reaper'),
(130736, 'spell_dk_soul_reaper');

-- Track Humanoids (5225) for Glyph of the Predator, and Shadowfiend (34433): their scripts handle these ids (as for
-- 114280 and Mindbender 123040), but only the other id was bound.
DELETE FROM `spell_script_names` WHERE (`spell_id`, `ScriptName`) IN ((5225, 'spell_dru_glyph_of_the_predator'), (34433, 'spell_pri_shadowfiend'));
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5225, 'spell_dru_glyph_of_the_predator'),
(34433, 'spell_pri_shadowfiend');
