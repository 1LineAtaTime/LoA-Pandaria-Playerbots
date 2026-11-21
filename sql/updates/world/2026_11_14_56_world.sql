-- Light's Hope Chapel: retire the classic Argent Dawn quests that went with the Cataclysm. Their enders stay no longer.
--  * The 24 Craftsman's Writs (9178, 9179, 9181-9188, 9190, 9191, 9194-9198, 9200-9206): started only by the writ items
--    22600-22624, which come only from the Sealed Craftsman's Writ 22568, the reward of 9141 / 9142, which have neither
--    starter nor ender here: no player can obtain a writ. 18 of them still listed Packmaster Stonebruiser 16283 as ender.
--  * Seven ender-only quests with no starter: Chromatic Mantle of the Dawn 5517 and the five Armaments of Battle token turn-ins
--    9221, 9223, 9225, 9227, 9228 (Quartermaster Miranda Breechlock 11536), and The Ice Guard 9211 (Mataus the Wrathcaster
--    16133, who is not spawned on this map).
--  * Sourced: TDB 12.1.0 has all 31 in `disables` as "Deprecated quest"; TDB 4.4.2 lacks 17 of the writs entirely and disables
--    the other 14. Neither has a starter for any of them.
--  * Ours, the way this core retires quests (2026_11_07_17, 2026_11_08_21, 2026_11_09_22: quest relations removed, templates
--    and no `disables` rows): they had no starter left, so only the 25 ender rows go. Nobody can hold one of these quests, so
--    no character is left without an ender. The NPCs keep their npcflags (Stonebruiser keeps QUESTGIVER as in both TDBs).
DELETE FROM `creature_questender` WHERE `quest` IN (9178,9179,9181,9182,9183,9184,9185,9186,9187,9188,9190,9191,9194,9195,9196,9197,9198,9200,9201,9202,9203,9204,9205,9206,5517,9211,9221,9223,9225,9227,9228);
DELETE FROM `creature_queststarter` WHERE `quest` IN (9178,9179,9181,9182,9183,9184,9185,9186,9187,9188,9190,9191,9194,9195,9196,9197,9198,9200,9201,9202,9203,9204,9205,9206,5517,9211,9221,9223,9225,9227,9228);
DELETE FROM `gameobject_queststarter` WHERE `quest` IN (9178,9179,9181,9182,9183,9184,9185,9186,9187,9188,9190,9191,9194,9195,9196,9197,9198,9200,9201,9202,9203,9204,9205,9206,5517,9211,9221,9223,9225,9227,9228);
DELETE FROM `gameobject_questender` WHERE `quest` IN (9178,9179,9181,9182,9183,9184,9185,9186,9187,9188,9190,9191,9194,9195,9196,9197,9198,9200,9201,9202,9203,9204,9205,9206,5517,9211,9221,9223,9225,9227,9228);
