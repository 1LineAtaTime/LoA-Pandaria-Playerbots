-- #989 (Refs #989): Deadmines heroic: remove static Helix' Crew (49137-49139) above Helix
-- Source: DeltaTho playerbots-fork PR 15, file 2026_10_26_16 (commit as in deltatho/playerbots-fork), ported as-is unless noted below.
-- Design check 2026-10-29: boss_helix_gearbreaker summons npc_helix_crew (49136) itself (src/server/scripts/EasternKingdoms/Deadmines/boss_helix_gearbreaker.cpp:745); the static AI-less copies are sniff leftovers. All 6 guids exist in our DB (map 36, spawnMask 4).
-- Idempotent: DELETE+INSERT / absolute UPDATEs.

-- The Deadmines (heroic): remove the static Helix' Crew (49137, 49138, 49139) on the walkway above Helix Gearbreaker
-- They are sniffed copies of the crew Helix summons on heroic, standing in the row where boss_helix_gearbreaker summons its
-- scripted Helix' Crew (49136) once he is engaged. They have no AI and stood there doing nothing.
DELETE FROM `creature` WHERE `guid` IN (326814, 326826) AND `id` = 49137;
DELETE FROM `creature` WHERE `guid` IN (326812, 326825) AND `id` = 49138;
DELETE FROM `creature` WHERE `guid` IN (326811, 326824) AND `id` = 49139;
DELETE `ca` FROM `creature_addon` `ca` LEFT JOIN `creature` `c` ON `c`.`guid` = `ca`.`guid` WHERE `ca`.`guid` IN (326811, 326812, 326814, 326824, 326825, 326826) AND `c`.`guid` IS NULL;
