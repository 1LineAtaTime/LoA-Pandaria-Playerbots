-- Refs #506, #439. 26706 "Endgame": the gunship boarding sequence. NEEDS the build that carries
-- npc_lorna_crowley_endgame, npc_wyvern_endgame and the reworked npc_hippogryph (zone_gilneas.cpp, build 27+).
-- Without that build the hippogryph would still credit the fly-over (#479) and these spawns are only scenery.
--
-- Intended (Wowpedia "Endgame (quest)" Notes, MediaWiki API read 2026-10-09; owner decision 2026-10-09: follow it):
-- the hippogryph drops the player on top of the airship among grunts; Lorna yells "Attack!", "Secure the rafters!",
-- "I want two sharpshooters..."; the party rappels down; "Hands up, greenskins!..." and the Navigator says "Dabu?";
-- "The rest of you, follow me downstairs!"; explosives in the furnace room; Korm Bonegrind appears ("That's... one
-- big orc."); kill him; the ship blows and the party escapes on wyverns back to Keel Harbor.
-- Port source: ShatterCore (Mr-Scooty/ShatterCore) b2d634ad6c gilneas_chapter_4.cpp:1470-2015 (C++) and
-- sql/updates/world/4.3.4/2026_07_05_01_world.sql:72-197 (static gunship + deck) and 2026_07_05_03_world.sql:27-29,115
-- (ScriptNames, "Attack!" text).
--
-- Fork state: the deck crew already exists as static map-654 spawns in the same re-based ship frame ShatterCore uses
-- (e.g. Lorna 223334 = ShatterCore 9000553, grunt 223311 = 9000549, ropes 166908/166909 = 9000503/9000507), but in
-- phase 1 with no ship under it, while the 26706 player is in phase 4194304 (phase_definitions 4714/13, after 24681).
-- Korm 43567 (guid 223342) stands at -982.7 2363.8 120.0, nowhere near the ship. The moving transport 203428 and its
-- map-749 crew are phase 1 (transports keep PHASEMASK_NORMAL here) and stay as they are; a 26706 player cannot see them.
--
-- This file:
--   * GO template 800100 "Orc Gunship" (type 5 generic, display 8253 = Nd_Hordegunship.wmo, which is in
--     vmaps/GameObjectModels.dtree, so the deck also has server-side collision) and one spawn at the ShatterCore frame
--     (-1465.76, 3251.48, 93.84, yaw 2.14335), phase 4194304. Entry 800100 and guid 4100001 are free in the test DB
--     (max gameobject_template entry 444006, max gameobject guid 4000076 on 2026-10-09).
--   * the 52 deck creatures (22 grunts, Lorna, 5 Worgen Warriors, 4 Sharpshooters, Admiral, Navigator, 3 roosting
--     Wyverns, 15 ropes) and the 2 rappel ropes -> phase 4194304. The 43718/40350 invisible triggers stay in phase 1.
--   * Korm moved to the furnace deck (ShatterCore 9000554: -1436.3203 3229.6905 105.4304 3.18799), phase 4194304,
--     MovementType 0; Korm and the grunts respawn after 120 s (ShatterCore 2026_07_05_01:197), so the event can
--     re-arm 123 s after a kill. His template keeps IMMUNE_TO_PC|IMMUNE_TO_NPC (unit_flags 768) until Lorna's
--     furnace beat clears it.
--   * 3 Incendiary Explosives 204458 in the furnace room (ShatterCore 9000504-9000506), guids 4100002-4100004.
--   * ScriptNames: 43566 npc_lorna_crowley_endgame, 43713 npc_wyvern_endgame. The wyvern's SmartAI rows (on summon
--     -> teleport the invoker to map 646 Deepholm, despawn) are removed: 43713 is only summoned by 81779 (Spell DBC).
--   * creature_text: Lorna group 7 "Attack!" (ShatterCore 2026_07_05_03:115), Navigator 43767 group 0 "Dabu?" in
--     Orcish (Wowpedia). No broadcast text ids known for either.
--
-- Backup (run before applying):
--   mysqldump -h<host> -P<port> -u<user> -p<password> world creature --where="guid IN (223342) OR (map=654 AND id IN (42141,43566,43651,43703,43764,43767,43791,43793) AND position_x BETWEEN -1560 AND -1380 AND position_y BETWEEN 3180 AND 3330)"
--   mysqldump -h<host> -P<port> -u<user> -p<password> world gameobject --where="guid IN (166908,166909)"
--   mysqldump -h<host> -P<port> -u<user> -p<password> world smart_scripts --where="entryorguid=43713 AND source_type=0"
-- Rollback:
--   UPDATE `creature` SET `phaseMask` = 1, `spawntimesecs` = 300, `MovementType` = 1, `wander_distance` = 3 WHERE `guid` IN (<the 52 guids below>);
--   UPDATE `creature` SET `phaseMask` = 1, `position_x` = -982.688, `position_y` = 2363.77, `position_z` = 119.978, `orientation` = 3.15209, `MovementType` = 2, `spawntimesecs` = 300 WHERE `guid` = 223342;
--   UPDATE `gameobject` SET `phaseMask` = 1 WHERE `guid` IN (166908, 166909);
--   DELETE FROM `gameobject` WHERE `guid` BETWEEN 4100001 AND 4100004;
--   DELETE FROM `gameobject_template` WHERE `entry` = 800100;
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 43566;
--   UPDATE `creature_template` SET `ScriptName` = '', `AIName` = 'SmartVehicleAI' WHERE `entry` = 43713;
--   (re-insert the two 43713 smart_scripts rows from the backup)
--   DELETE FROM `creature_text` WHERE (`CreatureID` = 43566 AND `GroupID` = 7) OR (`CreatureID` = 43767 AND `GroupID` = 0);
-- Re-runnable: UPDATE + DELETE-then-INSERT.

-- Static gunship (ShatterCore 2026_07_05_01:79-84)
DELETE FROM `gameobject_template` WHERE `entry` = 800100;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `data0`, `data1`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(800100, 5, 8253, 'Orc Gunship', '', '', '', 1, 0, 0, '', '', 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 4100001 AND 4100004;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(4100001, 800100, 654, 4714, 4714, 1, 4194304, -1465.76, 3251.48, 93.84, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),
-- Incendiary Explosives in the furnace room (ShatterCore 2026_07_05_01:182-186, 9000504-9000506)
(4100002, 204458, 654, 4714, 4714, 1, 4194304, -1494.8164, 3296.5706, 104.2867, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),
(4100003, 204458, 654, 4714, 4714, 1, 4194304, -1493.0065, 3297.7837, 104.2948, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),
(4100004, 204458, 654, 4714, 4714, 1, 4194304, -1496.3229, 3294.9650, 104.2769, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1);

-- Rappel ropes on the top deck
UPDATE `gameobject` SET `phaseMask` = 4194304 WHERE `guid` IN (166908, 166909) AND `id` = 204428;

-- Deck crew -> the 26706 phase, standing still.
-- Wave 27 fix: all 52 had MovementType 1 / wander_distance 3 (random movement). On the deck that has no navmesh,
-- t439 cp9 (build 27) received Lorna's yells (300 yd) but none of her says (40 yd) although the player hovered
-- 13.8 yd from her spawn the whole time, so she was no longer there (random movement off a deck with no navmesh
-- is the only thing that moves her; inference, no NPC probe). ShatterCore's deck rows are MovementType 0,
-- wander 0 (2026_07_05_01:88-178).
UPDATE `creature` SET `phaseMask` = 4194304, `MovementType` = 0, `wander_distance` = 0 WHERE `map` = 654 AND `guid` IN (
  223311, 223312, 223313, 223314, 223315, 223316, 223317, 223318, 223319, 223320, 223321,
  223322, 223323, 223324, 223325, 223326, 223327, 223328, 223329, 223330, 223331, 223332, -- Gunship Grunt 42141
  223334,                                                                                -- Lorna Crowley 43566
  223333, 223337, 223338, 223339, 223340,                                                -- Worgen Warrior 43651
  223307, 223308, 223335, 223336,                                                        -- Gilnean Sharpshooter 43703
  223309, 223310,                                                                        -- Admiral Stormblood 43764, Navigator 43767
  220393, 220394, 220395,                                                                -- Wyvern 43791 (roosting)
  220396, 220397, 220398, 220399, 220400, 220401, 220402, 220418, 220604, 222952,
  223170, 223303, 223304, 223305, 223306);                                               -- Gilneas Vehicle Rope 43793
UPDATE `creature` SET `spawntimesecs` = 120 WHERE `map` = 654 AND `id` = 42141 AND `guid` BETWEEN 223311 AND 223332;

-- Korm Bonegrind to the furnace deck (ShatterCore 2026_07_05_01:115, 9000554)
UPDATE `creature` SET `phaseMask` = 4194304, `position_x` = -1436.3203, `position_y` = 3229.6905, `position_z` = 105.4304,
  `orientation` = 3.18799, `MovementType` = 0, `wander_distance` = 0, `spawntimesecs` = 120 WHERE `guid` = 223342 AND `id` = 43567;

-- Scripts (ShatterCore 2026_07_05_03:27,29)
UPDATE `creature_template` SET `ScriptName` = 'npc_lorna_crowley_endgame' WHERE `entry` = 43566;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_wyvern_endgame' WHERE `entry` = 43713;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 43713 AND `source_type` = 0;

-- Texts
DELETE FROM `creature_text` WHERE (`CreatureID` = 43566 AND `GroupID` = 7) OR (`CreatureID` = 43767 AND `GroupID` = 0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(43566, 7, 0, 'Attack!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - gunship boarding'),
(43767, 0, 0, 'Dabu?', 12, 1, 100, 0, 0, 0, 0, 0, 0, 'Navigator - Hands up, greenskins');
