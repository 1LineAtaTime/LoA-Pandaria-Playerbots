-- Project SkyFire SF27 interaction/starter/profession fixes at df19329b209fa1d1ae80444b43f6201d5dc124f0.
-- Preserve unmentioned local rows/fields and active schema aliases.

-- 2026_08_17_world_01.sql
UPDATE `creature_template`
SET `faction`=32, `faction_A`=32, `faction_H`=32, `npcflag`=0
WHERE `entry`=49871;
UPDATE `creature_template`
SET `npcflag`=0
WHERE `entry`=42940;

-- 2026_08_24_world_02.sql
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=43923;
DELETE FROM `smart_scripts` WHERE `entryorguid`=43923 AND source_type=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(43923,0,0,0,0,0,100,0,4000,6000,9000,11000,0,11,81193,0,0,0,0,0,2,0,0,0,0,0,0,0,'Forlorn Spirit - In Combat - Cast Unrelenting Anguish'),
(43923,0,1,0,8,0,100,1,82198,0,0,0,0,80,4392300,0,0,0,0,0,1,0,0,0,0,0,0,0,'Forlorn Spirit - On Spellhit - Run Script')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);
DELETE FROM `smart_scripts` WHERE `entryorguid`=4392300 AND source_type=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4392300,9,0,0,0,0,100,0,2000,2000,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Forlorn Spirit - On Script - Die')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);
INSERT INTO `creature_template_addon` (`entry`,`path_id`,`mount`,`bytes1`,`bytes2`,`emote`,`auras`,`StandState`,`VisFlags`,`AnimTier`,`SheathState`,`PvPFlags`) VALUES
(43925,0,0,0,0,0,'82199',0,0,0,0,0)
ON DUPLICATE KEY UPDATE `path_id`=VALUES(`path_id`),`mount`=VALUES(`mount`),`bytes1`=VALUES(`bytes1`),`bytes2`=VALUES(`bytes2`),`emote`=VALUES(`emote`),`auras`=VALUES(`auras`),`StandState`=VALUES(`StandState`),`VisFlags`=VALUES(`VisFlags`),`AnimTier`=VALUES(`AnimTier`),`SheathState`=VALUES(`SheathState`),`PvPFlags`=VALUES(`PvPFlags`);
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=43925;
UPDATE `creature_template` SET `flags_extra`=128 WHERE `entry`=43925;
DELETE FROM `smart_scripts` WHERE `entryorguid`=43925 AND source_type=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(43925,0,0,0,54,0,100,0,0,0,0,0,0,80,4392500,0,0,0,0,0,1,0,0,0,0,0,0,0,'Soothing Incense Cloud - On Summon - Run Script')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);
DELETE FROM `smart_scripts` WHERE `entryorguid`=4392500 AND source_type=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4392500,9,0,0,0,0,100,0,2000,2000,0,0,0,33,43930,0,0,0,0,0,7,0,0,0,0,0,0,0,'Soothing Incense Cloud - On Script - Give Credit'),
(4392500,9,1,0,0,0,100,0,0,0,0,0,0,41,2000,2000,0,0,0,0,1,0,0,0,0,0,0,0,'Soothing Incense Cloud - On Script - Despawn')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);

-- 2026_08_25_world_00.sql
INSERT INTO gameobject_template_addon (entry,flags) VALUES (195074,4) ON DUPLICATE KEY UPDATE flags=flags | 4;
UPDATE `gameobject_loot_template` SET `ChanceOrQuestChance`=-100 WHERE `entry`=27260 AND `item`=46700;
DELETE FROM `gameobject_loot_template` WHERE `entry`=1701 AND `item`<>3418;

-- 2026_08_25_world_01.sql
DELETE FROM `quest_objective` WHERE `questId` IN (26383, 26385);
INSERT INTO `quest_objective` (`questId`,`id`,`index`,`type`,`objectId`,`amount`,`flags`,`description`) VALUES
(26383,2638300,0,10,0,0,0,'Use the teleportation tree in western Darnassus to reach Rut''theran Village, then speak to Vesprystus to secure a ride to Lor''danel.'),
(26385,2638500,0,10,0,0,0,'Speak to Vesprystus in Rut''theran Village to secure a ride to Lor''danel.')
ON DUPLICATE KEY UPDATE `type`=VALUES(`type`),`objectId`=VALUES(`objectId`),`amount`=VALUES(`amount`),`flags`=VALUES(`flags`),`description`=VALUES(`description`);
UPDATE `quest_template` SET `SpecialFlags`=`SpecialFlags`|2 WHERE `Id` IN (26383, 26385);
INSERT INTO quest_template_addon (ID,SpecialFlags) VALUES (26383,2),(26385,2) ON DUPLICATE KEY UPDATE SpecialFlags=SpecialFlags | 2;
DELETE FROM `smart_scripts` WHERE `entryorguid`=3838 AND `source_type`=0 AND `id` IN (3, 4);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(3838,0,3,0,64,0,100,0,0,0,0,0,0,15,26383,0,0,0,0,0,7,0,0,0,0,0,0,0,'Vesprystus - On Gossip Hello - Credit quest 26383'),
(3838,0,4,0,64,0,100,0,0,0,0,0,0,15,26385,0,0,0,0,0,7,0,0,0,0,0,0,0,'Vesprystus - On Gossip Hello - Credit quest 26385')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);

