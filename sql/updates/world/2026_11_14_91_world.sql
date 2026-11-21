-- Eastern Plaguelands: a Hidden Stash holds a little money.
--
-- The Hidden Stash 61387 (the drop Fiona's Lucky Charm adds to the zone's mobs) had one loot row, Mr. Grubbs 66076 at 30 %, and
-- no money: seven stashes in ten opened empty. Reported in game.
--
-- Sourced: Wowhead's item page gives the stash an average money content of 243-250 copper ("avgmoney" in its item data, MoP
--   Classic page fetched 2026-11-14) and lists Mr. Grubbs as its only item.
-- Ours: the range 2 s to 3 s around that average.
-- Container money is read from item_template_addon (ObjectMgr: MinMoneyLoot / MaxMoneyLoot); the stash had no row.

DELETE FROM `item_template_addon` WHERE `Id`=61387;
INSERT INTO `item_template_addon` (`Id`,`FlagsCu`,`FoodType`,`MinMoneyLoot`,`MaxMoneyLoot`,`SpellPPMChance`) VALUES
(61387, 0, 0, 200, 300, 0);
