-- Northern Barrens: one Boahn (3672), the rare Druid of the Fang outside the Wailing Caverns
-- There were three, none pooled: guid 35328 at (-586, -2326), where the 4.3.4 sniffs have him, in every phase, and a pair 2.8 yards
-- apart (263366, 264200) at (-707, -2295). Keep 35328 in the normal phase.
UPDATE `creature` SET `phaseMask` = 1 WHERE `guid` = 35328 AND `id` = 3672;
DELETE FROM `creature` WHERE `guid` IN (263366, 264200) AND `id` = 3672;
DELETE `ca` FROM `creature_addon` `ca` LEFT JOIN `creature` `c` ON `c`.`guid` = `ca`.`guid` WHERE `ca`.`guid` IN (263366, 264200) AND `c`.`guid` IS NULL;
