-- Eastern Plaguelands: things that stand still no longer wander.
--
-- Reported in game: The Uncrashable (the goblins' airship at Light's Hope Chapel) and Gertrude Nozzlerocket walked about. Both
-- were hidden until this zone's pass revealed them, so nobody had seen that the airship had random movement (3 yd) and
-- Gertrude a six-point waypoint path. The same check against the TDB for the zone's other spawns that are not ordinary mobs:
--   * the four tower-light stalkers 41178 and the general purpose bunny 172531 wandered 3 yd;
--   * the three Runeforges 28481 of Acherus wandered;
--   * all 38 Plaguewood Reanimators 45897 wandered, which walks them out of the channel they hold on the Corpsebeast remains.
--
-- Sourced: the TDB has every one of these spawns idle (MovementType 0, wander 0; both dumps hold the same rows).
-- Ours: nothing. Gertrude's waypoint_data rows (path 173982) are left in place, unused.
-- Hostile mobs that wander where the TDB shows them idle are left alone: a sniff does not record wandering reliably.

UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id` IN (48708, 48705) AND `guid` IN (173981, 173982);
UPDATE `creature_addon` SET `path_id`=0 WHERE `guid`=173982;

UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=41178 AND `guid` IN (173036, 173051, 175303, 175314);
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=35114 AND `guid`=172531;
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=28481 AND `guid` IN (173832, 173833, 173834);

UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=45897 AND `guid` IN (
    172895, 172896, 172921, 172922, 172926, 174263, 174264, 174265, 174266, 174267, 174387, 174671, 174674, 174676, 174677,
    174841, 175360, 175361, 175362, 175363, 175364, 175365, 175384, 175385, 175386, 175388, 175391, 175392, 175397, 175398,
    175400, 175401, 175402, 175405, 175406, 175408, 175409, 175413);
