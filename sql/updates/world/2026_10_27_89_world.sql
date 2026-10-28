-- Scarlet Monastery gold was the level 90 heroic amount. Scale it to each creature's level: gold * (average gold of open-world
-- normal mobs at that level) / (the same average at level 90: 3865 copper); at levels 30/31/32/33/34 those averages are 117/130/132/139/151 copper.
UPDATE `creature_template` SET `mingold` = 2431, `maxgold` = 2431 WHERE `entry` = 3977; -- High Inquisitor Whitemane (was 62200)
UPDATE `creature_template` SET `mingold` = 173, `maxgold` = 173 WHERE `entry` = 58555; -- Scarlet Fanatic (was 5052)
UPDATE `creature_template` SET `mingold` = 173, `maxgold` = 173 WHERE `entry` = 58569; -- Scarlet Purifier (was 5058)
UPDATE `creature_template` SET `mingold` = 163, `maxgold` = 163 WHERE `entry` = 58590; -- Scarlet Zealot (was 4772)
UPDATE `creature_template` SET `mingold` = 149, `maxgold` = 149 WHERE `entry` = 58605; -- Scarlet Judicator (was 4338)
UPDATE `creature_template` SET `mingold` = 93, `maxgold` = 93 WHERE `entry` = 58783; -- Scarlet Initiate (was 3079)
UPDATE `creature_template` SET `mingold` = 11884, `maxgold` = 11884 WHERE `entry` = 59223; -- Brother Korloff (was 304103)
UPDATE `creature_template` SET `mingold` = 123, `maxgold` = 123 WHERE `entry` = 59705; -- Scarlet Flamethrower (was 3671)
UPDATE `creature_template` SET `mingold` = 123, `maxgold` = 123 WHERE `entry` = 59746; -- Scarlet Centurion (was 3670)
UPDATE `creature_template` SET `mingold` = 10395, `maxgold` = 10395 WHERE `entry` = 59789; -- Thalnos the Soulrender (was 289664)
UPDATE `creature_template` SET `mingold` = 4484, `maxgold` = 4484 WHERE `entry` = 60040; -- Commander Durand (was 114737)
