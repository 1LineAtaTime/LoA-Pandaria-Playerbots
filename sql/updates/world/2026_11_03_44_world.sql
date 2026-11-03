-- Hillsbrad Foothills, the Sludge Fields: keep Twinkles' Invisible Mount 176005 (48009) in the air, where it carries the dead
-- Twinkles 47870 upside down. It stands 0.94 yd above the terrain (sniffed z 55.5028, ground 54.56), and with no movement row
-- it was a ground creature without DISABLE_GRAVITY. In the 4.3.4 client decomp, a remote unit created without moving, root,
-- hover or gravity flags falls: CMovement_C::SetUpdateInfo (Movement.cpp:2166-2231) calls TryStartFallingLocal, and
-- CMovementShared::TryStartFalling starts the fall unless DISABLE_GRAVITY, DISABLE_COLLISION, ROOT, FLYING or SWIMMING is
-- set. That put the flipped horse's hooves on the dirt, so only their tips showed. This corrects 8f6551c8: the 0.85 yd it
-- was missing came from the client dropping the gravity-bound mount, so raising the spawn to 56.35 could not help.
-- Like the 40 other Invisible creatures here and the Forsaken Front val'kyr, it gets Flight 1 (DisableGravity) and Ground 0:
-- Creature::UpdateMovementFlags then sets DISABLE_GRAVITY and CAN_FLY, which the create packet sends. It has
-- MovementType 0 and can't be selected, so nothing moves it. Neither TDB has a movement row for 48009; this is ours.
-- Its z goes back to the sniffed 55.5028, which should put the hooves about 1 yd above the ground and the legs out to the
-- belly, as on Wowhead screenshot 191760. Absolute values, safe to re-run.
UPDATE `creature` SET `position_z`=55.5028 WHERE `guid`=176005 AND `id`=48009;

DELETE FROM `creature_template_movement` WHERE `CreatureId`=48009;
INSERT INTO `creature_template_movement` (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Chase`,`Random`,`InteractionPauseTimer`) VALUES
(48009,0,0,1,0,NULL,NULL,NULL); -- Invisible Mount (Twinkles)
