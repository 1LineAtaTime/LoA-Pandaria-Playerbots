-- Timeless Isle: Spirit Healer at graveyard 4635 (Firewalker's Path Upper).
-- Before: the other registered Timeless Isle graveyards (4558, 4559, 4622, 4623, 4629 to 4634) have a Spirit Healer 72676 within
-- 0 to 19 yards, but 4635 has none within 160 yards, so a ghost released there cannot resurrect at a spirit healer.
-- This adds one spawn of the same template at the graveyard point, with the same values as the other healers.
-- Source: WorldSafeLocs.dbc, build 18414 (id 4635: map 870, -448.03 -5550.97 91.51, facing 110.4 degrees); game_graveyard_zone 4635
-- already points to zone 6757; the Wowhead MoP Classic map data for npc 72676 has pins at exactly this point.
-- Rollback: DELETE FROM `creature` WHERE `guid` = 5310650;

DELETE FROM `creature` WHERE `guid` = 5310650;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `phaseId`, `phaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`) VALUES
(5310650, 72676, 870, 6757, 0, 1, 1, 0, 0, -448.03, -5550.97, 91.51, 1.927, 120, 0, 0);
