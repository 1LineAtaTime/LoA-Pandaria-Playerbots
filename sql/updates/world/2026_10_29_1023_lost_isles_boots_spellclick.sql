-- #1023 Super Booster Rocket Boots 38802 (quest 24942): the accept hook (npc_ceint OnQuestAccept) casts the
-- summon 72889, which spawns the boots and seats the player. A player who leaves the boots needs a way back in,
-- so the boots get the standard ride-vehicle spellclick (as 38526 and 38929 have). The reference 5.4.8 world DB
-- has no such row; the boots there are only reachable through the summon.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 38802;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(38802, 46598, 1, 0);
