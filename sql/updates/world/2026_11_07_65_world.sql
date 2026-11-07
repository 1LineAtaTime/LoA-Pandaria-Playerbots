-- Arathi Highlands: twelve spawns that doubled another of the same entry (under 3 yd apart, within each other's 3 yd wander).
-- Sourced: each pair falls on one single TDB 442 spawn. Ours: which of the two goes; where one has a path, it stays.
-- Kept spawns are on the 5.4.8 ground. Other surplus spawns are spread out and were left alone.
DELETE FROM `creature` WHERE `guid` IN (161128,160983,161064,161069,161047,161044,161065,161067,161246,161228,161087,161200);
DELETE FROM `creature_addon` WHERE `guid`=160983;
