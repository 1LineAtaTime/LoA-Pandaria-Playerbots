-- Second battle for Andorhal: the Andorhal Defenders 44448 hold the line against the Deathguards too (npc_wpl_forsaken_trooper).
-- Sourced: Supporting the Troops (Horde) 27085, kill 15 Defenders; the Alliance mirror 27198 "Our defenders are equally matched
-- with their deathguards"; Ace in the Hole 27089 comment (Wowhead, 2011-04-15) "your minions will beat up everything".
-- Ours: the 85% value (the battle's other soldiers, 2026_11_10_95 and 2026_11_12_11). A Defender the Deathguards had worn below
-- half gave a Horde player no 27085 credit (no credit unless players dealt half the health). The mobilized Forsaken Troopers of
-- 27089 are now owned by and controlled by their player (C++ in this commit), so the limit does not stop them and their damage
-- counts as his.
DELETE FROM `creature_sparring_template` WHERE `creature_id`=44448;
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(44448,85,'Andorhal Defender - second battle for Andorhal');
