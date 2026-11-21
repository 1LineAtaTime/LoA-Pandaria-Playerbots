-- Western Plaguelands, The Reckoning (27144): the camera, the player's doppelganger and the chain that drags Koltira into the portal.
--
--  * Sourced (5.4.8 DBC): The Reckoning: Summon Camera 84211 (Reckoning Event Camera 45113, SummonProperties 161 = a vehicle,
--    Vehicle 1079, seats with CameraMode 42/43 "Western Plaguelands 4.x - The Reckoning 1/2"); Summon Your Doppelganger 84258
--    (45127, personal) and Mirror Image Aura 84259 (CLONE_CASTER and the two copy-weapon script effects, all on
--    TARGET_UNIT_NEARBY_ENTRY); The Reckoning: Chain 01 84239 (channelled dummy aura, TARGET_UNIT_NEARBY_ENTRY). The positions of
--    84211 and 84258 were already in spell_target_position (as in TDB 4.4.2).
--  * Sourced (TDB 4.4.2 creature_template_model): the doppelganger's second model (22903, the GM-view Rocket Chicken) has
--    Probability 0; mop had 1, so half of the doppelgangers were created as a chicken before the clone aura lands.
--  * Ours: the condition targets (84259 on the doppelganger, 84239 on Koltira), and who casts what (npc_wpl_reckoning_sylvanas).
--    spell_wpl_reckoning_mirror_image handles 84259's two script effects (the player casts 41055/45206 on the double; nothing did
--    before, so it got no weapons) and, like spell_wpl_reckoning_chain on 84239, picks the caster's own scene actor: group members
--    see each other's personal summons, which stand on the same spots.

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (84239,84259);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,7,84259,0,0,31,0,3,45127,0,0,0,0,'','Mirror Image Aura (The Reckoning) - target Your Doppelganger'),
(13,1,84239,0,0,31,0,3,45112,0,0,0,0,'','The Reckoning: Chain 01 - target Koltira Deathweaver');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (84239,84259);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84239,'spell_wpl_reckoning_chain'),
(84259,'spell_wpl_reckoning_mirror_image');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_wpl_reckoning_actor' WHERE `entry` IN (45113,45127);

UPDATE `creature_template_model` SET `Probability` = 0 WHERE `CreatureID` = 45127 AND `CreatureDisplayID` = 22903;
