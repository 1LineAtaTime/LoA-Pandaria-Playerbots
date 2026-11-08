-- Cry of the Banshee 28538: the Banshee lifts the agent and flies off with it (npc_hillsbrad_banshee, npc_hillsbrad_si7_agent, C++).
-- Sourced:
--  * Carried by Banshee 90874 (area script, basepoints 90873) and 90873 (control vehicle cast on her), 5.4.8 DBC. Nothing cast them.
--  * That she carries the agent off, speaking, while ranged players can still kill and loot it (Wowhead comments on 28538, 48752),
--    towards Tarren Mill (warcraft.wiki.gg caption).
-- Ours:
--  * VehicleId 1348 on 48752. No source gives her a vehicle: 0 in mop and both TDBs, no addon aura, no SET_VEHICLE_ID (aura 296) spell for her.
--    Inferred: no creature in mop or either TDB and no aura 296 spell uses 1348; its id falls among the vehicles of the entries around hers
--    (48739 has 1347, 48790 has 1352); and its one seat 9144 fits (at the shoulder, ride anim Strangulate, no NOT_SELECTABLE and no CAN_CONTROL,
--    usable by a forced ride, no spellclick).
--  * The path, the timings and that both vanish at the end. The agent goes by DespawnOrUnsummon, not action 41, so no living agent is ever hidden.
UPDATE `creature_template` SET `VehicleId`=1348, `AIName`='', `ScriptName`='npc_hillsbrad_banshee' WHERE `entry`=48752;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48752 AND `source_type`=0;

UPDATE `creature_template` SET `ScriptName`='npc_hillsbrad_si7_agent' WHERE `entry` IN (48741,48742);

-- The gaze stays where it was; the 6 s despawn goes (the Banshee or the agent's script ends it)
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (48741,48742) AND `source_type`=0 AND `id` IN (4,5);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48741,0,4,0,61,0,100,0,0,0,0,0,0,11,90903,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Operative - Linked - Gaze of the Banshee'),
(48742,0,4,0,61,0,100,0,0,0,0,0,0,11,90903,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Assassin - Linked - Gaze of the Banshee');

DELETE FROM `spell_script_names` WHERE `spell_id`=90874;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (90874,'spell_hillsbrad_carried_by_banshee');
