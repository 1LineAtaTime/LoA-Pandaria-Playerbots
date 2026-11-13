-- #982 quest-chain traps found by the DB-wide static scan (the #977 pattern). NOT APPLIED. Idempotent.
-- Needs a worldserver restart (quest_template_addon and conditions are cached at boot).
--
-- Root cause of the whole class (read 2026-10-28):
--   This core's Player::SatisfyQuestPreviousQuest (src/server/game/Entities/Player/Player.cpp:17029-17053) accepts a
--   NEGATIVE PrevQuestID only while that quest is QUEST_STATUS_INCOMPLETE. Upstream commit cba8d60e
--   ("[Core/Questing] Modernize quest database structure and loading (#262)", 2024-06-26) changed it from
--   "GetQuestStatus(prevId) != QUEST_STATUS_NONE" (still what AC does: azerothcore-wotlk
--   src/server/game/Entities/Player/PlayerQuest.cpp:1086, where GetQuestStatus also returns REWARDED) to
--   "== QUEST_STATUS_INCOMPLETE", without migrating the data. Our quest data (inherited lineage) still carries 165
--   negative PrevQuestIDs written for the old meaning "the other quest has been TAKEN (in the log, complete or done)".
--   Under the new meaning such a quest disappears the moment the other quest's objectives are done, and can never
--   be taken if the other quest has no objectives (it is COMPLETE on accept). When the gated quest is itself a hard
--   prerequisite of the next quest, completing the other quest first locks the chain (#977: 24930 / -14157).
--   The core-side alternative (one line, AC parity for all 165 rows) is described in
-- (not shipped); this file fixes only the rows that lock a chain.
--
-- Sources per row (read 2026-10-28):
--   AC   = local AC WotLK world DB (acore_world.quest_template_addon), the behaviour spec
--   TC434= TrinityCore TDB_full_world_434.22011 (scratch-963b/x-5), SF548 = SFDB_full_548_26.002 (scratch-963b/x-6)
--   EMU / AK = the reference 5.4.8 world DBs (same value as ours: inherited)
--   WP   = Wowpedia MediaWiki API quest page
--
-- 1) 10851 "The Totems of My Enemy" (Spiritcaller Dohgar 22312, Blade's Edge) had PrevQuestID -10843: takeable only
--    while "Since Time Forgotten..." (10843, Leoroxx 22004, kill 1 x 20768) is unfinished. 10853 "Spirit Calling"
--    needs 10851 (PrevQuestID), so killing the target and/or turning in 10843 before visiting Dohgar ends the
--    Mok'Nathal chain (10853 -> 10859 ...).
--    AC 10846 | TC434 0 | SF548 10846 | EMU/AK -10843 | WP: no previous for 10851. -> PrevQuestID 10846 (AC/SF548).
UPDATE `quest_template_addon` SET `PrevQuestID` = 10846 WHERE `ID` = 10851 AND `PrevQuestID` = -10843;

-- 2) 11265 "Of Keys and Cages" (24135, Howling Fjord) had PrevQuestID -11263: takeable only while "Dealing With
--    Gjalerbron" (11263, 24129, kill 15/8/8) is unfinished. 11268 "The Walking Dead" needs 11265.
--    AC 11261 | TC434 0 | SF548 11260 | EMU/AK -11263. -> PrevQuestID 11261 (AC; the same prerequisite as 11263).
UPDATE `quest_template_addon` SET `PrevQuestID` = 11261 WHERE `ID` = 11265 AND `PrevQuestID` = -11263;

-- 3) 12530 "Tormenting the Softknuckles" (Goregek 28214, Sholazar) had PrevQuestID -12529: takeable only while
--    "The Ape Hunter's Slave" (12529, Rakjak 28082, kill 8 + 6) is unfinished. 12533 "The Wasp Hunter's Apprentice"
--    needs 12530 (Frenzyheart chain).
--    AC 12528 | TC434 12528 | SF548 12528 | EMU/AK -12529 | WP previous = The Ape Hunter's Slave (series order).
--    -> PrevQuestID 12528 (AC/TC434/SF548: offered next to 12529, both after "Playing Along").
UPDATE `quest_template_addon` SET `PrevQuestID` = 12528 WHERE `ID` = 12530 AND `PrevQuestID` = -12529;

-- 4) 12823 "A Flawless Plan" (Gino 29432, Storm Peaks) had PrevQuestID -12822: takeable only while "Know No Fear"
--    (12822, Ricket 29428, kill 6 + 8) is unfinished. 12824 "Demolitionist Extraordinaire" needs 12823.
--    AC 12821 | TC434 0 | SF548 0 | EMU/AK -12822 | WP previous = Know No Fear (series order).
--    -> PrevQuestID 12821 (AC; the same prerequisite as 12822).
UPDATE `quest_template_addon` SET `PrevQuestID` = 12821 WHERE `ID` = 12823 AND `PrevQuestID` = -12822;

