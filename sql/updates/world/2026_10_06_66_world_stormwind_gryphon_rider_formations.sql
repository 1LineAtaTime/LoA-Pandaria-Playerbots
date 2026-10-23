-- #490 Stormwind Gryphon Riders 51348: the two 3-rider formations never move - their 4 wingmen hang in the air at the
-- leaders' spawn points (-8699.5, 937.3, 137.9) and (-8985.3, 666.3, 170.0), so only 10 of the 14 riders patrol.
-- Cause: 2026_10_04_80 copied groupAI = 2 from alexkulya 2210340 (itself TrinityCore 7.x 2017_03_15_04, written before TC's
-- FLAG_IDLE_IN_FORMATION existed). LoA's CreatureGroup::LeaderStartedMoving (CreatureGroups.cpp:262) skips every member whose
-- groupAI lacks FLAG_IDLE_IN_FORMATION (0x200, CreatureGroups.h:30). These 6 rows are the only groupAI = 2 rows in the test
-- world (all other formations use 0x2xx; 514 = 0x202 on 122 rows). 514 keeps the upstream "leader assists member" bit (0x2)
-- and adds "follow the leader while pathing".
-- Re-runnable.
--
-- BACKUP (run first, keep the file):
--   mysqldump -h<host> -P<port> -u<user> -p<password> --no-create-info --skip-extended-insert world creature_formations --where="leaderGUID IN (188801,188804)" > creature_formations-51348-before-490.sql
-- ROLLBACK:
--   UPDATE `creature_formations` SET `groupAI` = 2 WHERE `leaderGUID` IN (188801, 188804);

UPDATE `creature_formations` SET `groupAI` = 514 WHERE `leaderGUID` IN (188801, 188804);
