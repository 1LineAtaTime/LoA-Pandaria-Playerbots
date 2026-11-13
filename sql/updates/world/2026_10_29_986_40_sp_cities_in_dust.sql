-- ported from PR 15 2026_10_27_54_world.sql (#986)
-- Cities in Dust (27601): its objective, Cities in Dust Quest Credit (46019), is given only by the spell Cities in Dust (85952),
-- which nothing cast, and Sylvanas at the Forsaken Front (45617) had no gossip option to start the march.
-- Her option (the text of broadcast_text 46257, given without its id because the text has only a female form; shown while
-- the quest is in progress) casts Summon Sylvanas (85864): her actor 46026 brings Cromush
-- and the three val'kyr, sings Song of Sylvanas and marches with the player across the Battlefront to the Greymane Wall, where
-- the finale of broadcast_text 46139-46227 plays (Crowley and Bloodfang, Godfrey with Lorna, Walden and Ashbury, all the
-- player's personal summons) and the player is credited. Speakers as on warcraft.wiki.gg.
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11823 AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11823, 0, 0, 'Accompany Lady Sylvanas Windrunner across the Battlefront.', 0, 1, 1, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11823 AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11823, 0, 0, 0, 47, 0, 27601, 8, 0, 0, 0, 0, '', 'Lady Sylvanas Windrunner - show the march gossip option while Cities in Dust is in progress');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_sylvanas_forsaken_front_finale' WHERE `entry` = 45617;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_sylvanas_cities_in_dust' WHERE `entry` = 46026;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_cities_in_dust_actor' WHERE `entry` IN (46027, 46028, 46029, 46030, 46031, 46032, 46033, 46034, 46035, 46036);
-- Walden's combat SmartAI: he is only an actor of the finale
DELETE FROM `smart_scripts` WHERE `entryorguid` = 46029 AND `source_type` = 0 AND `id` IN (0, 1, 2);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (85877, 85878, 85925, 85926, 85927, 85944, 85950);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(85877, 'spell_silverpine_cities_in_dust_summon'),
(85878, 'spell_silverpine_cities_in_dust_summon'),
(85925, 'spell_silverpine_cities_in_dust_summon'),
(85926, 'spell_silverpine_cities_in_dust_summon'),
(85927, 'spell_silverpine_cities_in_dust_summon'),
(85944, 'spell_silverpine_execution'),
(85950, 'spell_silverpine_raise_forsaken_actors');

-- Where Crowley and Bloodfang come out of Gilneas and Godfrey's men come over the wall. No sniff has these spells' positions:
-- Godfrey and Walden use the sniffed spots of Lorna and Walden (static spawns 373753 and 373754, deleted below), the others are ours.
DELETE FROM `spell_target_position` WHERE `ID` IN (85877, 85878, 85925, 85926, 85927);
INSERT INTO `spell_target_position` (`ID`, `effIndex`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`, `target_orientation`) VALUES
(85877, 0, 0, -760, 1550, 28.5, 0),
(85878, 0, 0, -762, 1541, 28.5, 0),
(85925, 0, 0, -809.41, 1544.53, 28.0109, 6.19401),
(85926, 0, 0, -775.669, 1529.1, 28.8968, 1.77121),
(85927, 0, 0, -805, 1537, 28.2, 0);

-- The val'kyr hover
DELETE FROM `creature_template_addon` WHERE `entry` IN (46032, 46033, 46034);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(46032, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(46033, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(46034, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL);

-- Static spawns of the finale's summons: Ashbury 46030, Lorna 46027, Walden 46029, Crowley 46036
DELETE FROM `creature_addon` WHERE `guid` IN (373751, 373753, 373754, 373755);
DELETE FROM `creature` WHERE `guid` IN (373751, 373753, 373754, 373755) AND `id` IN (46027, 46029, 46030, 46036);

DELETE FROM `creature_text` WHERE `CreatureID` IN (46026, 46028, 46031, 46032, 46033, 46034, 46035, 46036);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(46026, 0, 0, 'Onward, $n! We will beat back the Alliance dogs and secure Lordaeron for the Forsaken! For the Horde!', 12, 0, 100, 0, 0, 0, 0, 46139, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 1, 0, 'I will leave your cities in dust and your lives in ruin!', 14, 0, 100, 0, 0, 0, 0, 46141, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 2, 0, 'Lay down your arms and surrender!', 14, 0, 100, 0, 0, 0, 0, 46142, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 3, 0, 'Look at them, $p. They scurry like rats, veering headlong to their doom. Surely Crowley and Bloodfang can see the futility in this!', 12, 0, 100, 0, 0, 0, 0, 46143, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 4, 0, 'To the Greymane Wall! We will force their hand.', 12, 0, 100, 0, 0, 0, 0, 46144, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 5, 0, 'Crowley, your forces have fallen before my mighty army. You have lost!', 14, 0, 100, 0, 0, 0, 0, 46188, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 6, 0, 'You frivolously throw away the lives of your people while your own king sits atop his throne of lies, nary lifting a finger to help.', 12, 0, 100, 0, 0, 0, 0, 46190, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 7, 0, 'Is Gilneas worth the lives that have been lost? The lives that will be lost? You cannot win.', 12, 0, 100, 0, 0, 0, 0, 46191, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 8, 0, 'And your daughter? You could have saved her... You could have offered her your blood, yet you did not. Why?', 12, 0, 100, 0, 0, 0, 0, 46194, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 9, 0, 'Nothing, yet...', 12, 0, 100, 0, 0, 0, 0, 46196, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 10, 0, 'I now present you with a choice - a choice that I was never given.', 12, 0, 100, 0, 0, 0, 0, 46197, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 11, 0, 'I offer you the life of Lorna for your unconditional surrender.', 12, 0, 100, 0, 0, 0, 0, 46198, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 12, 0, 'Choose your next words wisely, Crowley. Deny me and she will serve me in undeath - forever.', 12, 0, 100, 0, 0, 0, 0, 46199, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 13, 0, 'Bring her, Godfrey!', 14, 0, 100, 0, 0, 0, 0, 46200, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 14, 0, 'Release her, Godfrey.', 12, 0, 100, 0, 0, 0, 0, 46206, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 15, 0, 'Now leave here, Crowley, and never return.', 12, 0, 100, 0, 0, 0, 0, 46208, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 16, 0, 'Lordaeron belongs to the Forsaken.', 12, 0, 100, 0, 0, 0, 0, 46209, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 17, 0, 'SOLDIERS OF THE HORDE! WE ARE VICTORIOUS! LORDAERON IS W...', 14, 0, 100, 0, 0, 0, 0, 46211, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 18, 0, '%s dies.', 16, 0, 100, 0, 0, 0, 0, 46212, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 19, 0, 'I saw... only darkness.', 12, 0, 100, 0, 0, 0, 0, 46224, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 20, 0, 'And as I drifted towards nothingness, a brilliant light appeared, then another, and another... My dear val''kyr.', 12, 0, 100, 0, 0, 0, 0, 46225, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 21, 0, 'I know now - beyond a shadow of a doubt - that the val''kyr are our future.', 12, 0, 100, 0, 0, 0, 0, 46226, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46026, 22, 0, 'We will never stop fighting for Lordaeron. Never...', 12, 0, 100, 0, 0, 0, 0, 46227, 5, 'Lady Sylvanas Windrunner - Cities in Dust'),
(46036, 0, 0, 'It''s not over, Sylvanas. Not yet.', 12, 0, 100, 0, 0, 0, 0, 46189, 5, 'Lord Darius Crowley - Cities in Dust'),
(46036, 1, 0, 'We will die trying!', 12, 0, 100, 0, 0, 0, 0, 46193, 5, 'Lord Darius Crowley - Cities in Dust'),
(46036, 2, 0, 'LORNA? What... Where is she!? What have you done to her!?', 12, 0, 100, 0, 0, 0, 0, 46195, 5, 'Lord Darius Crowley - Cities in Dust'),
(46036, 3, 0, 'GODFREY! You deceitful maggot!', 12, 0, 100, 0, 0, 0, 0, 46203, 5, 'Lord Darius Crowley - Cities in Dust'),
(46036, 4, 0, 'Lorna... I... Release her. I will sound the retreat.', 12, 0, 100, 0, 0, 0, 0, 46204, 5, 'Lord Darius Crowley - Cities in Dust'),
(46036, 5, 0, 'Forgive me, daughter...', 12, 0, 100, 0, 0, 0, 0, 46210, 5, 'Lord Darius Crowley - Cities in Dust'),
(46035, 0, 0, 'You can''t be serious, Crowley. You miserable bastard!', 12, 0, 100, 0, 0, 0, 0, 46205, 5, 'Packleader Ivar Bloodfang - Cities in Dust'),
(46028, 0, 0, 'Hello, old friend!', 12, 0, 100, 0, 0, 0, 0, 46202, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 1, 0, 'Of course, mistress.', 12, 0, 100, 0, 0, 0, 0, 46207, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 2, 0, 'Something that should have been done a long time ago, you filthy animal.', 12, 0, 100, 0, 0, 0, 0, 46214, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 3, 0, 'Gilneas belongs to me, and so soon will the rest of Lordaeron!', 12, 0, 100, 0, 0, 0, 0, 46215, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 4, 0, 'NOW JOIN YOUR MISTRESS IN DEATH!', 12, 0, 100, 0, 0, 0, 0, 46216, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 5, 0, 'FALL BACK, BROTHERS! TO SHADOWFANG!', 14, 0, 100, 0, 0, 0, 0, 46217, 5, 'Lord Godfrey - Cities in Dust'),
(46028, 6, 0, 'This land will quiver and shake in the wake of our destruction!', 14, 0, 100, 0, 0, 0, 0, 46218, 5, 'Lord Godfrey - Cities in Dust'),
(46031, 0, 0, 'WHAT HAVE YOU DONE, GODFREY!', 12, 0, 100, 0, 0, 0, 0, 46213, 5, 'High Warlord Cromush - Cities in Dust'),
(46031, 1, 0, 'Fix her... FIX HER!', 12, 0, 100, 0, 0, 0, 0, 46219, 5, 'High Warlord Cromush - Cities in Dust'),
(46034, 0, 0, 'We are bound to her, sisters...', 12, 0, 100, 0, 0, 0, 0, 46220, 5, 'Agatha - Cities in Dust'),
(46032, 0, 0, 'It will destroy us, sister...', 12, 0, 100, 0, 0, 0, 0, 46221, 5, 'Arthura - Cities in Dust'),
(46032, 1, 0, 'Then let it be done.', 12, 0, 100, 0, 0, 0, 0, 46223, 5, 'Arthura - Cities in Dust'),
(46033, 0, 0, 'It is our sacred duty. The pact was sealed, the bargain made...', 12, 0, 100, 0, 0, 0, 0, 46222, 5, 'Daschla - Cities in Dust');
