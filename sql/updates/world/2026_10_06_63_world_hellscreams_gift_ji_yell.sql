-- #510 (epic #445): Hellscream's Gift (31014, Horde pandaren) - Ji Firepaw 62081's yell when the Ring of Valor challengers
-- come out. Not applied; build 13.
--
-- Intended design: Wowpedia MediaWiki API, page "Hellscream's Gift" ({{Removed|patch=7.0.3}}, i.e. live in 5.4.8),
--   fetched 2026-10-06, Notes:
--     Garrosh Hellscream yells: Hah! It looks like we're ready. Now, newcomers... prepare to fight.
--     Ji Firepaw yells: What is that thing?
--     Garrosh Hellscream yells: This is my gift to you: a gift and a trial.
-- Source of the row: world.broadcast_text 61608 "What is that thing?" (SoundEntriesID 27292, VerifiedBuild 18019), which
--   sits between Garrosh's 61607 (group 18) and 61609 (group 19). It has no creature_text row in LoA or in the MoP 5.4.8 data
--   (creature_text 62081 has groups 0-2 only), so nothing said it. Ji's other rows (groups 0-2, sounds
--   27290/27291/27293) are the template for the columns.
-- Used by: npc_garrosh_horde_way_quest (src/server/scripts/Kalimdor/zone_durotar.cpp), ACTION_GIFT_OF_HELLSCREAM,
--   4 s after the challengers are summoned: Ji->AI()->Talk(SAY_SPECIAL_3).
-- Re-runnable: DELETE + INSERT.
--
-- Rollback:
--   DELETE FROM `creature_text` WHERE `CreatureID` = 62081 AND `GroupID` = 3;

DELETE FROM `creature_text` WHERE `CreatureID` = 62081 AND `GroupID` = 3;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(62081, 3, 0, 'What is that thing?', 14, 0, 100, 0, 0, 27292, 0, 61608, 0, 'Ji Firepaw - Hellscream''s Gift trial - challengers summoned');
