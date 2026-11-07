-- Stable masters who cannot stable, world-wide on purpose.
-- PrepareGossipMenu skips a gossip option whose OptionNpcflag the creature lacks, so menu 9821's "I'd like to stable my pet here."
-- (flag 0x400000) never showed, and PetHandler's stable check also requires the flag. No spawn overrides npcflag for any of these.
-- Sourced: both TDB 4.4.2 and TDB 1210 have the stable flag on every entry below, with the values used here (compared for every
-- creature_template with subname 'Stable Master'; those not listed either have the flag already or lack it in the TDBs too).
-- Ours: Farah Tamina keeps mop's quest giver bit (she starts 25535 here); the TDBs have 4194433.

-- mop 129 (63 entries), both TDBs 4194433
UPDATE `creature_template` SET `npcflag`=4194433 WHERE `entry` IN
(543,9976,9981,9982,9989,10047,10057,10058,10063,11119,15131,17666,17896,18244,18250,18984,21517,22469,23392,24067,24905,24974,
 28057,35290,41903,42875,42911,42966,43019,43151,43379,43408,43617,43630,43766,43773,43877,43979,43982,43988,43994,44007,44354,
 44378,44382,44384,45297,45298,47934,48055,48095,48216,48887,49395,49431,49577,49593,49600,49689,49755,49767,49803,53780);
-- mop 1, both TDBs 4194433
UPDATE `creature_template` SET `npcflag`=4194433 WHERE `entry` IN (19018,21336,29250,44348);
-- mop 131, both TDBs 4194433 (see above)
UPDATE `creature_template` SET `npcflag`=4194435 WHERE `entry`=49408;
-- mop 1, both TDBs 4194304; neither entry has a spawn, so this changes nothing in game
UPDATE `creature_template` SET `npcflag`=4194304 WHERE `entry` IN (9896,19325);
