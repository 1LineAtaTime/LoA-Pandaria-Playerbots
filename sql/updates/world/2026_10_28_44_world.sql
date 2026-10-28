-- Hillsbrad Foothills: the scripted events of the Sludge Fields (zone_hillsbrad_sludge_stormpike.cpp)
-- Sourced: the spells (5.4.8 DBC), every line (broadcast_text), the camera start of 89144 (already in mop, as in the
-- Cataclysm Classic TDB), Stillwater's combat spells (his SmartAI in the same TDB). Ours: the camera path, the timings
-- and where Lydon and Johnny stand.

-- The quest stays an event quest: the scene's end spell (or the script, if it cannot hit) completes it.
UPDATE `quest_template_addon` SET `SpecialFlags`=2 WHERE `ID`=28192;

-- ---------------------------------------------------------------------------------------------------------------------------
-- No One Here Gets Out Alive (28192): Warden Stillwater 47781 casts nothing in mop, so the quest could not be completed.
-- Accepting it now starts the camera tour (Summon Sludge Fields Camera 89144, ride aura 89149 with phase 326, where the actor
-- Stillwater 47834 stands since 2026_10_28_41); Teleport to Sludge Fields Lab 89148 ends it and completes the quest.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_warden_stillwater_sludge' WHERE `entry`=47781;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_sludge_fields_camera' WHERE `entry`=47928;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (47781,47928) AND `source_type`=0;

DELETE FROM `creature_text` WHERE `CreatureID`=47928;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(47928,0,0,'|cFFFFFFFFThere''s trouble at the Sludge Fields...|r',41,0,100,0,0,0,0,48139,5,'Sludge Fields Camera - No One Here Gets Out Alive');

-- ---------------------------------------------------------------------------------------------------------------------------
-- A Blight Upon the Land (28237): Stillwater 48080 now runs on C++ (npc_hillsbrad_warden_stillwater_blight) instead of the
-- SmartAI of 2026_10_28_41: Lydon passes the sentence (48309-48311) before the fight, and the blow that would kill Stillwater
-- gives him his new form (the model of Warden Stillwater 48103) and SMASH!, which throws the players back and kills his
-- followers. Arnauld 48081 and the Sludge Guards 48083 keep their SmartAI from 2026_10_28_41 (data 1 1: attack).
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_warden_stillwater_blight' WHERE `entry`=48080;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48080 AND `source_type`=0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=48080;

DELETE FROM `creature_text` WHERE `CreatureID`=48080;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48080,0,0,'Lydon!? Alive?',12,0,100,0,0,0,0,48306,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,1,0,'Now I know where your allegiances lie, $n!',12,0,100,0,0,0,0,48307,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,2,0,'And you, Jenny? I thought we had a connection. You''ve broken my heart for the last time.',12,0,100,0,0,0,0,48308,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,3,0,'I AM A THREAT? I AM DERANGED?!',14,0,100,0,0,0,0,48312,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,4,0,'What I''ve created here is an evolution! THIS is how the Forsaken will thrive, not through some magical winged fairy sprinkling happy dust on corpses!',14,0,100,0,0,0,0,48313,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,5,0,'Kill them all! They will serve as subjects for our next study!',14,0,100,0,0,0,0,48314,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,6,0,'You think that you''ve won? THINK AGAIN! Stand before my new form and tremble!',14,0,100,0,0,0,0,48324,0,'Warden Stillwater - New form');

DELETE FROM `creature_text` WHERE `CreatureID`=48020 AND `GroupID` BETWEEN 10 AND 12;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48020,10,0,'Warden Stillwater, you are deranged and a threat to our way of life. By authority of the Dark Lady herself, I hereby impose upon you the harshest penalty allowed under Forsaken law - true death.',12,0,100,0,0,0,0,48309,0,'Master Apothecary Lydon - A Blight Upon the Land'),
(48020,11,0,'That goes for all of you traitorous scum who supported this fool!',12,0,100,0,0,0,0,48310,0,'Master Apothecary Lydon - A Blight Upon the Land'),
(48020,12,0,'KNEEL!',14,0,100,0,0,0,0,48311,0,'Master Apothecary Lydon - A Blight Upon the Land');

-- SMASH! (89450: damage and knockback, effect 2 instakill, all on area-entry targets) had no conditions, so it hit everything
-- around Stillwater, the player and Stillwater himself included. Damage and knockback for players, instakill for his followers.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=89450;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,3,89450,0,0,32,0,16,0,0,0,0,0,'','SMASH! - damage and knock back players'),
(13,4,89450,0,0,31,0,3,48081,0,0,0,0,'','SMASH! - kill Flesh-Shaper Arnauld'),
(13,4,89450,0,1,31,0,3,48083,0,0,0,0,'','SMASH! - kill the Sludge Guards');

