-- Eastern Plaguelands, Marauders of Darrowshire (27389): Scourge Champions drop a Fetid Skull 80 % of the time.
--
-- Sourced: both TDBs give the Fetid Skull 13157 a quest chance of 80 % on Scourge Champion 8529; mop had 50 %. Comments: "usually but
-- not 100 %" (2011-03-07 [+22]); 2013-09-10 counts 34 skulls from 49 kills. With half the skulls turning into Resonating Skulls, five
-- needed about 20 kills at 50 % and 12.5 at 80 %. The user's rule: the correct rate if certain, otherwise the less grindy one.
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-80 WHERE `entry`=8529 AND `item`=13157;
