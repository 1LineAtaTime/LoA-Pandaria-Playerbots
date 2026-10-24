-- The Stonecore: creature abilities
-- Most of the dungeon's creatures (the Twilight's Hammer, the earth elementals and the stone troggs) had no AI or ran classic rows:
-- port their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest
-- scripts already in place; gossip and quest reward scripts of the references are left out, keeping the dungeon (normal/heroic)
-- flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '43430,43537';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(43430, 0, 0, 0, 0, 0, 100, 0, 0, 0, 10000, 10000, 11, 81574, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Warbringer - In Combat - Cast ''Charge'''),
(43430, 0, 1, 0, 0, 0, 100, 0, 10000, 15000, 10000, 20000, 11, 81568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Warbringer - In Combat - Cast ''Spinning Slash'''),
(43537, 0, 0, 1, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - On Reset - Set Phase 1'),
(43537, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - Linked - Demorph'),
(43537, 0, 2, 0, 0, 1, 100, 0, 3000, 3000, 10000, 15000, 11, 81576, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - In Combat (Phase 1) - Cast ''Lava Burst'''),
(43537, 0, 3, 4, 0, 1, 100, 0, 9000, 9000, 12000, 12000, 11, 81459, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - In Combat (Phase 1) - Cast ''Force of Earth'''),
(43537, 0, 4, 0, 61, 1, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - Linked - Say Text 0'),
(43537, 0, 5, 0, 0, 1, 100, 0, 15000, 15000, 10000, 15000, 11, 81530, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - In Combat (Phase 1) - Cast ''Ground Shock'''),
(43537, 0, 6, 7, 8, 1, 100, 1, 81459, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - Hit by ''Force of Earth'' (Phase 1) - Set Phase 2'),
(43537, 0, 7, 0, 61, 1, 100, 1, 81459, 0, 0, 0, 3, 26693, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - Linked - Morph'),
(43537, 0, 8, 0, 0, 2, 100, 0, 5000, 5000, 15000, 15000, 11, 81463, 64, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - In Combat (Phase 2) - Cast ''Dust Storm'''),
(43537, 0, 9, 0, 6, 0, 100, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonecore Earthshaper - On Death - Demorph');
