-- Urda 2851: one Enraged Wyvern per aggro, not two. Sourced: TDB 442 and 1210 have a single summon on aggro (id 0, spell 14307, absent from 5.4.8) and the yell (id 2), no id 1.
-- Mop's id 0 already summons 9297 directly; id 1 was a second identical summon linked between them.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2851 AND `source_type` = 0 AND `id` = 1;
UPDATE `smart_scripts` SET `link` = 2 WHERE `entryorguid` = 2851 AND `source_type` = 0 AND `id` = 0;
