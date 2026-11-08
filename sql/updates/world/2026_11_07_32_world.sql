-- Deception and Trickery 28604: what the Stormpike Trainees say when they answer Water! 91399 (Water Barrel 49036).
-- Sourced: the lines (broadcast 49142-49144 and 49145-49148); the quest's reward text says "They won't die immediately, but they WILL die!".
-- Ours: the attribution of the lines to 48912 (by content; no source links them) and the timing of the script (walk, drink, sicken, die).
DELETE FROM `creature_text` WHERE `CreatureID`=48912;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48912,0,0,'Thank the Light! I''m parched!',12,0,34,0,0,0,0,49142,0,'Stormpike Trainee - drinks'),
(48912,0,1,'Ah, yer a lifesaver, lad!',12,0,33,0,0,0,0,49143,0,'Stormpike Trainee - drinks'),
(48912,0,2,'I feel like I could drink a lake!',12,0,33,0,0,0,0,49144,0,'Stormpike Trainee - drinks'),
(48912,1,0,'Ugh... I don''t feel so good.',12,0,25,0,0,0,0,49145,0,'Stormpike Trainee - poisoned'),
(48912,1,1,'So tired...',12,0,25,0,0,0,0,49146,0,'Stormpike Trainee - poisoned'),
(48912,1,2,'ARGH! THROAT ON FIRE!',12,0,25,0,0,0,0,49147,0,'Stormpike Trainee - poisoned'),
(48912,1,3,'Wha.. ungh...',12,0,25,0,0,0,0,49148,0,'Stormpike Trainee - poisoned');

DELETE FROM `smart_scripts` WHERE `entryorguid`=4891200 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(4891200,9,0,0,0,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Set Passive'),
(4891200,9,1,0,0,0,100,0,0,0,0,0,0,69,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Move to Barrel'),
(4891200,9,2,0,0,0,100,0,4000,4000,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Say Drink'),
(4891200,9,3,0,0,0,100,0,0,0,0,0,0,5,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Emote Eat (drink)'),
(4891200,9,4,0,0,0,100,0,6000,6000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Say Poisoned'),
(4891200,9,5,0,0,0,100,0,4000,4000,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Die');
