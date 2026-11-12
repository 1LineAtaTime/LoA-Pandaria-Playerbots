-- Second battle for Andorhal, the val'kyr layer (phase 272): Aradne and the Forsaken Troopers on the Undercity template.
-- Sourced: both TDBs (4.4.2 and 12.1.0) give Aradne 45235, the Depravity scene's Aradne 45246 and the Forsaken Troopers
-- 45240-45243 (45241-45243 credit as 45240) FactionTemplate 68 (Undercity); mop has 1801 (faction 67 "Horde", which is also
-- hostile to monster-group creatures). Both are hostile to the Alliance and friendly to the Horde and to the Deathguards (2213).
-- Ours: nothing.
UPDATE `creature_template` SET `faction`=68 WHERE `entry` IN (45235,45240,45241,45242,45243,45246);
