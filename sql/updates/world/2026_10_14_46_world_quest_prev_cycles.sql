-- Refs #809. quest_template_addon PrevQuestID / NextQuestID dependency cycles that made quests untakeable.
-- How the core decides (Player::SatisfyQuestPreviousQuest + SatisfyQuestDependentPreviousQuests, ObjectMgr.cpp ~4930):
--   a quest needs |PrevQuestID| rewarded (negative = must be active) AND any one of {its positive PrevQuestID, every quest whose
--   NextQuestID points at it} rewarded. So a NextQuestID that points BACKWARDS is as deadly as a PrevQuestID cycle, and a
--   quest with PrevQuestID 0 and several NextQuestID pointers at it is an OR (used for the Alliance/Horde twins).
-- In the test world DB (verified 2026-10-14) the same pattern repeats in every pair below: the later quest has
-- PrevQuestID = first (correct) but the first has PrevQuestID = later and/or NextQuestID = later (the reverse), and the later one has
-- NextQuestID = first. Rule applied: keep the later quest's PrevQuestID = first, give the first quest the real predecessor
-- (Wowpedia explicit) or 0, set first.NextQuestID = later and later.NextQuestID = 0.
-- Sources, all fetched 2026-10-14: Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&prop=wikitext, questbox
-- previous/next); Wowhead "Series" box on www.wowhead.com/mop-classic/quest=<id> (the tooltip API only confirms id <-> title);
-- the MoP 5.4.8 world data for comparison. Where Wowpedia is silent on a previous quest the value is 0 so both stay takeable.
-- Idempotent: every statement sets absolute values by primary key.

-- Cycles in PrevQuestID alone ---------------------------------------------------------------------------------------------
-- 29968 Green-ish Energy -> 29967 Boom Goes the Doonamite! (Horde, Jade Forest). Wowpedia 29967 previous = Green-ish Energy;
--   29968 has no previous (Wowhead series starts at it). The MoP data has 29943 on both; not used, Wowpedia/Wowhead are silent.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 29967 WHERE `ID` = 29968;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 29967;
-- 30048 Li Li and the Grain -> 30031 Taste Test (Valley of the Four Winds). Wowpedia 30031 previous = Li Li and the Grain;
--   Wowpedia 30048 previous = Chen's Resolution 30046 (the MoP data says 30053 Hop Hunting; Wowhead series starts at 30048).
UPDATE `quest_template_addon` SET `PrevQuestID` = 30046, `NextQuestID` = 30031 WHERE `ID` = 30048;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30031;
-- 32035 Got Silk? -> 30072 Where Silk Comes From. Wowpedia Got Silk? next = Where Silk Comes From and has no previous; Wowhead
--   series = Got Silk? > Where Silk Comes From. (Wowpedia's 30072 page says previous = Li Li's Day Off 29950; contradicted by its
--   own Got Silk? page and by Wowhead, so not used. The MoP data is cyclic here too.)
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 30072 WHERE `ID` = 32035;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30072;
-- 30183 Stalker Mastery -> 30182 Fox Mastery. Wowpedia Stalker Mastery next = Fox Mastery (no previous); Fox Mastery previous =
--   Stalker Mastery; the MoP data has 30183 PrevQuestId 0 and NextQuestId 30182.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 30182 WHERE `ID` = 30183;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30182;
-- 30499 Get Back Here! (Horde) -> 30484 Gauging Our Progress. Wowpedia: Get Back Here! (Horde) previous = The Jade Serpent 30000,
--   Gauging Our Progress previous = Get Back Here!; MoP data 30499 PrevQuestId = 30000. (Gauging's own next, 30485 Last Piece
--   of the Puzzle, is left as it is: Last Piece also follows 30466, so no extra gate is added.)
UPDATE `quest_template_addon` SET `PrevQuestID` = 30000, `NextQuestID` = 30484 WHERE `ID` = 30499;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30484;
-- 31303 The Seal is Broken -> 30500 Residual Fallout. Wowpedia 31303 previous = Last Piece of the Puzzle (Horde 30485 / Alliance
--   31362, both already have NextQuestID 31303, so PrevQuestID must be 0 for the OR to work); Residual Fallout previous = The Seal
--   is Broken. Wowhead series: Last Piece of the Puzzle > The Seal is Broken > Residual Fallout.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 30500 WHERE `ID` = 31303;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30500;
-- 30999 Path Less Traveled -> 30601 Instant Courage. Wowpedia 30999 previous = Cho's Missive (optional); Cho's Missive 31459 / 31460
--   (Horde / Alliance, both already NextQuestID 30999, Wowhead series starts there) so PrevQuestID 0 keeps the OR; Instant Courage
--   previous = Path Less Traveled (Wowpedia and Wowhead).
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 30601 WHERE `ID` = 30999;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30601;
-- 30765 Regaining Honor -> 30751 A Terrible Sacrifice. Wowpedia Regaining Honor: start Automatic, previous blank, next = A Terrible
--   Sacrifice; Wowhead series Regaining Honor > A Terrible Sacrifice (the MoP data has them the other way round). Wowpedia's
--   A Terrible Sacrifice previous (To the Wall! 30724) is not added: Regaining Honor sits between them.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 30751 WHERE `ID` = 30765;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 30751;
-- 31015 Your Private Collection after The Lorewalkers (Wowpedia 31015 previous = The Lorewalkers (Alliance 31367 / Horde 31368);
--   Wowhead series The Lorewalkers 31016 > Your Private Collection). 31016, 31367 and 31368 already have NextQuestID 31015, so
--   31015 needs PrevQuestID 0 (OR of the three) and no NextQuestID back at 31016; 31016 has no previous.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 0 WHERE `ID` = 31015;
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` = 31016;
-- 31727 Gambling Problem -> 31181 Fresh Pots (Dread Wastes, The Anglers). Wowhead series Gambling Problem > Fresh Pots. Wowpedia
--   Gambling Problem previous = The Root of the Problem / Feed or Be Eaten / The Kunchong Whisperer (an OR of three that cannot be
--   one PrevQuestID; none of them points a NextQuestID at it, so it is left open, 0). Wowpedia Fresh Pots previous = Mazu's Breath
--   31265, but Wowpedia marks Mazu's Breath "optional" after Gambling Problem, so Fresh Pots keeps PrevQuestID 31727.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 31181 WHERE `ID` = 31727;
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 31181;

-- Outside Pandaria ------------------------------------------------------------------------------------------------------
-- 1111 Wharfmaster Dizzywig had PrevQuestID = itself. Wowpedia (Wharfmaster Dizzywig (quest), id 1111) previous = Rocket Car Parts
--   1110, next = Parts for Kravel 1112. 1110 has no addon row (takeable).
UPDATE `quest_template_addon` SET `PrevQuestID` = 1110 WHERE `ID` = 1111;
-- 10835 Apothecary Antonivich had PrevQuestID = itself. Wowhead series Boiling Blood 10538 > Apothecary Antonivich > A Burden of Souls
--   10864 ...; 10538 already has NextQuestID 10835 (the MoP data has the same self reference; not a source).
UPDATE `quest_template_addon` SET `PrevQuestID` = 10538 WHERE `ID` = 10835;
-- 25300 The Eye of Twilight (Mount Hyjal) closed a 10-quest ring: 25300 prev 25584 The Return of the Ancients -> 25578 -> 25663 ->
--   25630 -> 25600 -> 25599 -> 25312 -> 25303 -> 25301 -> 25300. Wowpedia: The Eye of Twilight previous = The Shrine Reclaimed
--   (Alliance 25280 / Horde 25279, two ids, one PrevQuestID cannot hold both, neither has NextQuestID 25300), Wowhead series starts at
--   The Eye of Twilight. Nothing single-valued is explicit, so 0. The MoP data has 25297, which is a ring in the fork's data.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` = 25300;

