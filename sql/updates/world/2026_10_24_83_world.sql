-- Mana-Tombs: creature abilities
-- Most of the dungeon's creatures (the Ethereal Darkcasters, Priests and Spellbinders, the Nexus Stalkers, the void creatures) had
-- no AI or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows
-- needing events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping
-- the dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '18313';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18313, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Out of Combat - Disable Combat Movement (No Repeat) (Dungeon)'),
(18313, 0, 7, 8, 3, 1, 100, 7, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Between 0-15% Mana - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 8, 0, 61, 1, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Between 0-15% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 9, 0, 9, 1, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Within 25-80 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 10, 0, 9, 1, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Within 5-15 Range - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 11, 0, 9, 1, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Within 0-5 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 12, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - Between 30-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18313, 0, 13, 0, 0, 0, 100, 2, 0, 0, 10500, 10500, 11, 15790, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - In Combat - Cast Fireball'),
(18313, 0, 14, 0, 0, 0, 100, 4, 0, 0, 10500, 10500, 11, 22272, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - In Combat - Cast Fireball'),
(18313, 0, 15, 0, 0, 0, 100, 0, 5000, 8000, 10000, 15000, 11, 25603, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Ethereal Sorcerer - In Combat - Cast Slow');
