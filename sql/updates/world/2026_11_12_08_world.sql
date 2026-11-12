-- Western Plaguelands, I Ain't Sayin' You a Gourd-Digger... (26956): the sparkling plants come back after 120 s.
--
--  * Sourced: both TDBs (4.4.2 and 12.1.0) spawn the plants 205052 / 205053 / 205054 with spawntimesecs 120; mop had 300 s. Since
--    2026_11_12_03 the plants are the only source of the quest's monsters.
--  * Not changed: the number of plants. mop has 8 / 12 / 14 on Dalson's Farm, both TDBs 4 / 7 / 13 (24 in all, not 24 each), so
--    nothing is missing.

UPDATE `gameobject` SET `spawntimesecs` = 120 WHERE `id` IN (205052, 205053, 205054) AND `map` = 0;
