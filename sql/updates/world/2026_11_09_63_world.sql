-- Sharpbeak's ride between Doran Steelwing's camp and the top of Jintha'Alor (npc_sharpbeak in zone_hinterlands.cpp)
-- Sourced: TDB 442 and 1210: Doran Steelwing 43157's option on menu 11716 ("I need to get to the top of Jintha'Alor. Time to ride
-- Sharpbeak!", broadcast 44810) and his SmartAI (cast 83486 on the player, which makes the player cast 80657 and ride Sharpbeak
-- 43161); Sharpbeak 44776's spellclick 94120 (the ride back on 51125); 43161 and 51125's vehicle 1434, script name, faction,
-- flags, run speed and template addon. Wowhead: Sharpbeak rides you up "whenever you need (just talk to Doran Steelwing)".
-- Ours: closing the gossip window when the option is picked.
UPDATE `creature_template` SET `VehicleId`=1434, `ScriptName`='npc_sharpbeak', `faction`=694, `unit_flags`=33554440, `speed_run`=3.23333 WHERE `entry` IN (43161,51125);
DELETE FROM `creature_template_addon` WHERE `entry` IN (43161,51125);
INSERT INTO `creature_template_addon` (`entry`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(43161,0,0,0,0,3,0,1,0,0,0,0,0,0,NULL),
(51125,0,0,0,0,3,0,1,0,0,0,0,0,0,NULL);

-- The ride back from the summit (mop had the camp-to-summit force cast here)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=44776;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(44776,94120,3,0);

-- Doran's option
UPDATE `creature_template` SET `gossip_menu_id`=11716, `AIName`='SmartAI', `ScriptName`='' WHERE `entry`=43157;
DELETE FROM `gossip_menu_option` WHERE `MenuID`=11716 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(11716,0,0,'I need to get to the top of Jintha''Alor. Time to ride Sharpbeak!',44810,1,1,0,0,0,0,NULL,0,25996);

DELETE FROM `smart_scripts` WHERE `entryorguid`=43157 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(43157,0,0,1,62,0,100,0,11716,0,0,0,0,11,83486,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Doran Steelwing - On Gossip Option 0 Selected - Cast ''Sharpbeak Taxi - Camp to Jintha''Alor - Force Cast'''),
(43157,0,1,0,61,0,100,0,0,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Doran Steelwing - Linked - Close Gossip');
