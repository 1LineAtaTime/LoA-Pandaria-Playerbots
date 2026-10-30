-- #1030 creature_template.Health_mod computed against the classic base-health column while `exp` selects a later one.
--
-- The core takes creature health from creature_classlevelstats.basehp{exp} (CreatureBaseStats::BaseHealth[exp] in
-- Creature::SelectLevel) and multiplies it by Health_mod. These templates had Health_mod = health / basehp0 while their
-- exp is 1-4, so their health came out 1.5x to 16x too high (the #1026 Gallywix pattern).
--
-- Each row sets the Health_mod that gives, at the template's own level and exp column:
--   * AC parity:          the health AzerothCore gives the same creature at the same level (AC keeps these at exp 0);
--   * sniffed health N:   the creature's sniffed spawn health, for Cata/MoP creatures (consistent across spawns,
--                         not a placeholder, not a vehicle, hand-set modifier).
-- Left alone: level ranges, creatures spawned in instances or battlegrounds (difficulty-dependent health), vehicles,
-- script-set health, and rows where the spawn health may itself be a computed value rather than a sniff.
-- New values are exact: ceil(basehp{exp} x Health_mod) equals the target. Takes effect at the next restart.

-- AC parity (125)
UPDATE `creature_template` SET `Health_mod` = 12.1907 WHERE `entry` = 13840; -- 20 -> 12.1907, AC parity
UPDATE `creature_template` SET `Health_mod` = 4.8763 WHERE `entry` = 13841; -- 8 -> 4.8763, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4292 WHERE `entry` = 29475; -- 1 -> 0.4292, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6826 WHERE `entry` = 30001; -- 1.6 -> 0.6826, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.3122 WHERE `entry` = 30039; -- 3 -> 1.3122, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.43459 WHERE `entry` = 30058; -- 1 -> 0.43459, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 30066; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 30094; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 30326; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 42.3968 WHERE `entry` = 30378; -- 100 -> 42.3968, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 30481; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 4.4466 WHERE `entry` = 30488; -- 10.5522 -> 4.4466, AC parity
UPDATE `creature_template` SET `Health_mod` = 5.8972 WHERE `entry` = 30489; -- 13.8233 -> 5.8972, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 30721; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 30722; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 30734; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 30735; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 800.03155 WHERE `entry` = 30838; -- 2401.3 -> 800.03155, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 31031; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3826 WHERE `entry` = 31032; -- 1 -> 0.3826, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 31081; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6087 WHERE `entry` = 31646; -- 1.35 -> 0.6087, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4451 WHERE `entry` = 31706; -- 1 -> 0.4451, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.5958 WHERE `entry` = 31804; -- 1 -> 0.5958, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.5958 WHERE `entry` = 31805; -- 1 -> 0.5958, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.5958 WHERE `entry` = 31806; -- 1 -> 0.5958, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.5958 WHERE `entry` = 31916; -- 1 -> 0.5958, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4509 WHERE `entry` = 32206; -- 1 -> 0.4509, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 32286; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4509 WHERE `entry` = 32321; -- 1 -> 0.4509, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4509 WHERE `entry` = 32322; -- 1 -> 0.4509, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3156 WHERE `entry` = 32325; -- 0.8 -> 0.3156, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3945 WHERE `entry` = 32328; -- 1 -> 0.3945, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3945 WHERE `entry` = 32339; -- 1 -> 0.3945, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3607 WHERE `entry` = 32340; -- 1 -> 0.3607, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3607 WHERE `entry` = 32342; -- 1 -> 0.3607, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3156 WHERE `entry` = 32343; -- 0.8 -> 0.3156, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 32415; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 32416; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4324 WHERE `entry` = 32435; -- 1.02 -> 0.4324, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4324 WHERE `entry` = 32441; -- 1.02 -> 0.4324, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.8276 WHERE `entry` = 32504; -- 1.35 -> 0.8276, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 32509; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.2719 WHERE `entry` = 32523; -- 3 -> 1.2719, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.2719 WHERE `entry` = 32524; -- 3 -> 1.2719, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 32529; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 779.6589 WHERE `entry` = 32547; -- 1871.96 -> 779.6589, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3826 WHERE `entry` = 32631; -- 1 -> 0.3826, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4292 WHERE `entry` = 32743; -- 1 -> 0.4292, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 32749; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.5958 WHERE `entry` = 32763; -- 1 -> 0.5958, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4451 WHERE `entry` = 32804; -- 1 -> 0.4451, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4451 WHERE `entry` = 32812; -- 1 -> 0.4451, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.7937 WHERE `entry` = 33229; -- 1.87196 -> 0.7937, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.7937 WHERE `entry` = 33272; -- 1.87196 -> 0.7937, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.9259 WHERE `entry` = 33447; -- 2.18401 -> 0.9259, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 33513; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.9259 WHERE `entry` = 33518; -- 2.18401 -> 0.9259, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.9259 WHERE `entry` = 33522; -- 2.18401 -> 0.9259, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3826 WHERE `entry` = 33595; -- 1 -> 0.3826, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33608; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33609; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33610; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33611; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33612; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33613; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33614; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33615; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33616; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33617; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33618; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33619; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33621; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 33623; -- 3 -> 1.8389, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33630; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33631; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33633; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33634; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33635; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33636; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33637; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33638; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33639; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33640; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33641; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3945 WHERE `entry` = 33642; -- 1 -> 0.3945, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33674; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33675; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33676; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33677; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33678; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33679; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33680; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33681; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33682; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33683; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.606 WHERE `entry` = 33684; -- 1 -> 0.606, AC parity
UPDATE `creature_template` SET `Health_mod` = 1.3122 WHERE `entry` = 33854; -- 3 -> 1.3122, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 33871; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.9259 WHERE `entry` = 34210; -- 2.18401 -> 0.9259, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 34244; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 34252; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34435; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34476; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34477; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34478; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34479; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.6129 WHERE `entry` = 34480; -- 1 -> 0.6129, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.613 WHERE `entry` = 34481; -- 1 -> 0.613, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 34653; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 34654; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 34952; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 34953; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3499 WHERE `entry` = 35290; -- 1 -> 0.3499, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 35291; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 35344; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.3826 WHERE `entry` = 35496; -- 1 -> 0.3826, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4374 WHERE `entry` = 35498; -- 1 -> 0.4374, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 35594; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 35607; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 37734; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 38294; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 39130; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 39131; -- 1 -> 0.4239, AC parity
UPDATE `creature_template` SET `Health_mod` = 0.4239 WHERE `entry` = 39132; -- 1 -> 0.4239, AC parity

