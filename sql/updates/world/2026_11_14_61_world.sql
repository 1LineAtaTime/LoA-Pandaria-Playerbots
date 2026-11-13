-- Eastern Plaguelands, Pamela's Doll (27384): a Ghost of the Past comes when a player nears or loots a doll part; Pamela thanks you.
--
-- Sourced:
--  * Approach: wiki Notes ("Approaching any doll part will summon a Ghost of the Past"), comments 2013-12-19 [+5], 2014-08-21 [+1],
--    OLD 2007-09-23 [+7], LATE 2019-11-25 [+3] (also without the quest); retail TDB template 176117 Data2 (radius) = 4.
--  * On loot as well: wiki patch 4.2.0 ("Picking up a part can still summon a Ghost"), comments 2012-07-04, 2012-09-07: the parts'
--    Data7 is the linked trap 176117 (spell 17132, summon 10940 for 5 min at the trap).
--  * The five static Ghosts of the Past are sniffed summons (each within 3 yd of a part with its exact orientation; neither TDB
--    spawns 10940): deleted. Frost Armor 12556 in the addon (both TDBs "17327 12556").
--  * Ghosts give no XP and no kill loot (both TDBs flags_extra 64 and LootID 0; OLD comments); pickpocket loot stays (retail).
--  * Parts respawn in 120 s (both TDBs; mop 300; comment 2013-02-21 [+52] about the slow respawn).
--  * Trap display 0 (mop's 299 is a rock; the 4.4.2 TDB and AzerothCore use 1287, which has no row in the 5.4.8
--    GameObjectDisplayInfo.dbc, and the spawn loader refuses a spawn whose display is not in it). The trap is also made
--    server-only (Data8 1, ours): clients are never sent it (GameObject::IsNeverVisibleFor), while the server's own trap
--    search and the chest's linked-trap search do not look at visibility.
--  * Pamela's two lines on reward (both TDBs: SmartAI event 20 -> script 1092600, broadcast 6416 and 6417).
-- Ours: the trap's respawn 300 s (= the ghost's summon time); the creature_text groups 70/71 (the TDBs use 4/5); the static Pamela's
-- Doll 176247 at Pamela's feet (guid 157566, in neither TDB: she asks for a doll that already lies beside her) is deleted.
-- The cap of one ghost per part is spell_darrowshire_pamelas_doll_trap (17132): the linked trap fires on every opening of a part's
-- loot window. The loot path needs the core fix "Core/GameObject: Look for a linked trap around its object" only for a looter standing
-- more than 5 yd from the trap (17132 has range 5); the approach path works without it.
DELETE FROM `creature_addon` WHERE `guid` IN (173564,173569,173582,173611,173613);
DELETE FROM `creature` WHERE `guid` IN (173564,173569,173582,173611,173613) AND `id`=10940;
UPDATE `creature_template_addon` SET `auras`='17327 12556' WHERE `entry`=10940;
UPDATE `creature_template` SET `lootid`=0, `flags_extra`=`flags_extra`|64 WHERE `entry`=10940;
DELETE FROM `creature_loot_template` WHERE `entry`=10940; -- the kill loot the template no longer points at

UPDATE `gameobject_template` SET `displayId`=0, `Data2`=4, `Data8`=1 WHERE `entry`=176117;
DELETE FROM `gameobject` WHERE `guid` BETWEEN 3128400 AND 3128406;
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`) VALUES
(3128400,176117,0,139,2262,1,1,1407.58,-3680.22,77.2517,-0.890115,0,0,0,0,300,100,1),
(3128401,176117,0,139,2262,1,1,1408.95,-3657.77,82.0193,-0.541051,0,0,0,0,300,100,1),
(3128402,176117,0,139,2262,1,1,1420.38,-3659.88,82.0166,2.74016,0,0,0,0,300,100,1),
(3128403,176117,0,139,2262,1,1,1476.28,-3722.00,80.5144,1.44862,0,0,0,0,300,100,1),
(3128404,176117,0,139,2262,1,1,1405.48,-3716.77,78.1764,2.58308,0,0,0,0,300,100,1),
(3128405,176117,0,139,2262,1,1,1470.40,-3721.01,80.5162,2.9845,0,0,0,0,300,100,1),
(3128406,176117,0,139,2262,1,1,1411.16,-3657.82,77.694,-1.6057,0,0,0,0,300,100,1);
UPDATE `gameobject` SET `spawntimesecs`=120 WHERE `guid` IN (157554,157555,157556,157557,157558,157559,157567) AND `id` IN (176116,176142,176143);
DELETE FROM `gameobject` WHERE `guid`=157566 AND `id`=176247;

DELETE FROM `spell_script_names` WHERE `spell_id`=17132;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (17132,'spell_darrowshire_pamelas_doll_trap');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=10926;
DELETE FROM `creature_text` WHERE `CreatureID`=10926 AND `GroupID` IN (70,71);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(10926,70,0,'Hurray!  My doll!',12,0,100,0,0,0,6416,0,'Pamela Redpath - Pamela''s Doll rewarded'),
(10926,71,0,'You''ll keep the bad people away, won''t you?',12,0,100,1,0,0,6417,0,'Pamela Redpath - Pamela''s Doll rewarded');
DELETE FROM `smart_scripts` WHERE (`entryorguid`=10926 AND `source_type`=0 AND `id`=0) OR (`entryorguid`=1092600 AND `source_type`=9);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(10926,0,0,0,20,0,100,0,27384,0,0,0,80,1092600,0,0,0,0,0,1,0,0,0,0,0,0,0,'Pamela Redpath - On Quest Pamela''s Doll Rewarded - Run Script'),
(1092600,9,0,0,0,0,100,0,0,0,0,0,1,70,0,0,0,0,0,7,0,0,0,0,0,0,0,'Pamela Redpath - On Script - Say Line 70'),
(1092600,9,1,0,0,0,100,0,3000,3000,0,0,1,71,0,0,0,0,0,7,0,0,0,0,0,0,0,'Pamela Redpath - On Script - Say Line 71');
