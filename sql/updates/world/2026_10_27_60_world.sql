-- Korok the Colossus (27322, started by an item Korok drops): retail also lets High Warlord Cromush take it at the Forsaken Front
-- (45593, around Godfrey's body) and at the Battlefront (46125), as well as Forward Commander Onslaught and Cromush 45631. 45593
-- has the quest-ender flag on retail (npcflag 2) but none here.
DELETE FROM `creature_questender` WHERE `quest` = 27322 AND `id` IN (45593, 46125);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(45593, 27322),
(46125, 27322);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 45593;
