-- Andorhal: remove the doubled soldiers and Stickbone Berserkers in the town and the two stage-one soldiers on the river at
-- Sorrow Hill
-- Sourced: each removed spawn stands 0.1-0.3 yd from another spawn of the same entry (Andorhal Deathguard 44331 213790 on
-- 213569, Andorhal Defender 44332 213413 on 213245 and 213730 on 213721, Provincial Minuteman 44944 312045 on 312076, which
-- keeps its path). Deathguard 213791 and Defender 213782 stand 0.7 yd apart on the surface of the river at Sorrow Hill
-- (area 197, water 52.5, river bed 48.5), 150 yd from the town; neither TDB has 44331 or 44332 anywhere (they are the
-- "Summon Andorhal Deathguard/Defender" 83121/83135 summons; nothing casts those, so the town's copies stay).
-- Two more pairs of the same kind: Provincial Minuteman 44944 312087 on 312076 (0.37 yd) and Defender 44332 213662 on 213471
-- (0.47 yd). The Stickbone Berserkers 44329 (306 spawns, none in either TDB; 83701 "Summon Berserkers" is a dummy) stay as
-- The Endless Flow's targets, but 49 of them stood within 0.5 yd of another (35 stacks, 54 pairs): one per stack stays.
-- Ours: which spawn of each pair or stack goes (in a stack, one that walks a path stays if there is one; the five removed
-- walkers' paths are their own).
DELETE FROM `creature_addon` WHERE `guid` IN (213790,213413,213730,312045,213791,213782,312087,213662);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((213790,44331),(213413,44332),(213730,44332),(312045,44944),(213791,44331),(213782,44332),(312087,44944),(213662,44332));
DELETE FROM `waypoint_data` WHERE `id` IN (213233,213344,213575,213597,213743);
DELETE FROM `creature_addon` WHERE `guid` IN (213104,213200,213233,213234,213252,213287,213288,213297,213319,213326,213344,213369,213415,213430,213437,213445,213470,213472,213473,213518,213559,213567,213575,213585,213593,213597,213601,213608,213612,213615,213620,213622,213630,213631,213637,213642,213648,213659,213667,213684,213697,213702,213706,213712,213731,213739,213740,213741,213743);
DELETE FROM `creature` WHERE `id`=44329 AND `guid` IN (213104,213200,213233,213234,213252,213287,213288,213297,213319,213326,213344,213369,213415,213430,213437,213445,213470,213472,213473,213518,213559,213567,213575,213585,213593,213597,213601,213608,213612,213615,213620,213622,213630,213631,213637,213642,213648,213659,213667,213684,213697,213702,213706,213712,213731,213739,213740,213741,213743);