-- Cycles through NextQuestID ----------------------------------------------------------------------------------------------
-- 30457 Call Out Their Leader <- 31254 / 31255 The Road to Kun-Lai (twins). Both twins had PrevQuestID 30457 AND NextQuestID 30457, so
--   30457 (alts = {31254, 31255}) could never be reached. Wowpedia: Call Out Their Leader previous = The Road to Kun-Lai, which has
--   none; the MoP data has PrevQuestId 0 on both twins. Keep NextQuestID 30457, clear the PrevQuestID.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` IN (31254, 31255);
-- 31454 A Legend in the Making <- 31488 / 31489 Stranger in a Strange Land: 31454.NextQuestID = 31488 pointed back at 31488, whose only
--   alt was 31454. Wowhead series Stranger in a Strange Land > A Legend in the Making; Wowpedia 31454 previous = Stranger in a Strange
--   Land (optional); MoP data 31454 NextQuestId 0.
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 31454;
-- 2623 The Swamp Talker had NextQuestID = itself (harmless to the gate, since 2622 is its PrevQuestID, but meaningless). Wowpedia next blank.
UPDATE `quest_template_addon` SET `NextQuestID` = 0 WHERE `ID` = 2623;

-- Reported, NOT changed: ExclusiveGroup > 0 where the quest's PrevQuestID is a member of the same group (the core refuses a quest when
--   another quest of its group is rewarded, and these are not repeatable): 30603 (prev 30602, group 30602), 31976 (prev 31975, group 31975),
--   31980 (prev 31977, group 31975). The MoP data has group 0 / other prevs for them. The 41 other rows (BG Mark of Honor / Talismans,
--   repeatable) match upstream. No ExclusiveGroup cycle touches the quests fixed above.
