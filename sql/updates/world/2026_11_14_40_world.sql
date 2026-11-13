-- Eastern Plaguelands, Augustus' Receipt Book (27534): Augustus the Touched trades once his receipt book is back, greets by
-- state and yells his prices. Data replaces npc_augustus_the_touched (removed in the same commit), which offered trade only
-- for the retired quest 6164 (no starter, no ender: nobody could have it rewarded), so he never traded.
-- Sourced: the "Yes, Augustus. I would like to do business." option appears with this quest (wiki Notes; comment
--   2013-08-28 [+8]); the option, menu 4085 and both texts are mop's rows already; the yells, their emote and timer
--   (both TDBs: creature_text 12384 group 0, broadcast 7627-7629, SmartAI out of combat 1-15 s then every 150-180 s;
--   comments 2010-11-26 [+24], 2010-09-12, 2011-01-01).
-- Ours: "Before the Scourge came..." before the book and "Hi $n! Are you here to buy something?" after it (their wording
--   decides it); "Commerce! I crave it!" (broadcast 7622) added to the yells.
-- Default gossip honours both: Player::PrepareGossipMenu skips an option whose source-15 conditions fail and shows a vendor
-- option for npcflag 128 with a non-empty list (7 rows); Player::GetGossipTextId takes the last text whose source-14
-- conditions pass; SmartAI::OnGossipHello returns false, so the default menu is built.
-- His seven items also carry the condition (source 23): a vendor list asked for without the gossip (an addon or a
-- crafted packet) comes back empty before the hand-in (SendListInventory filters by it). Player::BuyItemFromVendorSlot
-- does not check vendor conditions (core-wide, not changed here).
UPDATE `creature_template` SET `ScriptName`='',`AIName`='SmartAI' WHERE `entry`=12384;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` IN (14,15) AND `SourceGroup`=4085;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`Comment`) VALUES
(15,4085,0,0,0,8,0,27534,0,0,0,'Augustus the Touched: trade once Augustus'' Receipt Book is rewarded'),
(14,4085,4979,0,0,8,0,27534,0,0,1,'Augustus the Touched: text before the receipt book'),
(14,4085,4980,0,0,8,0,27534,0,0,0,'Augustus the Touched: text after the receipt book');
DELETE FROM `creature_text` WHERE `CreatureID`=12384;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(12384,0,0,'You won''t find better prices anywhere!',14,0,100,22,0,0,0,7627,0,'Augustus the Touched'),
(12384,0,1,'I''m Augustus, and my prices are INSANE!',14,0,100,22,0,0,0,7628,0,'Augustus the Touched'),
(12384,0,2,'I''m Augustus!  You want it, I got it!  You wanna sell it, then I want it!',14,0,100,22,0,0,0,7629,0,'Augustus the Touched'),
(12384,0,3,'Commerce!  I crave it!',14,0,100,22,0,0,0,7622,0,'Augustus the Touched');
DELETE FROM `smart_scripts` WHERE `entryorguid`=12384 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(12384,0,0,0,1,0,100,0,1000,15000,150000,180000,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Augustus the Touched - Out of Combat - Say Line 0');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=23 AND `SourceGroup`=12384;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`Comment`) VALUES
(23,12384,4604,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,4605,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,4606,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,4607,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,4608,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,8948,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded'),
(23,12384,15902,0,0,8,0,27534,0,0,0,'Augustus the Touched: sells only once Augustus'' Receipt Book is rewarded');
