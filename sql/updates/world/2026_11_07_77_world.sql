-- Death From Below: the event (see zone_arathi_highlands.cpp for its sources).
-- Sourced: Daggerspine Marauder faction 74 and no XP (flags_extra 64) as both TDBs. Faction 14 is neutral to Shakes' 390, whose enemy list holds the marauders' faction 60, so they could not fight at all.
-- Sourced: Cannon Ball only hits marauders (conditions row of both TDBs); its position from AzerothCore, on the ground at 2.08 on the marauders' way, and where the Wowhead 5.x pins 25.4,84.6 show them.
-- Sourced: Shakes respawns in about 15 seconds (Wowhead comments of 2011-2014: 8 to 16 seconds; the TDB's 120 is a default).
-- Ours: the cannon does not despawn after a shot (flag 32): without it the core takes it away for its respawn time after the first click, and the comments fire it several times in one event.
-- Ours: the Blackwater Deckhands start no fights with NPCs (UNIT_FLAG_IMMUNE_TO_NPC 0x200, added to mop's 0x8000): their faction 121 is hostile to the marauders' mask and no source has them fighting.
-- They still fight players, but no longer come to each other's help when a player pulls one (Creature::CanAssistTo).
-- Ours: the event flag on the quest; no TDB has it, but the event is what its text asks for.
UPDATE `creature_template` SET `ScriptName` = 'npc_shakes_obreen' WHERE `entry` = 2610;
UPDATE `creature_template` SET `faction` = 74, `flags_extra` = 64, `ScriptName` = 'npc_daggerspine_marauder' WHERE `entry` = 2775;
UPDATE `creature_template` SET `unit_flags` = 33280 WHERE `entry` = 2636;
UPDATE `creature` SET `spawntimesecs` = 15 WHERE `guid` = 161256 AND `id` = 2610;

UPDATE `gameobject_template` SET `ScriptName` = 'go_faldir_cove_cannon' WHERE `entry` = 113531;
UPDATE `gameobject_template_addon` SET `flags` = 36 WHERE `entry` = 113531;

DELETE FROM `spell_target_position` WHERE `id` = 4170;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(4170,0,0,-2105.67,-2003.05,2.07,5.8);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 4170;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,4170,0,0,31,0,3,2775,0,0,0,0,'','Cannon Ball can only hit Daggerspine Marauder');

UPDATE `quest_template_addon` SET `SpecialFlags` = 2 WHERE `ID` = 26628;
