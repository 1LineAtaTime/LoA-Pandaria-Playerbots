-- ported from PR 15 2026_10_28_61_world.sql (#986)
-- Wandering Isle creatures of levels 4-11: MoP health; melee damage of expansion-4 creatures of levels 4-19.
--
-- 1. 136 templates of levels 4-11 spawned only on the Wandering Isle had exp 0 and so took the old health column, half of MoP's.
--    Retail puts all of them on health column 4, and Wowhead MoP Classic matches column 4 for every one it lists (114 of 119, the
--    other five after the modifier fixes below): Vordraka 4,752, Zhao-Ren 1,980, Pei-Wu Tiger 352.
-- 2. Health_mod where Wowhead MoP Classic and retail agree on another value: Zhao-Ren 55874 (1,980 health) and 55786 (4,752),
--    Barbed Ray (204), Injured Sailor (396).
-- 3. damage_exp4 (per hit, not multiplied by the attack time) for levels 4-19 from the client game table NpcDamageByClassExp4
--    (MoP Classic 5.5.4, damage per second, x 2 for a 2.0 s swing), continuing 2026_10_28_59 (levels 1-3). It held about three
--    times the table at levels 4-10 and 0 at most of 11-19, where creatures hit with attack power only; from level 20 it is within
--    about 10% of the table. The table is the curve Wowpedia's MoP notes confirm for normal mobs (level 3). With it the creatures
--    of change 1 keep about the melee they had on the old column. Elites are not covered: Vordraka (level 10 elite) hits for 39-48
--    on Wowpedia, about twice the table.
UPDATE `creature_classlevelstats` SET `damage_exp4` = 4.9539 WHERE `level` = 4 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 4.5824 WHERE `level` = 4 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 6.678 WHERE `level` = 5 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 6.1772 WHERE `level` = 5 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 8.5091 WHERE `level` = 6 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 7.8709 WHERE `level` = 6 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 9.1636 WHERE `level` = 7 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 8.4764 WHERE `level` = 7 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 9.8182 WHERE `level` = 8 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 9.0818 WHERE `level` = 8 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 10.4727 WHERE `level` = 9 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 9.6873 WHERE `level` = 9 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 11.1273 WHERE `level` = 10 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 10.2927 WHERE `level` = 10 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 15.7091 WHERE `level` = 11 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 14.5309 WHERE `level` = 11 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 19.6364 WHERE `level` = 12 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 18.1636 WHERE `level` = 12 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 24.2182 WHERE `level` = 13 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 22.4018 WHERE `level` = 13 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 28.1455 WHERE `level` = 14 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 26.0345 WHERE `level` = 14 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 32.7273 WHERE `level` = 15 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 30.2727 WHERE `level` = 15 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 36.6545 WHERE `level` = 16 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 33.9055 WHERE `level` = 16 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 41.2364 WHERE `level` = 17 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 38.1436 WHERE `level` = 17 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 45.1636 WHERE `level` = 18 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 41.7764 WHERE `level` = 18 AND `class` IN (8);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 54.5818 WHERE `level` = 19 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 50.4882 WHERE `level` = 19 AND `class` IN (8);

UPDATE `creature_template` SET `exp` = 4 WHERE `entry` IN (53705, 54567, 54568, 54975, 54976, 55021, 55477, 55478, 55479,
    55504, 55506, 55583, 55585, 55595, 55601, 55632, 55633, 55634, 55650, 55665, 55685, 55694, 55786, 55834, 55874, 55918,
    55940, 55942, 55943, 55944, 55946, 55999, 56009, 56134, 56172, 56174, 56195, 56241, 56274, 56360, 56362, 56393, 56394,
    56416, 56417, 56418, 56419, 56662, 56869, 57132, 57258, 57317, 57414, 57419, 57420, 57466, 57617, 57618, 57619, 57620,
    57621, 57622, 57623, 57624, 57709, 57710, 57712, 57720, 57721, 57739, 57741, 57748, 57754, 57778, 57803, 57853, 59497,
    59499, 59626, 59642, 59649, 59960, 59962, 59963, 60042, 60055, 60176, 60245, 60248, 60253, 60488, 60554, 60722, 60741,
    60770, 60780, 60848, 60851, 60852, 60853, 60854, 60858, 60873, 60874, 60877, 60878, 60888, 60889, 60890, 60891, 60892,
    60893, 60894, 60895, 60896, 60897, 60898, 60914, 61126, 61127, 64505, 64506, 64543, 64593, 64875, 64876, 64939, 65472,
    65473, 65558, 65559, 65742, 66689, 66690, 67185, 67186);

UPDATE `creature_template` SET `Health_mod` = 5 WHERE `entry` = 55874;  -- Zhao-Ren
UPDATE `creature_template` SET `Health_mod` = 12 WHERE `entry` = 55786; -- Zhao-Ren
UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 54976;  -- Barbed Ray
UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 55999;  -- Injured Sailor
