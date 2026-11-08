-- The Heart of the Matter 28484: Cave Yeti's heart at the less grindy rate.
-- Sourced: item 64380 lists Cave Yeti 2248 as a dropper (Wowhead, 3148 of 60353 kills, counting kills without the quest); both TDBs have no row, and no source gives a holder's rate.
-- Ours: -100, the same as the Ferocious Yeti (TDB). The rate is uncertain, and when it is, the user wants the less grindy value.
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-100 WHERE `entry`=2248 AND `item`=64380;