-- Cata/MoP creatures, sniffed health (39)
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 33037; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 33039; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 0.63401 WHERE `entry` = 35222; -- 10 -> 0.63401, sniffed health 49130
UPDATE `creature_template` SET `Health_mod` = 5.40973 WHERE `entry` = 35867; -- 50 -> 5.40973, sniffed health 290400
UPDATE `creature_template` SET `Health_mod` = 0.63401 WHERE `entry` = 36403; -- 10 -> 0.63401, sniffed health 49130
UPDATE `creature_template` SET `Health_mod` = 21.3553 WHERE `entry` = 36436; -- 275 -> 21.3553, sniffed health 1834250
UPDATE `creature_template` SET `Health_mod` = 21.3553 WHERE `entry` = 36676; -- 275 -> 21.3553, sniffed health 1834250
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 37142; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 53.4249 WHERE `entry` = 38225; -- 617.284 -> 53.4249, sniffed health 2867902
UPDATE `creature_template` SET `Health_mod` = 6.34017 WHERE `entry` = 38306; -- 100 -> 6.34017, sniffed health 491300
UPDATE `creature_template` SET `Health_mod` = 79.24893 WHERE `entry` = 38504; -- 1000 -> 79.24893, sniffed health 6141000
UPDATE `creature_template` SET `Health_mod` = 0.63401 WHERE `entry` = 39615; -- 10 -> 0.63401, sniffed health 49130
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 40344; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 40345; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 41124; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 1.1917 WHERE `entry` = 41138; -- 2 -> 1.1917, sniffed health 7048
UPDATE `creature_template` SET `Health_mod` = 1.1917 WHERE `entry` = 41158; -- 2 -> 1.1917, sniffed health 7048
UPDATE `creature_template` SET `Health_mod` = 0.57169 WHERE `entry` = 42673; -- 5.284 -> 0.57169, sniffed health 30689
UPDATE `creature_template` SET `Health_mod` = 1.58497 WHERE `entry` = 43099; -- 20 -> 1.58497, sniffed health 122820
UPDATE `creature_template` SET `Health_mod` = 1.8389 WHERE `entry` = 44084; -- 3 -> 1.8389, sniffed health 9156
UPDATE `creature_template` SET `Health_mod` = 55.1677 WHERE `entry` = 44106; -- 90 -> 55.1677, sniffed health 274680
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 44309; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 44310; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 44311; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 44312; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 17.8762 WHERE `entry` = 44313; -- 30 -> 17.8762, sniffed health 105720
UPDATE `creature_template` SET `Health_mod` = 0.86548 WHERE `entry` = 44875; -- 10 -> 0.86548, sniffed health 46460
UPDATE `creature_template` SET `Health_mod` = 6.1297 WHERE `entry` = 44984; -- 10 -> 6.1297, sniffed health 30520
UPDATE `creature_template` SET `Health_mod` = 3.0648 WHERE `entry` = 44985; -- 5 -> 3.0648, sniffed health 15260
UPDATE `creature_template` SET `Health_mod` = 1.2259 WHERE `entry` = 45262; -- 2 -> 1.2259, sniffed health 6104
UPDATE `creature_template` SET `Health_mod` = 6.5613 WHERE `entry` = 45534; -- 15 -> 6.5613, sniffed health 69780
UPDATE `creature_template` SET `Health_mod` = 1.7876 WHERE `entry` = 47368; -- 3 -> 1.7876, sniffed health 10572
UPDATE `creature_template` SET `Health_mod` = 18.1819 WHERE `entry` = 48292; -- 30 -> 18.1819, sniffed health 97110
UPDATE `creature_template` SET `Health_mod` = 1.58497 WHERE `entry` = 48568; -- 20 -> 1.58497, sniffed health 122820
UPDATE `creature_template` SET `Health_mod` = 1.58497 WHERE `entry` = 48569; -- 20 -> 1.58497, sniffed health 122820
UPDATE `creature_template` SET `Health_mod` = 9.0898 WHERE `entry` = 50609; -- 15 -> 9.0898, sniffed health 38850
UPDATE `creature_template` SET `Health_mod` = 4.9037 WHERE `entry` = 51375; -- 8 -> 4.9037, sniffed health 24416
UPDATE `creature_template` SET `Health_mod` = 0.86548 WHERE `entry` = 51434; -- 10 -> 0.86548, sniffed health 46460
UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 67071; -- 22.609 -> 1, sniffed health 135552