-- 2026_08_31_world_01.sql

-- 2026_09_01_world_01.sql
INSERT INTO gameobject_template_addon (entry,flags) VALUES (178087,0) ON DUPLICATE KEY UPDATE flags=flags & ~4;

-- 2026_09_01_world_02.sql
DELETE FROM `gameobject_loot_template` WHERE `entry` = 14980 AND `item` = 16332;
INSERT INTO `gameobject_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(14980,16332,100,1,0,1,1)
ON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);

-- 2026_09_06_world_00.sql
INSERT IGNORE INTO `creature_queststarter` (`id`,`quest`) VALUES
(20604,7792),
(20604,7798),
(20604,10356),
(20604,10357),
(20604,10358);
INSERT IGNORE INTO `creature_questender` (`id`,`quest`) VALUES
(20604,7792),
(20604,7798),
(20604,10356),
(20604,10357),
(20604,10358);
DELETE FROM `quest_objective` WHERE `questId` = 10358;
INSERT INTO `quest_objective` (`questId`,`id`,`index`,`type`,`objectId`,`amount`,`flags`,`description`) VALUES
(10358,1035800,0,1,14047,20,0,'')
ON DUPLICATE KEY UPDATE `type`=VALUES(`type`),`objectId`=VALUES(`objectId`),`amount`=VALUES(`amount`),`flags`=VALUES(`flags`),`description`=VALUES(`description`);

-- 2026_09_06_world_01.sql
UPDATE `creature_template` SET `npcflag` = 786433, `gossip_menu_id` = 7487 WHERE `entry` = 4161;
UPDATE `creature_template` SET `npcflag` = 786433, `gossip_menu_id` = 5108 WHERE `entry` = 4613;
UPDATE `creature_template` SET `npcflag` = 786433, `gossip_menu_id` = 5462 WHERE `entry` = 5130;
UPDATE `creature_template` SET `npcflag` = 786433, `gossip_menu_id` = 7484 WHERE `entry` = 52277;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`entry`,`text_id`) VALUES
(708,1260,708,1260),
(1626,2280,1626,2280),
(5108,6159,5108,6159),
(5461,6513,5461,6513),
(5462,6514,5462,6514),
(7440,9008,7440,9008),
(7484,9071,7484,9071),
(7487,9073,7487,9073),
(9838,13349,9838,13349)
ON DUPLICATE KEY UPDATE `MenuID`=VALUES(`MenuID`),`TextID`=VALUES(`TextID`);
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcFlag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`menu_id`,`id`,`option_icon`,`option_text`,`option_id`,`npc_option_npcflag`,`action_menu_id`,`action_poi_id`,`box_coded`,`box_money`,`box_text`) VALUES
(708,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,708,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(708,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,708,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(1626,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,1626,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(1626,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,1626,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(5108,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,5108,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(5108,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,5108,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(5461,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,5461,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(5461,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,5461,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(5462,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,5462,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(5462,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,5462,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(7440,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,7440,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(7440,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,7440,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(7484,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,7484,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(7484,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,7484,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(7487,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,7487,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(7487,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,7487,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL),
(9838,0,7,'How do I form a guild?',3413,10,262144,0,0,0,0,NULL,0,9838,0,7,'How do I form a guild?',10,262144,0,0,0,0,NULL),
(9838,1,8,'I want to create a guild crest.',3415,11,524288,0,0,0,0,NULL,0,9838,1,8,'I want to create a guild crest.',11,524288,0,0,0,0,NULL)
ON DUPLICATE KEY UPDATE `MenuID`=VALUES(`MenuID`),`OptionID`=VALUES(`OptionID`),`OptionIcon`=VALUES(`OptionIcon`),`OptionText`=VALUES(`OptionText`),`OptionBroadcastTextID`=VALUES(`OptionBroadcastTextID`),`OptionType`=VALUES(`OptionType`),`OptionNpcFlag`=VALUES(`OptionNpcFlag`),`ActionMenuID`=VALUES(`ActionMenuID`),`ActionPoiID`=VALUES(`ActionPoiID`),`BoxCoded`=VALUES(`BoxCoded`),`BoxMoney`=VALUES(`BoxMoney`),`BoxText`=VALUES(`BoxText`),`BoxBroadcastTextID`=VALUES(`BoxBroadcastTextID`),`option_icon`=VALUES(`option_icon`),`option_text`=VALUES(`option_text`),`option_id`=VALUES(`option_id`),`npc_option_npcflag`=VALUES(`npc_option_npcflag`),`action_menu_id`=VALUES(`action_menu_id`),`action_poi_id`=VALUES(`action_poi_id`),`box_coded`=VALUES(`box_coded`),`box_money`=VALUES(`box_money`),`box_text`=VALUES(`box_text`);

-- 2026_09_14_world_00.sql
INSERT INTO `trinity_string` (`entry`,`content_default`) VALUES
(2030,'Ticket %u completed and its response was sent to the player.')
ON DUPLICATE KEY UPDATE `content_default`=VALUES(`content_default`);

-- 2026_09_21_world_04.sql
DELETE FROM `disables` WHERE `sourceType` = 0 AND `entry` = 32979;
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(0,32979,64,'','','Ignore LOS for Flaming Torch - quest 10078 bunnies sit inside the towers')
ON DUPLICATE KEY UPDATE `flags`=VALUES(`flags`),`params_0`=VALUES(`params_0`),`params_1`=VALUES(`params_1`),`comment`=VALUES(`comment`);

-- 2026_09_21_world_09.sql
UPDATE `creature_template`
   SET `npcflag` = `npcflag` | 16777216
 WHERE `entry` IN (
    26191,

    28161,

    30272,

    35111,

    36287,

    36288,

    36289,

    36440,

    36459,

    36917,

    38150,

    38933,

    42548,

    42644,

    42646,

    42671,

    42673,

    43032,

    43048,

    43115,

    43182,

    44126,

    44427,

    44429,

    51396,

    51397,

    53517,

    53526,

    53561

   )
   AND (`npcflag` & 16777216) = 0 AND VehicleId=0 AND EXISTS (SELECT 1 FROM creature c WHERE c.id=creature_template.entry);
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(48671,90672,1,0)
ON DUPLICATE KEY UPDATE `cast_flags`=VALUES(`cast_flags`),`user_type`=VALUES(`user_type`);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 16777216 WHERE `entry` = 48671 AND VehicleId=0 AND EXISTS (SELECT 1 FROM creature c WHERE c.id=creature_template.entry);

-- 2026_10_04_world_02.sql
DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 19 AND `SourceGroup` = 0 AND `SourceEntry` = 13661 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,13661,0,0,8,0,13655,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (2 of 6) rewarded'),
(19,0,13661,0,0,8,0,13656,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (1 of 6) rewarded'),
(19,0,13661,0,0,8,0,13657,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (3 of 6) rewarded'),
(19,0,13661,0,0,8,0,13658,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (4 of 6) rewarded'),
(19,0,13661,0,0,8,0,13659,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (6 of 6) rewarded'),
(19,0,13661,0,0,8,0,13660,0,0,0,0,0,'','Heartfelt Appreciation requires Explorers'' League Document (5 of 6) rewarded')
ON DUPLICATE KEY UPDATE `NegativeCondition`=VALUES(`NegativeCondition`),`ErrorType`=VALUES(`ErrorType`),`ErrorTextId`=VALUES(`ErrorTextId`),`ScriptName`=VALUES(`ScriptName`),`Comment`=VALUES(`Comment`);
INSERT IGNORE INTO `creature_queststarter` (`id`,`quest`) VALUES
(1153,13661);

-- 2026_10_22_world_00.sql
DELETE FROM `playercreateinfo_action` WHERE `race` = 2 AND `class` = 7 AND `button` = 9;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(2,7,9,33697,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);
DELETE FROM `playercreateinfo_action` WHERE `race` = 2 AND `class` = 10 AND `button` = 9;
DELETE FROM `playercreateinfo_action` WHERE `race` = 2 AND `class` = 10 AND `button` = 81;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(2,10,9,33697,0),
(2,10,81,33697,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);
DELETE FROM `playercreateinfo_action` WHERE `race` = 11 AND `class` = 1 AND `button` = 9;
DELETE FROM `playercreateinfo_action` WHERE `race` = 11 AND `class` = 1 AND `button` = 81;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(11,1,9,28880,0),
(11,1,81,28880,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);
DELETE FROM `playercreateinfo_action` WHERE `race` = 11 AND `class` = 10 AND `button` = 9;
DELETE FROM `playercreateinfo_action` WHERE `race` = 11 AND `class` = 10 AND `button` = 81;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(11,10,9,121093,0),
(11,10,81,121093,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);
DELETE FROM `playercreateinfo_action` WHERE `race` = 10 AND `class` = 1 AND `button` = 9;
DELETE FROM `playercreateinfo_action` WHERE `race` = 10 AND `class` = 1 AND `button` = 81;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(10,1,9,69179,0),
(10,1,81,69179,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);
DELETE FROM `playercreateinfo_action` WHERE `race` = 10 AND `class` = 10 AND `button` = 9;
DELETE FROM `playercreateinfo_action` WHERE `race` = 10 AND `class` = 10 AND `button` = 81;
INSERT INTO `playercreateinfo_action` (`race`,`class`,`button`,`action`,`type`) VALUES
(10,10,9,129597,0),
(10,10,81,129597,0)
ON DUPLICATE KEY UPDATE `action`=VALUES(`action`),`type`=VALUES(`type`);

-- 2026_11_06_world_01.sql
UPDATE `creature_template` SET `trainer_type` = 2 WHERE `entry` IN (47400, 47418, 47419, 47420, 50247);
UPDATE `creature_template` SET `npcflag` = 81 WHERE `entry` = 50247;
UPDATE `creature_template` SET `gossip_menu_id` = 12180 WHERE `entry` = 47419;
DELETE FROM `gossip_menu_option` WHERE `MenuID` BETWEEN 12180 AND 12199 AND `OptionType` = 0
    AND `OptionText` LIKE 'Train me in %';
INSERT INTO `npc_trainer` (`entry`,`spell`,`spellcost`,`reqskill`,`reqskillvalue`,`reqlevel`) VALUES
(201010,2020,10,164,0,5),
(201010,2153,50,165,15,0),
(201010,2155,10,165,0,5),
(201010,2160,100,165,40,0),
(201010,2161,100,165,55,0),
(201010,2162,100,165,60,0),
(201010,2275,10,171,0,5),
(201010,2331,100,171,25,0),
(201010,2332,150,171,40,0),
(201010,2334,250,171,50,0),
(201010,2337,1000,171,55,0),
(201010,2372,10,182,0,0),
(201010,2385,50,197,10,0),
(201010,2386,200,197,65,0),
(201010,2392,50,197,40,0),
(201010,2393,25,197,1,0),
(201010,2394,50,197,40,0),
(201010,2395,300,197,70,0),
(201010,2396,200,197,70,0),
(201010,2397,200,197,60,0),
(201010,2402,250,197,75,0),
(201010,2581,10,186,0,0),
(201010,2658,200,186,65,0),
(201010,2659,200,186,50,0),
(201010,2661,100,164,35,0),
(201010,2662,50,164,1,0),
(201010,2665,100,164,65,0),
(201010,2666,200,164,70,0),
(201010,2737,50,164,15,0),
(201010,2738,50,164,20,0),
(201010,2739,50,164,25,0),
(201010,2964,100,197,75,0),
(201010,3116,100,164,65,0),
(201010,3170,50,171,15,0),
(201010,3293,250,164,35,0),
(201010,3294,500,164,70,0),
(201010,3304,50,186,50,0),
(201010,3319,50,164,20,0),
(201010,3320,100,164,25,0),
(201010,3323,100,164,40,0),
(201010,3324,200,164,45,0),
(201010,3326,250,164,75,0),
(201010,3753,75,165,25,0),
(201010,3755,100,197,45,0),
(201010,3756,150,165,55,0),
(201010,3759,200,165,75,0),
(201010,3816,50,165,35,0),
(201010,3840,100,197,35,0),
(201010,3841,200,197,60,0),
(201010,3842,300,197,70,0),
(201010,3911,10,197,0,5),
(201010,3914,50,197,30,0),
(201010,3922,115,202,30,0),
(201010,3923,130,202,30,0),
(201010,3925,150,202,50,0),
(201010,3929,250,202,65,0),
(201010,3931,250,202,65,0),
(201010,3977,200,202,60,0),
(201010,4039,10,202,0,5),
(201010,7408,300,164,65,0),
(201010,7414,10,333,0,5),
(201010,7420,50,333,15,0),
(201010,7426,100,333,40,0),
(201010,7430,150,202,50,0),
(201010,7457,250,333,50,0),
(201010,7623,50,197,30,0),
(201010,7624,50,197,30,0),
(201010,7748,250,333,60,0),
(201010,7771,200,333,70,0),
(201010,8465,50,197,40,0),
(201010,8615,10,393,0,0),
(201010,8776,50,197,15,0),
(201010,8880,100,164,30,0),
(201010,9065,150,165,70,0),
(201010,9983,100,164,30,0),
(201010,12045,50,197,20,0),
(201010,12046,300,197,75,0),
(201010,14293,50,333,10,0),
(201010,14807,200,333,70,0),
(201010,25245,10,755,0,5),
(201010,25278,200,755,50,0),
(201010,25280,200,755,50,0),
(201010,25283,100,755,30,0),
(201010,25284,400,755,60,0),
(201010,25287,400,755,70,0),
(201010,25490,300,755,50,0),
(201010,26926,50,755,5,0),
(201010,26927,300,755,50,0),
(201010,26928,100,755,30,0),
(201010,31252,100,755,20,0),
(201010,32178,100,755,20,0),
(201010,32179,100,755,20,0),
(201010,32801,200,755,50,0),
(201010,36523,600,755,75,0),
(201010,37818,500,755,65,0),
(201010,45375,10,773,0,5),
(201010,48248,50,773,35,0),
(201010,50598,400,773,75,0),
(201010,50605,400,773,75,0),
(201010,50612,400,773,75,0),
(201010,52739,150,773,35,0),
(201010,52843,50,773,35,0),
(201010,53462,300,773,75,0),
(201010,58472,50,773,15,0),
(201010,58484,50,773,15,0),
(201010,61288,1000,773,75,0),
(47400,-201010,0,0,0,0),
(47418,-201010,0,0,0,0),
(47419,-201010,0,0,0,0),
(47420,-201010,0,0,0,0),
(50247,-201010,0,0,0,0)
ON DUPLICATE KEY UPDATE `spellcost`=VALUES(`spellcost`),`reqskill`=VALUES(`reqskill`),`reqskillvalue`=VALUES(`reqskillvalue`),`reqlevel`=VALUES(`reqlevel`);

-- 2026_11_06_world_02.sql
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 15274 AND `ScriptName` = '';
DELETE FROM `smart_scripts` WHERE `entryorguid` = 15274 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(15274,0,0,0,8,0,100,0,28730,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (mana) - Cast Quest Credit 8346'),
(15274,0,1,0,8,0,100,0,25046,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (energy) - Cast Quest Credit 8346'),
(15274,0,2,0,8,0,100,0,50613,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (runic power) - Cast Quest Credit 8346'),
(15274,0,3,0,0,0,100,0,4000,4000,10000,15000,0,11,25602,0,0,0,0,0,5,0,0,0,0,0,0,0,'Mana Wyrm - In Combat - Cast Faerie Fire'),
(15274,0,4,0,8,0,100,0,69179,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (rage) - Cast Quest Credit 8346'),
(15274,0,5,0,8,0,100,0,80483,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (focus) - Cast Quest Credit 8346'),
(15274,0,6,0,8,0,100,0,129597,0,0,0,0,11,61314,0,0,0,0,0,7,0,0,0,0,0,0,0,'Mana Wyrm - On Spellhit Arcane Torrent (chi) - Cast Quest Credit 8346')
ON DUPLICATE KEY UPDATE `event_type`=VALUES(`event_type`),`event_phase_mask`=VALUES(`event_phase_mask`),`event_chance`=VALUES(`event_chance`),`event_flags`=VALUES(`event_flags`),`event_param1`=VALUES(`event_param1`),`event_param2`=VALUES(`event_param2`),`event_param3`=VALUES(`event_param3`),`event_param4`=VALUES(`event_param4`),`event_param5`=VALUES(`event_param5`),`action_type`=VALUES(`action_type`),`action_param1`=VALUES(`action_param1`),`action_param2`=VALUES(`action_param2`),`action_param3`=VALUES(`action_param3`),`action_param4`=VALUES(`action_param4`),`action_param5`=VALUES(`action_param5`),`action_param6`=VALUES(`action_param6`),`target_type`=VALUES(`target_type`),`target_param1`=VALUES(`target_param1`),`target_param2`=VALUES(`target_param2`),`target_param3`=VALUES(`target_param3`),`target_x`=VALUES(`target_x`),`target_y`=VALUES(`target_y`),`target_z`=VALUES(`target_z`),`target_o`=VALUES(`target_o`),`comment`=VALUES(`comment`);

-- 2026_11_06_world_03.sql
UPDATE `creature_template_addon` SET `auras` = '' WHERE `entry` = 59481;
