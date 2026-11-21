-- Combat Training (Alliance) 27174: Gory 45212 and Thurman's four settlers at the Writhing Haunt (zone_western_plaguelands.cpp)
-- Sourced: both TDBs give Gory faction 35 and npcflag 1 (gossip menu 11953, npc_text 16780 "Gory always wanted to be a farmer.");
-- broadcast_text 45366 "You want to fight?  Alright, Gory fight." and 45367 "Gory give up!  You farmers fight more good!"; 5.4.8
-- DBC 84460 Combat Training (dummy aura) and 84452 / 84456-84458 Summon Provincial Minuteman 1 (Pet) / 2-4 (45231-45234);
-- warcraft.wiki.gg (Combat Training, Alliance): "Accepting the quest makes four Provincial Minutemen (farmers) appear. You control
-- them with a basic pet interface", "When defeated, he gives up and tells the farmers they fight well"; Wowhead: "I got him on 1
-- health and he gave up"; quest text "Take a group of four settlers and challenge this abomination to combat".
-- Ours: the challenge option's text (no source has it), its condition (27174 in progress), the 84460 bounds (the Writhing Haunt,
-- area 202, while 27174 is in the log), Gory giving up below 10 % health, and his SmartAI (Cleave 40504, Scourge Hook 50335 at
-- 8-40 yd) moved into his C++ AI with the same timings.
UPDATE `creature_template` SET `faction`=35, `npcflag`=1, `AIName`='', `ScriptName`='npc_wpl_gory' WHERE `entry`=45212;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45212 AND `source_type`=0;
UPDATE `creature_template` SET `ScriptName`='npc_wpl_provincial_minuteman' WHERE `entry` IN (45231,45232,45233,45234);

DELETE FROM `creature_text` WHERE `CreatureID`=45212;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45212,0,0,'You want to fight?  Alright, Gory fight.',12,0,100,0,0,0,0,45366,0,'Gory - challenged'),
(45212,1,0,'Gory give up!  You farmers fight more good!',12,0,100,0,0,0,0,45367,0,'Gory - gives up');

DELETE FROM `gossip_menu_option` WHERE `MenuID`=11953 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(11953,0,0,'I challenge you to combat, Gory.',0,1,1,0,0,0,0,NULL,0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=11953 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11953,0,0,0,47,0,27174,8,0,0,0,0,'','Gory - challenge option while Combat Training is in progress');

DELETE FROM `spell_area` WHERE `spell`=84460 AND `area`=202;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84460,202,27174,0,0,0,2,1,10,11);

DELETE FROM `spell_script_names` WHERE `spell_id`=84460;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84460,'spell_wpl_combat_training_minutemen');
