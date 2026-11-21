-- Eastern Plaguelands, Villains of Darrowshire (27387) and Heroes of Darrowshire (27388): the four relics come back in 2 minutes.
--
-- Sourced: both TDBs respawn Davil's Libram 176206, Redpath's Shield 176207, Horgus' Skull 176208 and the Shattered Sword of Marduk
-- 176209 in 120 s; mop had 300 s. Each is one quest-only chest that the first looter empties (LootHandler: GO_JUST_DEACTIVATED until
-- respawn), so a second player waited five minutes per relic (comment 2012-01-17 [+2]: "with someone else at the house you cannot get
-- the libram").
UPDATE `gameobject` SET `spawntimesecs`=120 WHERE `guid` IN (157430,157432,157435,157437) AND `id` IN (176206,176207,176208,176209);
