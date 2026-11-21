-- Eastern Plaguelands, Tyr's Hand: the six Scarlet entries (9447-9452, level 41-43 since the Cataclysm) carry Sturdy Junkboxes
-- (16884), not Heavy ones (16885).
-- Sourced: Wowhead comment of 2011-03-14 on all six: Heavy Junkboxes left these pockets with the Shattering, Sturdy ones remain.
-- Ours: the chance stays what the Heavy Junkbox had on each table (no rate is given anywhere).
DELETE FROM `pickpocketing_loot_template` WHERE `entry` IN (9447, 9448, 9449, 9450, 9451, 9452) AND `item` IN (16884, 16885);
INSERT INTO `pickpocketing_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`) VALUES
(9447, 16884, 11.6997, '', 0, 1, 1),
(9448, 16884, 12.3901, '', 0, 1, 1),
(9449, 16884, 11.8811, '', 0, 1, 1),
(9450, 16884, 11.9226, '', 0, 1, 1),
(9451, 16884, 12.5427, '', 0, 1, 1),
(9452, 16884, 12.4969, '', 0, 1, 1);
