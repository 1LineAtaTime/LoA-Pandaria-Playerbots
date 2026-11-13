-- #963 (lane b): creature_text groups that core/SmartAI scripts call but that had no rows.
-- Every text below was verified against at least one external source; the BroadcastTextId is the
-- matching row of this DB's own broadcast_text (same string), so locales and sounds line up.
-- Idempotent: each block deletes exactly the (CreatureID, GroupID) it inserts.
-- Undo: delete the listed (CreatureID, GroupID) pairs; for 8719 restore group 0 to the single
--       row (8719,0,0,BroadcastTextId 52640) it had before.

-- 30196 Ebon Blade Prisoner, group 0 (OOC-LoS say to the player).
-- Source: TrinityCore TDB_full_world_335.26091 (2026-09-09) + TDB 442.26081 + TDB 1210.26091,
--   creature_text 30196/0/0-7, identical in all three. (AC acore_world has no rows.)
DELETE FROM `creature_text` WHERE `CreatureID`=30196 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(30196,0,0,'Hey you! There are keys on the vrykul. Kill them, get a key and come over here to let me out of this damned cage!',12,0,100,0,0,0,0,30882,0,'Ebon Blade Prisoner'),
(30196,0,1,'It gets the key. It comes here. It opens the lock.',12,0,100,0,0,0,0,30883,0,'Ebon Blade Prisoner'),
(30196,0,2,'Think you could bust me out of this cage?',12,0,100,0,0,0,0,30884,0,'Ebon Blade Prisoner'),
(30196,0,3,'Good thing I don''t really feel the cold.',12,0,100,0,0,0,0,30885,0,'Ebon Blade Prisoner'),
(30196,0,4,'I''m over here in this cage. You can''t miss it!',12,0,100,0,0,0,0,30886,0,'Ebon Blade Prisoner'),
(30196,0,5,'Let me out and we''ll fight side-by-side. The slaughter will be glorious!',12,0,100,0,0,0,0,30887,0,'Ebon Blade Prisoner'),
(30196,0,6,'Were you sent by Baron Sliver to get me out of this cage? Finally. Hurry up!',12,0,100,0,0,0,0,30888,0,'Ebon Blade Prisoner'),
(30196,0,7,'When you get me out of this cage, we''re going to make them wish they were never awakened!',12,0,100,0,0,0,0,30889,0,'Ebon Blade Prisoner');

-- 23685 Gordok Brew Barker, group 1 ("Yell Line 1 (P1)" of the Brewfest Dark Iron event).
-- Source: bromacia/TrinityCore-SymboCore sql/custom/gary_brewfest.sql line ~496 (same block in
--   PuniCZ/soulcore souldb/25_Brewfest.sql), fetched 2026-10-28: (@ENTRY,1,0,"SOMEONE TRY THIS SUPER BREW!",14,...).
--   Wowpedia "Gordok Brew Barker" Quotes (MediaWiki API, 2026-10-28) lists "SOMEONE TRY THIS SUPER BREW!!".
--   broadcast_text 22547 holds the exact string.
DELETE FROM `creature_text` WHERE `CreatureID`=23685 AND `GroupID`=1;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(23685,1,0,'SOMEONE TRY THIS SUPER BREW!!',14,0,100,0,0,0,0,22547,0,'Gordok Brew Barker');

-- 19354 Arzeth the Merciless, group 1 (said when hit by 'Fury of the Dreghood Elders', smart_scripts id 4).
-- Source: Wowpedia "Arzeth the Merciless" Quotes (MediaWiki API, 2026-10-28): yell
--   "That Broken worm gave you that staff, didn't he? ..."; broadcast_text 18349.
--   (TC 335 has no group 1: its link row says group 0; TC/AC/CPP/SkyFire/the reference 5.4.8 DBs have no group 1.)
DELETE FROM `creature_text` WHERE `CreatureID`=19354 AND `GroupID`=1;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(19354,1,0,'That Broken worm gave you that staff, didn''t he?  Did he also tell you he''s the one that sold out his tribe?  No matter, you will both pay for this!',14,0,100,0,0,0,0,18349,0,'Arzeth the Merciless');

-- 8719 Auctioneer Fitch, groups 0-2 (quest "Fencing the Goods" gossip chain: talk 0 -> text over -> 1 -> 2 + credit).
-- Source: Wowpedia "Fencing the Goods (Alliance)" (MediaWiki API, 2026-10-28) gives the three lines in this order;
--   broadcast_text 52638/52639/52640. The DB had only group 0 = 52640 (the last line).
DELETE FROM `creature_text` WHERE `CreatureID`=8719 AND `GroupID` IN (0,1,2);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(8719,0,0,'Look, in my business, discretion is important. I shouldn''t be talking about this at all.',12,0,100,0,0,0,0,52638,0,'Auctioneer Fitch'),
(8719,1,0,'The person you described was just here and he tried to persuade me to list a crateful of arcane objects.',12,0,100,0,0,0,0,52639,0,'Auctioneer Fitch'),
(8719,2,0,'Naturally, I told him that we don''t deal in stolen goods. This is the Stormwind auction house, not some smoke-filled, back alley swapmeet!',12,0,100,5,0,0,0,52640,0,'Auctioneer Fitch');

-- 45152 Magus Bisp, group 0 (gossip -> yell -> turns hostile; quest 27154 "Bagging Bisp").
-- Source: Wowpedia "Bagging Bisp" (MediaWiki API, 2026-10-28): yell "You'll pay with your life ..."; broadcast_text 45301 (EmoteID1 5).
DELETE FROM `creature_text` WHERE `CreatureID`=45152 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45152,0,0,'You''ll pay with your life for interfering with my plans, you miserable scum!',14,0,100,5,0,0,0,45301,0,'Magus Bisp');

