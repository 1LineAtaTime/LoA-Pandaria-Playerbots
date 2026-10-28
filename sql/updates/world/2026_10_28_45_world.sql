-- Hillsbrad Foothills: Kasha and Kingslayer Orkus, Southshore -> Purgation Isle -> Tarren Mill (zone_hillsbrad_purgation.cpp)

-- The Road to Purgation 28375 ("Accompany Kingslayer Orkus to Purgation Isle", SpecialFlags 2, no objectives).
-- 5.4.8: Summon Kasha 90123 summons Kasha 48467 (Vehicle 1329, SummonProperties 3038: vehicle, personal) and seats the
-- player with Ride to Purgation 90122 (also phase 348 "Road to Purgation" and a quest invisibility on the rider); Cancel
-- Cinematic 90128 completes the quest. Nothing started it, so the quest could not be completed. Orkus' gossip (menu 12430)
-- now calls Kasha; she flies the player and Orkus to Purgation Isle, where the player is credited and set down by the
-- landed Kasha 48471 and Orkus 48470. Sourced: the spells, broadcast texts 48671 (yell), 48675 (boss whisper), 48687/48688
-- (gossip texts), 48689 (option). Ours: the flight, the spot Kasha is summoned on, the npc_text ids (17475/17476 inferred
-- from the id order of the neighbouring npc_texts and broadcast texts; 17476 was a "Missing npc_text" placeholder).
UPDATE `npc_text` SET `text0_0`='I will not leave Kasha''s side.', `text0_1`='', `BroadcastTextID0`=48688, `lang0`=0, `Probability0`=1, `Emote0_0`=274 WHERE `ID`=17476;
DELETE FROM `npc_text` WHERE `ID`=17475;
INSERT INTO `npc_text` (`ID`,`text0_0`,`text0_1`,`BroadcastTextID0`,`lang0`,`Probability0`,`Emote0_0`) VALUES
(17475,'Will you accompany Orkus to Purgation Isle?','',48687,0,1,6);
DELETE FROM `gossip_menu` WHERE `MenuID`=12430 AND `TextID`=17475;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`) VALUES (12430,17475);
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12430 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12430,0,0,'I am ready for Purgation Isle.',48689,1,1,0,0,0,0,'',0,0);
-- The option and its text only while the quest is in progress (talking to him again restarts a flight cut short)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup`=12430 AND `SourceEntry` IN (17475,17476);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12430 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(14,12430,17475,0,0,47,0,28375,8,0,0,0,0,'','Kingslayer Orkus: "Will you accompany Orkus" while The Road to Purgation is in progress'),
(14,12430,17476,0,0,47,0,28375,8,0,1,0,0,'','Kingslayer Orkus: "I will not leave Kasha''s side" otherwise'),
(15,12430,0,0,0,47,0,28375,8,0,0,0,0,'','Kingslayer Orkus: ride option while The Road to Purgation is in progress');

-- Orkus 48218 keeps his SmartAI (Kasha Will Fly Again); the option has the player cast Summon Kasha
DELETE FROM `smart_scripts` WHERE `entryorguid`=48218 AND `source_type`=0 AND `id` IN (2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48218,0,2,3,62,0,100,0,12430,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus - On Gossip Option 0 Selected - Close Gossip'),
(48218,0,3,0,61,0,100,0,0,0,0,0,0,85,90123,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus - Linked - Invoker Cast ''Summon Kasha'' (The Road to Purgation)');

-- Summon Kasha 90123 targets a DB spot no sniff has: on the beach beside Orkus (ours)
DELETE FROM `spell_target_position` WHERE `id`=90123 AND `effIndex`=0;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(90123,0,0,-923.0,-583.5,3.2,2.0);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90122,90123) AND `ScriptName` IN ('spell_hillsbrad_ride_to_purgation','spell_hillsbrad_summon_kasha_road_to_purgation');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(90122,'spell_hillsbrad_ride_to_purgation'),
(90123,'spell_hillsbrad_summon_kasha_road_to_purgation');

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_kasha_road_to_purgation' WHERE `entry`=48467;

-- The Orkus who rides Kasha is a Purgation Isle Orkus 48470 (the one who receives the quest there)
DELETE FROM `creature_text` WHERE `CreatureID`=48470 AND `GroupID` IN (0,1);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48470,0,0,'TO PURGATION ISLE, KASHA! RIDE LIKE THUNDER!',14,0,100,0,0,0,0,48671,5,'Kingslayer Orkus - The Road to Purgation, take-off'),
(48470,1,0,'|cFFFFFFFF$n, Orkus and Kasha flew off to Purgation Isle...|r',42,0,100,0,0,0,0,48675,0,'Kingslayer Orkus - The Road to Purgation, to the rider');
