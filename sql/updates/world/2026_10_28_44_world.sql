-- Hillsbrad Foothills: the scripted events of the Sludge Fields and of the Stormpike quests (zone_hillsbrad_sludge_stormpike.cpp)
-- Sourced: the spells and actors (5.4.8 DBC), every line (broadcast_text), the camera start of 89144 (already in mop, as in the
-- Cataclysm Classic TDB), Stillwater's combat spells (his SmartAI in the same TDB). Ours: paths, timings, actor spots, the end
-- spots of the Cancel Cinematic spells, and the vehicles of the three Stormpike cameras (see below).

-- The quests stay event quests: the scene's end spell (or the script, if it cannot hit) completes them.
UPDATE `quest_template_addon` SET `SpecialFlags`=2 WHERE `ID` IN (28192,28506,28600,28616);

-- ---------------------------------------------------------------------------------------------------------------------------
-- No One Here Gets Out Alive (28192): Warden Stillwater 47781 casts nothing in mop, so the quest could not be completed.
-- Accepting it now starts the camera tour (Summon Sludge Fields Camera 89144, ride aura 89149 with phase 326, where the actor
-- Stillwater 47834 stands since 2026_10_28_41); Teleport to Sludge Fields Lab 89148 ends it and completes the quest.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_warden_stillwater_sludge' WHERE `entry`=47781;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_sludge_fields_camera' WHERE `entry`=47928;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (47781,47928) AND `source_type`=0;

DELETE FROM `creature_text` WHERE `CreatureID`=47928;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(47928,0,0,'|cFFFFFFFFThere''s trouble at the Sludge Fields...|r',41,0,100,0,0,0,0,48139,5,'Sludge Fields Camera - No One Here Gets Out Alive');

-- ---------------------------------------------------------------------------------------------------------------------------
-- A Blight Upon the Land (28237): Stillwater 48080 now runs on C++ (npc_hillsbrad_warden_stillwater_blight) instead of the
-- SmartAI of 2026_10_28_41: Lydon passes the sentence (48309-48311) before the fight, and the blow that would kill Stillwater
-- gives him his new form (the model of Warden Stillwater 48103) and SMASH!, which throws the players back and kills his
-- followers. Arnauld 48081 and the Sludge Guards 48083 keep their SmartAI from 2026_10_28_41 (data 1 1: attack).
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_warden_stillwater_blight' WHERE `entry`=48080;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48080 AND `source_type`=0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=48080;

DELETE FROM `creature_text` WHERE `CreatureID`=48080;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48080,0,0,'Lydon!? Alive?',12,0,100,0,0,0,0,48306,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,1,0,'Now I know where your allegiances lie, $n!',12,0,100,0,0,0,0,48307,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,2,0,'And you, Jenny? I thought we had a connection. You''ve broken my heart for the last time.',12,0,100,0,0,0,0,48308,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,3,0,'I AM A THREAT? I AM DERANGED?!',14,0,100,0,0,0,0,48312,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,4,0,'What I''ve created here is an evolution! THIS is how the Forsaken will thrive, not through some magical winged fairy sprinkling happy dust on corpses!',14,0,100,0,0,0,0,48313,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,5,0,'Kill them all! They will serve as subjects for our next study!',14,0,100,0,0,0,0,48314,0,'Warden Stillwater - A Blight Upon the Land'),
(48080,6,0,'You think that you''ve won? THINK AGAIN! Stand before my new form and tremble!',14,0,100,0,0,0,0,48324,0,'Warden Stillwater - New form');

DELETE FROM `creature_text` WHERE `CreatureID`=48020 AND `GroupID` BETWEEN 10 AND 12;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48020,10,0,'Warden Stillwater, you are deranged and a threat to our way of life. By authority of the Dark Lady herself, I hereby impose upon you the harshest penalty allowed under Forsaken law - true death.',12,0,100,0,0,0,0,48309,0,'Master Apothecary Lydon - A Blight Upon the Land'),
(48020,11,0,'That goes for all of you traitorous scum who supported this fool!',12,0,100,0,0,0,0,48310,0,'Master Apothecary Lydon - A Blight Upon the Land'),
(48020,12,0,'KNEEL!',14,0,100,0,0,0,0,48311,0,'Master Apothecary Lydon - A Blight Upon the Land');

-- SMASH! (89450: damage and knockback, effect 2 instakill, all on area-entry targets) had no conditions, so it hit everything
-- around Stillwater, the player and Stillwater himself included. Damage and knockback for players, instakill for his followers.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=89450;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,3,89450,0,0,32,0,16,0,0,0,0,0,'','SMASH! - damage and knock back players'),
(13,4,89450,0,0,31,0,3,48081,0,0,0,0,'','SMASH! - kill Flesh-Shaper Arnauld'),
(13,4,89450,0,1,31,0,3,48083,0,0,0,0,'','SMASH! - kill the Sludge Guards');

