-- Quest 14071 "Rolling with my Homies" (Kezan, map 648)
--
-- The Hot Rod (creature 34840) summoned by item 46856 ("Keys to the Hot Rod",
-- on-use spell 66393) spawned and immediately despawned again without ever
-- seating the player. Our creature_template row was missing:
--   * ScriptName  - Project SkyFire binds npc_original_hot_rod, whose
--                   PassengerBoarded hook seats the player's three homies,
--   * spell2      - 61178 (SPELL_GRAB_PASSENGER in BattlefieldWG.h), the
--                   passenger-grabbing vehicle spell present in every 5.4.8
--                   reference data (Project SkyFire base SQL).
-- Megs Dreadshredder (34874) was also missing her SmartAI binding and its 12
-- quest say/emote/aura rows.
--
-- Source: MoP 5.4.8 world data (creature_template 34840/34874/34957-34959
-- and smart_scripts 34874), mapped to this fork's smart_scripts schema. The
-- companion C++ is src/server/scripts/Maelstrom/zone_kezan.cpp.
--
-- NOTE on VehicleId: 34840 stays at 448. Our row and the
-- Project SkyFire base map648 SQL both use 448. A later Project SkyFire
-- Joslyn-Maddie update flips it to 468, but no Vehicle.db2/seat-layout evidence
-- is available for our 5.4.8 client data to justify deviating from the two
-- working reference databases, so it is intentionally left unchanged.

-- 34840 Hot Rod: restore the passenger-grabbing spell and the creature script.
UPDATE `creature_template` SET `spell2` = 61178, `ScriptName` = 'npc_original_hot_rod' WHERE `entry` = 34840;

-- 34957/34958/34959 Ace/Gobber/Izzy: board the player's Hot Rod when summoned.
UPDATE `creature_template` SET `ScriptName` = 'npc_friend_gob' WHERE `entry` IN (34957, 34958, 34959);

-- 34874 Megs Dreadshredder: quest say lines, emotes and tracking auras.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 34874;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 34874 AND `source_type` = 0;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
VALUES
(34874, 0,   0, 0, 20, 0, 100, 0, 28349, 0, 0, 0, 0,  1,     0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 28349 Rewarded - Say Line 0'),
(34874, 0,   1, 0, 20, 0, 100, 0, 14071, 0, 0, 0, 0,  1,     1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Rewarded - Say Line 1'),
(34874, 0,   2, 0, 64, 0, 100, 0,     0, 0, 0, 0, 0,  5,     1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Gossip Hello - Play Emote 1'),
(34874, 0,   3, 0, 20, 0, 100, 0, 14121, 0, 0, 0, 0,  5,     4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14121 Rewarded - Play Emote 4'),
(34874, 0,   4, 0, 20, 0, 100, 0, 28607, 0, 0, 0, 0,  5,     4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 28607 Rewarded - Play Emote 4'),
(34874, 0, 201, 0, 19, 0, 100, 0, 14071, 0, 0, 0, 0,  1,     2, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Accepted - Say Line 2'),
(34874, 0, 202, 0, 19, 0, 100, 0, 14071, 0, 0, 0, 0, 75, 73593, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Accepted - Add Aura 73593'),
(34874, 0, 203, 0, 19, 0, 100, 0, 14071, 0, 0, 0, 0, 75, 73595, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Accepted - Add Aura 73595'),
(34874, 0, 204, 0, 19, 0, 100, 0, 14071, 0, 0, 0, 0, 75, 73597, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Accepted - Add Aura 73597'),
(34874, 0, 205, 0, 20, 0, 100, 0, 14071, 0, 0, 0, 0, 28, 73593, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Rewarded - Remove Aura 73593'),
(34874, 0, 206, 0, 20, 0, 100, 0, 14071, 0, 0, 0, 0, 28, 73595, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Rewarded - Remove Aura 73595'),
(34874, 0, 207, 0, 20, 0, 100, 0, 14071, 0, 0, 0, 0, 28, 73597, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Megs Dreadshredder - On Quest 14071 Rewarded - Remove Aura 73597');
