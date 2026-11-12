-- Western Plaguelands, A New Era for the Plaguelands (26999): Plague Lurkers 1824 carry Wandering Plague.
--
--  * Sourced: creature_template_addon of both TDBs (4.4.2 and 12.1.0): 1824 auras '8247'. 8247 Wandering Plague is a proc aura
--    (trigger 3436: a 30 s disease, -5 % Stamina and +5 % physical damage taken, that spreads itself on through 3439 within 10 yd).
--  * mop had the template addon with no auras, and the one 1824 spawn with its own creature_addon (guid 212928, which keeps its
--    waypoint path) overrides the template, so it gets the aura too.

UPDATE `creature_template_addon` SET `auras` = '8247' WHERE `entry` = 1824;
UPDATE `creature_addon` SET `auras` = '8247' WHERE `guid` = 212928;
