-- Audrid Grenich's stock. Sourced: the Sells tab of Wowhead MoP Classic npc 44047 lists 14 items: mop's 8 plus these 6 (the TDB's 5 and Simple Dagger). The slot value follows mop's rows for her.
DELETE FROM `npc_vendor` WHERE `entry` = 44047 AND `item` IN (4565,17020,17030,17031,17032,17034);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`) VALUES
(44047,0,4565,0,0,0,1),
(44047,0,17020,0,0,0,1),
(44047,0,17030,0,0,0,1),
(44047,0,17031,0,0,0,1),
(44047,0,17032,0,0,0,1),
(44047,0,17034,0,0,0,1);
