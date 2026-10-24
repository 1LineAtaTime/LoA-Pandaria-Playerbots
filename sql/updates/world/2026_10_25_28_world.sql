-- Lost City of the Tol'vir: creature abilities
-- Most of the dungeon's creatures (the Neferset, the Ramkahen's foes and the city's beasts) had no AI or ran classic rows: port
-- their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest scripts
-- already in place; gossip and quest reward scripts of the references are left out, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '45065';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45065, 0, 0, 0, 0, 0, 100, 0, 1, 1, 1000, 1000, 11, 84144, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1 and 1 ms (and later repeats every 1000 and 1000 ms) - Self: Cast spell 84144 on Random hostile // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '44261,45063';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(44261, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 89212, 0, 0, 2, 'Sharptalon Eagle - In Combat - Cast ''Eagle Claw'''),
(45063, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 84125, 64, 0, 2, 'Bonesnapper Scorpid - In Combat - Cast ''Paralytic Venom''');
