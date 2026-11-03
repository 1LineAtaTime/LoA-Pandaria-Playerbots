-- Hillsbrad Foothills, the Sludge Fields: Do the Right Thing (28189) has two ways to "save" a Human Seedling 47872, and both count
-- for the kill-type objective (47872 x15):
--  * right-click it to pull it out of the dirt (spellclick 89095 "Do the Right Thing"; it jumps out, thanks you and goes away), or
--  * use the provided Shovel 63150 (also the reward Shovel of Mercy 65667) on it: 89089 "Shovel", "Bashes in the skull of a
--    Human Seedling", is an instakill on Human Seedlings only (conditions 17), and the kill gives the credit. Smashing a head puts
--    Head Asplode 89142 on the player: "You might want to rethink your definition of 'right.'" (Spell.dbc tooltip, 10 s).
-- Sources: warcraft.wiki.gg (objectives "Use the shovel to smash in the human seedling's heads. OR [Right Click] the human
-- seedlings to free them from the dirt."), Wowhead comments from 2010-2022 (free one, then shovel it for two credits; heads
-- explode; "bash some heads to get the best buff ever" = 89142; right-click/interact uproots them, select with left click or
-- /tar Human Seedling to use the Shovel), the Spell.dbc text of 89089 and 89142.
--
-- What runs: every one of the 28 seedling spawns has its own per-spawn SmartAI (-guid, ids 0-9, from the 5.4.8 full SAI import):
-- on respawn set the spellclick flag and bury it; on spellclick credit, remove the flag, store the clicker, jump out, run
-- actionlist 4787200 (random line) and despawn after 7 s. SmartScript::GetScript uses a per-spawn script when there is one, so
-- both ways already worked through these. Head Asplode is added to each per-spawn script as id 10 (on Shovel hit, aura on the
-- invoker). The TDBs have the Shovel condition but no script for 89142; applying it on the Shovel hit is ours.
--
-- The entry script of 47872 (ids 0-3 from 2026_10_28_41, a copy of the per-spawn click handling) never ran, since all spawns
-- have per-spawn scripts: removed. Actionlist 4787200 stays (the per-spawn scripts run it).
DELETE FROM `smart_scripts` WHERE `entryorguid`=47872 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-176520,-176537,-176538,-176540,-176543,-176544,-176545,-176546,-176547,-176548,-176549,-176550,-176551,-176854,-176855,-176856,-176858,-176859,-176860,-176862,-176863,-176864,-176865,-176866,-176867,-176868,-176871,-176872) AND `source_type`=0 AND `id`=10;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-176520,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176537,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176538,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176540,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176543,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176544,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176545,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176546,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176547,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176548,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176549,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176550,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176551,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176854,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176855,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176856,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176858,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176859,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176860,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176862,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176863,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176864,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176865,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176866,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176867,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176868,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176871,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker'),
(-176872,0,10,0,8,0,100,0,89089,0,0,0,0,75,89142,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellhit ''Shovel'' - Add Aura ''Head Asplode'' to Invoker');

DELETE FROM `smart_scripts` WHERE `entryorguid`=4787200 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4787200,9,0,0,0,0,100,0,2000,2000,0,0,0,1,0,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Human Seedling - On Script - Say Line Random'),
(4787200,9,1,0,0,0,100,0,3000,3000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Human Seedling - On Script - Despawn');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceGroup`=0 AND `SourceEntry`=89089;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(17,0,89089,0,0,31,1,3,47872,0,0,0,0,'','Shovel - target only Human Seedlings');
