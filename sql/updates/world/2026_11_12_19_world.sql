-- Western Plaguelands, Andorhal second battle: remove doubled Andorhal Defenders (44448) in the two cells where our layout
-- was 2-3x as dense as the sources (x 1300-1400 / y -1600..-1500: 21 -> 12; x 1200-1300 / y -1600..-1500: 9 -> 5).
--
-- Sourced: the Wowhead map pins of 44448 (cata 55 and mop-classic 42 pins, merged within 9 yd into spots): 10/8 pins and
--   7 spots in the first cell, 4/4 pins and 5 spots in the second.
-- Ours: which spawns go. Each merged pin spot keeps the nearest Defender within 20 yd (one per spot); the rest go. Of the
--   close pairs, 311866 stays and 311910 goes (3 yd apart, one spot); 311867/311868 (2.6 yd apart) both go, as their spot is
--   held by 311866 3-5 yd away; 311996/312006 (5 yd apart) both go, with no Defender pin within 23 yd. 311944 is kept
--   although it holds no spot of its own: it is the only Defender within 20 yd of Deathguard 3127343 (2026_11_12_18).
--   Every Deathguard that had a Defender within 20 yd still has one. 27085's quest area keeps 48 of its 60 Defenders
--   (62 in all, 15 kills needed).
-- None of the 13 has a waypoint path, formation, pool, game event, linked respawn, movement override or guid SmartAI;
-- 10 have a creature_addon (emote 333, aura 54913, no path), deleted with them.

DELETE FROM `creature_addon` WHERE `guid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `creature_formations` WHERE `leaderGUID` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039) OR `memberGUID` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `game_event_creature` WHERE `guid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `pool_creature` WHERE `guid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `linked_respawn` WHERE `guid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039) OR `linkedGuid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `creature_movement_override` WHERE `SpawnId` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
DELETE FROM `creature` WHERE `id`=44448 AND `guid` IN (311867,311868,311910,311911,311912,311915,311916,311927,311945,311996,312002,312006,312039);
