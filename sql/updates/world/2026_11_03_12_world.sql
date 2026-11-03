-- Hillsbrad Foothills, the Sludge Fields: Risen Hillsbrad Farmers 47859 and Peasants 47861 stood two on one spot 19 times
-- (16 pairs less than 0.2 yd apart, three 0.47-0.8 yd apart; the laborers wander 3 yd). 72 laborers stood in the fields;
-- the Cataclysm Classic TDB 442 sniff has 52 there, and even it stacks a few (0.0-0.6 yd pairs on five spots: one spawn
-- point seen as a farmer in one session and a peasant in another). One copy of each mop pair is removed, leaving 53.
-- The copy kept is the one whose entry matches the nearest sniffed laborer: on the sniffed spot (within 0.2 yd) for 10 pairs,
-- 2.7-8.3 yd away for the other 9. Where the sniff has both entries on that spot (176703/176704, 176706/176994) the choice
-- is arbitrary. No addon, formation, pool, event or SmartAI rows use these guids.
DELETE FROM `creature` WHERE `guid` IN (176699,176703,176706,176717,176973,176976,176977,176979,176988,176989,176995,176998,176999,177007,177008,177014,177024,177025,177027) AND `id` IN (47859,47861);
