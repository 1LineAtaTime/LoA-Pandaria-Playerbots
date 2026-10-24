-- The Oculus: creature abilities
-- Most of the dungeon's creatures (the Blue Dragonflight and the arcane constructs) had no AI or ran classic rows: port their
-- abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists
-- LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '27639,27640,27641,28276';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(27639, 0, 0, 0, 9, 0, 100, 2, 0, 40, 15000, 19000, 11, 50715, 0, 0, 0, 0, 0, 5, 40, 1, 0, 0, 0, 0, 0, 'Ring-Lord Sorceress - In Combat - Cast ''Blizzard'' (Normal Dungeon)'),
(27639, 0, 1, 0, 9, 0, 100, 4, 0, 40, 15000, 19000, 11, 59278, 0, 0, 0, 0, 0, 5, 40, 1, 0, 0, 0, 0, 0, 'Ring-Lord Sorceress - In Combat - Cast ''Blizzard'' (Heroic Dungeon)'),
(27639, 0, 2, 0, 0, 0, 100, 2, 9000, 12000, 15000, 18000, 11, 16102, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Sorceress - In Combat - Cast ''Flamestrike'' (Normal Dungeon)'),
(27639, 0, 3, 0, 0, 0, 100, 4, 9000, 12000, 15000, 18000, 11, 61402, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Sorceress - In Combat - Cast ''Flamestrike'' (Heroic Dungeon)'),
(27639, 0, 4, 0, 1, 0, 100, 0, 1000, 1000, 0, 0, 11, 51518, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Sorceress - Out of Combat - Cast ''Nexus Energy Cosmetic'''),
(27640, 0, 0, 0, 4, 0, 100, 3, 0, 0, 0, 0, 11, 50717, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Conjurer - On Aggro - Cast ''Charged Skin'' (Phase 1) (No Repeat) (Dungeon)'),
(27640, 0, 1, 0, 4, 0, 100, 5, 0, 0, 0, 0, 11, 59276, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Conjurer - On Aggro - Cast ''Charged Skin'' (Phase 1) (No Repeat) (Dungeon)'),
(27640, 0, 2, 0, 1, 0, 100, 0, 1000, 1000, 0, 0, 11, 51518, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ring-Lord Conjurer - Out of combat - Cast Visual'),
(27641, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 11, 50088, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Centrifuge Core - Out of Combat - Cast Energy Link'),
(28276, 0, 0, 0, 9, 0, 100, 2, 0, 20, 7000, 11000, 11, 51243, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Ley-Whelp - Within 0-20 Range - Cast ''Arcane Bolt'''),
(28276, 0, 1, 0, 9, 0, 100, 4, 0, 20, 7000, 11000, 11, 59215, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Ley-Whelp - Within 0-20 Range - Cast ''Arcane Bolt'''),
(28276, 0, 2, 0, 0, 0, 100, 2, 6000, 10000, 6000, 10000, 11, 62249, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Lay-Whelp (Normal) - Cast Arcane Bolt'),
(28276, 0, 3, 0, 0, 0, 100, 4, 6000, 10000, 6000, 10000, 11, 62250, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Lay-Whelp (Heroic) - Cast Arcane Bolt');
