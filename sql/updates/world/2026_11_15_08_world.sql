-- Light's Hope Chapel: Rohan the Assassin's and Huntsman Leopold's armor stories, Rayne's last story page and Scarlet
-- Commander Marjhan's greeting.
--
-- Rohan the Assassin 16131 (menu 7101) and Huntsman Leopold 16132 (menu 7102) had only their "Where can I find ..." option.
-- Both TDBs (4.4.2 and 12.1, identical here) give each a story option first, with menus that are not in this database:
--   7101 Rohan: 0 "What is it that you do exactly, Rohan?" -> 7118 (8380) -> "So what brings you to Light's Hope?" -> 7119
--        (8381) -> "What? Bonescythe?" -> 7120 (8382) -> "Wow, you're insane, aren't you?" -> 7121 (8383) -> "Hey wait,
--        Gadgetzan has a disco?" -> 7122 (8384, end); 1 "Where can I find bone fragments?" -> 7145 (8409, ours already)
--   7102 Leopold: 0 "What is Cryptstalker armor?" -> 7111 (8369) -> "Continue please." -> 7112 (8370, end); 1 "Where can I
--        find crypt fiend parts, Leopold?" -> 7144 (8408, ours already)
-- Rayne 16135: her story stopped at 7124; both TDBs go on with "What kind of services?" -> 7125 (8387, end).
-- Scarlet Commander Marjhan 16114: npc_text 8354 had BroadcastTextID0 0, so the server sent broadcast text 1 ("Help help! I'm
--   being repressed!") as her greeting; both TDBs give 8354 broadcast text 11944.
--
-- Sourced: every menu, text and option above from both TDBs (no conditions on any of them in either). Each npc_text row gets
--   its words, language and emotes from our broadcast_text row of the TDB's BroadcastTextID (all eight exist, wording as in
--   5.4.8), filled as Archmage Angela Dosantos' story texts 8388-8397 are; the option texts are the broadcast texts' words.
-- Ours: the options' OptionType 1 / OptionNpcflag 1 / OptionIcon 0 as on the other story options of these NPCs. The
--   existing "Where can I find" rows of 7101/7102 move from OptionID 0 to 1 with their targets 7145/7144 unchanged, and so
--   do their koKR translations (nothing else names these options: no conditions, no SmartAI). 8354 text0_1 is replaced by
--   broadcast text 11944's words; the old text differed from them only in its end ("$B" instead of a line break).

-- npc_text of the new story pages
DELETE FROM `npc_text` WHERE `ID` IN (8369, 8370, 8380, 8381, 8382, 8383, 8384, 8387);
INSERT INTO `npc_text` (`ID`,`text0_0`,`text0_1`,`BroadcastTextID0`,`lang0`,`Probability0`,`EmoteDelay0_0`,`Emote0_0`,`EmoteDelay0_1`,`Emote0_1`,`EmoteDelay0_2`,`Emote0_2`,`VerifiedBuild`) VALUES
(8369,'The preternatural ability of the crypt fiend to hunt and kill its foe is one studied and vaunted by men in my profession. I, myself, have slain thousands of the beasts in my crusade to rid this world of Scourge.$B$BWhile I loathe the crypt fiend for its merciless nature, its abilities must be admired. It has a job and it does it - very well.','',11979,0,1,1,1,1,1,1,1,0),
(8370,'I have discovered a use for the carapace of the crypt fiend and its various body parts. I am able to fashion an extremely light and very deadly set of armor by combining the fragments with some of the desecrated magical armors lost in Naxxramas.','',11981,0,1,0,1,0,1,0,0,0),
(8380,'Ah, as the title states I am an assassin for the Scarlet Crusade. I spend my days exterminating high priority targets and nights at the Gadgetzan discotheque. Dancing is my true calling. Assassination just pays the bills.','',12004,0,1,0,1,0,1,0,1,0),
(8381,'Good question, $r. In my younger days I was footloose and fancy free without a care in the world. In those days I would sing in a barber shop quartet out of Stormwind known as the Four Dons. That\'s where I learned how to craft the Bonescythe digs.','',12006,0,1,1,273,1,1,1,1,0),
(8382,'Yes, Bonescythe. You see, it was our tenor, an ornery fellow known as Don Julio, who got me started on the whole thing. In his travels throughout Lordaeron, Don Julio picked up many interesting skills, one of them being the ability to take the bones of fallen Scourge and fashion them into a lethal set of leather armor - perfectly suited for an assassin.','',12008,0,1,1,1,1,1,1,25,0),
(8383,'I\'m as sane as the next guy, $g fella:lady;. Believe what you will, but I AM capable of fashioning this armor. If you\'re interested, maybe we can make a deal. The sooner I can get out of the assassination business, the better.','',12010,0,1,1,1,1,1,0,0,0),
(8384,'Members only, scrub!\n','',12012,0,1,1,273,1,25,0,0,0),
(8387,'','I am a crafter of some renown, $c. With the proper materials, I am able to craft an armor known as Dreamwalker. Druids - and even some shaman - are drawn to this armor due to its magical properties. Are you interested?',12020,0,1,1,1,1,1,1,6,0);

