-- The Princess Trapped (26039): Mote of Myzrael 4435 drops at 80 % (quest-only) from the Drywhisker Kobold, Surveyor and Digger, not 50 %.
-- Sourced: TDB 442 and 1210 creature_loot_template, Chance 80, QuestRequired 1, on 2572, 2573 and 2574. Not certain of the retail rate; the TDB value is also the less grindy one.
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` = 4435 AND `entry` IN (2572,2573,2574);