-- ---------------------------------------------------------------------------------------------------------------------------
-- Matters of Loyalty (28600), March of the Stormpike (28506), Stormpike Apocalypse (28616): watch-the-scene quests whose summon
-- spells nothing cast, so they could never be completed. Accepting them now plays their scenes: Summon Aid of the Frostwolf
-- Camera 91356 (Voggah Deathgrip), Summon Tarren Mill Camera 90713 and Summon Stormpike Apocalypse 91608 (High Warlord Cromush),
-- ended by their Cancel Cinematic spells 91358, 90711 and 91607 (teleport back, QUEST_COMPLETE).
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_voggah_deathgrip' WHERE `entry`=13817;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_high_warlord_cromush' WHERE `entry`=48545;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (13817,48545) AND `source_type`=0;

-- The three cameras had no vehicle, so their summon spells could not seat the player. Their vehicles are not in any source;
-- these are unused 5.4.8 camera vehicles (camera seat 0 with the Sludge Fields camera's seat flags 0x42100200) whose ids sit
-- next to each scene's other vehicles: 1343 (four seats, next to the Confiscated Artillery 48699 of 1345), 1368 (one seat, next
-- to Galvangar 1367 and the wheelchair 1363), 1379 (four seats, as the scene's Seat 02-04 spells 91658-91663 expect). Inferred.
UPDATE `creature_template` SET `VehicleId`=1343, `AIName`='', `ScriptName`='npc_hillsbrad_tarren_mill_camera' WHERE `entry`=48696;
UPDATE `creature_template` SET `VehicleId`=1368, `AIName`='', `ScriptName`='npc_hillsbrad_aid_of_the_frostwolf_camera' WHERE `entry`=48986;
UPDATE `creature_template` SET `VehicleId`=1379, `AIName`='', `ScriptName`='npc_hillsbrad_stormpike_apocalypse_camera' WHERE `entry`=49152;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_cloned_image' WHERE `entry`=48700;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_scene_actor' WHERE `entry` IN (48969,48971,48980,49153,49165,49166,49168,49169,49171);

-- Drek'Thar 48969 is an actor of Summon Drek'thar/Crew (with Galvangar and the wheelchair); his static spawn next to Voggah is a
-- sniffed copy of the scene (retail has none).
DELETE FROM `creature_addon` WHERE `guid`=375621;
DELETE FROM `creature` WHERE `guid`=375621 AND `id`=48969;

-- Cancel Cinematic sets the player down where the scene started: in front of Voggah, or of Cromush (ours).
DELETE FROM `spell_target_position` WHERE `id` IN (90711,91358,91607) AND `effIndex`=0;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(90711,0,0,-17.96,-922.06,54.88,4.7822),
(91358,0,0,371.14,-1000.64,109.35,4.7298),
(91607,0,0,-17.96,-922.06,54.88,4.7822);

-- Global Thermonuclear War 91617 (the pocket-nukes going off) hits area-entry targets: only the nukes and the two engineers.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=91617;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,3,91617,0,0,31,0,3,49153,0,0,0,0,'','Global Thermonuclear War - Goblin Pocket-Nuke'),
(13,3,91617,0,1,31,0,3,49168,0,0,0,0,'','Global Thermonuclear War - Goblin Pocket-Nuke'),
(13,3,91617,0,2,31,0,3,49169,0,0,0,0,'','Global Thermonuclear War - Goblin Pocket-Nuke'),
(13,3,91617,0,3,31,0,3,49171,0,0,0,0,'','Global Thermonuclear War - Goblin Pocket-Nuke'),
(13,3,91617,0,4,31,0,3,49165,0,0,0,0,'','Global Thermonuclear War - Magruder'),
(13,3,91617,0,5,31,0,3,49166,0,0,0,0,'','Global Thermonuclear War - Lieutenant Jan Lagrave');

