-- Hillsbrad Foothills, Azurelode Mine: the Hungry Mine Creeper eats its murlocs, and the Infested Bears are ordinary bears.

-- Coastal Delicacies! (28146). The credit came straight from the murloc's death (2026_10_28_41) with nothing to show for it.
-- In 4.0.3 the spider walked to the corpse of each murloc its player killed and ate it (Wowhead comments: no killing blow
-- needed, other players' kills don't count), which is the spell 88898 "Devour Murloc" (dummy, visual, can target the dead).
-- npc_hillsbrad_hungry_mine_creeper now does that and gives the Murloc Eaten Proxy credit when it eats. The murlocs' own
-- credit and the spider's kill credit go.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_hungry_mine_creeper' WHERE `entry`=47662;
DELETE FROM `smart_scripts` WHERE `entryorguid`=47662 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (2374,2375,2376,2377) AND `source_type`=0 AND `id`=10;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=11 AND `SourceEntry` IN (2374,2375,2376,2377);

-- Past Their Prime (28115) and Glorious Harvest (28114). The Infested Bear 47204 was made unkillable: it stopped at 1 hp, gave
-- the quest credit, evaded and became unattackable, and only died (and vanished) once all 8 Creeper Eggs on its back were
-- picked, or 30 s after its eggs were told it had "died". Retail (TDB 12.x, Wowhead 4.0.x) has a normal bear that is killed
-- for the quest (and could even be tamed, losing its eggs) and only casts Sundering Swipe; picking an egg just removes that egg.
DELETE FROM `smart_scripts` WHERE `entryorguid`=47204 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(47204,0,0,0,0,0,100,0,7000,8000,20000,22000,0,11,84867,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Infested Bear - In Combat - Cast Sundering Swipe');

DELETE FROM `smart_scripts` WHERE `entryorguid`=47203 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(47203,0,0,0,73,0,100,0,0,0,0,0,0,41,500,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Creeper Egg - On Spellclick - Despawn');

-- The bear's spellclick (Ride Vehicle Hardcoded) is only there to seat its eggs; players could use it too and ride a bear that
-- had lost an egg. Friendly clickers only (the eggs are not players, so they still board).
UPDATE `npc_spellclick_spells` SET `user_type`=1 WHERE `npc_entry`=47204 AND `spell_id`=46598;
