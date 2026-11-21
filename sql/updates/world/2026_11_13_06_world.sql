-- Western Plaguelands, The Reckoning (27144): the camera's view pointed at the hall's ceiling.
--
--  * Sourced (5.4.8 DBC and the client's camera code): the Reckoning Event Camera's seats (Vehicle 1079, VehicleSeat 8465-8467)
--    use CameraMode 42/43, flags 0x144. Their second camera target (lockedTargetOffsetBase 1) is the channel object (flag 0x40)
--    of the passenger, the player (no 0x20 hop to the vehicle). The scene's aiming spells are The Reckoning: Channel Target
--    Sylvanas 84214, Channel Target Koltira 84219 and Channel Target Portal 84221: infinite dummy channels on
--    TARGET_UNIT_NEARBY_ENTRY (50 yd), castable while on a vehicle (attr6 0x1000). Nothing cast them, so the view had no second
--    target and tilted up into the ceiling (63 degrees for seat 0, 45 for seats 1 and 2).
--  * Ours: these conditions (no TDB has rows for these spells; the names give the targets), and when the seated player channels on
--    whom (npc_wpl_reckoning_sylvanas). spell_wpl_reckoning_channel_target picks the caster's own scene actor: group members see
--    each other's personal summons, which stand on the same spots.

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (84214,84219,84221);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,84214,0,0,31,0,3,45111,0,0,0,0,'','The Reckoning: Channel Target Sylvanas - target Lady Sylvanas Windrunner'),
(13,1,84219,0,0,31,0,3,45112,0,0,0,0,'','The Reckoning: Channel Target Koltira - target Koltira Deathweaver'),
(13,1,84221,0,0,31,0,3,45114,0,0,0,0,'','The Reckoning: Channel Target Portal - target Portal to Undercity');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (84214,84219,84221);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84214,'spell_wpl_reckoning_channel_target'),
(84219,'spell_wpl_reckoning_channel_target'),
(84221,'spell_wpl_reckoning_channel_target');
