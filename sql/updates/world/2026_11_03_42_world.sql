-- Hillsbrad Foothills, the Sludge Fields: raise the Invisible Mount 176005 (48009) that carries the dead Twinkles 47870 upside
-- down, so its legs show above the dirt from the hooves to the belly, as on Wowhead screenshot 191760; only the bottoms of
-- its hooves showed.
-- Measured (5.4.8 client data):
-- - The mount shows display 11686 (InvisibleStalker.m2), forced on the spawn, as in both sniffs, whose template models give
--   16891 (ImpOutland.m2) probability 0. Seat 9018 has attachment index 13, which the client (Wow-64.exe s_seatAttachments)
--   maps to M2 attachment 39: on the stalker it sits 0.027 yd above the origin and has no animation, and the imp has none.
--   So Twinkles' origin is the mount's spawn point, and the pitch of pi flips the horse around its hooves.
-- - CelestialHorse.m2 (display 31958, scale 1.1): the hooves are at z 0, the belly underside at 0.90, the shoulder joint at
--   1.33, the hip joint at 1.64 and the back at about 2.0: scaled, the legs reach 0.99 yd from the hooves to the belly
--   (1.1 x 0.90) and the back is about 2.2 yd from them.
-- - The terrain under it is flat at 54.56 (.map, no vmap surface).
-- By that geometry the sniffed z 55.5028 should already put the hooves 0.97 yd above the ground, but in game only their
-- bottoms show, so the client draws the horse about 0.85 yd lower than that. The cause is not in the data: the mount's display,
-- position and flags match both sniffs. The mount goes 0.85 yd up, to 56.35, so that the legs show down to the belly line,
-- 1 yd above the ground.
-- 16891 also gets probability 0, as in both sniffs. That changes nothing in game while the spawn forces 11686.
-- Absolute values, safe to re-run.
UPDATE `creature` SET `position_z`=56.35 WHERE `guid`=176005 AND `id`=48009;
UPDATE `creature_template_model` SET `Probability`=0 WHERE `CreatureID`=48009 AND `Idx`=0 AND `CreatureDisplayID`=16891;
