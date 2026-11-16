-- Lost Isles, Town-In-A-Box: five Wild Cluckers (38111) walked through the air over the town.
--
-- Why: in "Cluster Cluck" (24671) the player attaches Remote Control Fireworks to Wild Cluckers, and a clucker with
-- fireworks takes "a journey all over the town before landing by the poultryizer" (Wowpedia, Cluster Cluck). The
-- five spawns below show that flight: they carry "Cluster Cluck: Wild Clucker Loot FX" (66727) in creature_addon and
-- loop waypoint paths whose nodes climb 25-39 yd above the ground (up to z 44.7 over the town at z 5-6) and end on
-- the ground by the poultryizer. Nothing lets them fly: no creature_template_movement row, no override, every node is
-- move_flag 0. So Creature::UpdateMovementFlags never sets MOVEMENTFLAG_CAN_FLY, the waypoint splines go out as ground
-- moves, and the client draws a chicken walking up and down an invisible slope in mid-air. The other 25 cluckers
-- stand on the ground (MovementType 0) and are not touched.
--
-- Fix: let these five spawns fly (creature_movement_override.Flight = 2, CreatureFlightMovementType::CanFly).
-- Creature::UpdateMovementFlags then gives them MOVEMENTFLAG_CAN_FLY and their waypoint splines go out as flying moves.
DELETE FROM `creature_movement_override` WHERE `SpawnId` IN (233141, 233159, 233183, 233184, 233215);
INSERT INTO `creature_movement_override` (`SpawnId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES
(233141, NULL, NULL, 2, NULL, NULL, NULL, NULL),
(233159, NULL, NULL, 2, NULL, NULL, NULL, NULL),
(233183, NULL, NULL, 2, NULL, NULL, NULL, NULL),
(233184, NULL, NULL, 2, NULL, NULL, NULL, NULL),
(233215, NULL, NULL, 2, NULL, NULL, NULL, NULL);

-- Undo:
-- DELETE FROM `creature_movement_override` WHERE `SpawnId` IN (233141, 233159, 233183, 233184, 233215);
