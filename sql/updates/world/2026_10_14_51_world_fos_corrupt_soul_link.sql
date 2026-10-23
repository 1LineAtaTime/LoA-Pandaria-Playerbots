-- #563 FoS Bronjahm: Corrupt Soul (68839) expiry summons the Corrupted Soul Fragment via 68846 Draw Corrupted Soul.
-- AC data/sql/base/db_world/spell_linked_spell.sql:65; missing on MoP, so no fragment ever spawned.
DELETE FROM `spell_linked_spell` WHERE `spell_trigger`=-68839 AND `spell_effect`=68846;
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`,`type`,`comment`) VALUES
(-68839, 68846, 0, 'Draw Corrupted Soul at the end of Corrupt Soul');
