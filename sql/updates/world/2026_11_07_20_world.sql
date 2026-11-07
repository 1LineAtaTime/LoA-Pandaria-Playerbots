-- Eastpoint Tower: Nils Beerot's therapy session with Drull and Tog'thar (Wowhead quest 28620 comments).
-- Sourced: lines from broadcast_text 49325-49342. Opening, closing, order and "the replies aren't linked, it happens every so often": Wowhead 28620
-- comment (2010). Speakers of 49328-49331 (Drull), 49333, 49336, 49341 (Tog'thar): same comment. Nils's spot and facing (he faces his patients; mop had him
-- with his back to them): TDB 4.4.2 and 1210, guid 322632.
-- Ours: the 30 s - 2 min first delay, the 2-5 min repeat, the 6 s spacing, the talk emote on the closing line. Speakers inferred from text and id order:
-- 49326, 49327, 49332 (Drull: contiguous with his lines, 49332 is about Teddy), 49338 (Tog'thar).
UPDATE `creature` SET `position_x`=-583.775,`position_y`=-1069.7,`position_z`=65.7941,`orientation`=1.50258 WHERE `guid`=175852 AND `id`=49201;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=49201;
DELETE FROM `creature_text` WHERE `CreatureID` IN (49201,2239,2238);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(49201,0,0,'It has now been six weeks since your release. Are you ready to have the shackles removed?',12,0,100,0,0,0,0,49325,0,'Nils Beerot - therapy opening'),
(49201,1,0,'My official diagnosis is that you are both idiots! IDIOTS I SAY!',12,0,100,1,0,0,0,49342,0,'Nils Beerot - therapy closing'),
(2239,0,0,'I need time to think. It... it''s too soon.',12,0,100,0,0,0,0,49326,0,'Drull - therapy (speaker inferred)'),
(2239,0,1,'Six weeks? I spent six years in Durnholde! You cannot possibly expect me to just throw aside something that has been a part of my life for so long!',12,0,100,0,0,0,0,49327,0,'Drull - therapy (speaker inferred)'),
(2239,0,2,'What I want to know is this: WHY DO YOU CARE IF I HAVE A BALL AND CHAIN SHACKLED TO MY ANKLE!?',12,0,100,0,0,0,0,49328,0,'Drull - therapy'),
(2239,0,3,'Can''t you just see me for me, doc? Why does it always have to be about Teddy?',12,0,100,0,0,0,0,49329,0,'Drull - therapy'),
(2239,0,4,'NO, DAMN IT! For the hundredth time, NO!',12,0,100,0,0,0,0,49330,0,'Drull - therapy'),
(2239,0,5,'You''ll be free to take it off when I die.',12,0,100,0,0,0,0,49331,0,'Drull - therapy'),
(2239,0,6,'You know, when I was first captured I hated Teddy. He dug into my skin and prevented me from moving around freely. But let me tell you something, doc; after six years with the guy I can honestly say that I have feelings for him.',12,0,100,0,0,0,0,49332,0,'Drull - therapy (speaker inferred)'),
(2238,0,0,'I''m with Drull, doc. Laverne and I are happy together. Why try and separate us?',12,0,100,0,0,0,0,49333,0,'Tog''thar - therapy'),
(2238,0,1,'And we don''t appreciate you refering to our friends as a ''ball and chain,'' doc.',12,0,100,0,0,0,0,49336,0,'Tog''thar - therapy'),
(2238,0,2,'I''d go back if I could. I had a life in Durnholde. Towards the end there I was beaten maybe once a week at MOST.',12,0,100,0,0,0,0,49338,0,'Tog''thar - therapy (speaker inferred)'),
(2238,0,3,'Frankly, I''m getting a little tired of these therapy sessions. We''re fine! Just let us go.',12,0,100,0,0,0,0,49341,0,'Tog''thar - therapy');
DELETE FROM `smart_scripts` WHERE (`entryorguid`=49201 AND `source_type`=0) OR (`entryorguid`=4920100 AND `source_type`=9);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`target_type`,`target_param1`,`target_param2`,`comment`) VALUES
(49201,0,0,0,1,0,100,0,30000,120000,120000,300000,80,4920100,0,0,1,0,0,'Nils Beerot - OOC - Start therapy session'),
(4920100,9,0,0,0,0,100,0,0,0,0,0,1,0,0,0,1,0,0,'Nils Beerot - Script - Say opening'),
(4920100,9,1,0,0,0,100,0,6000,6000,0,0,84,0,0,0,19,2239,20,'Nils Beerot - Script - Drull answers'),
(4920100,9,2,0,0,0,100,0,6000,6000,0,0,84,0,0,0,19,2238,20,'Nils Beerot - Script - Tog''thar answers'),
(4920100,9,3,0,0,0,100,0,6000,6000,0,0,1,1,0,0,1,0,0,'Nils Beerot - Script - Say diagnosis');
