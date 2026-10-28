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

-- Heroes of the Horde! 28400 ("Steal the Alliance Battle Plans": item 64307 and an event, SpecialFlags 2)
-- 5.4.8 (Spell.dbc, wiki): Orkus 48503 (Summon Orkus 90205, from Purgation Isle Summon Controller II 90206) leads the player
-- up to the Stormpike camp, kills the Stormpike Battle Master 48515 and holds Vanndar 48508, Balinda 48507 and Ivar Bloodfang
-- 48485 (Summon Mountaineer 90258, Summon Bloodfang/Vanndar/Balinda 90209-90211) while the player loots the plans (quest drop
-- of 48515); Kasha 48504 (Summon Kasha 90278, Vehicle 1332) carries the player and the dying Orkus to Tarren Mill, where
-- Quest Complete 28400 (90379) credits the player before Cromush. mop had no actor spawns, nothing summoned them, event 27246
-- (the plans' Send Event 90283) is empty and nothing cast 90379: the quest could not be completed.
-- The event is scripted (C++). It starts from the gossip of the static Orkus 48470 on the beach (guid 3110070, see
-- 2026_10_28_42), whose place the player's own Orkus takes (instead of the controller aura: a summon from an aura restored at
-- login would not happen in this core's login order); talking to him again restarts an event cut short, and a player who
-- already holds the plans is picked up at once. Sourced: the spells, the texts (48744-48803, 48758 gossip, 48759 option).
-- Ours: the actors' spots, Orkus' hiding spot, the flight, the timings, the order of Orkus' dying words, and the use of the
-- unused menu 12440 / placeholder npc_text 17492 for his gossip (inferred: their ids sit between Ruberick's 48756 menu and
-- Harrison Jones' 48770 one, where 48757-48759 fall).
UPDATE `npc_text` SET `text0_0`='Are you ready? You will have to act quickly, $n. I do not know how long I can hold them all off for! Get the plans and get out!', `text0_1`='', `BroadcastTextID0`=48758, `lang0`=0, `Probability0`=1, `Emote0_0`=6 WHERE `ID`=17492;
DELETE FROM `gossip_menu` WHERE `MenuID`=12440 AND `TextID`=17492;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`) VALUES (12440,17492);
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12440 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12440,0,0,'I''m ready, Orkus. Let''s do this!',48759,1,3,0,0,0,0,'',0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup`=12440 AND `SourceEntry`=17492;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12440 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(14,12440,17492,0,0,47,0,28400,8,0,0,0,0,'','Kingslayer Orkus: "Are you ready?" while Heroes of the Horde! is in progress'),
(15,12440,0,0,0,47,0,28400,8,0,0,0,0,'','Kingslayer Orkus: start option while Heroes of the Horde! is in progress');

UPDATE `creature_template` SET `gossip_menu_id`=12440, `AIName`='', `ScriptName`='npc_hillsbrad_orkus_purgation_isle' WHERE `entry`=48470;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_orkus_heroes' WHERE `entry`=48503;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_kasha_heroes' WHERE `entry`=48504;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_purgation_actor' WHERE `entry` IN (48485,48507,48508,48515);

-- Summon Kasha 90278 targets a DB spot no sniff has: in the sky north-east of the camp, where she comes from (ours)
DELETE FROM `spell_target_position` WHERE `id`=90278 AND `effIndex`=0;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(90278,0,0,-1270.0,575.0,130.0,3.9);

DELETE FROM `creature_text` WHERE `CreatureID`=48503 AND `GroupID` BETWEEN 0 AND 15;
DELETE FROM `creature_text` WHERE `CreatureID`=48508 AND `GroupID`=0;
DELETE FROM `creature_text` WHERE `CreatureID`=48545 AND `GroupID` BETWEEN 20 AND 26;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48503,0,0,'STOP! GET DOWN, $n!',12,0,100,0,0,0,0,48744,5,'Kingslayer Orkus - Heroes of the Horde!, at the camp'),
(48503,1,0,'Look there, near the map table! The Stormpike Battle Master holds the Alliance battle plans!',12,0,100,0,0,0,0,48747,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,2,0,'CAREFUL! Vanndar, Balinda and Bloodfang are right there also.',12,0,100,0,0,0,0,48745,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,3,0,' I... I have a plan of my own.',12,0,100,0,0,0,0,48752,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,4,0,'I will kill the guard at the table and then I will draw the attention of the trio.',12,0,100,0,0,0,0,48753,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,5,0,'While I have them distracted you get the plans from the guard''s corpse. I will call Kasha to pick you up as soon as I see you get the plans.',12,0,100,0,0,0,0,48754,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,6,0,'Do not worry about Orkus, $n. I... I will be fine.',12,0,100,0,0,0,0,48755,5,'Kingslayer Orkus - Heroes of the Horde!'),
(48503,7,0,'CAN YOU SMELL WHAT THE LOK''TAR IS COOKIN''?!',14,0,100,0,0,0,0,48764,5,'Kingslayer Orkus - Heroes of the Horde!, attacks'),
(48503,8,0,'GET THE PLANS AND GO!',14,0,100,0,0,0,0,48765,5,'Kingslayer Orkus - Heroes of the Horde!, Battle Master dead'),
(48503,9,0,'KASHA, GET $P OUT OF HERE!',14,0,100,0,0,0,0,48769,5,'Kingslayer Orkus - Heroes of the Horde!, player has the plans'),
(48503,10,0,'FORGET ABOUT ME, KASHA! GO!',14,0,100,0,0,0,0,48771,5,'Kingslayer Orkus - Heroes of the Horde!, Kasha comes for him'),
(48503,11,0,'K... Kasha... you... came back for me...',12,0,100,0,0,0,0,48772,5,'Kingslayer Orkus - Heroes of the Horde!, flight'),
(48503,12,0,'I... I never knew anyone like you. You saved me, $p. More than once. I... I know it''s selfish, but I have something to ask of you.',12,0,100,0,0,0,0,48773,5,'Kingslayer Orkus - Heroes of the Horde!, flight'),
(48503,13,0,' Please take care of Kasha. Make sure she finds a cold place to live.',12,0,100,0,0,0,0,48779,5,'Kingslayer Orkus - Heroes of the Horde!, flight'),
(48503,14,0,'You are the best friend that Orkus has ever had, $p. *Orkus coughs*',12,0,100,0,0,0,0,48778,5,'Kingslayer Orkus - Heroes of the Horde!, flight'),
(48503,15,0,'Somewhere safe...',12,0,100,0,0,0,0,48774,5,'Kingslayer Orkus - Heroes of the Horde!, last words'),
(48508,0,0,'There''s no gettin'' away this time, orc! You''ve sung your last lok''tar!',14,0,100,0,0,0,0,48788,5,'Vanndar Stormpike - Heroes of the Horde!'),
(48545,20,0,'What happened? What have you got in your hands there?',12,0,100,0,0,0,0,48797,5,'High Warlord Cromush - Heroes of the Horde!, Kasha lands'),
(48545,21,0,'%s reads the Alliance battle plan.',16,0,100,0,0,0,0,48798,5,'High Warlord Cromush - Heroes of the Horde!'),
(48545,22,0,'He... He died to get this here?',12,0,100,0,0,0,0,48799,5,'High Warlord Cromush - Heroes of the Horde!'),
(48545,23,0,'%s lifts Orkus'' lifeless body up.',16,0,100,0,0,0,0,48800,5,'High Warlord Cromush - Heroes of the Horde!'),
(48545,24,0,'Do not mourn Orkus. He died with honor... with pride. He died a hero of the Horde.',12,0,100,0,0,0,0,48801,5,'High Warlord Cromush - Heroes of the Horde!'),
(48545,25,0,'From Lordaeron to Kalimdor, they will know his name and they will sing songs of glory, praising the fallen champion of the Horde.',12,0,100,0,0,0,0,48802,5,'High Warlord Cromush - Heroes of the Horde!'),
(48545,26,0,'Know that what you did will save hundreds, maybe thousands of lives. You are also a true hero of the Horde, $n.',12,0,100,0,0,0,0,48803,5,'High Warlord Cromush - Heroes of the Horde!');