-- Undo:
--   UPDATE `creature_template` SET `Health_mod` = 20 WHERE `entry` = 13840;
--   UPDATE `creature_template` SET `Health_mod` = 8 WHERE `entry` = 13841;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 29475;
--   UPDATE `creature_template` SET `Health_mod` = 1.6 WHERE `entry` = 30001;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 30039;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30058;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30066;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30094;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30326;
--   UPDATE `creature_template` SET `Health_mod` = 100 WHERE `entry` = 30378;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30481;
--   UPDATE `creature_template` SET `Health_mod` = 10.5522 WHERE `entry` = 30488;
--   UPDATE `creature_template` SET `Health_mod` = 13.8233 WHERE `entry` = 30489;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30721;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30722;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30734;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 30735;
--   UPDATE `creature_template` SET `Health_mod` = 2401.3 WHERE `entry` = 30838;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31031;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31032;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31081;
--   UPDATE `creature_template` SET `Health_mod` = 1.35 WHERE `entry` = 31646;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31706;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31804;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31805;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31806;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 31916;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32206;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32286;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32321;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32322;
--   UPDATE `creature_template` SET `Health_mod` = 0.8 WHERE `entry` = 32325;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32328;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32339;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32340;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32342;
--   UPDATE `creature_template` SET `Health_mod` = 0.8 WHERE `entry` = 32343;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32415;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32416;
--   UPDATE `creature_template` SET `Health_mod` = 1.02 WHERE `entry` = 32435;
--   UPDATE `creature_template` SET `Health_mod` = 1.02 WHERE `entry` = 32441;
--   UPDATE `creature_template` SET `Health_mod` = 1.35 WHERE `entry` = 32504;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32509;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 32523;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 32524;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32529;
--   UPDATE `creature_template` SET `Health_mod` = 1871.96 WHERE `entry` = 32547;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32631;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32743;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32749;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32763;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32804;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 32812;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33037;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33039;
--   UPDATE `creature_template` SET `Health_mod` = 1.87196 WHERE `entry` = 33229;
--   UPDATE `creature_template` SET `Health_mod` = 1.87196 WHERE `entry` = 33272;
--   UPDATE `creature_template` SET `Health_mod` = 2.18401 WHERE `entry` = 33447;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33513;
--   UPDATE `creature_template` SET `Health_mod` = 2.18401 WHERE `entry` = 33518;
--   UPDATE `creature_template` SET `Health_mod` = 2.18401 WHERE `entry` = 33522;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33595;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33608;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33609;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33610;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33611;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33612;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33613;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33614;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33615;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33616;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33617;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33618;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33619;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33621;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33623;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33630;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33631;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33633;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33634;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33635;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33636;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33637;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33638;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33639;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33640;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33641;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33642;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33674;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33675;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33676;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33677;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33678;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33679;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33680;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33681;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33682;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33683;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33684;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 33854;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 33871;
--   UPDATE `creature_template` SET `Health_mod` = 2.18401 WHERE `entry` = 34210;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34244;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34252;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34435;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34476;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34477;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34478;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34479;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34480;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34481;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34653;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34654;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34952;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 34953;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 35222;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35290;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35291;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35344;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35496;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35498;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35594;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 35607;
--   UPDATE `creature_template` SET `Health_mod` = 50 WHERE `entry` = 35867;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 36403;
--   UPDATE `creature_template` SET `Health_mod` = 275 WHERE `entry` = 36436;
--   UPDATE `creature_template` SET `Health_mod` = 275 WHERE `entry` = 36676;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 37142;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 37734;
--   UPDATE `creature_template` SET `Health_mod` = 617.284 WHERE `entry` = 38225;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 38294;
--   UPDATE `creature_template` SET `Health_mod` = 100 WHERE `entry` = 38306;
--   UPDATE `creature_template` SET `Health_mod` = 1000 WHERE `entry` = 38504;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 39130;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 39131;
--   UPDATE `creature_template` SET `Health_mod` = 1 WHERE `entry` = 39132;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 39615;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 40344;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 40345;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 41124;
--   UPDATE `creature_template` SET `Health_mod` = 2 WHERE `entry` = 41138;
--   UPDATE `creature_template` SET `Health_mod` = 2 WHERE `entry` = 41158;
--   UPDATE `creature_template` SET `Health_mod` = 5.284 WHERE `entry` = 42673;
--   UPDATE `creature_template` SET `Health_mod` = 20 WHERE `entry` = 43099;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 44084;
--   UPDATE `creature_template` SET `Health_mod` = 90 WHERE `entry` = 44106;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 44309;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 44310;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 44311;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 44312;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 44313;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 44875;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 44984;
--   UPDATE `creature_template` SET `Health_mod` = 5 WHERE `entry` = 44985;
--   UPDATE `creature_template` SET `Health_mod` = 2 WHERE `entry` = 45262;
--   UPDATE `creature_template` SET `Health_mod` = 15 WHERE `entry` = 45534;
--   UPDATE `creature_template` SET `Health_mod` = 3 WHERE `entry` = 47368;
--   UPDATE `creature_template` SET `Health_mod` = 30 WHERE `entry` = 48292;
--   UPDATE `creature_template` SET `Health_mod` = 20 WHERE `entry` = 48568;
--   UPDATE `creature_template` SET `Health_mod` = 20 WHERE `entry` = 48569;
--   UPDATE `creature_template` SET `Health_mod` = 15 WHERE `entry` = 50609;
--   UPDATE `creature_template` SET `Health_mod` = 8 WHERE `entry` = 51375;
--   UPDATE `creature_template` SET `Health_mod` = 10 WHERE `entry` = 51434;
--   UPDATE `creature_template` SET `Health_mod` = 22.609 WHERE `entry` = 67071;
