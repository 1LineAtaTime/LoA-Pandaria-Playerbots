-- Andorhal: remove the pre-Cataclysm quest objects and markers
-- Sourced: none of these is in either TDB's Andorhal box (TDB 4.4.2 and 1210), and every quest they served has no starter
-- left: Andorhal Silo Temporal Rift 175795 (spell focus 763 for Displacing Temporal Rift 16613, A Matter of Time 4971),
-- Andorhal Tower One-Four 176094-176097 and their marker creatures 10902-10905 (spell focus 805 for Placing Beacon Torch
-- 17016, All Along the Watchtowers 5097/5098), Musty Tome 176150/176151 and their pools (The Annals of Darrowshire 5154;
-- 176151 only drops the grey Ruined Tome), Blood of Heroes 176213 (5166/5167/7483) and the Darrowshire Town Square spell
-- focus 177528 (focus 944 for Create Relic Bundle 18987, the classic The Battle of Darrowshire 5721; the Cataclysm 27390
-- does not use it). The Blood of Heroes outside Andorhal are left to the zone's spawn fixes.
DELETE FROM `gameobject` WHERE `areaId`=193 AND `id` IN (175795,176094,176095,176096,176097,176213,177528);
DELETE FROM `pool_gameobject` WHERE `pool_entry` BETWEEN 14333 AND 14343;
DELETE FROM `pool_template` WHERE `entry` BETWEEN 14333 AND 14343;
DELETE FROM `gameobject` WHERE `areaId`=193 AND `id` IN (176150,176151);
DELETE FROM `creature` WHERE `guid` BETWEEN 399996 AND 399999 AND `id` IN (10902,10903,10904,10905);
