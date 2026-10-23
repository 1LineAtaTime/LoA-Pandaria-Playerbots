-- #507 (epic #441): remove the Darkmoon Faire <-> Brewfest gameobject cross-links in game_event_gameobject.
--
-- 32 gameobject spawns were linked to BOTH event 75 "Darkmoon Faire" and event 26 "Brewfest". When one of the
-- two events is active and the other is started, GameEventMgr::GameEventSpawn loads the same spawn a second
-- time. On continents LoA uses the spawn id as the runtime GUID (GameObject::LoadGameObjectFromDB:
-- `if (map->GetInstanceId() != 0)` is the only place a fresh guid is generated), so the second copy hits the
-- ASSERT in Trinity::Insert<GameObject> and the worldserver goes down. This happened on `.event start 75`
-- at 2026-10-04 15:00:06. The core side is guarded in GameEventMgr.cpp (same issue); this file removes
-- the wrong data.
--
-- Which link is wrong (reference: AzerothCore acore_world, read 2026-10-04):
--  * 31 Darkmoon Faire banners/signs (179965 "Darkmoon Faire Banner", 179964 "DARKMOON FAIRE - Coming Soon!",
--    180026 "Darkmoon Faire Signpost"): AC links these entries only to its Darkmoon Faire events (3/4/5) and
--    "Darkmoon Faire Building" events (23/71), plus two 180026 signposts on its "Brewfest Building" events
--    70/91, and never to Brewfest itself (AC 24). The Brewfest (26) link is removed; the Darkmoon Faire (75)
--    link stays.
--  * 1 "Hanging, Tall/Thin, Medium - Brewfest" (195273, guid 184347): AC links 195273 only to Brewfest (24).
--    The Darkmoon Faire (75) link is removed; the Brewfest (26) link stays.
-- The MoP 5.4.8 data has the same cross-links (shared upstream data).
-- Other event-75 overlaps (Midsummer fireworks 1+75, Hallow's End skeletons 12+75, Deepwind Gorge treasure
-- chest 83+75 on Darkmoon Island) are left as they are: no clear source says they are wrong, and the core
-- guard makes them harmless.
--
-- Rollback:
--   INSERT IGNORE INTO `game_event_gameobject` (`eventEntry`, `guid`) VALUES (75, 184347);
--   INSERT IGNORE INTO `game_event_gameobject` (`eventEntry`, `guid`) SELECT 26, `guid` FROM `gameobject` WHERE `guid` IN
--     (25644,25648,25670,25671,180999,181008,181009,184222,184223,184362,184402,184404,184410,184419,184422,184443,
--      184445,184446,184453,184471,184504,184512,184514,184518,184525,184530,184535,184550,184555,208569,209974);

DELETE FROM `game_event_gameobject`
WHERE `eventEntry` = 26
  AND `guid` IN (25644,25648,25670,25671,180999,181008,181009,184222,184223,184362,184402,184404,184410,184419,184422,184443,
                 184445,184446,184453,184471,184504,184512,184514,184518,184525,184530,184535,184550,184555,208569,209974)
  AND `guid` IN (SELECT `guid` FROM (SELECT `guid` FROM `gameobject` WHERE `id` IN (179964, 179965, 180026)) AS dmf);

DELETE FROM `game_event_gameobject`
WHERE `eventEntry` = 75 AND `guid` = 184347
  AND `guid` IN (SELECT `guid` FROM (SELECT `guid` FROM `gameobject` WHERE `id` = 195273) AS brew);
