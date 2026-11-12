-- Western Plaguelands, The Farmers' Militia (27084): the Ebon Gargoyles carry the Provincial Minutemen off.
--
-- The collar (Release Ebon Gargoyle 84009) makes each enemy within 15 yd of the clicked point cast Summon Ebon Gargoyle 83994;
-- the credit is its second effect (84289 on the player), untouched here. The gargoyle 44955 had no script, so it stood there for
-- good (duration -1).
--  * Sourced (5.4.8 DBC): Forcecast Ride Gargoyle 84007 (FORCE_CAST 84006 on TARGET_UNIT_SUMMONER) and Ride Gargoyle 84006
--    (aura 236 on TARGET_UNIT_NEARBY_ENTRY) are the gargoyle's boarding chain; Vehicle 1068 (seat 8436) is 44955's.
--  * Ours: npc_wpl_ebon_gargoyle (board, climb, drop, kill on landing, despawn), spell_wpl_ride_gargoyle (each minuteman boards
--    his own gargoyle), spell_wpl_ride_gargoyle_seat (seat 0: the seat has no CAN_ENTER_OR_EXIT flag), the 84006 condition.
--  * Ours [inferred]: Release Ebon Gargoyle only works on Provincial Minutemen 44944 (quest "The Farmers' Militia", Wowhead
--    guide "vicinity of one of the Provincial Minutemen"). Without it Defenders, Force-Commanders and enemy players in the circle
--    summoned a gargoyle too. Only effect 0 (mask 1) is filtered, so the credit (effect 1, no targets) still comes on every use.

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (84006,84009);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,84006,0,0,31,0,3,44955,0,0,0,0,'','Ride Gargoyle - target Ebon Gargoyle'),
(13,1,84009,0,0,31,0,3,44944,0,0,0,0,'','Release Ebon Gargoyle - only Provincial Minutemen');

UPDATE `creature_template` SET `ScriptName` = 'npc_wpl_ebon_gargoyle' WHERE `entry` = 44955;

DELETE FROM `spell_script_names` WHERE `spell_id` = 84006;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84006,'spell_wpl_ride_gargoyle'),
(84006,'spell_wpl_ride_gargoyle_seat');
