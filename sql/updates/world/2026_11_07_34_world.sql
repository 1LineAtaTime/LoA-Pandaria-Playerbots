-- Infiltration 28603 and Deception and Trickery 28604: Lilith 49035 as the player's personal companion and the SI:7 disguise
-- (zone_hillsbrad_headland.cpp). Replaces the stopgap of 2026_10_29_986_70: a static Lilith in Tarren Mill (guid 3110072) and the controller in
-- The Headland while 28604 was in progress.
-- Sourced:
--  * Lilith Summon Controller 91391 (force reaction Stormpike Guard 730 friendly), Summon Lilith 91393 (SummonProperties 3081, personal ally), SI:7
--    Operative Transform 91396 -> 91395 (on her master), all from the 5.4.8 DBC. The area is the DBC's: both 91391 and 91393 need area
--    group 2822 = Hillsbrad Foothills 267 (the stopgap had only The Headland 1680).
--  * No TDB spawns 49035, and quest_poi puts the turn-in of 28604 in The Headland.
--  * She comes as soon as 28603 is accepted and the disguise holds through the zone: Wowhead comments on 28603 and 28604 ("after accepting
--    Infiltration ... Lilith following me around", "on the way to where you do the quest, you pass the Alliance entrance to AV", its vendors usable,
--    "makes the Breaking the Hand quest much easier").
-- Ours:
--  * The statuses. She stays while 28603 or 28604 is in the log, and after 28603 is turned in with 28604 not yet taken, so she can still offer
--    it. That third row has autocast 0: it only keeps an aura the player already has. Abandoning a quest clears everything unless the other
--    quest is still in the log; retail left players stuck with her (comments on 28603).
--  * The transform 91395 is not saved. The controller and 91393 are not saved either (CHANGE_MAP interrupt with no duration: SpellMgr marks them
--    DONT_SAVE), and only the controller's removal clears the transform, so a saved transform would outlast them. At login spell_area gives the
--    controller back if a quest is in the log. The state after 28603 is turned in does not survive a relog or a map change: no Lilith, no
--    disguise, and 28604 at Melisara.
--  * Melisara also offers 28604. Lilith is its only starter in the sources, but a player who abandoned it (or left the zone) with 28603 turned in
--    has no Lilith; 28603 and 28605 already come from Melisara too.
DELETE FROM `creature` WHERE `guid`=3110072;
DELETE FROM `creature_addon` WHERE `guid`=3110072;

DELETE FROM `spell_area` WHERE `spell`=91391;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(91391,267,28603,0,0,0,2,1,10,0),
(91391,267,28604,0,0,0,2,1,10,0),
(91391,267,28566,28603,0,0,2,0,64,64);

DELETE FROM `spell_custom_attribute` WHERE `spell`=91395 AND `attribute`='SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB';
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(91395,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','SI:7 Operative Transform: Lilith Summon Controller and its summon are not saved, and only the controller clears this');

DELETE FROM `spell_script_names` WHERE `spell_id`=91391 AND `ScriptName`='spell_hillsbrad_lilith_summon_controller';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(91391,'spell_hillsbrad_lilith_summon_controller');

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_lilith' WHERE `entry`=49035;
UPDATE `quest_template_addon` SET `ScriptName`='quest_hillsbrad_lilith_disguise' WHERE `ID` IN (28603,28604);

DELETE FROM `creature_queststarter` WHERE `id`=2278 AND `quest`=28604;
INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES
(2278,28604);
