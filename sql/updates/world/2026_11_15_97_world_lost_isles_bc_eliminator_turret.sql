-- Lost Isles, 24901 "Town-In-A-Box: Under Attack": the B.C. Eliminator turrets (38526) seated the player twice per
-- click and could be driven around.
--
-- Why 1: 38526 has two spellclick rows, 46598 (Ride Vehicle Hardcoded) and 72240 (B.C. Eliminator). Both are
-- CONTROL_VEHICLE auras, and Unit::HandleSpellClick casts every matching row. Vehicle 640 has one seat, so the second
-- aura cannot stack with the first (Aura::CanStackWith: no free seat). The first ride aura is removed (the player is
-- unseated and placed at the turret's position), then the second one seats him again, all within one click. A click
-- while seated does the same swap the other way round. The reference data has the same two rows. Keep only the
-- turret's own ride spell 72240.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 38526 AND `spell_id` = 46598;

-- Why 2: the seat (VehicleSeat 7545) has CAN_CONTROL, so the server gives the rider movement control of the turret
-- (SMSG_CONTROL_UPDATE allowMove 1). Vehicle 640 carries VEHICLE_FLAG_FIXED_POSITION, but this core never reads that
-- flag, and the turret has no root of its own, so the rider could drive it across the town. Root it the way the other
-- fixed cannons are rooted (Antipersonnel Cannon 27894, Krazz Cannon 49680): creature_template_movement.Rooted = 1.
-- Creature::UpdateEntry then applies UNIT_STATE_ROOT + MOVEMENTFLAG_ROOT, the root survives boarding and exiting
-- (Unit::SetControlled keeps a template root), and WorldSession::HandleMovementOpcode re-adds MOVEMENTFLAG_ROOT for a
-- rooted mover. The rider can still turn and aim the cannon.
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 38526;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES
(38526, NULL, NULL, NULL, 1, NULL, NULL, NULL);

-- Undo:
-- INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES (38526, 46598, 1, 0);
-- DELETE FROM `creature_template_movement` WHERE `CreatureId` = 38526;
