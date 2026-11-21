-- Eastern Plaguelands: Vex'tul 45741 follows the player in Zul'Mashar (Out of the Ziggurat 27481 / Into the Flames 27482) on the
-- zone's follower script, and his death in the bonfire gives his own master the credit of Into the Flames.
--
--  * Before (SmartAI): he only followed (no assist: he watched his master fight), dropped dead at the border of Zul'Mashar and
--    left a corpse there; the summon aura 85334 was saved at logout (a relog left no Vex'tul until the player went out of the area
--    and back); the bonfire bunny 45738 sent the farewell to the nearest Vex'tul within 20 yd, so a thrower whose Vex'tul had
--    died or lagged behind got nothing, another player's Vex'tul could take the credit, and the dead Vex'tul lay on the fire
--    (the hand-in to him that despawned him is gone: 2026_11_14_38 made 27482's ender 45574 and 45741 neither giver nor ender).
--  * Now (npc_epl_vextul_follower, spell_epl_follower_summon / _aura, spell_epl_destroy_threshjin_body): the follower pattern of
--    2026_11_14_15 (never two, no aura for a dead player, goes with the aura when he leaves Zul'Mashar or the quests are done,
--    defensive, joins his master's fights, healable, brought along when left behind, back 30 s after dying); the body thrown on
--    the fire makes the thrower's own Vex'tul walk to the fire, say his two lines, jump in and die (85337), and his death credits
--    45738 to his master; with no Vex'tul of his own at hand the thrower is credited at once (ours).
--  * Sourced: the farewell (wiki 27482 notes; broadcast 45810, 45811 in creature_text 45741 groups 0/1, kept); Vex'tul Death 85337
--    (DBC, the quest's id block, used by nothing before); the area (85334 AreaGroup 2753 = 2273, existing spell_area row).
--    Ours: the steps' timing (the old SmartAI list's), the credit without a Vex'tul, not saving the aura.
--  * Data from eastwall-northpass's b1.sql (second pass), which ships only with this script.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_vextul_follower' WHERE `entry`=45741;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45741 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4574100 AND `source_type`=9;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=45741;

-- The bonfire bunny only takes the spell's hit now (the condition of 85327 on 45738 stays)
UPDATE `creature_template` SET `AIName`='' WHERE `entry`=45738;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45738 AND `source_type`=0;

DELETE FROM `spell_script_names` WHERE `spell_id`=85334 AND `ScriptName` IN ('spell_epl_follower_summon','spell_epl_follower_aura');
DELETE FROM `spell_script_names` WHERE `spell_id`=85327 AND `ScriptName`='spell_epl_destroy_threshjin_body';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85334,'spell_epl_follower_summon'),
(85334,'spell_epl_follower_aura'),
(85327,'spell_epl_destroy_threshjin_body');

DELETE FROM `spell_custom_attribute` WHERE `spell`=85334;
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(85334,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Vex''tul (27481, 27482): spell_area casts it again, with the follower, at login');
