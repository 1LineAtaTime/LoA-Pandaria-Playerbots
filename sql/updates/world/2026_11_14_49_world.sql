-- Acherus: The Ebon Hold (area 4281): the Scourge attack's spawns out of the normal hold, the attack whole, the hold's
-- missing Spirit Healer and runeforge, one Nemesis Trainer.
-- The hold has two states, by the legacy phaseMask: mask 1 (everyone, and a death knight after the Chapter V quests) and
-- mask 256 (a death knight from accepting Taking Back Acherus 13165 until 13188/13189 is rewarded: spell_area 58354
-- replaces mask 1 by 256). Both TDBs put the attack in phase 177 and the hold in 169; AzerothCore (3.3.5) phases it with
-- the same spell and masks. Every change below is where both TDBs and AzerothCore agree, except where a line says so.
-- Templates shared with map 609 (the starting zone) are not touched; nothing here is on map 609.
--
-- 1. 23 Knights of the Ebon Blade 31094 of the attack stood in mask 1, 0.0-1.0 yd from AzerothCore's attack knights: a
--    3.3.5-lineage import whose attack masks became 1. The attack keeps its own 23 (both TDBs: phase 177 only).
DELETE FROM `creature_addon` WHERE `guid` IN (303455,303457,303459,303460,303461,303462,303463,303467,303468,303469,303470,303471,303472,303473,303474,303475,303479,303480,303481,303491,303517,303518,303519);
DELETE FROM `creature` WHERE `id`=31094 AND `map`=0 AND `phaseMask`=1 AND `guid` IN (303455,303457,303459,303460,303461,303462,303463,303467,303468,303469,303470,303471,303472,303473,303474,303475,303479,303480,303481,303491,303517,303518,303519);
-- 2. Nine attack knights in mask 384 stood 0.6-2.0 yd from a mask-256 one (nothing on map 0 gives mask 128, so both
--    layers show together): 32 knights in the attack, 23 after (AzerothCore 23, TDBs 18).
DELETE FROM `creature_addon` WHERE `guid` IN (89910,89911,89912,89913,89914,89918,89919,89920,89922);
DELETE FROM `creature` WHERE `id`=31094 AND `map`=0 AND `phaseMask`=384 AND `guid` IN (89910,89911,89912,89913,89914,89918,89919,89920,89922);
-- 3. The attack's Koltira and Thassarian are 31088 / 31089 (both TDBs phase 177, AzerothCore the attack masks, on exactly
--    the spots of 303451 / 303452, which stood in mask 1 by the hall teleporter). The attack held the Light's Hope battle's
--    entries 29199 / 29200 instead (no map-0 spawn in any source); those two spawns go.
DELETE FROM `creature_addon` WHERE `guid` IN (89753,89754);
DELETE FROM `creature` WHERE `guid` IN (89753,89754) AND `id` IN (29199,29200) AND `map`=0;
UPDATE `creature` SET `phaseMask`=256 WHERE `guid` IN (303451,303452) AND `id` IN (31088,31089);
-- 4. The six Scourge Necromancers 31096 of the attack had mask 64, which nothing gives on map 0: nobody saw them. Both TDBs:
--    six, phase 177. AzerothCore has five in 256 and its sixth with mop's mask 64; the TDBs decide. They count for 13166
--    (KillCredit1 31100). The two teleport pads in mask 64 are dead copies of 72009 / 72010 (mask 257, same spots).
UPDATE `creature` SET `phaseMask`=256 WHERE `id`=31096 AND `map`=0 AND `areaId`=4281 AND `phaseMask`=64;
DELETE FROM `gameobject` WHERE `guid` IN (1339,1347) AND `id` IN (191538,191539) AND `phaseMask`=64;
-- 5. The Hall <-> Heart teleport triggers 29588 / 29589 and their pads stood twice in mask 1: the mask-257 pair
--    (102564 / 102565, pads 72009 / 72010 of 191538 / 191539) serves both states, as the TDBs and AzerothCore have one pair
--    per state. The triggers' teleport auras come from the template addon (54742 / 54745), so the kept pair works. The
--    4.x pad entries 207580 / 207581 are in no source on these spots.
DELETE FROM `creature_addon` WHERE `guid` IN (173823,173824);
DELETE FROM `creature` WHERE `guid` IN (173823,173824) AND `id` IN (29588,29589) AND `phaseMask`=1;
DELETE FROM `gameobject` WHERE `guid` IN (157615,157620) AND `id` IN (207580,207581);
-- 6. The attack's knights cannot be attacked by the death knight: IMMUNE_TO_PC beside mop's IMMUNE_TO_NPC (both TDBs,
--    sniffed: template 768, spawns 33536). AzerothCore differs (no immunity: its 3.3.5 knights fight and respawn); the
--    sniff decides for this client. 31094 is spawned only here.
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|256 WHERE `entry`=31094;
-- 7. Two Highlord's Nemesis Trainers stood inside each other at 2460.3 -5593.0: 32547 (level 83, npc_training_dummy) and
--    54344 (level 88, no script: it never reset). Both TDBs and AzerothCore have 32547 alone; 54344 goes. (The spawns
--    dossier's second pass preferred 54344 on MoP Classic map pins; kept to the sources that agree, see the reply.)
DELETE FROM `creature_addon` WHERE `guid`=308371;
DELETE FROM `creature` WHERE `guid`=308371 AND `id`=54344;
-- 8. The hold's Spirit Healer (both TDBs phase 169, AzerothCore every mask; mask 257 = both states) and the fourth
--    runeforge 190557 (both TDBs phases 169 / 175, AzerothCore mask 1) were missing.
DELETE FROM `creature` WHERE `guid`=3129600;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`curhealth`) VALUES
(3129600,6491,0,139,4281,1,257,2356.65,-5663.06,382.257,0.586577,300,1);
DELETE FROM `gameobject` WHERE `guid`=3129600;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`) VALUES
(3129600,190557,0,139,4281,1,1,2411.87,-5626.11,420.644,0.296705,0.147809,0.989016,180,255,1);
