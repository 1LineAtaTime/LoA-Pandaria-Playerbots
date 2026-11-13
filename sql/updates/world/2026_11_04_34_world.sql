-- Kasha 48217 (guid 312689) over the Southshore shore: the healed Kasha circling Kingslayer Orkus once Kasha Will Fly
-- Again 28354 is rewarded (her Apply Quest Invis Zone 2 aura 89276 is detected through spell_area 89277, 2026_10_28_42).
-- She flies an 8-point path at z 17.05 over water (terrain -0.3, sea level 0) with AnimTier 3, but 48217 has no movement
-- row: as a ground creature, Creature::UpdateMovementFlags switches the fly tier off, the client drops a gravity-bound unit
-- in the air, and WaypointMovementGenerator builds each leg on the navmesh, so she swam the path at sea level instead.
-- Flight 1 (DisableGravity), Ground 0, Swim 0 (as 48009), for this spawn only (creature_movement_override): the other Kasha 48217 (312260) is
-- the collapsed one lying at the shore (StandState 7) and must keep gravity. Neither TDB spawns 48217 or has a movement
-- row for it; this is ours, as for Twinkles' mount (2026_11_03_44).
DELETE FROM `creature_movement_override` WHERE `SpawnId`=312689;
INSERT INTO `creature_movement_override` (`SpawnId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Chase`,`Random`,`InteractionPauseTimer`) VALUES
(312689,0,0,1,0,NULL,NULL,NULL);
