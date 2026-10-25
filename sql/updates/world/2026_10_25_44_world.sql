-- Lieutenant Sanders hangs from his noose (vehicle 659) instead of standing beside it

-- The noose hangs still, as sniffed (the world dump gave it random movement)
UPDATE `creature` SET `wander_distance` = 0, `MovementType` = 0 WHERE `guid` = 192577;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 38936;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(38936, 46598, 1, 0);

-- Sanders is a minion accessory: the accessories are installed again on the noose's first update, and a non-minion one is not
-- unsummoned when it is put off, so it was left standing beside the tree as a second Sanders
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 38936;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(38936, 13158, 0, 1, 'Lieutenant Sanders'' Noose - Lieutenant Sanders', 8, 0);

-- The chain: the Noose Focus above the noose chains Sanders by the foot, to a second, also invisible Noose Focus at his foot (the
-- old standalone Sanders spawn's guid). The sniffed Sanders Foot Noose (73444) is a chain of fixed length that falls short of
-- him, so this uses a Base -> Base chain beam instead (Ongolongo - Chain Visual). Both foci hang still, as sniffed.
DELETE FROM `creature` WHERE `guid` = 192578 AND `id` IN (13158, 39093);
DELETE FROM `creature_addon` WHERE `guid` = 192578;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `position_x`, `position_y`, `position_z`,
`orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`) VALUES
(192578, 39093, 0, 85, 4918, 1, 1, 11686, 2533.23, -920.179, 61.0012, 4.53786, 300, 0, 0, 42, 0, 0);
UPDATE `creature` SET `wander_distance` = 0, `MovementType` = 0 WHERE `guid` = 192579;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 39093;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (39093, -192579) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `target_type`, `target_param1`, `target_param2`, `comment`) VALUES
(-192579, 0, 0, 0, 1, 0, 100, 0, 1000, 1000, 5000, 5000, 11, 79807, 34, 10, 192578, 39093, 'Lieutenant Sanders'' Noose Focus - Out of Combat - Cast ''Ongolongo - Chain Visual'' on the Noose Focus at his foot (Aura Not Present)');

-- A hanging prisoner holds no weapon
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 13158;
