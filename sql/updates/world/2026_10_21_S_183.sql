-- Refs #183 (lane S audit 2026-10-21, build-83 DBErrors): SmartAI rows that can never run.
-- 1) orphans: creature 27754 (4 rows) and gameobject 96036 (1 row) have no template.
-- 2) creatures whose registered C++ ScriptName takes priority over SmartAI (AIName empty): 18166 npc_khadgar, 25967 npc_zephyr,
--    31848 npc_zidormi_dalaran, 64267 npc_kraxik_tornado, 64656 celestial_experience_sha - behaviour-neutral.
-- 3) 66654 Thunder Hold Supplies: its 2 rows give the kill credit for quest 31768 (objective 66308) but AIName is empty, so they never
--    run - set SmartAI (gameplay change: the munitions objective becomes creditable).
-- Undo: restore from the PM's pre-apply dump of world.smart_scripts / world.creature_template (rows listed above);
--       UPDATE creature_template SET AIName='' WHERE entry=66654;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=27754;
DELETE FROM `smart_scripts` WHERE `source_type`=1 AND `entryorguid`=96036;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (18166,25967,31848,64267,64656);
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=66654 AND `AIName`='' AND `ScriptName`='';
