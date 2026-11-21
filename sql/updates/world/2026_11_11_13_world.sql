-- Western Plaguelands, Drudges... <Sigh> (27157): Shadril 45156 leaves its Acid Cloud on its killer as it dies and drains health
-- around itself in the fight.
--
--  * Sourced: Wowhead MoP Classic npc 45156 abilities (Pierce Armor 86070, Acid Cloud 86071, Leech Poison 86072); its template
--    spells 86071/86072/86070; both TDBs' death cast of 86071 (with target_type 0, which SmartScript::GetTargets turns into no
--    target: never cast, there or here). DBC: 86071 puts an 8 s persistent damage cloud at an enemy (TARGET_DEST_TARGET_ENEMY);
--    86072 drains health from enemies within its radius around the caster (TARGET_SRC_CASTER + TARGET_UNIT_SRC_AREA_ENEMY).
--  * Ours: the death cast's target (the killer: SMART_EVENT_DEATH's invoker; the cast is triggered, so a dead caster may cast it,
--    Spell::CheckCast) and Leech Poison's timer (6-10 s into the fight, then every 15-20 s).

UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 45156;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45156 AND `source_type` = 0 AND `id` IN (1, 3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45156,0,1,0,6,0,100,1,0,0,0,0,0,11,86071,3,0,0,0,0,7,0,0,0,0,0,0,0,0,'Shadril - On Death - Cast Acid Cloud on Killer'),
(45156,0,3,0,0,0,100,0,6000,10000,15000,20000,0,11,86072,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Shadril - In Combat - Cast Leech Poison');
