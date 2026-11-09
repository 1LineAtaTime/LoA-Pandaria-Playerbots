-- Scholomancer (Horde) 26924: Koltira Deathweaver 44452 gets a script, so that he says Thassarian's promise when he gives the quest
-- Sourced: broadcast_text 45516 "Gandling is strong, but do not worry. I will join you shortly after you begin to fight him." (Thassarian's
-- group 0 for the Alliance quest 27162: same text, emote and range).
-- Ours: Koltira saying it as well, for the Horde copy of the quest. The credit fix itself is in zone_western_plaguelands.cpp.
UPDATE `creature_template` SET `ScriptName`='npc_koltira_deathweaver_andorhal' WHERE `entry`=44452;
DELETE FROM `creature_text` WHERE `CreatureID`=44452 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44452,0,0,'Gandling is strong, but do not worry.  I will join you shortly after you begin to fight him.',12,0,100,396,0,0,0,45516,0,'Koltira Deathweaver - Scholomancer accepted');
