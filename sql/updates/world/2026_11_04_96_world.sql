-- Hillsbrad Foothills: Stormpike Rendezvous (28348) plays in the Orkus Camera, as in 5.4.8 (zone_hillsbrad_sludge_stormpike.cpp)
-- LoA's old SmartAI played the scene on the world: Kingslayer Orkus 48218 summoned the actors (visible to everyone in his phase)
-- 30-50 yd behind a player standing at him, and set Vanndar and Balinda (level 85 elites, Stormpike faction) on the scene's Orkus;
-- a creature entering combat loses its immunity to players, so they then attacked the player. The 5.4.8 spells are a camera
-- scene: 89971 Summon Orkus Camera (48395, sniffed vehicle 1325) seats the player with 89970 Orkus Camera (phase 343 "Stormpike
-- Rendezvous", quest invisibility 4), and 89987 Cancel Cinematic teleports the player back and completes the quest.
-- Sourced: the spells and phase (5.4.8 DBC), the lines (broadcast_text 48624-48647), the scene's order (warcraft.wiki.gg,
-- Wowhead comments). Ours: the camera's spot (89971) and the end spot in front of Orkus (89987); see the script for the rest.

-- Orkus keeps his SmartAI (the drowning Orkus, the Road to Purgation gossip); the C++ script derives from it and starts the scene
UPDATE `creature_template` SET `AIName`='SmartAI', `ScriptName`='npc_hillsbrad_kingslayer_orkus_southshore' WHERE `entry`=48218;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48218 AND `source_type`=0 AND `id` IN (0,1);
DELETE FROM `smart_scripts` WHERE `entryorguid`=4821800 AND `source_type`=9;

-- The camera and the actors (the actors are the player's private summons, passive and immune: they never fight)
UPDATE `creature_template` SET `VehicleId`=1325, `AIName`='', `ScriptName`='npc_hillsbrad_orkus_camera' WHERE `entry`=48395;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_scene_actor' WHERE `entry` IN (48396,48397,48403,48407);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (48396,48397,48403,48407) AND `source_type`=0;

-- The ride: hidden server-side instead of the invisibility, which hid the whole world from the rider
DELETE FROM `spell_script_names` WHERE `spell_id`=89970;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(89970,'spell_hillsbrad_scene_camera_ride');

-- The camera 10 yd above the shore between Orkus and the docks, looking at the docks with Orkus 312261 and Kasha 312260 at its
-- back (they stay in sight in the scene phase); Cancel Cinematic sets the player down in front of Orkus (ours)
DELETE FROM `spell_target_position` WHERE `id` IN (89971,89987) AND `effIndex`=0;
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(89971,0,0,-920.0,-575.0,10.0,0.37),
(89987,0,0,-925.6,-581.6,1.8,3.467);

-- The scene's lines, to the scene's player only (TextRange 5): the actors are the player's private summons. The narration comes
-- from the camera as boss emotes, like the other Hillsbrad cameras'.
DELETE FROM `creature_text` WHERE `CreatureID` IN (48395,48397,48403,48407);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48395,0,0,'|cFFFFFFFFI had just finished making use of the facilities on the docks when...|r',41,0,100,0,0,0,0,48624,5,'Orkus Camera - Stormpike Rendezvous'),
(48395,1,0,'$B$B|cFFFFFFFFAs I emerged from the outhouse, I saw them! It should be noted that Orkus prefers silence when he is doing his duty.|r$B',41,0,100,0,0,0,0,48626,5,'Orkus Camera - Stormpike Rendezvous'),
(48397,0,0,'Ugh, the lok''tar must have been cooking up a turd - which would explain the smell.',12,0,100,0,0,0,0,48625,5,'Kingslayer Orkus - Stormpike Rendezvous'),
(48397,1,0,'By Hellscream''s axe!',12,0,100,5,0,0,0,48627,5,'Kingslayer Orkus - Stormpike Rendezvous'),
(48397,2,0,'You face Kingslayer Orkus now, dwarf!',12,0,100,5,0,0,0,48643,5,'Kingslayer Orkus - Stormpike Rendezvous'),
(48397,3,0,'KASHA! Your master needs you!',14,0,100,22,0,0,0,48644,5,'Kingslayer Orkus - Stormpike Rendezvous'),
(48403,0,0,'Oy, ''Linda, where are we meetin'' up with this Ivar Bloodfang fella? An'' what in Bronzebeard''s name is that terrible smell?',12,0,100,1,0,0,0,48635,5,'Vanndar Stormpike - Stormpike Rendezvous'),
(48403,1,0,'Look there, ''Linda! What in the...',12,0,100,0,0,0,0,48641,5,'Vanndar Stormpike - Stormpike Rendezvous'),
(48403,2,0,'ENOUGH TOYIN'' AROUND!!!',14,0,100,0,0,0,0,48645,5,'Vanndar Stormpike - Stormpike Rendezvous'),
(48403,3,0,'Let''s get outta here, ''Linda. We dun wanna draw any unwanted attention!',12,0,100,0,0,0,0,48646,5,'Vanndar Stormpike - Stormpike Rendezvous'),
(48407,0,0,'They are massing at Purgation Isle, just west of us here, off the coast. We are to meet them there, general.',12,0,100,1,0,0,0,48639,5,'Captain Balinda Stonehearth - Stormpike Rendezvous'),
(48407,1,0,'As for the smell... *Balinda sniffs at the air* I think it''s coming from over there...',12,0,100,1,0,0,0,48640,5,'Captain Balinda Stonehearth - Stormpike Rendezvous'),
(48407,2,0,'Aye, general!',12,0,100,0,0,0,0,48647,5,'Captain Balinda Stonehearth - Stormpike Rendezvous');

-- The quest stays an event quest: Cancel Cinematic (or the script, if it cannot hit) completes it
UPDATE `quest_template_addon` SET `SpecialFlags`=2 WHERE `ID`=28348;
