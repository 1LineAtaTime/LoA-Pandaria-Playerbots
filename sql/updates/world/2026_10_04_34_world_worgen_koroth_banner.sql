-- Refs #439, #476. Quest 24472 "Introductions Are in Order" (Gilneas, map 654, Koroth's Den, phase 131072).
--
-- Intended (Wowpedia "Introductions Are in Order", MediaWiki API, read 2026-10-04): on looting Koroth's Banner,
-- Koroth the Hillbreaker yells "Who dares to touch Koroth's banner!?" / "You puny thief! Koroth find you!
-- Koroth smash your face in!" and comes after the thief.
-- Fork today: zone_gilneas.cpp registers npc_koroth_the_hillbreaker (moves Koroth along KorothPath, which starts
-- 17 yd from spawn guid 219512 of entry 36294, and says lines 0/1) and go_koroth_banner (on JUST_DEACTIVATED,
-- i.e. after the banner chest is looted, calls Koroth). Neither name is bound to any row, so the scene never
-- plays. creature_text 36294 groups 0/1 hold exactly the two Wowpedia yells (BroadcastText 36914/36913).
-- The quest itself (kill 4 Ogre Minions + loot item 49742 from GO 201594) already works without this.
-- Existing classes only: no core build needed.
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 36294;
--   UPDATE `gameobject_template` SET `ScriptName` = '' WHERE `entry` = 201594;
-- Re-runnable: plain UPDATEs.

UPDATE `creature_template` SET `ScriptName` = 'npc_koroth_the_hillbreaker' WHERE `entry` = 36294;
UPDATE `gameobject_template` SET `ScriptName` = 'go_koroth_banner' WHERE `entry` = 201594;
