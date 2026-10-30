-- ported from PR 15 2026_10_27_68_world.sql (#986)
-- Ruins of Gilneas: spawns standing twice on the same spot (same entry, within a yard; for those walking a path, the same path).
-- Where one of the pair has a waypoint path and the other none, the one with the path stays. The removed spawns' paths go too.
DELETE FROM `creature_addon` WHERE `guid` IN (307480, 307575, 307550, 307612, 307522, 307587, 307549, 307592, 307602, 307553, 307502, 307509, 307539);
DELETE FROM `creature` WHERE `guid` IN (307480, 307575, 307550, 307612, 307522, 307587, 307549, 307592, 307602, 307553, 307502, 307509, 307539)
    AND `id` IN (45280, 45292, 45375, 45376, 45481, 45483, 45499, 45535, 45536, 45537, 45538);
DELETE FROM `waypoint_data` WHERE `id` IN (307575, 307550, 307612, 307522, 307502, 307509);
