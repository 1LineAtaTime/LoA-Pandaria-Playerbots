-- Studies in Lethality (28324): sampling a Blight Slime (48136) with the Lethality Analyzer must not start combat.
-- Collect Specimen (89752) and Depleted (89756) carry SPELL_ATTR1_NO_THREAT in the 5.4.8 DBC, which this core does not treat
-- as "no initial aggro"; SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE does that per spell, as for Polymorph and Earthbind.
DELETE FROM `spell_custom_attribute` WHERE `spell` IN (89752, 89756) AND `attribute` = 'SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE';
INSERT INTO `spell_custom_attribute` (`spell`, `attribute`, `comment`) VALUES
(89752, 'SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE', 'Collect Specimen'),
(89756, 'SPELL_ATTR0_CU_DONT_SET_IN_COMBAT_STATE', 'Depleted');
