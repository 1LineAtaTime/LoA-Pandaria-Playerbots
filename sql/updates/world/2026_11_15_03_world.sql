-- A Crazy Grab Bag 15902 (Augustus the Touched, Eastern Plaguelands) always holds one item.
--
-- Reported in game: the bag opened empty. Its loot was six references (8245, 8283, 8284, 8288, 8289, 8292: 299 uncommon items
-- of item level 56-61), each rolled by itself at 12.2-18.65 %: no item at all in about 38 of 100 bags, two or more in others.
-- A reference row cannot be put in a group to roll exactly one (LootTemplate::Process hands the row's groupid to the
-- referenced template as the group to roll), so the bag gets one reference of its own that holds the same 299 items in one
-- equal-chance group, taken at 100 %.
--
-- Ours: one item per bag, each of the 299 equally likely (before, the six lists weighed 12-19 % each; they hold 47-52 items,
--   so the difference per item is small). The six shared references are not changed.

DELETE FROM `reference_loot_template` WHERE `entry` = 15902;
INSERT INTO `reference_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`)
SELECT DISTINCT 15902, `item`, 0, '', 1, 1, 1 FROM `reference_loot_template`
WHERE `entry` IN (8245,8283,8284,8288,8289,8292) AND `mincountOrRef` > 0;

DELETE FROM `item_loot_template` WHERE `entry` = 15902;
INSERT INTO `item_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(15902,15902,100,'',0,-15902,1);
