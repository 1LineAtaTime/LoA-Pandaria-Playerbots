-- Wandering Isle creatures of levels 1-3: MoP health and melee damage.
--
-- 1. damage_exp4 (melee damage per hit of expansion-4 creatures; it is not multiplied by the attack time) for levels 1-3 held
--    8/11/14, three to eight times the client game table NpcDamageByClassExp4 (Mists of Pandaria Classic 5.5.4, damage per
--    second, here x 2 for a 2.0 s swing). Wowpedia's MoP-era notes agree with the table: Jaomin Ro (level 3) hits for 3-4 on
--    cloth and Living Air (level 3) for ~7, where these values gave 16-23. Levels 4 and up are left as they are: no MoP note
--    for a normal mob there yet, and the level-10 elite Vordraka hits for 39-48, more than the table gives.
-- 2. Seventeen level 1-2 templates spawned only on the Wandering Isle had exp 0 (the "required expansion" the 5.4.8 client
--    is sent, 0 for all of these on retail too) and so took the old health column, half of MoP's: Training Target 57873 had
--    42 health where its twin 53714 and Wowhead MoP Classic have 84. Retail puts them on health column 4 (or a later re-tune),
--    and Wowhead MoP Classic matches column 4 for every one it lists. At levels 1-3 both damage columns now give the same
--    melee, so only their health changes.
UPDATE `creature_classlevelstats` SET `damage_exp4` = 1.0227 WHERE `level` = 1 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 0.946  WHERE `level` = 1 AND `class` = 8;
UPDATE `creature_classlevelstats` SET `damage_exp4` = 2.1777 WHERE `level` = 2 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 2.0144 WHERE `level` = 2 AND `class` = 8;
UPDATE `creature_classlevelstats` SET `damage_exp4` = 3.402  WHERE `level` = 3 AND `class` IN (1, 2, 4);
UPDATE `creature_classlevelstats` SET `damage_exp4` = 3.1469 WHERE `level` = 3 AND `class` = 8;

-- Mottled Rabbit, Master Li Fei, Fire/Earth Central Temple Turn-in Controller, Balloon Arrival Controller, Training Target,
-- Mossback Toad (2), Corsac Fox (2), Merchant Lorvo, Trainee Nim, Trainee Guang, Deng, Cai, Kong, Jie
UPDATE `creature_template` SET `exp` = 4 WHERE `entry` IN (54133, 54135, 56550, 56566, 56679, 57873, 60915, 53704, 54134, 54943,
    57797, 60183, 60244, 60249, 60250, 60259, 60260);
