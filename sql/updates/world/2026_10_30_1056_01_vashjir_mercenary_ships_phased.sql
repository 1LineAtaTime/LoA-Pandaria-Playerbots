-- Vashj'ir mercenary ships: phased moving transports, as on retail.
--
-- Retail (Warcraft Wiki, MediaWiki API: "Call of Duty (Alliance)" rev 2026-07-30, "Call of Duty (Horde)" rev 2026-07-30,
-- "Stormwind Harbor" rev 2026-09-29; Wowhead tooltip API: quests 14482 / 25924 "Ride the mercenary ship to Vashj'ir"):
--   * Accepting 14482 "Call of Duty" (Recruiter Burns, Stormwind) puts the player in a new phase in the harbor; the
--     mercenary ship docks where Stormwind's Pride (The Kraken) normally takes players to Borean Tundra, and "during the
--     quest the boat to Valiance Keep is phased out and inaccessible, replaced with the mercenary ship".
--   * Horde 25924 "Call of Duty" (Commander Thorak, Durotar): the mercenary ship docks at Bladefist Bay.
--   * Both ships sail to Kelp'thar Forest, where a scripted kraken attack drops the player to Erunak Stonespeaker (until
--     7.0.3, which replaced the ride with a teleport on accept). This core has no kraken script: the ride ends at the
--     Vashj'ir stop above Kelp'thar Forest, and the quest is credited by Erunak Stonespeaker (36915 / 41618, SmartAI
--     "area explored or event happens" within 30 yd), so a rider jumps off and swims to him. The Earthshrine "Portal to
--     Vashj'ir" (207691 / 207690) only appears once the quest is rewarded (object_visibility_state 6), as on retail.
--
-- Needs 2026_10_30_1056_00_transports_phase_columns.sql (the `transports` phase columns) applied first.
--
-- Data:
--   * phase 192 ("Quest Zone-Specific 17") while 14482 is in the log, in Stormwind City (1519) and Kelp'thar Forest (4815);
--   * phase 193 ("Quest Zone-Specific 18") while 25924 is in the log, in Durotar (14) and Kelp'thar Forest (4815);
--     both phases are unused by any spawn, phase area or script here (Phase.dbc 5.4.8 has no named harbor phase);
--   * 197195 (Alliance Ship to Vashj'ir) spawned again, in phase 192; 203466 (Horde Ship to Vashj'ir) in phase 193;
--   * 190536 (Stormwind's Pride, to Valiance Keep) inverse phase 192: phased out for those players only, so it and the
--     mercenary ship are never seen together at the shared Stormwind berth. Everyone else sees exactly what they saw.
--   Both mercenary ships also stop at the same Vashj'ir sea point; with these phases no player sees both there either.
--
-- Backup (state before this file): no `transports` row for entry 197195; guid 17 =
-- (17, 203466, 'Horde Ship to Vashj''ir'); guid 8 = (8, 190536, 'Stormwind Harbor and Valiance Keep, Borean Tundra ("The
-- Kraken")'); no `phase_area` row and no CONDITION_SOURCE_TYPE_PHASE (26) condition for phases 192 / 193.
-- Undo:
--   DELETE FROM `transports` WHERE `entry` = 197195;
--   UPDATE `transports` SET `phaseUseFlags` = 0, `phaseid` = 0, `phasegroup` = 0, `name` = 'Horde Ship to Vashj''ir' WHERE `entry` = 203466;
--   UPDATE `transports` SET `phaseUseFlags` = 0, `phaseid` = 0, `phasegroup` = 0 WHERE `entry` = 190536;
--   DELETE FROM `phase_area` WHERE `PhaseId` IN (192, 193) AND `AreaId` IN (1519, 4815, 14);
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` IN (192, 193) AND `SourceEntry` = 0;
-- Takes effect at the next worldserver start (transports are spawned once at startup). Re-runnable.

-- the mercenary ships, each in its quest phase; Stormwind's Pride phased out of the Alliance one
DELETE FROM `transports` WHERE `entry` = 197195;
INSERT INTO `transports` (`guid`, `entry`, `name`, `phaseUseFlags`, `phaseid`, `phasegroup`) VALUES
(9, 197195, 'Alliance Ship to Vashj''ir (phase of quest 14482)', 0, 192, 0);
UPDATE `transports` SET `phaseUseFlags` = 0, `phaseid` = 193, `phasegroup` = 0, `name` = 'Horde Ship to Vashj''ir (phase of quest 25924)' WHERE `entry` = 203466;
UPDATE `transports` SET `phaseUseFlags` = 2, `phaseid` = 192, `phasegroup` = 0 WHERE `entry` = 190536;

-- the quest phases (QUESTSTATE mask 10 = incomplete or complete, i.e. in the log)
DELETE FROM `phase_area` WHERE `PhaseId` IN (192, 193) AND `AreaId` IN (1519, 4815, 14);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(1519, 192, 'Stormwind City - Call of Duty (14482) in the log: the mercenary Ship to Vashj''ir, Stormwind''s Pride phased out'),
(4815, 192, 'Kelp''thar Forest - Call of Duty (14482) in the log: the Alliance mercenary Ship to Vashj''ir'),
(14,   193, 'Durotar - Call of Duty (25924) in the log: the Horde mercenary Ship to Vashj''ir at Bladefist Bay'),
(4815, 193, 'Kelp''thar Forest - Call of Duty (25924) in the log: the Horde mercenary Ship to Vashj''ir');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` IN (192, 193) AND `SourceEntry` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 192, 0, 0, 0, 47, 0, 14482, 10, 0, 0, 0, 0, '', 'Phase 192 while Call of Duty (14482) is in the log'),
(26, 193, 0, 0, 0, 47, 0, 25924, 10, 0, 0, 0, 0, '', 'Phase 193 while Call of Duty (25924) is in the log');
