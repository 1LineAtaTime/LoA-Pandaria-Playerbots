-- Western Plaguelands, The Good People of Hearthglen (27153): show each NPC's answer after the question.
--
-- Morris Vant 45148 (menu 11934), Outfitter Mendelev 45149 (11936), Bree Ironstock 45150 (11938) and High Cleric Alphus 45151
-- (11940) give their kill credit from SmartAI on option 0, but the option had ActionMenuID 0 and the script closed the window, so
-- the answer pages (menus 11935/11937/11939/11941, npc_text 16755/16757/16759/16761, already present) were never shown.
--  * Sourced: retail TDB 1210 and Cataclysm Classic TDB 442 both have ActionMenuID 11935/11937/11939/11941 on these options.
--  * Ours: drop the linked SMART_ACTION_CLOSE_GOSSIP (id 1). SmartAI::OnGossipSelect returns false when no SEND_GOSSIP_MENU
--    action runs, so HandleGossipSelectOptionOpcode then calls Player::OnGossipSelect, which sends ActionMenuID's menu
--    (GOSSIP_OPTION_GOSSIP). The answer menus have no options, so they cannot give the credit again.
--  * Magus Bisp 45152 (menu 11942) has ActionMenuID 0 in both TDBs too (he turns hostile): unchanged.

UPDATE `gossip_menu_option` SET `ActionMenuID` = 11935 WHERE `MenuID` = 11934 AND `OptionID` = 0;
UPDATE `gossip_menu_option` SET `ActionMenuID` = 11937 WHERE `MenuID` = 11936 AND `OptionID` = 0;
UPDATE `gossip_menu_option` SET `ActionMenuID` = 11939 WHERE `MenuID` = 11938 AND `OptionID` = 0;
UPDATE `gossip_menu_option` SET `ActionMenuID` = 11941 WHERE `MenuID` = 11940 AND `OptionID` = 0;

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (45148,45149,45150,45151) AND `source_type` = 0 AND `id` IN (0,1);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45148,0,0,0,62,0,100,0,11934,0,0,0,33,45148,0,0,0,0,0,7,0,0,0,0,0,0,0,'Morris Vant - On Gossip Option 0 Selected - Quest Credit'),
(45149,0,0,0,62,0,100,0,11936,0,0,0,33,45149,0,0,0,0,0,7,0,0,0,0,0,0,0,'Outfitter Mendelev - On Gossip Option 0 Selected - Quest Credit'),
(45150,0,0,0,62,0,100,0,11938,0,0,0,33,45150,0,0,0,0,0,7,0,0,0,0,0,0,0,'Bree Ironstock - On Gossip Option 0 Selected - Quest Credit'),
(45151,0,0,0,62,0,100,0,11940,0,0,0,33,45151,0,0,0,0,0,7,0,0,0,0,0,0,0,'High Cleric Alphus - On Gossip Option 0 Selected - Quest Credit');
