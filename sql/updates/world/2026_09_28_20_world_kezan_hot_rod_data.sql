-- Quest 14071 "Rolling with my Homies" (Kezan, map 648): Hot Rod 34840 data.
--
-- * Spellclick: without this row right-clicking the Hot Rod casts nothing, so a
--   player who leaves the car has no way back in. 46598 is
--   VEHICLE_SPELL_RIDE_HARDCODED. Source: MoP npc_spellclick_spells.
-- * unit_flags: our row carried UNIT_FLAG_PLAYER_CONTROLLED (0x01000000) on top
--   of UNIT_FLAG_CAN_SWIM (0x8000). The vehicle code sets that flag itself while
--   a player drives; MoP ships 32768 (CAN_SWIM only).
--
-- Rollback:
--   DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 34840 AND `spell_id` = 46598;
--   UPDATE `creature_template` SET `unit_flags` = 16793600 WHERE `entry` = 34840;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 34840;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(34840, 46598, 1, 0);

UPDATE `creature_template` SET `unit_flags` = 32768 WHERE `entry` = 34840;
