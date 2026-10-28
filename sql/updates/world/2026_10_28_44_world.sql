-- Hillsbrad Foothills: the scripted events of the Sludge Fields (zone_hillsbrad_sludge_stormpike.cpp)
-- Sourced: the spells (5.4.8 DBC), the line (broadcast_text), the camera start of 89144 (already in mop, as in the
-- Cataclysm Classic TDB). Ours: the camera path and the timings.

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