-- 5) 11920 "Cultists Among Us" (Cultist Shrine GO 187851, Borean Tundra) has PrevQuestID -11789: takeable only while
--    "A Soldier in Need" (11789, collect 1 x 35119) is unfinished; 11791 "Notify Arlos" needs 11920.
--    AC -11789 (with AC's "taken or done" meaning) | TC434 0 | SF548 -11789 | EMU/AK -11789 | WP previous = A Soldier
--    in Need. -> keep the AC meaning exactly: drop the negative PrevQuestID and require 11789 in the log (incomplete or
--    complete) or rewarded through a quest-accept condition (CONDITION_QUESTSTATE 47, mask 2|8|64 = 74;
--    ConditionMgr.cpp:516-527).
UPDATE `quest_template_addon` SET `PrevQuestID` = 0 WHERE `ID` = 11920 AND `PrevQuestID` = -11789;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 11920 AND `ConditionTypeOrReference` = 47 AND `ConditionValue1` = 11789;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 11920, 0, 0, 47, 0, 11789, 74, 0, 0, 0, 0, '', '#982: Cultists Among Us needs A Soldier in Need taken (in log or done), AC meaning of PrevQuestID -11789');

-- 6) Cloth donations to the Exodar (20604, sort 3557) and Silvermoon (20612, sort 3487) require THEMSELVES, so every
--    donation after Wool can never be taken (scan class T4, a dependency cycle, not the negative-PrevQuestID class):
--    10356 Mageweave has PrevQuestID 7798, and 7798 is in ExclusiveGroup -7792 ("all of": Player.cpp:17055-17092 then
--    requires every other member - 7792 AND 10356 itself - rewarded). Same for 10360 (PrevQuestID 10359, group -10359)
--    and 10361 (PrevQuestID 10360). 10357 / 10362 Runecloth get the whole group through NextQuestID and 10358
--    "Additional Runecloth" follows 10357, so all of 10356 10357 10358 10360 10361 10362 are dead.
--    AC 0/0/0 for all six rows | TC434 0/0/0 | SF548 0/0/0 | EMU/AK same as ours | WP "A Donation of Silk/Mageweave":
--    available at level 26 / 40, no prerequisite. -> PrevQuestID/NextQuestID/ExclusiveGroup 0 (AC).
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 0, `ExclusiveGroup` = 0
 WHERE `ID` IN (7792, 7798, 10356, 10359, 10360, 10361) AND `ExclusiveGroup` IN (-7792, -10359);

-- 7) 30127 "Threat from Dojan" (Horde, Krasarang Wilds, level 86) has no quest giver and no quest ender here (scan class
--    T2: a hard prerequisite with no starter), but 30129 "The Mogu Agenda" (Kang Bramblestaff 56112) has PrevQuestID
--    30127, so the Horde Krasarang chain stops after "Blind Them!" (30124).
--    WP "Threat from Dojan (Horde)": start Kor Bloodtusk, previous Skitterer Stew / Blind Them!; WP "The Mogu Agenda":
--    previous Threat from Dojan (Horde) + Herbal Remedies (so 30129's PrevQuestID 30127 is right, the giver is missing).
--    EMU creature_queststarter + creature_questender (58114, 30127) | SF548 creature_queststarter (58114, 30127) |
--    AK the same gap as ours | not in TC434 (MoP). Kor Bloodtusk (58114, npcflag 2) has 2 spawns, phaseMask 1.
--    -> add the giver and ender rows (EMU).
DELETE FROM `creature_queststarter` WHERE `id` = 58114 AND `quest` = 30127;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES (58114, 30127);
DELETE FROM `creature_questender` WHERE `id` = 58114 AND `quest` = 30127;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES (58114, 30127);

-- Not changed (reviewed, see the report): 25189/25190/25193/25194 (Durotar, Watershed Patrol credits come from the
-- sub-chains' rewards), 12557/12597/12598/12599 (Pa'Troll credits are the task quests' RewardSpell), 12690 (Sabotage
-- credit is 12690's RewardSpell), 308/3450 and the never-takeable 1103/1191/1789/1790/3375/3483/8599 (removed in
-- 4.0.3, no live giver), the Townlong Steppes dailies 31040-31048/31105/31106/31116/31118/31198/31200 (daily hub).
--
-- Undo:
--   UPDATE `quest_template_addon` SET `PrevQuestID` = -10843 WHERE `ID` = 10851 AND `PrevQuestID` = 10846;
--   UPDATE `quest_template_addon` SET `PrevQuestID` = -11263 WHERE `ID` = 11265 AND `PrevQuestID` = 11261;
--   UPDATE `quest_template_addon` SET `PrevQuestID` = -12529 WHERE `ID` = 12530 AND `PrevQuestID` = 12528;
--   UPDATE `quest_template_addon` SET `PrevQuestID` = -12822 WHERE `ID` = 12823 AND `PrevQuestID` = 12821;
--   UPDATE `quest_template_addon` SET `PrevQuestID` = -11789 WHERE `ID` = 11920 AND `PrevQuestID` = 0;
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 11920 AND `ConditionTypeOrReference` = 47 AND `ConditionValue1` = 11789;
--   UPDATE `quest_template_addon` SET `NextQuestID` = 10357, `ExclusiveGroup` = -7792 WHERE `ID` IN (7792, 7798, 10356);
--   UPDATE `quest_template_addon` SET `PrevQuestID` = 7798 WHERE `ID` = 10356;
--   UPDATE `quest_template_addon` SET `NextQuestID` = 10362, `ExclusiveGroup` = -10359 WHERE `ID` IN (10359, 10360, 10361);
--   UPDATE `quest_template_addon` SET `PrevQuestID` = 10359 WHERE `ID` = 10360;
--   UPDATE `quest_template_addon` SET `PrevQuestID` = 10360 WHERE `ID` = 10361;
--   DELETE FROM `creature_queststarter` WHERE `id` = 58114 AND `quest` = 30127;
--   DELETE FROM `creature_questender` WHERE `id` = 58114 AND `quest` = 30127;
