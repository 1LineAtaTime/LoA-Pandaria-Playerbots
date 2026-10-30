-- #1026 Lost Isles 25251 "Final Confrontation": Trade Prince Gallywix (39582) had about 1.55M health, so the fight in the
-- Ultimate Footbomb Uniform (39598) took about 20 minutes.
--
-- Intended health is about 98k:
--   * the spawn row (guid 375338) stores the sniffed health, curhealth = 98260;
--   * Wowpedia "Final Confrontation" (written 2010-06-30, unchanged since): "the Trade Prince, who has almost 100,000 HP".
-- Health_mod 20 was computed against the classic (basehp0) column: 98260 / 4913 = 20.0. The template is exp 3, so the
-- core takes basehp3 (77490 at level 85, class 2) and gives 77490 x 20 = 1549800 (15.8 times too much). The reference
-- 5.4.8 data has the same mismatch (18.4419 at exp 3 = 1.43M).
-- Health_mod 1.26803 at exp 3 gives ceil(77490 x 1.26803) = 98260. The uniform spells hit for their tooltip values
-- (about 5.2k per Illegal Contact + Field Goal + Blitz cycle), so the fight now takes about 30 s to 1.5 min.
UPDATE `creature_template` SET `Health_mod` = 1.26803 WHERE `entry` = 39582;

-- Undo:
--   UPDATE `creature_template` SET `Health_mod` = 20 WHERE `entry` = 39582;
