-- Syndicate Thief 24477 (39 spawns in Stromgarde after the doubles are gone, 22 in TDB 442) had no loot or pickpocket table: coin only.
-- Sourced: TDB 442 and 1210 creature_template_difficulty give it LootID 2588 and PickPocketLootID 2588, the Syndicate Prowler's tables.
-- mop's 2588 holds Sigil of Strom 4440 at -100, so thieves now count for Sigil of Strom 26030; Wowhead's dropped-by tab of 4440 lists them too.
UPDATE `creature_template` SET `lootid`=2588,`pickpocketloot`=2588 WHERE `entry`=24477;
DELETE FROM `creature_loot_template` WHERE `entry`=24477 AND `item`=4440;