-- The scenes' lines. The narration comes from the cameras as boss emotes (the white text in the middle of the screen); all
-- scene lines go to the scene's player only (TextRange 5), as the actors are the player's private summons. Cromush's yell
-- greets the player back from the scene: personal too, or every scene would yell over the whole Tarren Mill hub.
DELETE FROM `creature_text` WHERE `CreatureID` IN (48696,48969,49165,49166);
DELETE FROM `creature_text` WHERE `CreatureID`=48545 AND `GroupID`=10;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48969,0,0,'So you have come seeking our aid?',12,0,100,0,0,0,0,49089,5,'Drek''Thar - Matters of Loyalty'),
(48969,1,0,'%s coughs.',16,0,100,0,0,0,0,49090,5,'Drek''Thar - Matters of Loyalty'),
(48969,2,0,'I... *cough* I have been alive for a very long time. In that time I have seen and done terrible things.',12,0,100,0,0,0,0,49091,5,'Drek''Thar - Matters of Loyalty'),
(48969,3,0,'Things that still keep me awake at night.',12,0,100,0,0,0,0,49092,5,'Drek''Thar - Matters of Loyalty'),
(48969,4,0,'But these terrible things that I have done and the people that I have harmed - I know them... I face them... and I feel remorse for them.',12,0,100,0,0,0,0,49093,5,'Drek''Thar - Matters of Loyalty'),
(48969,5,0,'But the Forsaken. *cough* What do they feel?',12,0,100,0,0,0,0,49094,5,'Drek''Thar - Matters of Loyalty'),
(48969,6,0,'They ravage the land and destroy everything that they touch. How many lives have been lost to their vile poisons?',12,0,100,0,0,0,0,49095,5,'Drek''Thar - Matters of Loyalty'),
(48969,7,0,'How many innocents have fallen before the Forsaken war machine?',12,0,100,0,0,0,0,49096,5,'Drek''Thar - Matters of Loyalty'),
(48969,8,0,'Countless... countless lives... *cough*',12,0,100,0,0,0,0,49097,5,'Drek''Thar - Matters of Loyalty'),
(48969,9,0,'Yes... I have done terrible things, but nothing could ever be as terrible as lending aid to the Forsaken.',12,0,100,0,0,0,0,49098,5,'Drek''Thar - Matters of Loyalty'),
(48969,10,0,'You go back to that spineless orc who would not come see me and you tell him that the Frostwolf clan will not aid the Forsaken. Not now, not ever!',12,0,100,0,0,0,0,49099,5,'Drek''Thar - Matters of Loyalty'),
(48969,11,0,'BEGONE!',14,0,100,0,0,0,0,49100,5,'Drek''Thar - Matters of Loyalty'),
(48696,0,0,'|cFFFFFFFFSofera''s Naze, the first of the four fingers of Hillsbrad.|r',41,0,100,0,0,0,0,48893,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,1,0,'|cFFFFFFFFThe Stormpike have enlisted the help of SI:7 operatives. This is their scouting outpost.|r',41,0,100,0,0,0,0,48904,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,2,0,'|cFFFFFFFFCorrahn''s Dagger, the second of the four fingers of Hillsbrad.|r',41,0,100,0,0,0,0,48901,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,3,0,'|cFFFFFFFFThe Stormpike are using this as a forward base camp for their soldiers.|r',41,0,100,0,0,0,0,48906,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,4,0,'|cFFFFFFFFThe Headland, the third of the four fingers of Hillsbrad.|r',41,0,100,0,0,0,0,48902,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,5,0,'|cFFFFFFFFThey train their military here, teaching them new and exciting ways to kill us.|r',41,0,100,0,0,0,0,48907,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,6,0,'|cFFFFFFFFGavin''s Naze, the fourth and final finger of Hillsbrad.|r',41,0,100,0,0,0,0,48903,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,7,0,'|cFFFFFFFFMost well protected of the fingers. Their terrible siege engines are assembled here.|r',41,0,100,0,0,0,0,48908,5,'Tarren Mill Camera - March of the Stormpike'),
(48696,8,0,'|cFFFFFFFFTogether the fingers represent the grasp of Stormpike closing upon us. They must be destroyed.|r',41,0,100,0,0,0,0,48909,5,'Tarren Mill Camera - March of the Stormpike'),
(49166,0,0,'Ye can defuse it, right Magruder?!',12,0,100,0,0,0,0,49249,5,'Lieutenant Jan Lagrave - Stormpike Apocalypse'),
(49165,0,0,'Holy... What in the hell is this?',12,0,100,0,0,0,0,49250,5,'Magruder - Stormpike Apocalypse'),
(49165,1,0,'Are you kidding me? Look at all this crap! There''s like a million wires. I''m a three wire kind of guy...',12,0,100,0,0,0,0,49251,5,'Magruder - Stormpike Apocalypse'),
(49165,2,0,'RUN!!!!!!',14,0,100,0,0,0,0,49252,5,'Magruder - Stormpike Apocalypse'),
(48545,10,0,'LOK''TAR! HILLSBRAD IS OURS!',14,0,100,0,0,0,0,49255,5,'High Warlord Cromush - Stormpike Apocalypse');
