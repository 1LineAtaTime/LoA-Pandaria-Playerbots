-- ported from PR 15 2026_10_27_52_world.sql (#986)
-- Rise, Godfrey (27472): its objective, Lord Godfrey Reborn (45617), is credited only by Godfrey Kill Credit (85177), which nothing
-- cast. Accepting it now casts Summon Forsaken Front Actors (85149, removed by the quest's reward spell Cancel 85149): Sylvanas,
-- Cromush, the three val'kyr and the bodies of Godfrey, Walden and Ashbury appear as the player's own actors on the spots of the
-- camp around Godfrey's body (hidden while the quest is in progress). Arthura raises the three, they take their undead forms and
-- stand, the scene of broadcast_text 45740-45751 plays and Sylvanas casts Godfrey Kill Credit. The aura brings the actors back for
-- a scene that was cut short; spell_area gives it again on entering the Forsaken Front with the quest in progress.
INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES (27472);
UPDATE `quest_template_addon` SET `SourceSpellID` = 85149 WHERE `ID` = 27472;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (85149, 85197);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(85149, 'spell_silverpine_summon_forsaken_front_actors'),
(85149, 'spell_silverpine_summon_forsaken_front_actors_aura'),
(85197, 'spell_silverpine_raise_forsaken_actors');

DELETE FROM `spell_area` WHERE `spell` = 85149;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(85149, 5387, 27472, 0, 0, 0, 2, 1, 8, 11);

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_sylvanas_rise_godfrey' WHERE `entry` = 45605;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_forsaken_front_actor' WHERE `entry` IN (45606, 45607, 45608, 45609, 45610, 45611, 45612);

-- As the camp around Godfrey's body: Sylvanas on her horse, the three dead, the val'kyr hovering
DELETE FROM `creature_template_addon` WHERE `entry` IN (45605, 45606, 45607, 45608, 45609, 45610, 45611);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(45605, 0, 29257, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(45606, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '29266'),
(45607, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '29266'),
(45608, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '29266'),
(45609, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(45610, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(45611, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL);

-- Speakers as on warcraft.wiki.gg (Rise, Godfrey)
DELETE FROM `creature_text` WHERE `CreatureID` IN (45605, 45606, 45612);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45605, 0, 0, 'Rise, Lord Vincent Godfrey. Be reborn as an instrument of my vengeance!', 12, 0, 100, 0, 0, 0, 0, 45740, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45605, 1, 0, 'You live to serve me, Godfrey. Together we will crush the worgen uprising and retake Gilneas for the Forsaken.', 12, 0, 100, 0, 0, 0, 0, 45742, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45605, 2, 0, 'Yes, of course...', 12, 0, 100, 0, 0, 0, 0, 45744, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45605, 3, 0, 'Prepare your men. The Gilneas Liberation Front amasses at the Greymane Wall. Soon their reinforcements from Stormwind will arrive. We haven''t a minute to waste.', 12, 0, 100, 0, 0, 0, 0, 45748, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45605, 4, 0, 'Armor, weapons, whatever you may need, you will find at the supply depot behind me. Go now.', 12, 0, 100, 0, 0, 0, 0, 45750, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45605, 5, 0, 'Once again you have impressed me, $n. Without your aid, we would have not made it this far. Walk with me...', 12, 0, 100, 0, 0, 0, 0, 45751, 5, 'Lady Sylvanas Windrunner - Rise, Godfrey'),
(45606, 0, 0, 'I live again...', 12, 0, 100, 0, 0, 0, 0, 45741, 5, 'Lord Godfrey - Rise, Godfrey'),
(45606, 1, 0, 'The irony of what I have become is not lost upon me, Lady Sylvanas.', 12, 0, 100, 0, 0, 0, 0, 45745, 5, 'Lord Godfrey - Rise, Godfrey'),
(45606, 2, 0, 'Though as I have always said, better dead than worgen.', 12, 0, 100, 0, 0, 0, 0, 45746, 5, 'Lord Godfrey - Rise, Godfrey'),
(45606, 3, 0, 'Or in this case, better undead than worgen.', 12, 0, 100, 0, 0, 0, 0, 45747, 5, 'Lord Godfrey - Rise, Godfrey'),
(45606, 4, 0, 'As you wish, Banshee Queen.', 12, 0, 100, 0, 0, 0, 0, 45749, 5, 'Lord Godfrey - Rise, Godfrey'),
(45612, 0, 0, 'For the Horde...', 12, 0, 100, 0, 0, 0, 0, 45743, 5, 'High Warlord Cromush - Rise, Godfrey');
