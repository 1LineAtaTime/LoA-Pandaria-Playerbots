-- A Crazy Grab Bag 15902: the three uncommon items its own reference 15902 (2026_11_15_03) left out.
--
-- Sourced: 4.4.2 TDB, item_loot_template 15902 -> references 24016, 24031, 24033 (304 items). Compared with our reference
--   15902 (299 items, all uncommon) it has five more: 18672 Elemental Ember, 18674 Hardened Stone Band, 18679 Frigid Ring
--   (uncommon, item level 59-61 like the rest; none of them is in any of our reference tables) and 18337 Orphic Bracers,
--   18339 Eidolon Cloak (rare, item level 45).
-- Ours: the three uncommon items join the bag's one equal-chance group (groupid 1, chance 0) like its other rows; the two
--   rare items stay out, as the bag holds only uncommon items here.

DELETE FROM `reference_loot_template` WHERE `entry` = 15902 AND `item` IN (18672, 18674, 18679);
INSERT INTO `reference_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(15902,18672,0,'',1,1,1),
(15902,18674,0,'',1,1,1),
(15902,18679,0,'',1,1,1);