-- 46425 Ramkahen Prisoner, group 2 (linked after group 1 "Freedom at last ... Caimas took him").
-- Source: broadcast_text 46522 is the next line of the same chain as 46520/46521 (groups 0/1 already in the DB)
--   and names Caimas (the warlord of group 1); Wowpedia has no quote list for this NPC.
--   Group 3 is still unknown everywhere (see report).
DELETE FROM `creature_text` WHERE `CreatureID`=46425 AND `GroupID`=2;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(46425,2,0,'I must warn you.  Caimas does horrible things to prisoners... he forces them to fight his pet beasts.  Prepare for the worst, friend.',12,0,100,1,0,0,0,46522,0,'Ramkahen Prisoner');

-- 54924 Zhi-Zhi, group 1 ("Talk Final" after the duel is won).
-- Source: Wowpedia "Zhi-Zhi" Quotes (MediaWiki API, 2026-10-28): say "Alright, enough, enough! STOP HITTING ZHI-ZHI!"; broadcast_text 53204.
DELETE FROM `creature_text` WHERE `CreatureID`=54924 AND `GroupID`=1;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(54924,1,0,'Alright, enough, enough! STOP HITTING ZHI-ZHI!',12,0,100,1,0,0,0,53204,0,'Zhi-Zhi');

-- 55488 Corastrasza, group 0 (after gossip option, then casts 104569).
-- Source: Wowpedia "Corastrasza" Quotes (MediaWiki API, 2026-10-28): say "Here's what's left of the message we found, <name> ...";
--   broadcast_text 54349 (female text column).
DELETE FROM `creature_text` WHERE `CreatureID`=55488 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(55488,0,0,'Here''s what''s left of the message we found, $n. Can you make any sense of it?',12,0,100,0,0,0,0,54349,0,'Corastrasza');

-- 69305 Sunreaver Scout / 69267 Silver Covenant Scout, group 0 (freed in daily "Soul Surrender").
-- Source: Wowpedia "Soul Surrender (Horde)" and "Soul Surrender (Alliance)" (MediaWiki API, 2026-10-28);
--   broadcast_text 71507-71532. Only the thank-you lines a scout says to its own faction are ported to group 0;
--   the hostile lines it says to the other faction (71517-71520, 71523-71526) need a team split in the SmartAI (see report).
DELETE FROM `creature_text` WHERE `CreatureID` IN (69305,69267) AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(69305,0,0,'Thank you, friend. You have rescued me from a terrible fate.',12,0,100,0,0,0,0,71507,0,'Sunreaver Scout'),
(69305,0,1,'Did the Sunreavers send you? Thank you for the well-timed rescue.',12,0,100,0,0,0,0,71508,0,'Sunreaver Scout'),
(69305,0,2,'Thank you, champion. Please, free the others as well!',12,0,100,0,0,0,0,71510,0,'Sunreaver Scout'),
(69305,0,3,'How could the mogu be capable of such atrocity? Thank goodness you arrived when you did.',12,0,100,0,0,0,0,71511,0,'Sunreaver Scout'),
(69305,0,4,'You have saved my life, champion. For that, I am in your debt.',12,0,100,0,0,0,0,71513,0,'Sunreaver Scout'),
(69305,0,5,'I knew the Sunreavers would not desert me. Thank you, champion.',12,0,100,0,0,0,0,71516,0,'Sunreaver Scout'),
(69267,0,0,'Did the Silver Covenant send you? Thank you for the well-timed rescue.',12,0,100,0,0,0,0,71514,0,'Silver Covenant Scout'),
(69267,0,1,'I knew the Silver Covenant would not desert me. Thank you, champion.',12,0,100,0,0,0,0,71515,0,'Silver Covenant Scout'),
(69267,0,2,'Thank you, champion. Please, free the others as well!',12,0,100,0,0,0,0,71528,0,'Silver Covenant Scout'),
(69267,0,3,'How could the mogu be capable of such atrocity? Thank goodness you arrived when you did.',12,0,100,0,0,0,0,71529,0,'Silver Covenant Scout'),
(69267,0,4,'Thank you, friend. You have rescued me from a terrible fate.',12,0,100,0,0,0,0,71531,0,'Silver Covenant Scout'),
(69267,0,5,'You have saved my life, champion. For that, I am in your debt.',12,0,100,0,0,0,0,71532,0,'Silver Covenant Scout');

-- 25730 En'kilah Necrolord, groups 0-7: its SmartAI turns it into High Overlord Saurfang (action 36 -> 25751) and then
-- talks; at runtime CreatureTextMgr reads entry 25751 (whose 8 groups exist), but SmartAIMgr::IsTextValid in this core
-- checks the script owner's entry (25730) at load and drops those 8 talk rows ("using non-existent Text id 0..7").
-- Mirror 25751's rows under 25730 (same BroadcastTextIds) so the rows load. The same rows sit under 25751 in
-- TrinityCore TDB 335.26091 smart_scripts (entry 25751) and under 25730 in SkyFire SFDB 26.002 / the reference DB.
DELETE FROM `creature_text` WHERE `CreatureID`=25730 AND `GroupID` BETWEEN 0 AND 7;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`)
SELECT 25730,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,'En''kilah Necrolord (as High Overlord Saurfang 25751)'
FROM `creature_text` WHERE `CreatureID`=25751 AND `GroupID` BETWEEN 0 AND 7;
