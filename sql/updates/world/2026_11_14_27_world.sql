-- Eastern Plaguelands: quest-item drop rates for A Gift For Fiona (27372), To Kill With Purpose (27451) and Dark Garb (27452).
-- Rule for this project: the correct rate when a source makes it certain, otherwise the less grindy candidate.
--
--  * Plaguehound Blood 60986 (27372, "from Runts and Frenzied ... a bit higher than 50 %", comments 2010-11-28 [+8],
--    2011-02-14): Plaguehound Runts 8596 drop it at 58 %, Frenzied Plaguehounds 8598 at 0.94 %, a low-sample Wowhead figure
--    (they live across the east of the zone and are seldom killed for the quest). Not certain -> the Runts' rate (ours).
--  * Living Rot 15447 (27451): mop had 50 % on all eight Corin's Crossing undead. Sources disagree: both TDBs 100 % (90 % on the
--    Overstuffed Golem 45851), a 2011 comment [+8] "not 1:1", Wowhead MoP Classic 31-39 % of all kills (which counts players
--    without the quest, so a floor). Not certain -> the TDBs' values, the less grindy ones.
--  * Death Cultist Headwear 61280 / Robes 61281 (27452) from Vile Tutors 8548 and Dark Summoners 8551: 28 % each, Wowhead's
--    observed average of a "progressive loot" rate (it rose with each failed kill) that this core cannot reproduce; a flat
--    28 % is grindier than retail was. Ours: 50 %.

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -58 WHERE `entry` = 8598 AND `item` = 60986;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 15447 AND `entry` IN (8525, 8526, 8531, 8538, 8541, 8543, 12262);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -90 WHERE `item` = 15447 AND `entry` = 45851;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `entry` IN (8548, 8551) AND `item` IN (61280, 61281);
