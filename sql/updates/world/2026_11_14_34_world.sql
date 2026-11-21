-- Eastern Plaguelands, Dark Garb (27452): four pairs of cultists at the Corin's Crossing death camp stood inside each other
-- (within 0.5 yd), so each pull brought two.
--
-- Sourced: both TDBs have ONE spawn at each of these spots; the member that matches their entry stays:
--   (2031, -4424)  Vile Tutor 8548 173555 stays, Dark Summoner 8551 174779 goes
--   (1966, -4512)  Dark Summoner 8551 173130 stays, its double 174173 goes
--   (1982, -4532)  Vile Tutor 8548 175002 stays, Dark Summoner 8551 173358 goes
--   (1970, -4522)  two Dark Summoners 8551 where both TDBs have a Vile Tutor 8548: 174237 goes, 173170 stays an 8551 (ours:
--                  the entry is left; 14 + 26 cultists still give both items).
-- No pool, formation, game event or guid-scripted SmartAI rows refer to the four; their creature_addon rows (if any) go too.

DELETE FROM `creature` WHERE `guid` IN (174779, 174173, 174237, 173358) AND `id` IN (8548, 8551);
DELETE FROM `creature_addon` WHERE `guid` IN (174779, 174173, 174237, 173358);
