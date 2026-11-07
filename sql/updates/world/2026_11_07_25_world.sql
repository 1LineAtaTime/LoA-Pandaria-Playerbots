-- Durnholde: D-1000's forms. The form templates are sniffed and unspawned: 51343 (yeti, display 25113), 51344 (gargoyle, 30403), 51345 (frost wyrm, 26594).
-- Sourced: the order and thresholds, 70% yeti with Swipe, 50% gargoyle with bolts, 25% frost wyrm with Frost Breath (Wowhead npc_49268 comment, 2011; shapeshifting also in two 2010
-- comments and 28647 in 2012); the four abilities and 95970 as Frost Breath (Wowhead Abilities tab); Enrage and the Swipe and Frost Breath repeats (TDB 4.4.2 and 1210).
-- Ours: morphing rather than changing entry (51344 and 51345 are level 1, faction 35, so they read as model donors), each ability only in its form, the first
-- timers, and Gargoyle Strike every 4-6 s for the comment's "spam" (the TDBs have 12-14 s).
-- Not done: "last form he takes is immune" to Hammer of Justice (28647 comment, 2012): one report, and no aura or template flag in any source.
DELETE FROM `smart_scripts` WHERE `entryorguid`=49268 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`target_type`,`target_param1`,`target_param2`,`comment`) VALUES
(49268,0,0,0,2,0,100,1,0,30,0,0,11,61369,0,0,1,0,0,'D-1000 - Below 30% HP - Cast Enrage'),
(49268,0,1,0,0,1,100,0,3000,5000,10000,22000,11,31279,0,0,2,0,0,'D-1000 - Yeti form - Cast Swipe'),
(49268,0,2,0,0,2,100,0,1000,2000,4000,6000,11,16564,0,0,2,0,0,'D-1000 - Gargoyle form - Cast Gargoyle Strike'),
(49268,0,3,0,0,4,100,0,2000,4000,20000,24000,11,95970,0,0,2,0,0,'D-1000 - Frost wyrm form - Cast Frost Breath'),
(49268,0,4,5,2,0,100,1,0,70,0,0,3,51343,0,0,1,0,0,'D-1000 - Below 70% HP - Morph to yeti'),
(49268,0,5,0,61,0,100,0,0,0,0,0,22,1,0,0,1,0,0,'D-1000 - Linked - Set phase 1'),
(49268,0,6,7,2,0,100,1,0,50,0,0,3,51344,0,0,1,0,0,'D-1000 - Below 50% HP - Morph to gargoyle'),
(49268,0,7,0,61,0,100,0,0,0,0,0,22,2,0,0,1,0,0,'D-1000 - Linked - Set phase 2'),
(49268,0,8,9,2,0,100,1,0,25,0,0,3,51345,0,0,1,0,0,'D-1000 - Below 25% HP - Morph to frost wyrm'),
(49268,0,9,0,61,0,100,0,0,0,0,0,22,3,0,0,1,0,0,'D-1000 - Linked - Set phase 3'),
(49268,0,10,0,25,0,100,0,0,0,0,0,3,0,0,0,1,0,0,'D-1000 - On reset - Back to elemental form');
