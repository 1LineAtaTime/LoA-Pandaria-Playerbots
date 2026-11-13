-- Eastern Plaguelands / Kun-Lai Summit: the Sha of Anger's "Overcome by Anger" no longer falls on everyone passing Crown
-- Guard Tower. ELM General Purpose Bunny (All Phases) 35114 carried npc_sha_of_anger_bunny on its template because Kun-Lai
-- uses the same entry; its one Eastern Plaguelands spawn (172531, 1763.7 -3626.6, both TDBs with no script) therefore
-- put 129356 (red screen, +20 % damage done and taken) on every player within 30 yd: the Crown Guard road passes 12 yd
-- from it and the caravan ride of Into the Breach 15 yd.
-- The script moves from the template to the ten Kun-Lai spawns (zone 5841, guids 508145, 508158, 511004, 511045, 511130,
-- 511231, 511325, 511355, 511455, 511499): Creature::GetScriptId takes the spawn's ScriptName first, and the spawn names
-- are registered at load (ObjectMgr's DISTINCT creature.ScriptName list). Nothing else names 35114 (no C++, SmartAI target,
-- condition or summon). The template aura 129356 affects only the bunny itself and stays.
UPDATE `creature_template` SET `ScriptName`='' WHERE `entry`=35114;
UPDATE `creature` SET `ScriptName`='npc_sha_of_anger_bunny' WHERE `id`=35114 AND `zoneId`=5841;
UPDATE `creature` SET `ScriptName`='' WHERE `id`=35114 AND `zoneId`<>5841;
