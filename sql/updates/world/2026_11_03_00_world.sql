-- Hillsbrad Foothills, the Sludge Fields: the Risen Hillsbrad Peasants (47861), Risen Hillsbrad Farmers (47859) and Infected
-- Sludge Guards (48017) were immune to NPCs (template unit_flags 0x8200: IMMUNE_TO_NPC besides CAN_SWIM), so the Sludge Guards
-- (47786) holding the lines around the field never fought them: neither side can start combat with, or attack, a unit immune
-- to NPCs. Retail (TDB 1210) and the Cataclysm TDB have no immunity on them.
UPDATE `creature_template` SET `unit_flags` = 0x8000 WHERE `entry` = 47861;
UPDATE `creature_template` SET `unit_flags` = 0x8000 WHERE `entry` = 47859;
UPDATE `creature_template` SET `unit_flags` = 0x8000 WHERE `entry` = 48017;
