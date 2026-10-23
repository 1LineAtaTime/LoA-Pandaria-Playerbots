-- #508 (epic #445, from the #438 hand-off): Goblin arrival at Bladefist Bay (Durotar, map 1).
-- Kor'kron Loyalist 39609 (spawn 204536; ender of 25266 "Warchief's Emissary", starter of 25267 "Message for Garrosh")
-- gets the missing "Use my wolf to reach Garrosh" gossip option and the SmartAI that makes the player summon and ride
-- the Battleworg 39611 to Garrosh in Grommash Hold.
--
-- Intended design (Wowhead MoP-classic tooltip API, fetched 2026-10-04):
--   nether.wowhead.com/mop-classic/tooltip/quest/25266 "Deliver the Sealed Package to the Kor'kron Loyalist on the docks
--   at Bladefist Bay in Durotar."; .../quest/25267 "Deliver the SI:7 Emblem to Garrosh Hellscream at Grommash Hold in
--   Orgrimmar."
-- LoA before this file: gossip_menu 11245 (npc_text 15669) exists but has NO option, 39609 has no AI.
-- Source (MoP 5.4.8 world data, dump 2026-09-17):
--   gossip_menu_option (menu_id 11245, id 0, option_id 1, npc_option_npcflag 1, 'Use my wolf to reach Garrosh'),
--   no conditions on menu 11245; creature_template 39609 AIName 'SmartAI';
--   smart_scripts (39609, 0, 3): GOSSIP_SELECT 11245/0 -> INVOKER_CAST 74031 on ACTION_INVOKER;
--   creature_template 39611 (Battleworg, VehicleId 802) unit_flags 0.
-- DBC: 74031 "Message For Garrosh: Summon Battleworg" = SUMMON 39611 with
--   SummonProperties 827 (category 4 = vehicle) and basepoints 74030 "Riding Battleworg" (CONTROL_VEHICLE), so the
--   invoker is seated on summon. Vehicle 802 seat 7836 has VEHICLE_SEAT_FLAG_CAN_CONTROL (0x800): the player drives
--   the worg to Orgrimmar himself, which is why neither DB has waypoints for 39611.
-- Battleworg 39611: LoA unit_flags 16809984 carries 0x01000000 (this core's UNIT_FLAG_PLAYER_CONTROLLED); the vehicle
--   code sets it itself while a player drives (same defect as the Hot Rod in #207,
--   2026_09_28_20_world_kezan_hot_rod_data.sql). MoP 5.4.8 ships 0.
-- Test-DB schema: gossip_menu_option as reshaped by 2024_08_26_00_world.sql (MenuID/OptionID, action and box columns
--   inline); smart_scripts has event_param5 / target_param4. SMART_EVENT_GOSSIP_SELECT = 62, SMART_EVENT_LINK = 61,
--   SMART_ACTION_INVOKER_CAST = 85, SMART_ACTION_CLOSE_GOSSIP = 72, SMART_TARGET_ACTION_INVOKER = 7
--   (src/server/game/AI/SmartScripts/SmartScriptMgr.h). The linked close-gossip row is a LoA addition (MoP 5.4.8 leaves
--   the window open while the player is seated).
-- Re-runnable: every INSERT is preceded by a DELETE of the same keys; the UPDATEs are idempotent.
--
-- Rollback:
--   DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11245;
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 39609 AND `source_type` = 0;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 39609;
--   UPDATE `creature_template` SET `unit_flags` = 16809984 WHERE `entry` = 39611;

-- Kor'kron Loyalist: the missing gossip option.
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11245;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11245, 0, 0, 'Use my wolf to reach Garrosh', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);

-- Kor'kron Loyalist: selecting the option makes the player summon and mount the Battleworg.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 39609 AND `ScriptName` = '';

DELETE FROM `smart_scripts` WHERE `entryorguid` = 39609 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39609, 0, 0, 1, 62, 0, 100, 0, 11245, 0, 0, 0, 0, 85, 74031, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Loyalist - On Gossip Option 0 Selected - Invoker Cast Message For Garrosh: Summon Battleworg'),
(39609, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Loyalist - On Link - Close Gossip');

-- Battleworg: drop the template PLAYER_CONTROLLED flag (MoP 5.4.8 value).
UPDATE `creature_template` SET `unit_flags` = 0 WHERE `entry` = 39611;
