-- ported from PR 15 2026_10_27_67_world.sql (#986)
-- The Great Escape (27438) starts at Tempest's Reach, where retail's Flight of the Val'kyr (85073) and Summon Belmont (85079) put
-- Arthura and Belmont and where our data has the quest's Belmont (45312) kneeling by Godfrey's corpse, now shown. The static
-- Belmont 45474 at Forward Command is a copy of the escort Summon Belmont (84944) summons, absent from both world databases; it
-- stood in the overrun camp. It was the only ender of A Man Named Godfrey and Resistance is Futile, so Belmont 45312 takes
-- them instead (ours): at Emberstone until A Man Named Godfrey is completed, then at Tempest's Reach.
DELETE FROM `creature_addon` WHERE `guid` = 373716;
DELETE FROM `creature` WHERE `guid` = 373716 AND `id` = 45474;
DELETE FROM `creature_questender` WHERE `id` = 45312 AND `quest` IN (27406, 27423);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(45312, 27406),
(45312, 27423);
