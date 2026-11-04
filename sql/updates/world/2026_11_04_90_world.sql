-- *Gurgle* HELP! *Gurgle* (28345): Kingslayer Orkus drowning in the shallow water off the Ruins of Southshore.
--
-- Sourced:
-- - warcraft.wiki.gg (quest 28345): objective "Help Kingslayer Orkus get out of the water". Its notes list, in order,
--   "Explain to Orkus that he is standing in shallow water." (48606, no speaker), "I will not survive! ... HELP ME!" (48604),
--   "Your feet are touching the ground, Orkus. This is shallow water. Just stand up and walk to shore." (48605, marked with
--   the gossip icon), Orkus says "Oh... OH! YES! Orkus has regained his senses! ..." (48601), yells "Kasha! Come to me! I am
--   fine!" (48602), Kasha "flies to shore, but collapses when landing", and he yells "KASHA! NO!!" (48603). Kingslayer Orkus
--   page: the player finds him "on the verge of drowning", tells him the water is shallow, and he goes back to the shore.
-- - Quest data (15595): AreaDescription "Orkus Safely Escorted Out of the Water" (Wowhead MoP Classic shows it as the
--   objective), i.e. an event objective: SpecialFlags 2 (EXPLORATION_OR_EVENT), so the quest is no longer complete on accept.
-- - Wowhead (MoP Classic and Cataclysm Classic) has one Orkus 48218 pin at 49.8 74.2 = (-924.1, -571.5): in 0.86 yd of water
--   (terrain -0.86, sea level 0), 12 yd off the old spawn on the dry shore. Wowhead's quotes for 48218 are 48601 (says),
--   48602 and 48603 (yells); 48604 "I will not survive!$B$B*GULP*$B$BHELP ME!" is not among them and has $B breaks: it is his
--   gossip text while drowning.
-- - 48218 has Drowning 89662 (permanent dummy aura with visual 19046) as its sniffed spell1: his drowning state.
-- Neither TDB (442, 1210) spawns Orkus at Southshore or has his gossip, texts or SmartAI.
--
-- Ours (no source shows how retail kept the drowning Orkus apart from the one ashore; Phase.dbc has no Southshore phase):
-- - Two Orkus spawns, phased by 28345: the drowning one (new guid 4100050, in the water with Drowning) in phase 186 "Quest
--   Zone-Specific 11" while 28345 is none/in progress/failed (mask 41), the one ashore (312261, kneeling beside the collapsed
--   Kasha, as before) in phase 187 "Quest Zone-Specific 12" once 28345 is complete or rewarded (mask 66). Both phases are
--   generic and unused in mop_world; their granting spells are in Northrend and Vashj'ir.
-- - Which line is which: the option text is 48606, which carries the gossip icon tag; 48605 has male and female texts, as a
--   player's line does, and the player says it (SIMPLE_TALK on the player, personal) on picking the option.
-- - The rescue: the player says 48605, Orkus loses Drowning, says 48601 and runs to 312261's spot, yells 48602 and summons
--   a Kasha (48217) over the sea who flies to the collapsed Kasha's spot (312260) and lies down; he yells 48603 and kneels.
--   The rescuer is credited about 13 s after the 48602 yell (Kasha has landed), which swaps the drowning Orkus and the
--   summoned Kasha for 312261 and 312260 on the same spots (the turn-in and Stormpike Rendezvous' scene stay on 312261).
--   About a minute later he walks back into the water.
-- - SmartAI event phases of the drowning Orkus: 1 in the water (the option starts the rescue), 2 busy (on his way ashore or
--   back: the option only closes the gossip, so nobody is credited while he is in the water or walking), 3 ashore (the
--   option credits whoever picks it, each player for himself; the rescuer too, if he picks it again before the timed credit
--   once Kasha has landed).
-- - Lines: 48605 is personal (only the rescuer sees it). Orkus' 48601-48603 keep the normal say/yell range: CreatureTextMgr
--   sends them through PlayerDistWorker, which checks WorldObject::InSamePhase(WorldObject*) = phase mask AND IsPhased
--   (phase ids), so only players who see this Orkus (phase 186) hear him, and players beside 312261 (phase 187) do not.
--   Bystanders in 186 watch the shared scene and hear it.
-- - Kasha (48217) gets SmartAI for the summon (JUST_SUMMONED, point 1). Phase 200 now also waits for 28345 to be complete,
--   so the collapsed Kasha no longer lies there before Orkus called her.
-- - Gossip texts: 17474 (48604) for the drowning Orkus; 17476 "I will not leave Kasha's side" only once 28345 is complete.
--   The id 17474 (the gap between 12429's 17473 and 12430's 17475) is a guess at the retail one.

-- The quest waits for the rescue
UPDATE `quest_template_addon` SET `SpecialFlags`=2 WHERE `ID`=28345;

-- The drowning Orkus in the water, and the Orkus ashore
DELETE FROM `creature` WHERE `guid`=4100050;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(4100050,48218,0,267,271,1,1,186,0,35625,0,-924.1,-571.5,-0.86,4.32813,300,0,0,0,30951,0,0,0,0,0,0,0,'',0,0);
DELETE FROM `creature_addon` WHERE `guid`=4100050;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(4100050,0,0,0,0,0,0,0,0,0,0,0,0,0,'89662');
UPDATE `creature` SET `phaseId`=187, `phaseGroup`=0 WHERE `guid`=312261 AND `id`=48218;

DELETE FROM `phase_area` WHERE `AreaId`=267 AND `PhaseId` IN (186,187);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(267,186,'Hillsbrad Foothills - Kingslayer Orkus 48218 drowning off Southshore until *Gurgle* HELP! *Gurgle* is complete'),
(267,187,'Hillsbrad Foothills - Kingslayer Orkus 48218 ashore at Southshore once *Gurgle* HELP! *Gurgle* is complete');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup` IN (186,187) AND `SourceEntry`=267;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=200 AND `SourceEntry`=267 AND `ConditionValue1`=28345;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup`=12430 AND `SourceEntry` IN (17474,17476) AND `ConditionValue1`=28345;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12430 AND `SourceEntry`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,186,267,0,0,47,0,28345,41,0,0,0,0,'','Hillsbrad Foothills - phase 186 (drowning Orkus) while *Gurgle* HELP! *Gurgle* is not complete'),
(26,187,267,0,0,47,0,28345,66,0,0,0,0,'','Hillsbrad Foothills - phase 187 (Orkus ashore) once *Gurgle* HELP! *Gurgle* is complete or rewarded'),
(26,200,267,0,0,47,0,28345,66,0,0,0,0,'','Hillsbrad Foothills - phase 200 (collapsed Kasha) only after *Gurgle* HELP! *Gurgle* is complete'),
(14,12430,17474,0,0,47,0,28345,41,0,0,0,0,'','Kingslayer Orkus: "I will not survive!" while *Gurgle* HELP! *Gurgle* is not complete'),
(14,12430,17476,0,0,47,0,28345,66,0,0,0,0,'','Kingslayer Orkus: "I will not leave Kasha''s side" only once he is ashore'),
(15,12430,1,0,0,47,0,28345,8,0,0,0,0,'','Kingslayer Orkus: "Explain to Orkus that he is standing in shallow water" while *Gurgle* HELP! *Gurgle* is in progress');

