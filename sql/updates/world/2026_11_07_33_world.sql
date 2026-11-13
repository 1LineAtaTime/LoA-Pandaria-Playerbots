-- Infiltration 28603: Instructor Rufus 48922 and Drill Sergeant Magnus 48923 can be insulted into a fight, as the quest's "coerce them to attack
-- you" asks. A Wowhead comment on 28603: "if you just walk up to them to talk you'll insult them or something and they will attack you ...
-- Even as a SI 7". While disguised (Stormpike Guard forced friendly) nothing else can start the fight.
-- Captain Crudbeard 48924 keeps faction 2261 (faction 469; mop, both TDBs and TCPP), which the force reaction does not cover: he attacks on sight,
-- disguise or not (comments on 28603 "remains aggressive" and 28556 "he attacks you in disguise"), so he has no insult option or fight script.
-- His insult 49165 and replies 49167-49168 stay unused. He keeps the menu Alliance characters can read ("he's not exactly friendly even to Alliance
-- players", comment on 48924).
-- Sourced:
--  * The menus and texts: TDB 442 gossip_menu 12472-12474 -> npc_text 17546-17548 -> broadcast 49158/49162/49166.
--  * The options and replies: broadcast 49159/49163 and 49160-49161/49164 (the wiki quotes the menu texts). The option <-> reply pairing is by id
--    order and content. npcflag 1: both TDBs and TCPP. Magnus' emote: broadcast 49164.
-- Ours: the option and creature_text rows (no TDB has them), the mask 8 gate (in progress only), faction 14 for the fight, restored on reset.
-- Starter rows: in both TDBs the auto-accept quests list their own kill objectives as extra starters (28566: 48878, 48880; 28605: 49116; 28603:
-- 48922-48924), but none of those templates is a questgiver there and Wowhead lists none: a sniff artifact, which mop kept only for
-- 28603 (with npcflag 3). 48878, 48880 and 49116 already have no starter rows and npcflag 0. Melisara keeps 28603.
DELETE FROM `creature_queststarter` WHERE `quest`=28603 AND `id` IN (48922,48923,48924);
UPDATE `creature_template` SET `npcflag`=1, `gossip_menu_id`=12472 WHERE `entry`=48922;
UPDATE `creature_template` SET `npcflag`=1, `gossip_menu_id`=12473 WHERE `entry`=48923;
UPDATE `creature_template` SET `npcflag`=1, `gossip_menu_id`=12474 WHERE `entry`=48924;

DELETE FROM `npc_text` WHERE `ID` IN (17546,17547,17548);
INSERT INTO `npc_text` (`ID`,`text0_0`,`BroadcastTextID0`,`Probability0`) VALUES
(17546,'OY! Piss off, eh?',49158,1),
(17547,'I''m busy, lad. Why don''t ye go find some Forsaken goo and take a bath, eh?',49162,1),
(17548,'Leave or I''ll be forced to pound yer face in!',49166,1);

DELETE FROM `gossip_menu` WHERE `MenuID` IN (12472,12473,12474);
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES
(12472,17546,0),
(12473,17547,0),
(12474,17548,0);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (12472,12473,12474);
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`VerifiedBuild`) VALUES
(12472,0,0,'Dirty, stinkin'' Wildhammer. You know these dwarves only consider you as half a dwarf, right? What does that make you? A gnome?',49159,1,1,0),
(12473,0,0,'I guess it''s true then. I didn''t want to believe that you were a yellow-bellied snow licker, but there''s no doubt about it now.',49163,1,1,0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (12472,12473,12474);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,12472,0,0,0,47,0,28603,8,0,0,0,0,'','Instructor Rufus insult - Infiltration in progress'),
(15,12473,0,0,0,47,0,28603,8,0,0,0,0,'','Drill Sergeant Magnus insult - Infiltration in progress');

DELETE FROM `creature_text` WHERE `CreatureID` IN (48922,48923);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48922,0,0,'What did ye say, Sally?! The only one of us who resembles a gnome is you - after I''m done cuttin'' you into wee pieces!',12,0,100,0,0,0,0,49160,0,'Instructor Rufus - insulted'),
(48922,1,0,'Come on then, Nancy! Let''s go!',12,0,100,0,0,0,0,49161,0,'Instructor Rufus - insulted'),
(48923,0,0,'What did ye call me? I''ll show ye what a "snow licker" can do!',12,0,100,1,0,0,0,49164,0,'Drill Sergeant Magnus - insulted');

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (48922,48923) AND `id` BETWEEN 10 AND 15;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48922,0,10,11,62,0,100,0,12472,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Instructor Rufus - On Gossip Select - Close Gossip'),
(48922,0,11,12,61,0,100,0,0,0,0,0,0,1,0,4000,0,0,0,0,7,0,0,0,0,0,0,0,0,'Instructor Rufus - Linked - Say Line 0'),
(48922,0,12,13,61,0,100,0,0,0,0,0,0,2,14,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Instructor Rufus - Linked - Set Hostile'),
(48922,0,13,0,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Instructor Rufus - Linked - Attack'),
(48922,0,14,0,52,0,100,0,0,48922,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Instructor Rufus - On Text Over 0 - Say Line 1'),
(48922,0,15,0,25,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Instructor Rufus - On Reset - Restore Faction'),
(48923,0,10,11,62,0,100,0,12473,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Drill Sergeant Magnus - On Gossip Select - Close Gossip'),
(48923,0,11,12,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Drill Sergeant Magnus - Linked - Say Line 0'),
(48923,0,12,13,61,0,100,0,0,0,0,0,0,2,14,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Drill Sergeant Magnus - Linked - Set Hostile'),
(48923,0,13,0,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Drill Sergeant Magnus - Linked - Attack'),
(48923,0,15,0,25,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Drill Sergeant Magnus - On Reset - Restore Faction');
