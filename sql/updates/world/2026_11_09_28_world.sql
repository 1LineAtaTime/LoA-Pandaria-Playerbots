-- The Hinterlands: quest item drop rates.
-- Sourced: TDB 4.4.2 and 1210 (identical): Snapjaw Gizzard 58867 from Saltwater Snapjaw 2505 75 % (Snapjaws, Lad! 26485 needs 8);
-- the Cache of Shadra 204120 (its TDB loot id is 30296; mop's is 204120) gives the Fang of Shadra 58281 and the Eye of Shadra 58282 at
-- 100 % each, where mop rolled them independently at 48 % and 52 %, so about half the openings gave a player on The Fang of Shadra
-- (26406/26529) nothing.
-- Uncertain, the less grindy value: Shadra-Spawn Venom Sample 58239 from Spawn of Shadra 42879 90 % as both TDBs (mop 27 %; the Horde Venomous Secrets 26368 needs 6; the Alliance
-- 26525 is a kill objective instead).
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-75 WHERE `entry`=2505 AND `item`=58867;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-90 WHERE `entry`=42879 AND `item`=58239;
UPDATE `gameobject_loot_template` SET `ChanceOrQuestChance`=-100 WHERE `entry`=204120 AND `item` IN (58281,58282);
