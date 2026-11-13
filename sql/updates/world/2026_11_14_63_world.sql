-- Eastern Plaguelands, Villains of Darrowshire (27387): the spectral battles at Horgus' Skull and the Shattered Sword of Marduk.
--
-- Sourced:
--  * Ghosts surround the skull and fight one another, shouting (wiki Notes quote the lines): both TDBs' creature_text for 11288 and
--    11289 (8 lines each, broadcast 6827-6842) and their SmartAI row "in combat, 20 %, every 20-30 s, say".
--  * Picking up the skull or the sword makes them vanish (wiki Notes; OLD comments 2008-05-22, 2008-10-05): both TDBs' conditions on
--    17678 Despawn Spectral Combatants (four entries) and their SmartAI "on spellhit 17678 cast 17680 Spirit Spawn-out" (60 s
--    invisibility) on all four. The chests' linked traps 176591/176865 (spawned, guids 157433/157431) cast 17678.
--  * Spectral Marauders 11620 stand still among the Spectral Corpses 11621 and cannot be attacked: both TDBs unit_flags 256, static.
--  * The fight goes on (both TDBs respawn the skull's ghosts in 30 s; mop had 300 s, so with health mod 0.4 the Betrayers were dead
--    within a minute and the hollow stood empty for five; here the sparring cap below keeps them alive). mop also had every spawn doubled (12 Betrayers, 6 Defenders, 8 Marauders
--    against TDB 7, 3, 4): the extra copies go (which of each pair goes is ours).
-- NOT as retail in one point (decisions.md / plan.md "Held": the core keeps refusing IMMUNE_TO_PC targets to NPCs): both TDBs flag the
-- Betrayers 11288 and Defenders 11289 IMMUNE_TO_PC (256) on factions 43 / 1454, so in this core they could never fight. Instead
-- (ours) neither carries 256 and they are put on the FactionTemplate pair 1690 / 1691 (factions 963 / 964, no reputation): each is
-- friendly to both player teams and hostile to the other (hostile.py), so they fight each other while no player, pet or bot can
-- attack them and they never attack a player (Unit::IsValidAttackTarget: no attack when either side is friendly). 43 left the
-- Betrayers neutral and attackable at level 57-58. Both get a sparring cap at 85 % (creature_sparring_template, ours): NPC damage
-- stops there, so the fight goes on for good instead of the Betrayers (health mod 0.4) dying every few seconds.
-- The vanishing (ours in its mechanism): the spellhit row still casts 17680 Spirit Spawn-out, and a linked row despawns the spectre
-- 1 s later; it comes back with its spawn time, 60 s for all four entries (= 17680's duration). Before, a fighting spectre lost
-- sight of its foe, evaded, and the evade stripped the 60 s invisibility (SmartAI::EnterEvadeMode RemoveAuras): it was back in 1-2 s.
-- Needs the core fix "Core/GameObject: Look for a linked trap around its object, not around the user" for the vanishing: 17678 has
-- range 0, so without it the chest finds its trap only for a looter standing on it.
UPDATE `creature_template` SET `unit_flags`=`unit_flags`&~256 WHERE `entry` IN (11288,11289);
UPDATE `creature_template` SET `faction`=1690 WHERE `entry`=11288;
UPDATE `creature_template` SET `faction`=1691 WHERE `entry`=11289;
DELETE FROM `creature_sparring_template` WHERE `creature_id` IN (11288,11289);
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(11288,85,'Spectral Betrayer - endless fight with the Spectral Defenders at Horgus'' Skull'),
(11289,85,'Spectral Defender - endless fight with the Spectral Betrayers at Horgus'' Skull');
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|256 WHERE `entry`=11620;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (11288,11289,11620,11621);

DELETE FROM `creature_text` WHERE `CreatureID` IN (11288,11289);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(11288,0,0,'Hail the Lich King!',12,0,100,0,0,0,6827,0,'Spectral Betrayer'),
(11288,0,1,'Your soul is doomed!',12,0,100,0,0,0,6828,0,'Spectral Betrayer'),
(11288,0,2,'Join us, or fall forever!',12,0,100,0,0,0,6829,0,'Spectral Betrayer'),
(11288,0,3,'There is no hope!',12,0,100,0,0,0,6830,0,'Spectral Betrayer'),
(11288,0,4,'Our master wants you!',12,0,100,0,0,0,6831,0,'Spectral Betrayer'),
(11288,0,5,'Darrowshire will fall!',12,0,100,0,0,0,6832,0,'Spectral Betrayer'),
(11288,0,6,'Scourge will prevail!',12,0,100,0,0,0,6833,0,'Spectral Betrayer'),
(11288,0,7,'All hope is lost!',12,0,100,0,0,0,6834,0,'Spectral Betrayer'),
(11289,0,0,'We must not give up!',12,0,100,0,0,0,6835,0,'Spectral Defender'),
(11289,0,1,'Fight!  Fight for Darrowshire!',12,0,100,0,0,0,6836,0,'Spectral Defender'),
(11289,0,2,'Our captain has fallen!',12,0,100,0,0,0,6837,0,'Spectral Defender'),
(11289,0,3,'The Alliance has abandoned us!',12,0,100,0,0,0,6838,0,'Spectral Defender'),
(11289,0,4,'Where is the Silver Hand?  We need their help!',12,0,100,0,0,0,6839,0,'Spectral Defender'),
(11289,0,5,'Fight for your families!  Don''t give up!',12,0,100,0,0,0,6840,0,'Spectral Defender'),
(11289,0,6,'If we fall, then Darrowshire is doomed!',12,0,100,0,0,0,6841,0,'Spectral Defender'),
(11289,0,7,'We must stop these villains!',12,0,100,0,0,0,6842,0,'Spectral Defender');

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid`=11288 AND `id` IN (1,2,3)) OR (`entryorguid`=11289 AND `id` IN (2,3,4)) OR (`entryorguid`=11620 AND `id` IN (1,2)) OR (`entryorguid`=11621 AND `id` IN (0,1)));
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(11288,0,1,0,0,0,20,0,5000,30000,20000,30000,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Betrayer - In Combat - Say Line 0'),
(11288,0,2,3,8,0,100,0,17678,0,5000,5000,11,17680,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Betrayer - On Spellhit Despawn Spectral Combatants - Cast Spirit Spawn-out'),
(11288,0,3,0,61,0,100,0,0,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Betrayer - Linked - Despawn in 1 s (back with its 60 s spawn time, ours)'),
(11289,0,2,0,0,0,20,0,5000,30000,20000,30000,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Defender - In Combat - Say Line 0'),
(11289,0,3,4,8,0,100,0,17678,0,5000,5000,11,17680,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Defender - On Spellhit Despawn Spectral Combatants - Cast Spirit Spawn-out'),
(11289,0,4,0,61,0,100,0,0,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Defender - Linked - Despawn in 1 s (back with its 60 s spawn time, ours)'),
(11620,0,1,2,8,0,100,0,17678,0,5000,5000,11,17680,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Marauder - On Spellhit Despawn Spectral Combatants - Cast Spirit Spawn-out'),
(11620,0,2,0,61,0,100,0,0,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Marauder - Linked - Despawn in 1 s (back with its 60 s spawn time, ours)'),
(11621,0,0,1,8,0,100,0,17678,0,5000,5000,11,17680,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Corpse - On Spellhit Despawn Spectral Combatants - Cast Spirit Spawn-out'),
(11621,0,1,0,61,0,100,0,0,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Spectral Corpse - Linked - Despawn in 1 s (back with its 60 s spawn time, ours)');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=17678;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,17678,0,0,31,0,3,11288,0,0,0,0,'','Despawn Spectral Combatants hits Spectral Betrayer'),
(13,1,17678,0,1,31,0,3,11289,0,0,0,0,'','Despawn Spectral Combatants hits Spectral Defender'),
(13,1,17678,0,2,31,0,3,11620,0,0,0,0,'','Despawn Spectral Combatants hits Spectral Marauder'),
(13,1,17678,0,3,31,0,3,11621,0,0,0,0,'','Despawn Spectral Combatants hits Spectral Corpse');

DELETE FROM `creature_addon` WHERE `guid` IN (173043,173044,173045,173046,173049,173047,173048,173050,173540,173541,173542,173543);
DELETE FROM `creature` WHERE `guid` IN (173043,173044,173045,173046,173049) AND `id`=11288;
DELETE FROM `creature` WHERE `guid` IN (173047,173048,173050) AND `id`=11289;
DELETE FROM `creature` WHERE `guid` IN (173540,173541,173542,173543) AND `id`=11620;
UPDATE `creature` SET `spawntimesecs`=60, `wander_distance`=5, `MovementType`=1 WHERE `id` IN (11288,11289);
UPDATE `creature` SET `wander_distance`=0, `MovementType`=0, `spawntimesecs`=60 WHERE `id` IN (11620,11621);
