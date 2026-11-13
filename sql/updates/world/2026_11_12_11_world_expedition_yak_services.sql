-- Grand Expedition Yak62809 carries Mystic Birdhat62821 and Cousin Slowhands62822.
-- Restore the native5.4.8 reforger flag and matching LoA merchant stock.
-- Preserve existing vendor entries/custom prices. No artificial OPEN_REFORGE packet is sent.
UPDATE creature_template SET npcflag=npcflag|134217728 WHERE entry=62821;
UPDATE creature_template SET npcflag=npcflag|128|4096 WHERE entry=62822;
INSERT IGNORE INTO npc_vendor (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`) VALUES
(62822,0,159,0,0,0,1),
(62822,0,4470,0,0,0,1),
(62822,0,58274,0,0,0,1),
(62822,0,60335,0,0,0,1),
(62822,0,63388,0,0,0,1),
(62822,0,64670,0,0,0,1),
(62822,0,79249,0,0,0,1),
(62822,0,81400,0,0,0,1),
(62822,0,81401,0,0,0,1),
(62822,0,81406,0,0,0,1),
(62822,0,81407,0,0,0,1),
(62822,0,81409,0,0,0,1),
(62822,0,81414,0,0,0,1),
(62822,0,81415,0,0,0,1);
