-- Eastern Plaguelands: the two mounted Argent Sentries fly again.
--
-- 2026_11_14_94 removed the Argent Sentry's creature_template_movement row (Ground 1, Swim 0, Flight 2) so that the sentries
-- on foot stop moving through the air. Two of the 35 spawns, guids 174027 and 174079, are mounted (mount display 22471) and
-- patrol a path in the air above Light's Hope Chapel (16 to 26 points, 94 to 117 yd up; the ground is at about 90). Without
-- flight they fell and were set back onto their path (reported in game for 174027).
--
-- Sourced: the spawns, mounts and paths (as they were).
-- Ours: flight for these two spawns only, in creature_movement_override, with the values the template row had; Swim 1 as the
--   default has it.

DELETE FROM `creature_movement_override` WHERE `SpawnId` IN (174027, 174079);
INSERT INTO `creature_movement_override` (`SpawnId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Chase`,`Random`,`InteractionPauseTimer`) VALUES
(174027, 1, 1, 2, 0, NULL, NULL, NULL),
(174079, 1, 1, 2, 0, NULL, NULL, NULL);
