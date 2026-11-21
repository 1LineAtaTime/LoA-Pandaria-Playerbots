-- Eastern Plaguelands: the Argent Sentries walk.
--
-- An Argent Sentry on patrol at Light's Hope Chapel moved through the air with a swimming animation (reported in game for
-- guid 174025). The entry's creature_template_movement row says Ground 1, Swim 0, Flight 2 (CanFly): a foot soldier that may
-- fly, so its waypoint movement was sent as flying. The row came with the base conversion of the old InhabitType column
-- (2024_07_24_00_world.sql). It is the only humanoid spawned in the zone that is set to fly.
--
-- Sourced: neither TDB has a creature_template_movement row for 16378, which is the default: on the ground, able to swim, no
--   flight (CreatureMovementData's defaults in this core). The row is removed.
-- Ours: nothing.

DELETE FROM `creature_template_movement` WHERE `CreatureId`=16378;
