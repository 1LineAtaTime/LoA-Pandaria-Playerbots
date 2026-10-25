-- Nowhere to Run (27195): it has no objective here, so it was complete on accept and Master Forteski never went into the Deep Elem
-- Mine. It now needs its quest event: accepting it from Forteski (or talking to him while it is in progress) has him lead his
-- Veteran Forsaken Troopers into the mine, where Packleader Ivar Bloodfang springs the trap; Forteski's last words give the event
-- before the bomb kills him and his troopers, who return shortly. Bloodfang and the bomb are summoned for the scene, so their
-- sniffed spawns go.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_master_forteski' WHERE `entry` = 45228;

INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES (27195);
UPDATE `quest_template_addon` SET `SpecialFlags` = 2 WHERE `ID` = 27195;

DELETE FROM `creature` WHERE `guid` IN (307413, 307414);
DELETE FROM `creature_addon` WHERE `guid` IN (307413, 307414);

DELETE FROM `creature_text` WHERE `CreatureID` IN (45228, 45236);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45228, 0, 0, 'Ready then? Let''s get a move on! Try and keep up, $n.', 12, 0, 100, 0, 0, 0, 0, 45386, 0, 'Master Forteski - Nowhere to Run'),
(45228, 1, 0, 'Onward, brothers and sisters!', 12, 0, 100, 0, 0, 0, 0, 45387, 0, 'Master Forteski - Nowhere to Run'),
(45228, 2, 0, 'What''s this then?', 12, 0, 100, 0, 0, 0, 0, 45388, 0, 'Master Forteski - Nowhere to Run'),
(45228, 3, 0, 'BOMB! IT''S A TRAP! RUN!', 14, 0, 100, 0, 0, 0, 0, 45391, 0, 'Master Forteski - Nowhere to Run'),
(45228, 4, 0, 'Remember this day as the day that Forteski saved your hide. Now run! Get back to Sylvanas!', 12, 0, 100, 0, 0, 0, 0, 45394, 0, 'Master Forteski - Nowhere to Run'),
(45236, 0, 0, 'Your doom!', 12, 0, 100, 0, 0, 0, 0, 45389, 0, 'Packleader Ivar Bloodfang - Nowhere to Run');

-- Explosion throws Forteski, his troopers and the players with them (Forteski's warning saves them: the blast only throws them
-- back up the tunnel) and sets off the bomb wagons
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 84474;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 84474, 0, 0, 31, 0, 3, 45228, 0, 0, 0, 0, '', 'Explosion - Target Master Forteski'),
(13, 1, 84474, 0, 1, 31, 0, 3, 45225, 0, 0, 0, 0, '', 'Explosion - Target Veteran Forsaken Trooper'),
(13, 1, 84474, 0, 2, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Explosion - Target Player'),
(13, 2, 84474, 0, 0, 31, 0, 5, 205271, 0, 0, 0, 0, '', 'Explosion - Target Bomb Wagon');
