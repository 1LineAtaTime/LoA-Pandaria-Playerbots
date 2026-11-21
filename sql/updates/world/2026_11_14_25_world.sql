-- Eastern Plaguelands, Traveling Companions (27381): the conversations the quest is made of.
--
-- The wiki ("Conversation with ...") lists five people asked "Are you interested in joining our caravan? We're headed to
-- Light's Hope Chapel." (broadcast 45662): Argus Highbeacon says yes on a condition, the other four decline.
--
-- 1. Argus Highbeacon 45451 (top of Crown Guard Tower, menu 12034): option 0 gave the credit from SmartAI and the linked row
--    closed the window, so his answer (menu 12030, npc_text 16862 = broadcast 45663 "Well, I am headed in that direction ...
--    I'll make you a deal: help me clear the roads, and yes, I'll join your group.") was never shown. Sourced: both TDBs have
--    ActionMenuID 12030 on 12034/0. Ours: drop the close-gossip row, as the Western Plaguelands fix of 27153 (7887f1a49):
--    SmartAI::OnGossipSelect returns false when no SEND_GOSSIP_MENU action runs, so the handler then calls
--    Player::OnGossipSelect, which sends 12030 after the credit. The answer page has no options; a second ask gives no second
--    credit (objective x1) and shows the page again.
-- 2. Urk Gagbaz 45500 (menu 12033): "Ugh! No, I could never. Afraid of horses." (broadcast 45666). Sourced: both TDBs have
--    option 12033/0 -> menu 12028 -> npc_text 16860; all three were missing in mop (ids checked free).
-- 3. Carlin Redpath 11063 (menu 3864): option 2 existed with ActionMenuID 0 and OptionNpcflag 0, unconditioned. Sourced: both
--    TDBs have 3864/2 -> 12026 (npc_text 16858 = broadcast 45665 "I've spent far too long in that town ...", present in mop).
--    Carlin's SmartAI handles only option 1 (the Annals); option 2 is not scripted.
-- 4. Chromie 10667 (menu 2911): "No thanks! Maybe sometime in the future. Or the past. Not today." (broadcast 45667). Sourced:
--    both TDBs have 2911/0 -> 12027 (npc_text 16859, present in mop); the option was missing.
-- 5. Janice Myers 44232 (flight master, menu 12032): "I'm sorry, I can't. I've been tasked to watch this hippogryph post. These
--    things might seem tame, but they really require a twenty-four hour vigil." Sourced: the wiki's conversation and
--    broadcast 45668 (in mop). Neither TDB has her option, answer menu or text. Ours: option 0 (her flight option is 1), and
--    the ids menu 12029 / npc_text 16861: free in mop and in both TDBs, and the gaps in the retail sequence of this quest's
--    answers (menus 12026 Carlin, 12027 Chromie, 12028 Urk, 12030 Argus; texts 16858, 16859, 16860, 16862), so most likely
--    hers (inferred).
-- The four refusals show only while 27381 is in the log and not yet complete (condition 9), like mop's own condition on Argus'
-- option. Ours: the conditions (neither TDB conditions these options).
-- 6. Argus' greeting: menu 12034 has two texts, 16864 "Yes, traveler?" (broadcast 45670) and 16963 "It's good to have
--    traveling companions. The road gets lonely out here." (broadcast 46137), both sniffed, neither conditioned, so the
--    second always showed (Player::GetGossipTextId keeps the last text whose conditions pass). Ours, as the story runs: he
--    agrees to come once the roads are cleared (Rough Roads 27382, his own quest; then The Trek Continues 27448 takes him
--    along, and his caravan copies at the later stops share this menu), so 16864 until 27382 is rewarded, 16963 after.
--    Broadcast 45664 ("You've helped me clear the roads ... Let's get going!") stays unused: no source says which menu showed it.

-- 1. Argus
UPDATE `gossip_menu_option` SET `ActionMenuID` = 12030 WHERE `MenuID` = 12034 AND `OptionID` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45451 AND `source_type` = 0 AND `id` = 1 AND `action_type` = 72;
UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 45451 AND `source_type` = 0 AND `id` = 0;

-- 2. Urk
DELETE FROM `npc_text` WHERE `ID` = 16860;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`)
SELECT 16860, `Text`, `Text1`, `ID`, 0, 1 FROM `broadcast_text` WHERE `ID` = 45666;
DELETE FROM `gossip_menu` WHERE `MenuID` = 12028;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES (12028, 16860);
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 12033 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`) VALUES
(12033, 0, 0, 'Are you interested in joining our caravan?  We''re headed to Light''s Hope Chapel.', 45662, 1, 1, 12028);

-- 3. Carlin
UPDATE `gossip_menu_option` SET `OptionNpcflag` = 1, `ActionMenuID` = 12026 WHERE `MenuID` = 3864 AND `OptionID` = 2;

-- 4. Chromie
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 2911 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`) VALUES
(2911, 0, 0, 'Are you interested in joining our caravan?  We''re headed to Light''s Hope Chapel.', 45662, 1, 1, 12027);

-- 5. Janice
DELETE FROM `npc_text` WHERE `ID` = 16861;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`)
SELECT 16861, `Text`, `Text1`, `ID`, 0, 1 FROM `broadcast_text` WHERE `ID` = 45668;
DELETE FROM `gossip_menu` WHERE `MenuID` = 12029;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES (12029, 16861);
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 12032 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`) VALUES
(12032, 0, 0, 'Are you interested in joining our caravan?  We''re headed to Light''s Hope Chapel.', 45662, 1, 1, 12029);

-- 6. Argus' greeting by quest state
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 14 AND `SourceGroup` = 12034 AND `SourceEntry` IN (16864, 16963);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(14, 12034, 16864, 0, 0, 8, 0, 27382, 0, 0, 1, 0, 0, '', 'Argus Highbeacon - "Yes, traveler?" until Rough Roads is rewarded'),
(14, 12034, 16963, 0, 0, 8, 0, 27382, 0, 0, 0, 0, 0, '', 'Argus Highbeacon - "It''s good to have traveling companions" once Rough Roads is rewarded');

-- The four refusals: only while Traveling Companions is in the log, incomplete
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND ((`SourceGroup` = 12033 AND `SourceEntry` = 0) OR (`SourceGroup` = 3864 AND `SourceEntry` = 2) OR (`SourceGroup` = 2911 AND `SourceEntry` = 0) OR (`SourceGroup` = 12032 AND `SourceEntry` = 0));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12033, 0, 0, 0, 9, 0, 27381, 0, 0, 0, 0, 0, '', 'Urk Gagbaz - caravan question while Traveling Companions is in the log'),
(15, 3864, 2, 0, 0, 9, 0, 27381, 0, 0, 0, 0, 0, '', 'Carlin Redpath - caravan question while Traveling Companions is in the log'),
(15, 2911, 0, 0, 0, 9, 0, 27381, 0, 0, 0, 0, 0, '', 'Chromie - caravan question while Traveling Companions is in the log'),
(15, 12032, 0, 0, 0, 9, 0, 27381, 0, 0, 0, 0, 0, '', 'Janice Myers - caravan question while Traveling Companions is in the log');
