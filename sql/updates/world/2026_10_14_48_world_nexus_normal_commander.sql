-- #553 The Nexus (576): the normal-mode commander (27949, the Alliance-side Commander that the instance script turns into the
-- Horde Commander 27947 for Alliance groups) had spawnMask 4 (heroic only), like the heroic-only 26796, so normal Nexus had
-- no commander. AC acore_world: 27949 spawnMask 1 (WotLK normal), 26796 spawnMask 2 (heroic). On 5.4.8 the normal 5-man
-- difficulty is DIFFICULTY_NORMAL = 1 -> spawnMask 2; heroic stays 4. The MoP 5.4.8 data has the same gap.
UPDATE `creature` SET `spawnMask` = 2 WHERE `guid` = 95896 AND `id` = 27949;