-- The story menus
DELETE FROM `gossip_menu` WHERE `MenuID` IN (7111, 7112, 7118, 7119, 7120, 7121, 7122, 7125);
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES
(7111,8369,0),
(7112,8370,0),
(7118,8380,0),
(7119,8381,0),
(7120,8382,0),
(7121,8383,0),
(7122,8384,0),
(7125,8387,0);

-- Options: 7101/7102 in the TDB order, and the steps of the stories
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (7101, 7102) AND `OptionID` IN (0, 1);
DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (7111, 7118, 7119, 7120, 7121, 7124) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(7101,0,0,'What is it that you do exactly, Rohan?',12003,1,1,7118,0,0,0,NULL,0,0),
(7101,1,0,'Where can I find bone fragments?',12077,1,1,7145,0,0,0,NULL,0,0),
(7102,0,0,'What is Cryptstalker armor?',11978,1,1,7111,0,0,0,NULL,0,0),
(7102,1,0,'Where can I find crypt fiend parts, Leopold?',12075,1,1,7144,0,0,0,NULL,0,0),
(7111,0,0,'Continue please.',11980,1,1,7112,0,0,0,NULL,0,0),
(7118,0,0,'So what brings you to Light\'s Hope?',12005,1,1,7119,0,0,0,NULL,0,0),
(7119,0,0,'What? Bonescythe?',12007,1,1,7120,0,0,0,NULL,0,0),
(7120,0,0,'Wow, you\'re insane, aren\'t you?',12009,1,1,7121,0,0,0,NULL,0,0),
(7121,0,0,'Hey wait, Gadgetzan has a disco?',12011,1,1,7122,0,0,0,NULL,0,0),
(7124,0,0,'What kind of services?',12019,1,1,7125,0,0,0,NULL,0,0);

-- The "Where can I find" translations follow their option to OptionID 1 (the options show their broadcast text's
-- translation, so these rows are a fallback only)
DELETE FROM `gossip_menu_option_locale` WHERE `MenuID` IN (7101, 7102) AND `OptionID` IN (0, 1) AND `Locale` = 'koKR';
INSERT INTO `gossip_menu_option_locale` (`MenuID`,`OptionID`,`Locale`,`OptionText`,`BoxText`) VALUES
(7101,1,'koKR','어디로 가면 해골 조각을 찾을 수 있나요?',NULL),
(7102,1,'koKR','어디로 가면 지하마귀 발톱을 찾을 수 있나요, 레오폴드?',NULL);

-- Scarlet Commander Marjhan's greeting
UPDATE `npc_text` SET `text0_0` = '', `text0_1` = 'The Argent Dawn might be impotent idealists but their delusional leader knows that they are outmatched. I have been sent here as an emissary of the Scarlet Crusade. An ambassador, if you will... It is doubtful that we could even dent the armies of Kel\'Thuzad - united or not - but we might be able to contain them long enough for some sort of miracle to happen.$B$BAs for me? I\'ll die in battle...$B$B<Marjhan shrugs.>\n',
  `BroadcastTextID0` = 11944, `lang0` = 0, `Probability0` = 1,
  `EmoteDelay0_0` = 0, `Emote0_0` = 1, `EmoteDelay0_1` = 0, `Emote0_1` = 1, `EmoteDelay0_2` = 0, `Emote0_2` = 0
WHERE `ID` = 8354;
