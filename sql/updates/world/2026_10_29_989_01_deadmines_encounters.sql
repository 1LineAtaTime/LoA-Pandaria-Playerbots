-- #989 (Refs #989): Deadmines instance_encounters (Cookie ends normal, Vanessa heroic)
-- Source: DeltaTho playerbots-fork PR 15, file 2026_10_26_15 (commit as in deltatho/playerbots-fork), ported as-is unless noted below.
-- Design check 2026-10-29: Wowpedia Deadmines (MediaWiki API): bosses Cookie (normal) and Vanessa VanCleef (heroic); Wowhead tooltip API names 47162 Glubtok, 47296 Helix Gearbreaker, 43778 Foe Reaper 5000, 47626 Admiral Ripsnarl, 47739 Captain Cookie, 49541 Vanessa VanCleef. the reference 5.4.8 world DB has the same normal rows.
-- Idempotent: DELETE+INSERT / absolute UPDATEs.

-- The Deadmines (map 36): dungeon encounters for both difficulties, so the Dungeon Finder completes the normal mode
-- Only Vanessa VanCleef (1081, heroic only in DungeonEncounter.dbc) was listed, so a normal Deadmines (LFGDungeons 6), which ends
-- with "Captain" Cookie, was never finished. Cookie ends the normal mode, Vanessa the heroic one (LFGDungeons 326).
DELETE FROM `instance_encounters` WHERE `entry` IN (1060, 1062, 1063, 1064, 1065, 1081);
INSERT INTO `instance_encounters` (`entry`, `difficulty`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`) VALUES
(1064, 'DUNGEON_NORMAL', 0, 47162, 0, 'Glubtok'),
(1064, 'DUNGEON_HEROIC', 0, 47162, 0, 'Glubtok'),
(1065, 'DUNGEON_NORMAL', 0, 47296, 0, 'Helix Gearbreaker'),
(1065, 'DUNGEON_HEROIC', 0, 47296, 0, 'Helix Gearbreaker'),
(1063, 'DUNGEON_NORMAL', 0, 43778, 0, 'Foe Reaper 5000'),
(1063, 'DUNGEON_HEROIC', 0, 43778, 0, 'Foe Reaper 5000'),
(1062, 'DUNGEON_NORMAL', 0, 47626, 0, 'Admiral Ripsnarl'),
(1062, 'DUNGEON_HEROIC', 0, 47626, 0, 'Admiral Ripsnarl'),
(1060, 'DUNGEON_NORMAL', 0, 47739, 6, '"Captain" Cookie'),
(1060, 'DUNGEON_HEROIC', 0, 47739, 0, '"Captain" Cookie'),
(1081, 'DUNGEON_HEROIC', 0, 49541, 326, 'Vanessa VanCleef');
