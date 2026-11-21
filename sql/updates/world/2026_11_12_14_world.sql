-- Western Plaguelands, The Depravity of the Forsaken (27205, Alliance): the scene of what Thassarian is looking at.
-- The quest has no objective and is complete on accept; the scene is display only and the hand-in never waits for it.
--
--  * Sourced (5.4.8 DBC): The Depravity of the Forsaken: Summon Camera 84483 (Depravity Event Camera 45245, SummonProperties 161 =
--    a vehicle), Summon Aradne 84484 (45246) and Summon Val'kyr 2 84503 (45252), both SummonProperties 2993 (personal); Force
--    Phase 84525 (phase 272 "Andorhal 04A", on the camera's seat-0 passenger); Aradne Lightning 84532. Vehicle 1092: the only
--    vehicle whose seats (8486, 8493) use CameraMode 51/52 "Western Plaguelands 4.x - Depravity 1/2", both locked on
--    (1370.64, -1466.36, 120.0); 45245 had VehicleId 0 (as in both TDBs, which carry no VehicleId for 45113 either).
--  * Sourced (TDB 4.4.2 spell_target_position, sniffed, VerifiedBuild 25928): the camera at (1373.79, -1291.94, 58), 2.8 yd north of
--    Thassarian; Aradne and the val'kyr both at (1371.83, -1497.85, 105.349), 31 yd past the locked point, 211 yd from the camera
--    and 6.6 degrees below the camera's line to the point (inside the zoom's 20 degrees). The sniff has no orientation.
--  * Sourced (broadcast_text 45495): "Is that a.. a val'kyr?".
--  * Sourced (TDB 4.4.2 creature_template_model): the camera's second model (22903, the GM-view Rocket Chicken) has
--    Probability 0; mop had 1, so half of the cameras would have been a chicken in front of the view.
--  * Ours: the orientations (the camera toward the locked point, the actors toward the camera); object_visibility 250 yd ('Large', map-wide with the default thresholds; active,
--    so their grid keeps updating) for Aradne and the val'kyr, which stand beyond this core's 90 yd continent sight range; Aradne
--    Lightning only on the val'kyr beside her (otherwise its 25 yd area takes any unit); the speaker of 45495 (Thassarian, personal
--    to the player); the timing (npc_wpl_depravity_camera).

UPDATE `creature_template` SET `VehicleId` = 1092, `AIName` = '', `ScriptName` = 'npc_wpl_depravity_camera' WHERE `entry` = 45245;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_wpl_depravity_actor' WHERE `entry` IN (45246,45252);

UPDATE `creature_template_model` SET `Probability` = 0 WHERE `CreatureID` = 45245 AND `CreatureDisplayID` = 22903;

DELETE FROM `spell_target_position` WHERE `id` IN (84483,84484,84503);
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(84483,0,0,1373.79,-1291.94,58,4.694),
(84484,0,0,1371.83,-1497.85,105.349,1.561),
(84503,0,0,1371.83,-1497.85,105.349,1.561);

DELETE FROM `object_visibility` WHERE `type` = 'Creature' AND `entry` IN (45246,45252);
INSERT INTO `object_visibility` (`type`,`entry`,`distance`,`active`,`importance`,`comment`) VALUES
('Creature',45246,250,1,'Large','Eastern Kingdoms - Western Plaguelands - Andorhal - Aradne (The Depravity of the Forsaken scene, 211 yd from its camera)'),
('Creature',45252,250,1,'Large','Eastern Kingdoms - Western Plaguelands - Andorhal - Val''kyr (The Depravity of the Forsaken scene, 211 yd from its camera)');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 84532;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,84532,0,0,31,0,3,45252,0,0,0,0,'','The Depravity of the Forsaken: Aradne Lightning - target the scene''s Val''kyr');

DELETE FROM `creature_text` WHERE `CreatureID` = 44453 AND `GroupID` = 9;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44453,9,0,'Is that a.. a val''kyr?',12,0,100,0,0,0,0,45495,5,'Thassarian - The Depravity of the Forsaken, to the player in the scene');

DELETE FROM `spell_script_names` WHERE `spell_id` = 84525;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84525,'spell_wpl_depravity_force_phase');
