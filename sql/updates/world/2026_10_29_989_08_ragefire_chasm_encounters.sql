-- #989 (2026-10-29, lane B: RFC LFG completion never fired - lfgstate stayed 5 after all 4 bosses): the MoP Ragefire Chasm
-- layout (989_05) brought the bosses but no instance_encounters rows, so InstanceScript::UpdateEncounterState never reaches
-- LFGMgr::FinishDungeon. Rows from the reference 5.4.8 world DB: encounters 1443-1446, last = Lava Guard
-- Gordoth 61528 -> LFG dungeon 4 (Ragefire Chasm). Idempotent. Undo: DELETE FROM instance_encounters WHERE entry IN (1443,1444,1445,1446).
DELETE FROM `instance_encounters` WHERE `entry` IN (1443, 1444, 1445, 1446);
INSERT INTO `instance_encounters` (`entry`, `difficulty`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`) VALUES
(1443, 'DUNGEON_NORMAL', 0, 61408, 0, 'Adarogg'),
(1444, 'DUNGEON_NORMAL', 0, 61412, 0, 'Dark Shaman Koranthal'),
(1445, 'DUNGEON_NORMAL', 0, 61463, 0, 'Slagmaw'),
(1446, 'DUNGEON_NORMAL', 0, 61528, 4, 'Lava Guard Gordoth');
