-- Eastern Plaguelands, Gathering Some Grub(s) 27456 (Rimblat Earthshatter, Light's Hope Chapel).
--  * Carrion Devourers 8605 drop two Slabs of Carrion Worm Meat 13853 every time; they dropped one or two. Sourced: "Carrion
--    Grubs drop 1 Slab - Carrion Devourers drop 2 Slabs" (comment 2010-12-08) and the quest text ("larger worms mean more
--    meat"). Carrion Grubs 8603 keep their one.
--  * Rimblat Earthshatter 16134 had faction 1194 (Earthen Ring), so he showed each player's Earthen Ring standing. Sourced: both
--    TDBs give him 2167 (faction 1135), neutral to both sides; comments 2011-07-14 and 2010-11-25 (he stays neutral whatever
--    the player's Earthen Ring standing; the quest gives Argent Crusade reputation). His invisibility aura and his copies at
--    the caravan's stops are not changed here.
UPDATE `creature_loot_template` SET `mincountOrRef`=2, `maxcount`=2 WHERE `entry`=8605 AND `item`=13853;
UPDATE `creature_template` SET `faction`=2167 WHERE `entry`=16134;
