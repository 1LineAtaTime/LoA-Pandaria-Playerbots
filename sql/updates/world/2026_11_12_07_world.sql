-- Western Plaguelands, Zen'Kiki, the Druid (26953): Zen'Kiki's shapeshift antics. In Moonkin form he Moonfires himself, in Cat
-- form he pounces about, in Bear form he roars "Maul! Swipe! MANGLE!", in Aquatic form he is stuck.
--
--  * Sourced: Wowhead comments on 26953: "Moonkin: He'll use moonfire on himself. NOTE: it will damage him ... Bear: He hits out at
--    nothing ... Cat: He'll pounce all over the place / Aquatic: He'll turn upside-down and flop around" (2010-11-25, +32);
--    "spamming moonfire on himself (Yowch! Ow, mon! Oof! Stop dat!)" (+5); "Here come de moonfire! Yowch!" (+60); "It burns like
--    the MOON!"; "Shapeshift! Time for Cat Form! / Leap! / Meow! / Pounce! / Rawr!" (+7); "Shapeshift! Try to swim away now, mon! /
--    I seem to be... stuck.." (+39); "no cooldown between his moonfires". The DBC spells: 83091 Moonfire Spam (5 s, triggers
--    82816 Moonfire, damage on the caster himself, every 350 ms) and 83453 Pounce (a jump to a random spot near the caster), and
--    broadcast texts 44855-44862 (the moonfire yelps), 44863-44870 (the cat lines), 44875-44877 ("Rawr!!", "Maul!  Swipe!
--    MANGLE!", "Zen'Kiki is in control!"), 44893-44894 ("A little help here...", "I seem to be... stuck...") that nothing used.
--  * Ours: when in each form these come (Moonfire Spam 1 s after the shift with three yelps; three pounces 2 s apart, each with a
--    cat line; the bear lines between the existing ones; the stuck line 1.5 s into Aquatic form) and the grouping of the texts.
--  * The form scripts already shifted him and said the shift line (Bear and Aquatic also put him on their Offset Vehicles 44555 /
--    44552, summoned by the form spells); only the antics were missing.
--  * The two static spawns of those Offset Vehicles (guids 213805, 311591: invisible, wandering the wildlife fields) are summons
--    the form spells make for 10 s; neither TDB spawns them, and one could be boarded instead of the one just summoned.

DELETE FROM `creature_text` WHERE `CreatureID` = 44904 AND `GroupID` IN (9, 10, 11, 12);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44904,9,0,'Ouch!',12,0,100,0,0,0,0,44855,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,1,'Oof!',12,0,100,0,0,0,0,44856,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,2,'Ow, mon!',12,0,100,0,0,0,0,44857,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,3,'It burns!',12,0,100,0,0,0,0,44858,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,4,'Yowch!',12,0,100,0,0,0,0,44859,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,5,'Stop dat!',12,0,100,0,0,0,0,44860,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,6,'It burns like the moon!',12,0,100,0,0,0,0,44861,0,'Zen''Kiki - Moonfire Spam'),
(44904,9,7,'Cut it out!',12,0,100,0,0,0,0,44862,0,'Zen''Kiki - Moonfire Spam'),
(44904,10,0,'Leap!',12,0,100,0,0,0,0,44863,0,'Zen''Kiki - Pounce'),
(44904,10,1,'Pounce!',12,0,100,0,0,0,0,44864,0,'Zen''Kiki - Pounce'),
(44904,10,2,'Attack!',12,0,100,0,0,0,0,44865,0,'Zen''Kiki - Pounce'),
(44904,10,3,'Meow!',12,0,100,0,0,0,0,44866,0,'Zen''Kiki - Pounce'),
(44904,10,4,'Get back over here, mon!',12,0,100,0,0,0,0,44867,0,'Zen''Kiki - Pounce'),
(44904,10,5,'Here comes Zen''Kiki!',12,0,100,0,0,0,0,44868,0,'Zen''Kiki - Pounce'),
(44904,10,6,'Rawr!',12,0,100,0,0,0,0,44869,0,'Zen''Kiki - Pounce'),
(44904,10,7,'Haha!',12,0,100,0,0,0,0,44870,0,'Zen''Kiki - Pounce'),
(44904,11,0,'Rawr!!',12,0,100,0,0,0,0,44875,0,'Zen''Kiki - Bear Form'),
(44904,11,1,'Maul!  Swipe!  MANGLE!',12,0,100,0,0,0,0,44876,0,'Zen''Kiki - Bear Form'),
(44904,11,2,'Zen''Kiki is in control!',12,0,100,0,0,0,0,44877,0,'Zen''Kiki - Bear Form'),
(44904,12,0,'A little help here...',12,0,100,0,0,0,0,44893,0,'Zen''Kiki - Aquatic Form'),
(44904,12,1,'I seem to be... stuck...',12,0,100,0,0,0,0,44894,0,'Zen''Kiki - Aquatic Form');

