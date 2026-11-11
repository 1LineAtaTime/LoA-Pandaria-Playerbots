-- Vendor and bind gossip options that never showed.
-- These gossip_menu_option rows were imported as plain gossip (OptionType 1) with OptionNpcflag 0. Player::PrepareGossipMenu
-- skips any option whose OptionNpcflag shares no bit with the creature's npcflag, so 0 hides it for everyone: Eliza Darkgrin
-- (Jintha'Alor), Sijambi, Reese Langston and others could not be bought from, and several innkeepers could not bind.
-- Sourced: the retail TDB 1210 gives each of these rows OptionNpc 1 (vendor) or 5 (binder); this core's equivalents are
-- OptionType 3 / OptionNpcflag 128 (vendor; the window offers repairs when the NPC repairs) and OptionType 8 / 65536 (inn).
-- Left out: Sijambi's unsniffed second pair (menu 11919 options 2 and 3, VerifiedBuild 0 in the TDB), which repeats options 0
-- and 1. The option shows only when the creature has the matching npcflag (and, for vendors, items), as before.
UPDATE `gossip_menu_option` SET `OptionType`=3, `OptionNpcflag`=128 WHERE `OptionType`=1 AND `OptionNpcflag`=0 AND (`MenuID`,`OptionID`) IN
((702,0),(2831,1),(10898,0),(11159,0),(11402,1),(11437,1),(11453,0),(11465,0),(11505,0),(11506,0),(11520,1),(11522,1),(11538,1),
(11646,0),(11801,0),(11822,1),(11919,1),(11920,0),(12012,0),(12014,0),(12015,0),(12017,0),(12022,0),(12025,1),(12033,1),(12040,1),
(12100,3),(12260,0),(12280,0),(12439,1),(12443,1),(12599,1),(12642,0),(12656,0),(12823,0),(12838,1));
UPDATE `gossip_menu_option` SET `OptionType`=8, `OptionNpcflag`=65536 WHERE `OptionType`=1 AND `OptionNpcflag`=0 AND (`MenuID`,`OptionID`) IN
((2831,0),(11402,0),(11437,0),(11520,0),(11522,0),(11538,0),(11822,0),(11919,0),(12003,0),(12025,0),(12040,0),(12100,2),(12599,0));

-- Eliza Darkgrin 42622 (Poisons & Reagents, Jintha'Alor): the reagents both TDBs sell (Arcane Powder, Ankh, Rune of
-- Teleportation, Rune of Portals, Maple Seed) and the Simple Dagger, which complete Wowhead MoP Classic's list of 14.
DELETE FROM `npc_vendor` WHERE `entry`=42622 AND `item` IN (4565,17020,17030,17031,17032,17034);
INSERT INTO `npc_vendor` (`entry`,`slot`,`item`,`maxcount`,`incrtime`,`ExtendedCost`,`type`) VALUES
(42622,0,4565,0,0,0,1),(42622,0,17020,0,0,0,1),(42622,0,17030,0,0,0,1),(42622,0,17031,0,0,0,1),(42622,0,17032,0,0,0,1),(42622,0,17034,0,0,0,1);