-- Gossip: the drowning text and the option
DELETE FROM `npc_text` WHERE `ID`=17474;
INSERT INTO `npc_text` (`ID`,`text0_0`,`text0_1`,`BroadcastTextID0`,`lang0`,`Probability0`,`VerifiedBuild`) VALUES
(17474,'I will not survive!$B$B*GULP*$B$BHELP ME!','',48604,0,1,0);
DELETE FROM `gossip_menu` WHERE `MenuID`=12430 AND `TextID`=17474;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES
(12430,17474,0);
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12430 AND `OptionID`=1;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12430,1,0,'Explain to Orkus that he is standing in shallow water.|TInterface\\GossipFrame\\GOSSIPGOSSIPICON.blp:32|t',48606,1,1,0,0,0,0,'',0,0);

-- His lines (types from Wowhead's quotes), and the player's line (group 3: said by the player through SIMPLE_TALK, personal)
DELETE FROM `creature_text` WHERE `CreatureID`=48218 AND `GroupID` IN (0,1,2,3);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48218,0,0,'Oh... OH! YES! Orkus has regained his senses! The foul human mage must have cursed Orkus'' mind. Quickly we must get to shore! Kasha is hurt!',12,0,100,0,0,0,0,48601,0,'Kingslayer Orkus - *Gurgle* HELP! *Gurgle*: rescued'),
(48218,1,0,'Kasha! Come to me! I am fine!',14,0,100,0,0,0,0,48602,0,'Kingslayer Orkus - *Gurgle* HELP! *Gurgle*: ashore, calls Kasha'),
(48218,2,0,'KASHA! NO!!',14,0,100,0,0,0,0,48603,0,'Kingslayer Orkus - *Gurgle* HELP! *Gurgle*: Kasha collapses'),
(48218,3,0,'Your feet are touching the ground, Orkus. This is shallow water. Just stand up and walk to shore.',12,0,100,0,0,0,0,48605,5,'Kingslayer Orkus - *Gurgle* HELP! *Gurgle*: the player''s line (said by the player)');

