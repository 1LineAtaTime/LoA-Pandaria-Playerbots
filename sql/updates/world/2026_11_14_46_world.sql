-- Eastern Plaguelands services: Gek Nozzlerocket's sister and plane at Light's Hope are seen, Khaelyn Steelwing offers her
-- own flight line, and two graveyards get a Spirit Healer.
-- Gertrude Nozzlerocket 48705 and The Uncrashable 48708 wear 87871 (Generic Quest Invisibility 21, type 29) and nothing in
-- Eastern Kingdoms detects it (103458's only spell_area row is area 5805 in Pandaria), so they were invisible to everyone
-- while Gek 48704 stood alone. Sourced: she was seen there in Mists (Wowhead MoP Classic pin 73.8,53, 4 yd from her spawn).
-- Ours: the rule, everyone at Light's Hope Chapel (2268), as mop's own 5805 row has it (no source gives retail's rule;
-- Gek's breadcrumb 27762 has MinLevel 44 itself).
DELETE FROM `spell_area` WHERE `spell`=103458 AND `area`=2268;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(103458,2268,0,0,0,0,2,1,0,0);
-- Khaelyn Steelwing (menu 12236): "Show me where I can fly." (both TDBs; text and broadcast as menu 6944 option 0, which the
-- tower flight masters use). Without an option row the default menu's "I need a ride." showed.
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12236 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12236,0,2,'Show me where I can fly.',12271,4,8192,0,0,0,0,NULL,0,0);
-- Spirit Healers at graveyards 1448 Northdale and 1450 west of Thondroril, linked to the zone (game_graveyard_zone) with no
-- healer within 100 yd: a ghost released there had to run back to its corpse. Ours: the positions (5 yd in front of the
-- graveyard point along its facing, z from the terrain; phaseMask 65535 as most
-- of mop's Spirit Healers); a sniff sees a healer only while dead, so neither TDB has one.
DELETE FROM `creature` WHERE `guid` IN (3129601,3129602);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`curhealth`) VALUES
(3129601,6491,0,139,2272,1,65535,3123.53,-4807.77,100.79,3.945,300,1),
(3129602,6491,0,139,139,1,65535,2212.95,-2925.64,108.60,2.601,300,1);
