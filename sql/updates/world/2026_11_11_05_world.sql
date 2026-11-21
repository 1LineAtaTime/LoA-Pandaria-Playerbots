-- Whinny 83605 (Learning the Ropes 27000, This Means WAR 27001): a hit must not put the Northridge Spider in combat.
-- Ours: after the script's effect handler, Spell::DoAllEffectOnTarget made the horse CombatStart the spider. A passive spider
-- that was already fighting then evades (its fear removed, back home); any other stays in combat with nobody to fight.
-- SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE skips that combat, for the rider too.
DELETE FROM `spell_custom_attribute` WHERE `spell`=83605;
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(83605,'SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE','Whinny');
