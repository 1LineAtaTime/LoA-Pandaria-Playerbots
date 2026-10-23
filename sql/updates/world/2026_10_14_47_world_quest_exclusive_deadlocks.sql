-- Refs #810. Non-repeatable quests whose PrevQuestID is a member of their own ExclusiveGroup could never be taken.
-- How the core decides (Player::SatisfyQuestExclusiveGroup, Player.cpp ~17171): a quest with ExclusiveGroup > 0 is refused as soon as ANY
--   other quest of the group has a status other than NONE, or is rewarded (the "rewarded" test is skipped only when BOTH quests are
--   repeatable, QUEST_SPECIAL_FLAGS_REPEATABLE in quest_template_addon.SpecialFlags). The group is the set loaded in ObjectMgr.cpp ~4947.
--   Player::SatisfyQuestPreviousQuest needs |PrevQuestID| rewarded. So prev-in-same-group + non-repeatable = needs the prev rewarded AND
--   forbids it rewarded: untakeable. Verified 2026-10-14 on the test world DB: exactly these 4 rows (verification query at the bottom).
-- Sources, all fetched 2026-10-14: Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&prop=wikitext, questbox
--   previous/next/start); Wowhead www.wowhead.com/mop-classic/quest=<id> Series box; the MoP 5.4.8 data; the AC WotLK
--   data/sql/base/db_world/quest_template_addon.sql for the BG quest. Idempotent: every statement sets absolute values by primary key.

-- 30603 The Broketooth Ravage. Wowpedia: The Rabbitsfoot (30602) -> The Broketooth Ravage (30603) -> A Monkey Idol / No Pack Left Behind /
--   Breaking Brooketooth; Wowhead series 3. The Rabbitsfoot / The Broketooth Ravage. 30603.PrevQuestID = 30602 is right, the shared
--   ExclusiveGroup 30602 is the bug: the MoP data has 30603 ExclusiveGroup 0 (30602 keeps its own single-member group 30602, harmless).
UPDATE `quest_template_addon` SET `ExclusiveGroup` = 0 WHERE `ID` = 30603;

-- The Returning Champion, pet battle line (account quests, group 31975 = four sibling quests: Alliance 31975 / 31976, Horde 31977 / 31980).
--   Wowpedia: 31975 (Alliance) and 31977 (Horde) start from Stone Cold Trixxy after Grand Master Trixxy 31897 / Battle Pet Tamers: Kalimdor;
--   31976 (Alliance) and 31980 (Horde) start from Lydia Accoste (Grand Master Lydia Accoste 31915); each faction's pair are alternative routes
--   to Battle Pet Tamers: Outland. Wowhead shows the four as one step (no quest of the pair precedes the other). So the group (one route only)
--   is right and a PrevQuestID pointing at the sibling is the bug.
-- 31976: no previous on Wowpedia/Wowhead; the MoP data has 31975 (the same bug). 31915 cannot be used: in this DB it only follows the
--   Horde-only Eastern Kingdoms quest 31903 (the Alliance one, 31902, exists in neither DB), so an Alliance gate on it would never open.
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` = 31976;
-- 31980: Wowpedia start = Lydia Accoste, the MoP data has PrevQuestId = 31915 (Grand Master Lydia Accoste, Horde-reachable via 31903).
UPDATE `quest_template_addon` SET `PrevQuestID` = 31915 WHERE `ID` = 31980;

-- 31975 itself is left at PrevQuestID 0 (MoP data 0), 31977 at 0 (MoP data has 31897; Wowpedia agrees, but it is not a deadlock).

-- 8398 Claiming Arathi Basin (Horde repeat, group 8374, PrevQuestID 8396 which is in the same group). It is the 4th hit: the other
--   repeat quests of the group (8391, 8392, 8397, ...) have SpecialFlags 1 and are exempt; 8398 had 0. AC WotLK quest_template_addon has
--   (8398, ..., PrevQuestID 8396, ..., ExclusiveGroup 8374, ..., SpecialFlags 1); the MoP data has 0 like the fork (same bug).
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 1 WHERE `ID` = 8398;

-- Verification (must return 0 rows after applying; before: 8398, 30603, 31976, 31980):
-- SELECT a.`ID`, a.`PrevQuestID`, a.`ExclusiveGroup` FROM `quest_template_addon` a
--   JOIN `quest_template_addon` p ON p.`ID` = ABS(a.`PrevQuestID`) AND p.`ExclusiveGroup` = a.`ExclusiveGroup`
--   WHERE a.`ExclusiveGroup` > 0 AND a.`PrevQuestID` <> 0 AND a.`PrevQuestID` <> a.`ID` AND (a.`SpecialFlags` & 1) = 0;
