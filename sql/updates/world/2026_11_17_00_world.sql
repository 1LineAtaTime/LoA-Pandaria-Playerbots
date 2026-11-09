-- Silverpine Forest: Horde Hauler (44731) and Horde Coffin Hauler (44764) drive their waypoint path without the ettin that pulls them.
-- The Subdued Forest Ettins (44737 / 46560) stand next to the carts as separate wandering spawns. They are installed as vehicle accessory
-- on seat 1 (the front/yoke seat) by the full hauler accessory set in 2026_10_29_986_18_sp_wagons.sql (engineer, ettin, troopers, coffins),
-- so the accessory rows are NOT rewritten here (a rewrite would drop the engineer, trooper and coffin passengers).
-- Only the removal of the loose ettin spawns is kept; it is a no-op where they are already gone.
-- Source: Vehicle/VehicleSeat DBC (seat 0 is the driver position on the cart, seat 1 the front position); Warcraft Wiki, Horde Hauler:
-- "pulled by a Subdued Forest Ettin".
DELETE FROM `creature` WHERE `guid` IN (187562,187573,187582,187591,187600,187630,187637,187644,188358,188036,188058,188087,188104,188124,188179,188195,188200,188222) AND `id` IN (44737, 46560);
