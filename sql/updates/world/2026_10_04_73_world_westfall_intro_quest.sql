-- Epic #441 (alexkulya audit rows QT002). Staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- Westfall intro: quest chain gating for "Hero's Call: Westfall!" (26378/28562) -> "Murder Was The Case That
-- They Gave Me" (26209) -> "Hot On the Trail" (26213/26214), the Furlbrow crime-scene intro scene
-- (Lieutenant Horatio Laine + Stormwind Investigators, areatrigger 5983) and crime-scene visibility.
-- Source: alexkulya MoP commit d2f7163, sql/updates/world/2026_09_08_00_world_westfall_intro_quest.sql
-- What is ported (data only, no C++ needed):
--  * NextQuestID=0 on both "Hero's Call: Westfall!" breadcrumbs. In LoA NextQuestID=26209 puts them into
--    26209's DependentPreviousQuests (ObjectMgr.cpp ~4940), so 26209 cannot be taken unless a breadcrumb was
--    rewarded. PrevQuestID=26209 on 26213/26214 (LoA: 0, so they were offered before 26209). Breadcrumb
--    conditions (type 19) as in TC.
--  * Stormwind Investigator (42309) is no longer a quest ender of 184 "Furlbrow's Deed".
--  * spell_area: Detect Quest Invis 0 (79489) in Westfall (zone 40). The whole crime scene in LoA
--    (Farmer/Verna Furlbrow, Old Blanchy, Crime Scene Alarm-o-Bots, investigator 214357) carries
--    79488 "Apply Quest Invis Zone 0" (invisibility type 4) and LoA had no detection source anywhere, so it
--    was invisible to players. Verified: no other creature in Westfall uses invisibility type 4.
--  * areatrigger 5983 -> SmartTrigger -> Set Data 1 1 on Horatio (guid 214362, same guid/entry/position in
--    LoA and alexkulya); Horatio's missing actionlist 4230800 (LoA row 42308/1 already calls it, but it had
--    no rows). Horatio rows id 0-3 replaced (identical to LoA's except a 120 s cooldown on the Data Set event).
--  * 214354 (second Stormwind Investigator) kneels + quest-invis aura like 214357 (guid verified in both DBs).
-- LoA adaptations:
--  * quest_template PrevQuestId/NextQuestId -> LoA quest_template_addon PrevQuestID/NextQuestID.
--  * Horatio actionlist row 6: alexkulya action 85 (INVOKER_CAST) -> 11 (CAST) on self. In LoA, SMART_ACTION_SET_DATA
--    (SmartScript.cpp ~1080) passes no invoker, so INVOKER_CAST would never fire. LoA's own 4230801 uses 11 too.
--  * Horatio rows 4-6 (LoA's Sentinel Hill scene, data 5993) and actionlist 4230801 are kept (only ids 0-3 replaced).
--  * NOT ported (the hobo-witness rework; see audit rows SQ010): creature/creature_template/creature_addon/
--    smart_scripts/creature_text changes for 42383/42384/42386/42391/42413, the (22,1,42384/42386) condition
--    DELETE, the Ragamuffin spawn DELETE, spell_script_names 79168/79169. LoA already has a SmartAI version.
--  * NOT ported: 42311 Alarm-o-Bot movement change (waypoints -> random) - cosmetic, LoA keeps its paths.
--  * Gossip 11635 conditions / gossip_menu_option updates dropped: identical in LoA already.
--  * creature_text / ruRU locale dropped: LoA already has 42308 groups 0-2 and 42309 groups 0-2.

DELETE FROM `creature_questender` WHERE `id` = 42309 AND `quest` = 184;

DELETE FROM `spell_area` WHERE `spell` = 79489 AND `area` = 40;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(79489, 40, 0, 0, 0, 0, 2, 1, 64, 11);

UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` IN (26378, 28562);
UPDATE `quest_template_addon` SET `PrevQuestID` = 26209 WHERE `ID` IN (26213, 26214);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceGroup` = 0 AND `SourceEntry` IN (26378, 28562, 26209);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 1 AND `SourceEntry` = 5983 AND `SourceId` = 2;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 5983, 2, 0, 14, 0, 26209, 0, 0, 0, 0, 0, '', 'If Invoker has never finished, not has active quest ''Murder Was The Case That They Gave Me'''),
(19, 0, 26378, 0, 0, 8, 0, 26209, 0, 0, 1, 0, 0, '', 'Checks if Player has not been rewarded quest'),
(19, 0, 28562, 0, 0, 8, 0, 26209, 0, 0, 1, 0, 0, '', 'Checks if Player has not been rewarded quest'),
(19, 0, 26209, 0, 0, 28, 0, 184, 0, 0, 1, 0, 0, '', 'Quest ''Murder Was The Case That They Gave Me'' available if quest ''Furlbrow''s Deed'' has not been completed.'),
(19, 0, 26209, 0, 0, 28, 0, 26378, 0, 0, 1, 0, 0, '', 'Quest ''Murder Was The Case That They Gave Me'' available if quest ''Hero''s Call Westfall!'' has not been completed.');

DELETE FROM `creature_addon` WHERE `guid` = 214354;
INSERT INTO `creature_addon` (`guid`, `StandState`, `VisFlags`, `SheathState`, `PvPFlags`, `auras`) VALUES
(214354, 8, 1, 1, 16, '79488');

DELETE FROM `areatrigger_scripts` WHERE `entry` = 5983;
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES
(5983, 'SmartTrigger');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 5983 AND `source_type` = 2;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 42308 AND `source_type` = 0 AND `id` IN (0, 1, 2, 3);
DELETE FROM `smart_scripts` WHERE `entryorguid` = 4230800 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5983, 2, 0, 0, 46, 0, 100, 0, 5983, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 10, 214362, 42308, 0, 0, 0, 0, 0, 'Areatrigger - On Trigger - Set Data 1 1 (Lieutenant Horatio Laine)'),
(42308, 0, 0, 1, 38, 0, 100, 0, 1, 1, 120000, 120000, 59, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Data Set 1 1 - Set Run Off'),
(42308, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 80, 4230800, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Data Set 1 1 (Link) - Start Script'),
(42308, 0, 2, 0, 34, 0, 100, 0, 8, 1, 0, 0, 90, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On MI 1 - Set Unit Field Bytes 1 (8)'),
(42308, 0, 3, 0, 34, 0, 100, 0, 8, 2, 0, 0, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On MI 2 - Set Orientation'),
(4230800, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 45, 0, 1, 0, 0, 0, 0, 10, 214357, 42309, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Set Data 0 1 (Stormwind Investigator)'),
(4230800, 9, 1, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 10, 214357, 42309, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Set Data 1 1 (Stormwind Investigator)'),
(4230800, 9, 2, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 45, 2, 1, 0, 0, 0, 0, 10, 214354, 42309, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Set Data 2 1 (Stormwind Investigator)'),
(4230800, 9, 3, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, -9851.94, 910.24, 29.9428, 0, 'Lieutenant Horatio Laine - On Script - Move To Position (1)'),
(4230800, 9, 4, 0, 0, 0, 100, 0, 150, 150, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Say Text Line 0'),
(4230800, 9, 5, 0, 0, 0, 100, 0, 5000, 5000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Say Text Line 1'),
(4230800, 9, 6, 0, 0, 0, 100, 0, 1500, 1500, 0, 0, 11, 78935, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Cast Spell ''Horatio''s Sunglasses'''),
(4230800, 9, 7, 0, 0, 0, 100, 0, 2300, 2300, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Say Text Line 2'),
(4230800, 9, 8, 0, 0, 0, 100, 0, 10500, 10500, 0, 0, 69, 2, 0, 0, 0, 0, 0, 8, 0, 0, 0, -9849.856, 914.9618, 30.27025, 0, 'Lieutenant Horatio Laine - On Script - Move To Position (2)'),
(4230800, 9, 9, 0, 0, 0, 100, 0, 0, 0, 0, 0, 91, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Remove Unit Field Bytes 1 (8)'),
(4230800, 9, 10, 0, 0, 0, 100, 0, 0, 0, 0, 0, 28, 78935, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Horatio Laine - On Script - Remove Aura Due To Spell ''Horatio''s Sunglasses''');
