-- ported from PR 15 2026_10_27_81_world.sql (#986)
-- The Forsaken Front: Agatha and Arthura of the camp after Rise, Godfrey hover by Sylvanas but were set to wander (random
-- movement), which walked them along the ground in their flying pose, half sunk into it. Keep them in place, like Daschla.
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE (`guid`, `id`) IN ((306534, 45625), (306538, 45627));
