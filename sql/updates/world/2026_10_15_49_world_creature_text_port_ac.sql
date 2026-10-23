-- Refs #183 #186 (creature_text rows the scripts talk with but the table lacks)
-- ERROR CLASSES:
--  (a) boot: "SmartAIMgr: Entry N SourceType 0 Event 0 Action 1 using non-existent Text id G, skipped." (SmartScriptMgr.cpp:1306) - the whole Talk row is skipped, so
--      the NPC stays silent: 13 lines for 2719 Dustbelcher Lord, 13601 Tinkerer Gizlock, 15324 Qiraji Gladiator, 24198 Plagued Dragonflayer Rune-Caster (AC rows),
--      14860 Flik g1, 59296, 59392, 60572 g0-4 (5 lines), 66693 (MoP rows).
--  (b) runtime: "CreatureTextMgr: Could not find Text for Creature(X) Entry N in 'creature_text' table. Ignoring." / "Could not find TextGroup G ..." (CreatureTextMgr.cpp:235):
--      123 lines in the test log for 29506 Orland Schaeffer (32), 32711 Warp-Huntress Kula (27), 32714 Moon Priestess Nici (30), 32718 Disidra Stormglory (23),
--      32720 Violetta (8), 32751 Backbiter (1), 28355 Wright Williams group 5 (2).
-- SOURCE OF TRUTH: AC WotLK creature_text (acore_world) for the WotLK/classic creatures - 17 rows copied 1:1 (columns map 1:1, SoundType=0);
--      MoP 5.4.8 creature_text for the MoP/Cata-sniffed creatures - 16 rows (text/type/language/probability/emote/duration/sound/BroadcastTextId/TextRange copied).
--      All BroadcastTextId values used exist in LoA broadcast_text (checked). INSERT IGNORE: re-runnable, never overwrites a LoA row.
-- NOT RESOLVABLE here (no AC/MoP rows): see the report - 19477 Fleeing Dreghood Warrior group 1 (169 lines; AC/LoA only have group 0), 32709/32710/32713/32702 (Argent-Tournament-style
--      Dalaran champions, no rows in AC either), 55659 Wild Imp, 30194/30196 Ebon Blade Prisoner, 27160 7th Legion Infantryman, ~20 one-line NPCs, and 49 more SmartAI talk rows.
-- VERIFY: SELECT CreatureID,GroupID,COUNT(*) FROM creature_text WHERE CreatureID IN (2719,13601,15324,24198,28355,29506,32711,32714,32718,32720,32751,14860,59296,59392,60572,66693) GROUP BY 1,2;
-- EXPECTED DBErrors.log DROP: 13 boot lines + 123 runtime lines = 136.
-- UNDO: DELETE FROM creature_text WHERE comment LIKE '%[port: %creature_text]';  (the marker is appended to every inserted row's comment)
INSERT IGNORE INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(2719,0,0,'Raaar!!! Me smash $r!',12,0,100,0,0,0,0,1925,0,'Dustbelcher Lord [port: AC creature_text]'),
(2719,0,1,'Me smash! You die!',12,0,100,0,0,0,0,1926,0,'Dustbelcher Lord [port: AC creature_text]'),
(2719,0,2,'I''ll crush you!',12,0,100,0,0,0,0,1927,0,'Dustbelcher Lord [port: AC creature_text]'),
(13601,0,0,'Mine! Mine! Mine!  Gizlock is the ruler of this domain!  You shall never reveal my presence!',14,0,100,0,0,0,0,8852,0,'Tinkerer Gizlock [port: AC creature_text]'),
(15324,0,0,'%s becomes enraged!',16,0,100,0,0,0,0,24144,0,'Qiraji Gladiator [port: AC creature_text]'),
(24198,0,0,'%s loses all self control and begins to attack friend or foe alike!',16,0,100,0,0,0,0,0,0,'Plagued Dragonflayer Handler [port: AC creature_text]'),
(28355,5,0,'Greetings.',12,7,100,3,0,0,0,43337,0,'Wright Williams - vendor greeting (AC group 5) [port: AC creature_text]'),
(28355,5,1,'Greetings, $c.',12,7,100,3,0,0,0,43330,0,'Wright Williams - vendor greeting (AC group 5) [port: AC creature_text]'),
(28355,5,2,'Greetings! Please have a look around.',12,7,100,3,0,0,0,43333,0,'Wright Williams - vendor greeting (AC group 5) [port: AC creature_text]'),
(28355,5,3,'Let me know if you need help finding anything, $c.',12,7,100,3,0,0,0,43335,0,'Wright Williams - vendor greeting (AC group 5) [port: AC creature_text]'),
(28355,5,4,'Welcome. May I help you find something?',12,7,100,3,0,0,0,43336,0,'Wright Williams - vendor greeting (AC group 5) [port: AC creature_text]'),
(29506,0,0,'Welcome traveler!',12,0,100,0,0,0,0,0,0,'Orland Schaeffer [port: AC creature_text]'),
(32711,0,0,'So peaceful...',12,0,100,0,0,0,0,0,0,'Warp-Huntress Kula [port: AC creature_text]'),
(32714,0,0,'Elune bless you, champion.',12,0,100,0,0,0,0,0,0,'Dalaran priest [port: AC creature_text]'),
(32718,0,0,'Be well, champion.',12,0,50,0,0,0,0,0,0,'Dalaran shamy [port: AC creature_text]'),
(32720,0,0,'Good to see you, champion.',12,0,50,0,0,0,0,0,0,'Dalaran mage [port: AC creature_text]'),
(32751,0,0,'...',12,0,0,0,0,0,0,11044,0,' [port: AC creature_text]'),
(14860,1,0,'Get back here!',12,0,100,0,0,0,0,10147,0,'Flik [port: creature_text]'),
(14860,1,1,'Hey, someone help me catch this thing!',12,0,100,0,0,0,0,10145,0,'Flik [port: creature_text]'),
(14860,1,2,'It''s getting away!',12,0,100,0,0,0,0,10146,0,'Flik [port: creature_text]'),
(59296,0,0,'Hozen whakkin'' stinky wikket!',12,0,0,0,0,0,0,0,0,' [port: creature_text]'),
(59296,0,1,'Wikkets is furry food!',12,0,0,0,0,0,0,0,0,' [port: creature_text]'),
(59296,0,2,'Grookie wikket!',12,0,0,0,0,0,0,0,0,' [port: creature_text]'),
(59296,0,3,'Ookin'' dooker!',12,0,0,0,0,0,0,0,0,' [port: creature_text]'),
(59392,0,0,'It''s about time we got another shipment. I heard they were having trouble at the mines, but our work cannot wait.',12,0,0,0,0,0,0,0,0,'Love''s Labor [port: creature_text]'),
(60572,0,0,'You can''t stop what has begun!',14,0,100,0,0,0,0,0,0,'Say Summon Guardian 0 [port: creature_text]'),
(60572,1,0,'Can you feel it? Da Thunder is getting closer...',12,0,100,16,0,0,0,0,0,'Say Summon Guardian 1 [port: creature_text]'),
(60572,2,0,'Da Thunder King will return!',14,0,100,16,0,0,0,0,0,'Say Summon Guardian 2 [port: creature_text]'),
(60572,3,0,'Enough! I''ll handle you myself!',14,0,100,0,0,0,0,0,0,'Say Engage Combat [port: creature_text]'),
(60572,4,0,'My death will not stop da storm that is coming! Da Thunder King will rise!',14,0,100,0,0,0,0,0,0,'Say Death [port: creature_text]'),
(66693,0,0,'Keep your flea ridden paws out of the food, you mangy beasts!',14,0,0,0,0,0,0,0,0,'Zandalari Overlord [port: creature_text]'),
(66693,0,1,'Work harder you lazy mongrels, or I''ll roast you alive!',14,0,0,0,0,0,0,0,0,'Zandalari Overlord [port: creature_text]'),
(66693,0,2,'Guard these supplies with your lives, you filthy mutts!',14,0,0,0,0,0,0,0,0,'Zandalari Overlord [port: creature_text]');
