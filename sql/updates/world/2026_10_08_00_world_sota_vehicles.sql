-- #736 Strand of the Ancients vehicles.
-- Source: AC WotLK creature_template (27894 VehicleId 160) and creature_template_spell (28781, index 1 = 60206 Ram);
-- Wowpedia "Battleground Demolisher" (rev 2021-08-03): "equipped with a battering ram and a catapult".
-- Back up first:  mysqldump mop_world creature_template --where="entry IN (27894,28781)"

-- Battleground Demolisher (28781) has only Hurl Boulder (spell1). Add Ram (60206, in the 5.4.8 Spell.dbc: knockback +
-- 1500 + 1500 siege damage in front, 4 s cooldown). The MoP 5.4.8 data has the same gap as LoA, so the value comes from AC.
UPDATE `creature_template` SET `spell2` = 60206 WHERE `entry` = 28781 AND `spell2` = 0;

-- SotA Antipersonnel Cannon (27894): passengers were shown standing next to the cannon. VehicleId 453 is the IoC Horde
-- Gunship Cannon vehicle (seat 4906 = attachment point 2, standing gunner, no lower ride anim). AC uses vehicle 160
-- (seat 2029 = attachment 13, VEHICLE_SEAT_FLAG_HAS_LOWER_ANIM_FOR_RIDE, ride anim 484 ReclinedMount = seated). Both
-- are FIXED_POSITION with a CAN_CONTROL|CAN_CAST seat, so the bot gunner logic is unchanged.
UPDATE `creature_template` SET `VehicleId` = 160 WHERE `entry` = 27894 AND `VehicleId` = 453;

-- Undo:
-- UPDATE `creature_template` SET `spell2` = 0 WHERE `entry` = 28781 AND `spell2` = 60206;
-- UPDATE `creature_template` SET `VehicleId` = 453 WHERE `entry` = 27894 AND `VehicleId` = 160;
