-- Gilneas, quest 14212 "Sacrifices": run the native per-player Crowley ride instead of the shared static horse.
-- The ride scripts (zone_gilneas.cpp: npc_lord_darius_crowley_c2 summons a horse per player, npc_crowley_horse drives
-- it along path 35231 and seats its own mounted Crowley) were only bound by an UPDATE that required the ScriptName to
-- be set already, so on this database Crowley (35552) and his horse (35231) stayed unbound. Only the one static horse
-- worked: a single shared ride with a 300 s respawn after every use, which sometimes never left its spawn.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_lord_darius_crowley_c2', `npcflag` = 3
WHERE `entry` = 35552 AND `ScriptName` = '';
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_crowley_horse'
WHERE `entry` = 35231 AND `ScriptName` = '';
-- The horse is now a per-player summon: the static one goes, with its spellclick boarding bridge and SmartAI. The static mounted
-- Crowley (35230, passive) stays as scenery.
DELETE FROM `creature` WHERE `guid` = 371595 AND `id` = 35231;
-- Its SmartAI rows go too (the horse runs C++ now; its route stays in `waypoints` 35231, which the C++ reads).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35231 AND `source_type` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 9 AND `SourceEntry` = 35231;