-- The drowning Orkus (this spawn only; the entry's SmartAI stays with the Orkus ashore)
DELETE FROM `smart_scripts` WHERE `entryorguid`=-4100050 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4821801,4821802) AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-4100050,0,0,1,25,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Reset - Set Event Phase 1 (in the water)'),
(-4100050,0,1,0,61,0,100,0,0,0,0,0,0,59,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Set Walk'),
-- Ashore (phase 3): whoever picks the option is credited at once, each player for himself
(-4100050,0,2,3,62,4,100,0,12430,1,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Gossip Option 1 Selected (ashore) - Close Gossip'),
(-4100050,0,3,0,61,0,100,0,0,0,0,0,0,15,28345,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Quest Credit *Gurgle* HELP! *Gurgle*'),
-- On his way ashore or back (phase 2): the option does nothing. Ids below the phase 1 handler, which switches to phase 2
(-4100050,0,4,0,62,2,100,0,12430,1,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Gossip Option 1 Selected (busy) - Close Gossip'),
-- In the water (phase 1): the rescue
(-4100050,0,5,6,62,1,100,0,12430,1,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Gossip Option 1 Selected - Close Gossip'),
(-4100050,0,6,7,61,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Store Invoker'),
(-4100050,0,7,8,61,0,100,0,0,0,0,0,0,84,3,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Invoker Says Line 3 (Your feet are touching the ground)'),
(-4100050,0,8,9,61,0,100,0,0,0,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Set Event Phase 2'),
(-4100050,0,9,0,61,0,100,0,0,0,0,0,0,80,4821801,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Run Script (rescued)'),
(-4100050,0,10,11,34,2,100,0,8,1,0,0,0,22,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Point 1 Reached (ashore) - Set Event Phase 3'),
(-4100050,0,11,0,61,0,100,0,0,0,0,0,0,80,4821802,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Run Script (calls Kasha)'),
(-4100050,0,12,13,38,4,100,0,1,1,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Data Set 1 1 (Kasha landed) - Say Line 2'),
(-4100050,0,13,0,61,0,100,0,0,0,0,0,0,90,8,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Kneel'),
(-4100050,0,14,15,34,2,100,0,8,2,0,0,0,66,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - On Point 2 Reached (back in the water) - Set Home Orientation'),
(-4100050,0,15,16,61,0,100,0,0,0,0,0,0,75,89662,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Add Aura ''Drowning'''),
(-4100050,0,16,0,61,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Linked - Set Event Phase 1'),
-- Rescued: stands up and goes ashore
(4821801,9,0,0,0,0,100,0,0,0,0,0,0,28,89662,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Remove Aura ''Drowning'''),
(4821801,9,1,0,0,0,100,0,0,0,0,0,0,66,0,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Face the Player'),
(4821801,9,2,0,0,0,100,0,3000,3000,0,0,0,1,0,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Say Line 0 (after the player''s line)'),
(4821801,9,3,0,0,0,100,0,5000,5000,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Set Run'),
(4821801,9,4,0,0,0,100,0,0,0,0,0,0,69,1,0,0,0,0,0,8,0,0,0,0,-928.592,-582.609,1.55523,0,'Kingslayer Orkus (drowning) - Script - Move to the Shore (the spot of the Orkus ashore, 312261)'),
-- Ashore: calls Kasha, credits the rescuer once she has come down (13 s after the yell), then back into the water
(4821802,9,0,0,0,0,100,0,0,0,0,0,0,59,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Set Walk'),
(4821802,9,1,0,0,0,100,0,0,0,0,0,0,66,0,0,0,0,0,0,8,0,0,0,0,0,0,0,3.36848,'Kingslayer Orkus (drowning) - Script - Set Orientation (as 312261)'),
(4821802,9,2,0,0,0,100,0,1000,1000,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Say Line 1'),
(4821802,9,3,0,0,0,100,0,1000,1000,0,0,0,12,48217,3,75000,0,0,0,8,0,0,0,0,-945,-545,20,5.03,'Kingslayer Orkus (drowning) - Script - Summon Kasha over the sea'),
(4821802,9,4,0,0,0,100,0,12000,12000,0,0,0,15,28345,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Quest Credit *Gurgle* HELP! *Gurgle* (stored player)'),
(4821802,9,5,0,0,0,100,0,60000,60000,0,0,0,91,8,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Stand Up'),
(4821802,9,6,0,0,0,100,0,2000,2000,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kingslayer Orkus (drowning) - Script - Set Event Phase 2 (leaving the shore)'),
(4821802,9,7,0,0,0,100,0,0,0,0,0,0,69,2,0,0,0,0,0,8,0,0,0,0,-924.1,-571.5,-0.86,0,'Kingslayer Orkus (drowning) - Script - Move back into the Water');

-- Kasha: the one Orkus calls flies to the shore and collapses where the collapsed Kasha (312260) lies. JUST_SUMMONED and
-- point 1 only concern that summon; the two static Kashas (312260 lying, 312689 on her waypoint path) run neither.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=48217;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48217 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48217,0,0,1,54,0,100,0,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kasha - On Just Summoned - Set Fly'),
(48217,0,1,2,61,0,100,0,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kasha - Linked - Set Run'),
(48217,0,2,0,61,0,100,0,0,0,0,0,0,69,1,0,0,0,0,0,8,0,0,0,0,-932.118,-584.066,1.50794,0,'Kasha - Linked - Fly to the Shore'),
(48217,0,3,4,34,0,100,0,8,1,0,0,0,60,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kasha - On Point 1 Reached - Set Fly Off'),
(48217,0,4,5,61,0,100,0,0,0,0,0,0,66,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0.802851,'Kasha - Linked - Set Orientation (as 312260)'),
(48217,0,5,6,61,0,100,0,0,0,0,0,0,90,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Kasha - Linked - Collapse (Stand State Dead)'),
(48217,0,6,0,61,0,100,0,0,0,0,0,0,45,1,1,0,0,0,0,23,0,0,0,0,0,0,0,0,'Kasha - Linked - Set Data 1 1 on Kingslayer Orkus');
