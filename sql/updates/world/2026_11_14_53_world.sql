-- Eastern Plaguelands, Light's Hope Chapel: two conversations as both TDBs (4.4.2 and 12.1.0; all sourced, nothing ours).
--  * Crusade Commander Korfax 16112, menu 7099: mop showed "Korfax, where can I find Dark Iron scraps?" twice. Both TDBs have
--    option 0 "What is Dreadnaught armor?" (broadcast 11960) -> menu 7106 (npc_text 8363) -> "Continue please." (11962) ->
--    7107 (8364) -> "Anything else?" (11964) -> 7108 (8365), and option 1 the Dark Iron scraps (-> 7151).
--  * Rimblat Earthshatter 16134, menu 7103: "What brings a tauren to the Plaguelands?" (11983) -> 7113 (8371) -> "Continue
--    please." (11985) -> 7114 (8372) -> "Continue please." -> 7115 (8373).
--  * The npc_text rows take their text from broadcast_text (8363 and 8364 existed with probability 0; 8365, 8371-8373 were
--    missing).
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (7099,7106,7107,7103,7113,7114);
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(7099,0,0,'What is Dreadnaught armor?',11960,1,1,7106,0,0,0,NULL,0,0),
(7099,1,0,'Korfax, where can I find Dark Iron scraps?',12088,1,1,7151,0,0,0,NULL,0,0),
(7106,0,0,'Continue please.',11962,1,1,7107,0,0,0,NULL,0,0),
(7107,0,0,'Anything else?',11964,1,1,7108,0,0,0,NULL,0,0),
(7103,0,0,'What brings a tauren to the Plaguelands?',11983,1,1,7113,0,0,0,NULL,0,0),
(7113,0,0,'Continue please.',11985,1,1,7114,0,0,0,NULL,0,0),
(7114,0,0,'Continue please.',11985,1,1,7115,0,0,0,NULL,0,0);

DELETE FROM `gossip_menu` WHERE `MenuID` IN (7106,7107,7108,7113,7114,7115);
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES
(7106,8363,0),(7107,8364,0),(7108,8365,0),(7113,8371,0),(7114,8372,0),(7115,8373,0);

DELETE FROM `npc_text` WHERE `ID` IN (8363,8364,8365,8371,8372,8373);
INSERT INTO `npc_text` (`ID`,`text0_0`,`text0_1`,`BroadcastTextID0`,`Probability0`)
SELECT `n`.`ID`,`b`.`Text`,`b`.`Text1`,`b`.`ID`,1 FROM `broadcast_text` `b`
JOIN (SELECT 8363 AS `ID`, 11961 AS `bt` UNION ALL SELECT 8364,11963 UNION ALL SELECT 8365,11965 UNION ALL SELECT 8371,11984 UNION ALL SELECT 8372,11986 UNION ALL SELECT 8373,11987) `n` ON `n`.`bt`=`b`.`ID`;
