-- Angry Scrubbing Bubbles 28329 (Ruins of Southshore): starts when you kill an Angry Blight Slime, not from Serge Hinott.
-- Wowhead MoP Classic Quick Facts: no Start, End Serge Hinott; warcraft.wiki.gg: "Start: Automatic", "The quest starts
-- automatically when you kill Angry Blight Slimes"; TDB 442 and retail 1210 list the slime 48319 as its only starter (the
-- sniffed quest push). (Its template aura "Kill Me!" 86559 is a plain dummy, also worn by Silverpine's Bloodfang
-- Stalker 45195 that starts nothing, so it is no evidence.) mop had Hinott 2391 as the starter and nothing pushed the quest. Hinott stays its ender (both TDBs); the quest is handed in from the log
-- (QUEST_FLAGS_AUTO_COMPLETE, 2026_10_28_47), which offers Trail of Filth 28330 at once (RewardNextQuest).
DELETE FROM `creature_queststarter` WHERE `id`=2391 AND `quest`=28329;

-- The push is ours (no SmartAI in either TDB), as for Terrible Little Creatures 28355 in 2026_10_28_42: the killer gets the
-- quest on the slime's death. The kill credit is given before JustDied, so the slime that starts the quest does not count.
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=48319 AND `id`=10;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48319,0,10,0,6,0,100,0,0,0,0,0,0,7,28329,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Angry Blight Slime - On Death - Add Quest ''Angry Scrubbing Bubbles'' to killer');

-- SMART_ACTION_ADD_QUEST checks nothing itself. Only after Trouble at Southshore 28251 is rewarded (the wiki's previous
-- quest; Horde-only, so this also keeps the Horde quest from Alliance players) and while 28329 has never been taken
-- (mask 1: an abandoned quest is pushed again, a rewarded one is not). Only the killing player gets the push: group
-- members get it from their own killing blows, and a pet's killing blow pushes nothing (the invoker is the pet).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=11 AND `SourceEntry`=48319 AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,11,48319,0,0,47,0,28251,64,0,0,0,0,'','Angry Blight Slime: push 28329 only after Trouble at Southshore is rewarded'),
(22,11,48319,0,0,47,0,28329,1,0,0,0,0,'','Angry Blight Slime: push 28329 only if never taken');
