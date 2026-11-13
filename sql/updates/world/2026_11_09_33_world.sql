-- Gorkas (4314): one Enraged Wyvern per aggro, not two (as Urda in Hammerfall).
-- Sourced: TDB 4.4.2 and 1210 have a single summon on aggro (id 0, spell 14307, absent from 5.4.8) and the line (id 2), no id 1.
-- mop's id 0 already summons 9297 directly; id 1 was a second identical summon linked between them.
DELETE FROM `smart_scripts` WHERE `entryorguid`=4314 AND `source_type`=0 AND `id`=1;
UPDATE `smart_scripts` SET `link`=2 WHERE `entryorguid`=4314 AND `source_type`=0 AND `id`=0;
