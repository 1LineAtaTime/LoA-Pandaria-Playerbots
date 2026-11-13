-- Eastern Plaguelands, Amidst Death, Life (27421): Rayne's Seeds credited the ziggurat but no flower ever bloomed.
--
-- What the player should see (sourced): on the pedestal of each Fungal Vale ziggurat the seeds credit that ziggurat and "create
-- flower beds that spread out over about half the ziggurat's floor before dying out" (wiki note); "You can plant more seeds in the
-- same place for increased effect", "I just sat there watching flowers bloom" (comments 2013-06-27, 2010-12-03, 2010-11-28).
-- The data holds the pieces, unconnected: six spells 85439-85444 "Summon Flower, Red / Red Stemmed / Blurple / Blurple Stemmed /
-- Green / Green Stemmed" (each a wild summon of one Growing Flower 45832-45837 at a random point within 5 yd of its caster,
-- 50 s, SummonProperties 64 with no personal-spawn bit, so everyone in sight sees it), cast by nothing; and 230 invisible
-- Necropolis Flowers bunnies 45485 in three rings of 76-77, 0.3-15.1 yd around each ziggurat's controller (45486 south-east,
-- 45487 north-east, 45488 west, as quest_objective names them; the controllers are 156-187 yd apart), with no AI.
--  * Ours (no sniff or Wowhead "used by" records the caster): on the seed's hit the controller sets data 1 1 on every Necropolis
--    Flowers bunny within 20 yd (its own ring only); each bunny waits 0-8 s and casts one of the six at random, so the bed
--    spreads over the floor and dies out after 50 s. Credit stays per player (the controller's existing row).
--  * Cap: one flower per bunny, so at most 76-77 a ring. The bunny's data-set row has a 58 s cooldown (8 s longest delay + 50 s
--    flower life; SMART_EVENT_DATA_SET runs its action and then RecalcTimer(cooldown), which deactivates the event until
--    UpdateTimer counts it down: SmartScript.cpp ProcessEvent / RecalcTimer / UpdateTimer). A seed planted while the bed is up
--    gives its credit and nothing else, and cannot cancel a cast still waiting (a new timed action list would replace it,
--    SmartScript::SetScript9); a seed after the bed has died plants a new one. Without the cap Rayne's Seeds (not consumed,
--    5 s cooldown) kept some 500 flowers alive per ring.
--  * The controllers' spell-hit cooldown goes from 10 s to 1 s: a second player's seed within 10 s of the first was swallowed
--    (no credit, no flowers) and had to be cast again.
--  * Growing Flowers 45832, 45836, 45837 get NOT_SELECTABLE | IMMUNE_TO_PC | IMMUNE_TO_NPC like 45833-45835 already have, so
--    no flower can be clicked or attacked. Factions stay (35 here; 190 on 45833-45835, which also stand as 25 static
--    decorations in Twilight Highlands, untouched).
--  * Entry 45485 has spawns only in Eastern Plaguelands (230, all on these rings); 45486-45488 one each.

UPDATE `creature_template` SET `unit_flags` = 33555200 WHERE `entry` IN (45832, 45836, 45837);
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 45485;

UPDATE `smart_scripts` SET `event_param3` = 1000, `event_param4` = 1000 WHERE `entryorguid` IN (45486, 45487, 45488) AND `source_type` = 0 AND `id` = 0 AND `event_type` = 8 AND `event_param1` = 84964;
UPDATE `smart_scripts` SET `link` = 2 WHERE `entryorguid` IN (45486, 45487, 45488) AND `source_type` = 0 AND `id` = 1;

DELETE FROM `smart_scripts` WHERE (`entryorguid` IN (45486, 45487, 45488) AND `source_type` = 0 AND `id` = 2) OR (`entryorguid` = 45485 AND `source_type` = 0) OR (`entryorguid` BETWEEN 4548500 AND 4548505 AND `source_type` = 9);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45486, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 11, 45485, 20, 0, 0, 0, 0, 0, 0, 'Ziggurat - On Link - Set Data 1 1 on Necropolis Flowers within 20 yd'),
(45487, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 11, 45485, 20, 0, 0, 0, 0, 0, 0, 'Ziggurat - On Link - Set Data 1 1 on Necropolis Flowers within 20 yd'),
(45488, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 11, 45485, 20, 0, 0, 0, 0, 0, 0, 'Ziggurat - On Link - Set Data 1 1 on Necropolis Flowers within 20 yd'),
(45485, 0, 0, 0, 38, 0, 100, 0, 1, 1, 58000, 58000, 0, 87, 4548500, 4548501, 4548502, 4548503, 4548504, 4548505, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - On Data Set 1 1 (58 s cooldown) - Call Random Timed Action List (one flower)'),
(4548500, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85439, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Red'),
(4548501, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85440, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Red Stemmed'),
(4548502, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85441, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Blurple'),
(4548503, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85442, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Blurple Stemmed'),
(4548504, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85443, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Green'),
(4548505, 9, 0, 0, 0, 0, 100, 0, 0, 8000, 0, 0, 0, 11, 85444, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Necropolis Flowers - After 0-8 s - Cast Summon Flower, Green Stemmed');
