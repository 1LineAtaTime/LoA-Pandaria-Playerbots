-- Halls of Stone: creature abilities
-- Most of the dungeon's creatures (the iron dwarves, the stone giants and the titan constructs) had no AI or ran classic rows: port
-- their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action
-- lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags.
-- The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '27965';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(27965, 0, 0, 1, 4, 0, 100, 3, 0, 0, 0, 0, 11, 51496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - On Aggro - Cast ''Chiseling Ray'' (No Repeat) (Normal Dungeon)'),
(27965, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - On Aggro - Increment Phase By 1 (No Repeat) (Heroic Dungeon)'),
(27965, 0, 2, 0, 9, 1, 100, 2, 0, 40, 6000, 9000, 11, 51496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - Within 0-40 Range - Cast ''Chiseling Ray'' (No Repeat) (Heroic Dungeon)'),
(27965, 0, 3, 4, 4, 0, 100, 5, 0, 0, 0, 0, 11, 59034, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - On Aggro - Cast ''Chiseling Ray'' (No Repeat) (Heroic Dungeon)'),
(27965, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - Between 0-7% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(27965, 0, 5, 0, 9, 1, 100, 4, 0, 40, 6000, 9000, 11, 59034, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - Within 0-40 Range - Cast ''Chiseling Ray'' (Phase 1) (No Repeat) (Dungeon)'),
(27965, 0, 6, 0, 3, 2, 100, 6, 15, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dark Rune Shaper - Between 15-100% Mana - Decrement Phase By 1 (No Repeat) (Dungeon)'),
(27965, 0, 7, 0, 1, 0, 100, 0, 2000, 2000, 4000, 4000, 11, 50563, 0, 0, 0, 0, 0, 19, 28055, 50, 0, 0, 0, 0, 0, 'Dark Rune Shaper - On Reset - Cast ''Carve Stone''');
