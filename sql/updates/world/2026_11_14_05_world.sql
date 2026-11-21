-- Eastern Plaguelands, Fiona's Caravan: Fiona's and Beezil's greetings follow the story, Beezil offers to join, and the
-- template and quest fields both TDBs have.
--
--  * Before: Fiona's menu 12022 has five greetings and no conditions, so every player read the same one for the whole zone;
--    Beezil's menu 12081 has two greetings and no conditions and lacked the option of both TDBs that asks him to join;
--    Greasing the Wheel 27369 and A Gift For Fiona 27372 did not lead on to Onward 27373; the freed Gidwin 45431 could be
--    attacked; Tarenar 45729 had no gossip flag; the map point of Boys Will Be Boys 27455 was inside the chapel.
--  * Sourced: the texts and the option (menu 12081 option 0 "You know, if you're looking for a place to hide out..."
--    broadcast 46119, to menu 12082 "Sure, I'll join you", TDB 4.4.2); RewardNextQuest 27373 on 27369 and 27372, 45431
--    IMMUNE_TO_PC, 45729 GOSSIP (both TDBs); the hand-in of 27455 is the statue Gidwin, 2262,-5308 ("the map for this quest
--    is wrong", 518484, 419121).
--  * Ours: when each greeting shows, read from what it says:
--      16848 "Where are those lunkheads?"          while Gidwin (27369) or Tarenar (27372) is not back
--      16945 "Looks like they found their way back home. I'm relieved."   both back, Onward not yet taken
--      16946 "I'd like to set up shop in Light's Hope Chapel someday."    on the road, until Gidwin runs off
--      16947 "Why would he run off like that?!"     Ix'lar 27487 or Impatience 27488 handed in, until Gidwin's Fate
--                                                  Revealed 27526 is handed in (she learns it at 27487's hand-in)
--      16948 "Good to see you again, $n!"          from 27526 rewarded
--      Beezil 16956 (hiding, "here solely due to my hatred of the Scourge") until he has joined, 16958 "Hey boss" after;
--      "joined" = the flag quest [FLAG] Beezil's Flag 27563 rewarded, which section 2's gossip script on 46022 does when
--      the option is chosen (until then the option opens 12082 and he stays where he is). The option is shown until then.
--    The conditions of each menu exclude one another, so the order in which the core reads the texts does not matter.

-- Fiona's greetings (menu 12022).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup`=12022;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(14,12022,16848,0,0,8,0,27369,0,0,1,0,0,'','Fiona: "Where are those lunkheads?" while Gidwin is not back'),
(14,12022,16848,0,1,8,0,27372,0,0,1,0,0,'','Fiona: "Where are those lunkheads?" while Tarenar is not back'),
(14,12022,16945,0,0,8,0,27369,0,0,0,0,0,'','Fiona: both are back (Gidwin)'),
(14,12022,16945,0,0,8,0,27372,0,0,0,0,0,'','Fiona: both are back (Tarenar)'),
(14,12022,16945,0,0,47,0,27373,1,0,0,0,0,'','Fiona: Onward not taken yet'),
(14,12022,16946,0,0,47,0,27373,74,0,0,0,0,'','Fiona: on the road'),
(14,12022,16946,0,0,8,0,27487,0,0,1,0,0,'','Fiona: on the road, Gidwin not yet run off (Ix''lar)'),
(14,12022,16946,0,0,8,0,27488,0,0,1,0,0,'','Fiona: on the road, Gidwin not yet run off (Impatience)'),
(14,12022,16947,0,0,8,0,27487,0,0,0,0,0,'','Fiona: "Why would he run off like that?!" (Ix''lar handed in)'),
(14,12022,16947,0,0,8,0,27526,0,0,1,0,0,'','Fiona: "Why would he run off like that?!" until Gidwin is back'),
(14,12022,16947,0,1,8,0,27488,0,0,0,0,0,'','Fiona: "Why would he run off like that?!" (Impatience handed in)'),
(14,12022,16947,0,1,8,0,27526,0,0,1,0,0,'','Fiona: "Why would he run off like that?!" until Gidwin is back'),
(14,12022,16948,0,0,8,0,27526,0,0,0,0,0,'','Fiona: "Good to see you again" once Gidwin is back');

-- Beezil (menu 12081): the offer to join, and his two greetings.
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12081 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12081,0,0,'You know, if you''re looking for a place to hide out, you''re free to join us in our caravan.',46119,1,1,12082,0,0,0,'',0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` IN (14,15) AND `SourceGroup`=12081;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,12081,0,0,0,8,0,27563,0,0,1,0,0,'','Beezil: offer a place in the caravan until he has joined'),
(14,12081,16956,0,0,8,0,27563,0,0,1,0,0,'','Beezil: hiding at Light''s Hope (not joined)'),
(14,12081,16958,0,0,8,0,27563,0,0,0,0,0,'','Beezil: "Hey boss" (joined)');

-- Sourced fields (both TDBs).
UPDATE `quest_template` SET `RewardNextQuest`=27373 WHERE `ID` IN (27369,27372);
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|256 WHERE `entry`=45431;   -- IMMUNE_TO_PC
UPDATE `creature_template` SET `npcflag`=`npcflag`|1 WHERE `entry`=45729;           -- GOSSIP

-- Boys Will Be Boys: the map point on the statue Gidwin (373696), not inside the chapel.
UPDATE `quest_poi_points` SET `X`=2262, `Y`=-5308 WHERE `QuestID`=27455 AND `Idx1`=0 AND `Idx2`=0;
