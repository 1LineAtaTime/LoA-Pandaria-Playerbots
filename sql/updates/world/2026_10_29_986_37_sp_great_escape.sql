-- ported from PR 15 2026_10_27_51_world.sql (#986)
-- The Great Escape (27438): nothing cast Quest Credit: The Great Escape (85091), the quest's only way to be completed.
-- Accepting it from Deathstalker Commander Belmont now casts The Great Escape Initialize (89225) -> Flight of the Val'kyr (85073):
-- Arthura (45555, Vehicle 1119) takes the player and Lord Godfrey's corpse (45556), Belmont sees them off and she flies them over
-- the Greymane Wall to the Forsaken Front, where the player is credited and set down beside Sylvanas.
-- Talking to Belmont again restarts an escape that was cut short; reaching Sylvanas 45525 with the quest still open completes it.
INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES (27438);
UPDATE `quest_template_addon` SET `SourceSpellID` = 89225 WHERE `ID` = 27438;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (89225, 85073, 85079);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(89225, 'spell_silverpine_the_great_escape_initialize'),
(85073, 'spell_silverpine_flight_of_the_valkyr'),
(85079, 'spell_silverpine_summon_belmont_great_escape');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_belmont_great_escape' WHERE `entry` IN (45312, 45474);
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_arthura_great_escape' WHERE `entry` = 45555;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_sylvanas_forsaken_front' WHERE `entry` = 45525;

-- Arthura hovers; Lord Godfrey's corpse is dead, as his body at the Forsaken Front (45576)
DELETE FROM `creature_template_addon` WHERE `entry` IN (45555, 45556);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(45555, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(45556, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '29266');

-- The farewell (broadcast_text 45701-45707; 45705 belongs to someone else)
DELETE FROM `creature_text` WHERE `CreatureID` IN (45555, 45557, 45312, 45474);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45555, 0, 0, 'What about you, Belmont? How will you get back?', 12, 0, 100, 0, 0, 0, 0, 45701, 5, 'Arthura - The Great Escape'),
(45555, 1, 0, 'So be it. Farewell, Belmont.', 12, 0, 100, 0, 0, 0, 0, 45706, 5, 'Arthura - The Great Escape'),
(45555, 2, 0, 'Oh my... hang on tight!', 12, 0, 100, 0, 0, 0, 0, 45707, 5, 'Arthura - The Great Escape'),
(45557, 0, 0, 'I''ll be fine, Arthura. The worgen lost our scent. It''s doubtful that they even know we''re here.', 12, 0, 100, 0, 0, 0, 0, 45702, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45557, 1, 0, 'I''ll probably make it back to the Forsaken Front before you two, er, three.', 12, 0, 100, 0, 0, 0, 0, 45703, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45557, 2, 0, 'Now move out! You must not keep the Banshee Queen waiting.', 12, 0, 100, 0, 0, 0, 0, 45704, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45312, 0, 0, 'I''ll be fine, Arthura. The worgen lost our scent. It''s doubtful that they even know we''re here.', 12, 0, 100, 0, 0, 0, 0, 45702, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45312, 1, 0, 'I''ll probably make it back to the Forsaken Front before you two, er, three.', 12, 0, 100, 0, 0, 0, 0, 45703, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45312, 2, 0, 'Now move out! You must not keep the Banshee Queen waiting.', 12, 0, 100, 0, 0, 0, 0, 45704, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45474, 0, 0, 'I''ll be fine, Arthura. The worgen lost our scent. It''s doubtful that they even know we''re here.', 12, 0, 100, 0, 0, 0, 0, 45702, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45474, 1, 0, 'I''ll probably make it back to the Forsaken Front before you two, er, three.', 12, 0, 100, 0, 0, 0, 0, 45703, 5, 'Deathstalker Commander Belmont - The Great Escape'),
(45474, 2, 0, 'Now move out! You must not keep the Banshee Queen waiting.', 12, 0, 100, 0, 0, 0, 0, 45704, 5, 'Deathstalker Commander Belmont - The Great Escape');
