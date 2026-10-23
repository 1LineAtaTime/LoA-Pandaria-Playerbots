-- Refs #498: zeppelin master arrival yells + arrival horn (minimal port of alexkulya b8e79c4; gossip classes NOT ported).
-- C++ side: src/server/scripts/World/zeppelins.cpp (AddSC_zeppelins) - ScriptNames below bind it to the 5 transports.
-- Undo:
--   UPDATE `gameobject_template` SET `ScriptName` = '' WHERE `entry` IN (164871,175080,176495,181689,186238);
--   DELETE FROM `creature_text` WHERE `CreatureID` IN (3150,12137,9566,26539,26540,3149,26538);
--   DELETE FROM `event_scripts` WHERE `id` IN (15312,15314,15318,15320,15322,15324,19126,19127,19137,19139,21868,21870) AND `command` = 16;
-- Pre-state (test world DB 2026-10-21): the 5 transports have ScriptName '' and AIName ''; none of the creature_text rows below
-- exist; no event_scripts rows exist for these ids. creature_text for 9564/12136/26537 (existing group 0) is kept as is.

UPDATE `gameobject_template` SET `ScriptName` = 'zeppelin_the_thundercaller' WHERE `entry` = 164871;
UPDATE `gameobject_template` SET `ScriptName` = 'zeppelin_the_iron_eagle' WHERE `entry` = 175080;
UPDATE `gameobject_template` SET `ScriptName` = 'zeppelin_the_purple_princess' WHERE `entry` = 176495;
UPDATE `gameobject_template` SET `ScriptName` = 'zeppelin_the_cloudkisser' WHERE `entry` = 181689;
UPDATE `gameobject_template` SET `ScriptName` = 'zeppelin_the_mighty_wind' WHERE `entry` = 186238;

DELETE FROM `creature_text` WHERE `CreatureID` IN (3150, 12137, 9566, 26539, 26540, 3149, 26538);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(3150, 0, 0, 'The zeppelin to Grom''gol has just arrived! All aboard for Stranglethorn!', 14, 0, 100, 22, 0, 0, 0, 22075, 0, 'Hin Denburg - Zeppelin Arrival'),
(3150, 1, 0, 'Hey! Did I remember to refill the tank before the zeppelin left? Oh, well!', 12, 0, 100, 0, 0, 0, 0, 22076, 0, 'Hin Denburg - Zeppelin Departure'),
(12137, 0, 0, 'The zeppelin to Undercity has just arrived! All aboard for Tirisfal Glades!', 14, 0, 100, 22, 0, 0, 0, 22099, 0, 'Squibby Overspeck - Zeppelin Arrival'),
(9566, 0, 0, 'The zeppelin to Orgrimmar has just arrived! All aboard for Durotar!', 14, 0, 100, 22, 0, 0, 0, 22077, 0, 'Zapetta - Zeppelin Arrival'),
(9566, 1, 0, 'There goes the zeppelin to Orgrimmar. I hope there''s no explosions this time.', 12, 0, 100, 0, 0, 0, 0, 22080, 0, 'Zapetta - Zeppelin Departure'),
(26539, 0, 0, 'The zeppelin to Vengeance Landing has just arrived! All aboard for Howling Fjord!', 14, 0, 100, 22, 0, 0, 0, 29540, 0, 'Meefi Farthrottle - Zeppelin Arrival'),
(26540, 0, 0, 'The zeppelin to Undercity has just arrived! All aboard for Tirisfal Glades!', 14, 0, 100, 22, 0, 0, 0, 29545, 0, 'Drenk Spannerspark - Zeppelin Arrival'),
(3149, 0, 0, 'The zeppelin to Orgrimmar has just arrived! All aboard for Durotar!', 14, 0, 100, 22, 0, 0, 0, 22077, 0, 'Nez''raz - Zeppelin Arrival'),
(26538, 0, 0, 'The zeppelin to Orgrimmar has just arrived! All aboard for Durotar!', 14, 0, 100, 22, 0, 0, 0, 29551, 0, 'Nargo Screwbore - Zeppelin Arrival');

-- Arrival horn (sound 11804, datalong2 2 = distance dependent) on the TaxiPathNode.dbc ArrivalEventID of each stop (5.4.8 DBC verified)
DELETE FROM `event_scripts` WHERE `id` IN (15312, 15314, 15318, 15320, 15322, 15324, 19126, 19127, 19137, 19139, 21868, 21870) AND `command` = 16;
INSERT INTO `event_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `dataint`, `x`, `y`, `z`, `o`) VALUES
(15312, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Purple Princess, node 16 (Tirisfal approach)
(15314, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Purple Princess, arrival in Stranglethorn Vale
(15318, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Thundercaller, arrival in Durotar
(15320, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Thundercaller, arrival in Tirisfal Glades
(15322, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Iron Eagle, arrival in Durotar
(15324, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Iron Eagle, arrival in Stranglethorn Vale
(19126, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Cloudkisser, arrival in Howling Fjord
(19127, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Cloudkisser, arrival in Tirisfal Glades
(19137, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Mighty Wind, arrival in Durotar
(19139, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Mighty Wind, arrival in Borean Tundra
(21868, 0, 16, 11804, 2, 0, 0, 0, 0, 0), -- The Zephyr, arrival in Orgrimmar (Durotar)
(21870, 0, 16, 11804, 2, 0, 0, 0, 0, 0); -- The Zephyr, arrival in Thunder Bluff (Mulgore)
