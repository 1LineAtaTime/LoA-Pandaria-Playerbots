-- Eastern Plaguelands, Gidwin's Fate Revealed (27526), the zone's finale: the scene at the slaughterhouse door, which did not
-- exist. Data for the C++ in zone_eastern_plaguelands_finale.cpp (at_epl_gfr_slaughterhouse, npc_epl_gfr_camera,
-- npc_epl_gfr_actor, spell_epl_gfr_mirror_image).
--
--  * Before: AreaTrigger 6300 (the door) was unscripted and nothing gave "Find Gidwin Goldbraids" (45940): the quest could not
--    be completed. The camera 45965 had no VehicleId, every actor was faction 35 at level 1, and five summon positions were missing.
--  * Sourced: the summon spells and their positions (DBC 85649-85663, 85680-85683, 85833; spell_target_position of both TDBs,
--    85657 / 85658 / 85659 / 85833 sniffed in build 25950); the camera's vehicle 1156 (Vehicle.dbc: one seat, 8645, CameraMode
--    70 "Eastern Plaguelands 4.x - GFR Event Camera"); factions and levels of TDB 4.4.2 (2244 for the party and the double,
--    2245 for the Baroness, hostile both ways; 474 Beezil; 35 George and the ghosts); the six lines (broadcast texts 46064-46069:
--    says, and Tarenar's "Charge!" a yell, as the wiki's templates and the comments that saw only the yell); the double's
--    Mirror Image Aura (TDB: spell_gen_clone on 85682; this core's spell_gen_clone hooks a different effect, so its own script).
--  * Ours: the facings of 85657 / 85658 / 85659 (the TDBs have none: the Baroness's 5.01, towards the party); the position of
--    85656 Vex'tul (Live), which no source has (Argus's entry spot, from where he walks in and fights).
--  * creature_text groups: these four entries had none, so they start at 0.

-- Positions
DELETE FROM `spell_target_position` WHERE `id` IN (85656,85657,85658,85659,85833);
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(85656,0,0,2993.03,-3453.21,152.206,0.692997),
(85657,0,0,3007.52,-3426.23,152.624,5.01269),
(85658,0,0,3011.12,-3427.91,152.381,5.01269),
(85659,0,0,3000.76,-3422.93,149.809,5.01269),
(85833,0,0,2993.03,-3453.21,152.206,0.692997);

-- The camera is a vehicle
UPDATE `creature_template` SET `VehicleId` = 1156 WHERE `entry` = 45965;

-- Factions and levels (TDB 4.4.2; 45956 51, 45960 60 and 45962 43 are already right)
UPDATE `creature_template` SET `faction` = 2244 WHERE `entry` IN (45955,45957,45958,45959,45960,45963,45978);
UPDATE `creature_template` SET `faction` = 2245 WHERE `entry` = 45956;
UPDATE `creature_template` SET `faction` = 474 WHERE `entry` = 46043;
UPDATE `creature_template` SET `minlevel` = 45, `maxlevel` = 45 WHERE `entry` IN (45955,45959);
UPDATE `creature_template` SET `minlevel` = 46, `maxlevel` = 46 WHERE `entry` = 45957;
UPDATE `creature_template` SET `minlevel` = 61, `maxlevel` = 61 WHERE `entry` = 45958;
UPDATE `creature_template` SET `minlevel` = 43, `maxlevel` = 43 WHERE `entry` = 45963;
UPDATE `creature_template` SET `minlevel` = 42, `maxlevel` = 42 WHERE `entry` = 45964;
UPDATE `creature_template` SET `minlevel` = 40, `maxlevel` = 40 WHERE `entry` = 45978;
UPDATE `creature_template` SET `minlevel` = 44, `maxlevel` = 44 WHERE `entry` = 46043;

-- Nobody's kill: the Baroness dies by the script, and no actor gives experience (ours; players also do no damage to them)
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 64 WHERE `entry` IN (45955,45956,45957,45958,45959,45960,45961,45962,45963,45964,45978,46043);

-- Scripts
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_epl_gfr_camera' WHERE `entry` = 45965;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_epl_gfr_actor' WHERE `entry` IN (45955,45956,45957,45958,45959,45960,45961,45962,45963,45964,45978,46043);
DELETE FROM `areatrigger_scripts` WHERE `entry` = 6300;
INSERT INTO `areatrigger_scripts` (`entry`,`ScriptName`) VALUES
(6300,'at_epl_gfr_slaughterhouse');

-- The double's looks and weapons: 85682's three effects pick a nearby Your Doppelganger (the script picks the caster's own)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85682;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,7,85682,0,0,31,0,3,45978,0,0,0,0,'','GFR: Mirror Image Aura - target Your Doppelganger');
DELETE FROM `spell_script_names` WHERE `spell_id` IN (85682,85683);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85682,'spell_epl_gfr_mirror_image');

-- The lines
DELETE FROM `creature_text` WHERE `CreatureID` IN (45955,45956,45957,45958);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45955,0,0,'You''ll never get away with this, you ugly, stinkin'' witch!',12,0,100,0,0,0,0,46064,0,'Gidwin Goldbraids (GFR) - tied on the table'),
(45956,0,0,'Oh really?  What exactly will you do, dwarf?  There''s nobody to save you here.',12,0,100,0,0,0,0,46065,0,'Baroness Anastari (GFR) - to Gidwin'),
(45956,1,0,'Just the two of you?  You''ll never stand a chance.',12,0,100,0,0,0,0,46067,0,'Baroness Anastari (GFR) - to Tarenar and the player'),
(45957,0,0,'You''re wrong, banshee.  We''ve come to take Gidwin back.',12,0,100,0,0,0,0,46066,0,'Tarenar Sunstrike (GFR) - walks in'),
(45957,1,0,'Charge!',14,0,100,0,0,0,0,46069,0,'Tarenar Sunstrike (GFR) - the attack'),
(45958,0,0,'I beg to differ.',12,0,100,0,0,0,0,46068,0,'Argus Highbeacon (GFR) - the party walks in');
