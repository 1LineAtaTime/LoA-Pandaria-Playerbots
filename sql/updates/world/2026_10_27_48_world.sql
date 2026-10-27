-- A Wolf in Bear's Clothing (27510): the Inconspicuous Bears are worgen druids that only show themselves when beaten
-- (their script turns them into Worgen Druids 45751 at 1% health). The template addon put Worgen Druid Form, the
-- transform into 45751, on them from the start, so they never looked like bears. TCPP has no such aura.
UPDATE `creature_template_addon` SET `auras`='' WHERE `entry`=45750 AND `auras`='85359';
-- The bears were spawned 69 times, in two sniffed batches whose spawns stand 1-4 yards apart: within the 3 yards each one
-- wanders, so they are the same bears twice (retail has 29). Remove the later spawn of each such pair (48 are left).
DELETE FROM `creature_addon` WHERE `guid` IN (188256,188259,188260,188271,188272,188276,188278,188280,188288,188365,188368,188369,188379,188383,188384,188385,188387,188388,188391,188440,188441);
DELETE FROM `creature` WHERE `id`=45750 AND `guid` IN (188256,188259,188260,188271,188272,188276,188278,188280,188288,188365,188368,188369,188379,188383,188384,188385,188387,188388,188391,188440,188441);
