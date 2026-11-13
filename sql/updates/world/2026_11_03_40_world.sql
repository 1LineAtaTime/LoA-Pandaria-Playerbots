-- Hillsbrad Foothills, the Sludge Fields: Twinkles 47870, Johnny Awesome's dead Celestial Steed, lies upside down with its
-- body in the dirt and only its legs sticking out (Wowhead screenshot 191760, Cataclysm). The 4.4.2 and retail (12.1.0)
-- sniffs do it with an Invisible Mount 48009 on exactly Twinkles' spot: its VehicleId 1294 has one seat (9018, attachment 13,
-- no offset) with a passenger pitch of 3.14159, which turns the passenger over around its feet. The model has no Death or
-- Dead animation, so without the seat it stands upright. Neither TDB says how Twinkles boards; the accessory link is ours.
-- - Invisible Mount 176005 stands still, as in both sniffs (it was a 3 yd random mover).
-- - It carries Twinkles on seat 0 as a minion accessory (manual despawn): if Twinkles ever leaves the seat it is unsummoned
--   instead of standing upright on its own, and the mount reseats a new one when it is reset.
--   The ride spell 46598 is cast by Twinkles despite its Permanent Feign Death stun (USABLE_WHILE_STUNNED, no mechanic).
--   Only Twinkles may use that spellclick; the mount is not selectable anyway.
-- - The static Twinkles 176006, on the mount's spot, is replaced by the accessory.
-- Absolute values, safe to re-run.
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `guid`=176005 AND `id`=48009;

DELETE FROM `vehicle_template_accessory` WHERE `entry`=48009;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(48009, 47870, 0, 1, 'Invisible Mount - Twinkles (upside down)', 8, 0);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=48009;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(48009, 46598, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=48009;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 48009, 46598, 0, 0, 31, 0, 3, 47870, 0, 0, 0, 0, '', 'Invisible Mount: only Twinkles may ride it');

DELETE FROM `creature_addon` WHERE `guid`=176006;
DELETE FROM `creature` WHERE `guid`=176006 AND `id`=47870;
