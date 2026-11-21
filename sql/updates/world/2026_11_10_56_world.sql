-- Western Plaguelands: remove quest starters that are not sniffed.
-- Sourced: the Wowhead MoP Classic Quick Facts name only the other starter of each quest, and neither TDB (4.4.2, 1210) has these rows,
-- except the Hearthglen five, which both TDBs carry with VerifiedBuild 0 (not from a sniff):
--  * Victory, For Now: High Executor Derrington 10837 (26926) and Commander Ashlam Valorfist 10838 (27165) are their enders; Koltira and
--    Thassarian start them. A player could take and hand in each at the camp, skipping the trip from Andorhal.
--  * The Battle for Andorhal 27158 and 28749: Thassarian 44453 is their ender (Valorfist and War-Mage Erallier start them).
--  * The Menders' Stead 27175: Field Agent Kaartish 44454 is its ender (Argent Officer Pureheart starts it).
--  * Northridge Lumber Mill 26935: Nathaniel Dumah 11616 is its ender (Kaartish starts it).
--  * The Abandoned Crypt 27170: the Broken Weapons Crate 205258 is its ender (High Priest Thel'danis starts it); the crate still starts
--    Ambushed! 27171.
--  * The Good People of Hearthglen 27153 and Bagging Bisp 27154: Morris Vant 45148, Outfitter Mendelev 45149, Bree Ironstock 45150, High
--    Cleric Alphus 45151 and Magus Bisp 45152 are the quests' gossip targets (Daria L'Rayne starts both, and ends them).
-- Each quest keeps its sourced starter. The NPCs keep their npcflags, as in both TDBs.
DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN ((10837,26926),(10838,27165),(44453,27158),(44453,28749),(44454,27175),(11616,26935),
(45148,27153),(45149,27153),(45150,27153),(45151,27153),(45152,27154));
DELETE FROM `gameobject_queststarter` WHERE `id`=205258 AND `quest`=27170;