-- The four form scripts (In Combat - Random Script of 44904, id 4). Delays are from the previous row.
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 44904;
DELETE FROM `smart_scripts` WHERE `source_type` = 9 AND `entryorguid` IN (4490400, 4490401, 4490402, 4490403);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4490400,9,0,0,0,0,100,0,1,1,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Set Phase 2'),
(4490400,9,1,0,0,0,100,0,1,1,0,0,0,11,82815,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Moonkin Form'),
(4490400,9,2,0,0,0,100,0,1,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 0'),
(4490400,9,3,0,0,0,100,0,1000,1000,0,0,0,11,83091,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Moonfire Spam (on himself)'),
(4490400,9,4,0,0,0,100,0,500,500,0,0,0,1,9,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 9'),
(4490400,9,5,0,0,0,100,0,1500,1500,0,0,0,1,9,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 9'),
(4490400,9,6,0,0,0,100,0,1500,1500,0,0,0,1,9,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 9'),
(4490401,9,0,0,0,0,100,0,1,1,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Set Phase 2'),
(4490401,9,1,0,0,0,100,0,1,1,0,0,0,11,83452,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Cat Form'),
(4490401,9,2,0,0,0,100,0,1,1,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 1'),
(4490401,9,3,0,0,0,100,0,1000,1000,0,0,0,11,83453,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Pounce'),
(4490401,9,4,0,0,0,100,0,1,1,0,0,0,1,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 10'),
(4490401,9,5,0,0,0,100,0,2000,2000,0,0,0,11,83453,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Pounce'),
(4490401,9,6,0,0,0,100,0,1,1,0,0,0,1,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 10'),
(4490401,9,7,0,0,0,100,0,2000,2000,0,0,0,11,83453,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Pounce'),
(4490401,9,8,0,0,0,100,0,1,1,0,0,0,1,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 10'),
(4490402,9,0,0,0,0,100,0,1,1,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Set Phase 2'),
(4490402,9,1,0,0,0,100,0,1,1,0,0,0,11,83093,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Bear Form'),
(4490402,9,2,0,0,0,100,0,1,1,0,0,0,11,46598,0,0,0,0,0,19,44555,5,0,0,0,0,0,0,'Zen''Kiki - Script - Ride Bear Form Offset Vehicle'),
(4490402,9,3,0,0,0,100,0,1,1,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 2'),
(4490402,9,4,0,0,0,100,0,1500,1500,0,0,0,1,11,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 11'),
(4490402,9,5,0,0,0,100,0,2000,2000,0,0,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 4'),
(4490402,9,6,0,0,0,100,0,2000,2000,0,0,0,1,11,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 11'),
(4490402,9,7,0,0,0,100,0,2000,2000,0,0,0,1,5,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 5'),
(4490403,9,0,0,0,0,100,0,1,1,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Set Phase 2'),
(4490403,9,1,0,0,0,100,0,1,1,0,0,0,11,83085,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Cast Aquatic Form'),
(4490403,9,2,0,0,0,100,0,1,1,0,0,0,11,46598,0,0,0,0,0,19,44552,5,0,0,0,0,0,0,'Zen''Kiki - Script - Ride Aquatic Form Offset Vehicle'),
(4490403,9,3,0,0,0,100,0,1,1,0,0,0,1,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 3'),
(4490403,9,4,0,0,0,100,0,1500,1500,0,0,0,1,12,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 12'),
(4490403,9,5,0,0,0,100,0,2000,2000,0,0,0,1,6,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 6'),
(4490403,9,6,0,0,0,100,0,3000,3000,0,0,0,1,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Zen''Kiki - Script - Say Line 7');

-- Static copies of the 10 s form vehicles
DELETE FROM `creature` WHERE `guid` IN (213805, 311591) AND `id` IN (44552, 44555);
