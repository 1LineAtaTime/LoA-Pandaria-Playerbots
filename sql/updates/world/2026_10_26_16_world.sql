-- The Deadmines (heroic): remove the static Helix' Crew (49137, 49138, 49139) on the walkway above Helix Gearbreaker
-- They are sniffed copies of the crew Helix summons on heroic, standing in the row where boss_helix_gearbreaker summons its
-- scripted Helix' Crew (49136) once he is engaged. They have no AI and stood there doing nothing.
DELETE FROM `creature` WHERE `guid` IN (326814, 326826) AND `id` = 49137;
DELETE FROM `creature` WHERE `guid` IN (326812, 326825) AND `id` = 49138;
DELETE FROM `creature` WHERE `guid` IN (326811, 326824) AND `id` = 49139;
DELETE `ca` FROM `creature_addon` `ca` LEFT JOIN `creature` `c` ON `c`.`guid` = `ca`.`guid` WHERE `ca`.`guid` IN (326811, 326812, 326814, 326824, 326825, 326826) AND `c`.`guid` IS NULL;
