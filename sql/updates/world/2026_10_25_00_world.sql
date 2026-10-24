-- Sholazar Basin: creature abilities
-- Most of the zone's creatures (the Frenzyheart and Oracles' foes, the Mosswalkers, the elementals and wildlife) had no AI or ran
-- classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events,
-- actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted. The creatures no reference
-- scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '28083,28111,29033,29684,32517';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28083, 0, 0, 1, 1, 0, 100, 0, 0, 0, 60000, 90000, 11, 20567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Out of Combat - Cast ''Ragnaros Submerge Visual'''),
(28083, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 18, 33554434, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Out of Combat - Set Flags Not Attackable & Not Selectable'),
(28083, 0, 2, 3, 4, 0, 100, 0, 0, 0, 0, 0, 28, 20567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Remove Aura ''Ragnaros Submerge Visual'''),
(28083, 0, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 28, 29147, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Remove Aura ''Tunnel Bore Passive'''),
(28083, 0, 4, 5, 61, 0, 100, 0, 0, 0, 0, 0, 11, 37752, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Cast ''Stand'''),
(28083, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Disable Combat Movement'),
(28083, 0, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 19, 33554434, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Remove Flags Not Attackable & Not Selectable'),
(28083, 0, 7, 0, 61, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Aggro - Set Event Phase 1'),
(28083, 0, 8, 0, 0, 1, 100, 0, 1000, 1000, 2100, 4500, 11, 20567, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - In Combat - Cast ''Ragnaros Submerge Visual'''),
(28083, 0, 9, 10, 9, 0, 100, 0, 40, 60, 0, 0, 11, 68909, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 40-60 Range - Cast ''Submerge Visual'''),
(28083, 0, 10, 11, 61, 0, 100, 0, 0, 0, 0, 0, 11, 29147, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 40-60 Range - Cast ''Tunnel Bore Passive'''),
(28083, 0, 11, 0, 61, 0, 100, 0, 0, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 40-60 Range - Set Event Phase 2'),
(28083, 0, 12, 13, 9, 1, 100, 0, 0, 8, 0, 0, 28, 20567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 0-8 Range - Remove Aura ''Ragnaros Submerge Visual'' (Phase 1)'),
(28083, 0, 13, 14, 61, 1, 100, 0, 0, 0, 0, 0, 11, 37752, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 0-8 Range - Cast ''Stand'' (Phase 1)'),
(28083, 0, 14, 15, 61, 1, 100, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 0-8 Range - Disable Combat Movement (Phase 1)'),
(28083, 0, 15, 0, 61, 1, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within 0-8 Range - Set Event Phase 1 (Phase 1)'),
(28083, 0, 16, 17, 7, 0, 100, 0, 0, 0, 0, 0, 11, 20567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Cast ''Ragnaros Submerge Visual'''),
(28083, 0, 17, 18, 61, 0, 100, 0, 0, 0, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Enable Combat Movement'),
(28083, 0, 18, 19, 61, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Set Event Phase 0'),
(28083, 0, 19, 20, 61, 0, 100, 0, 0, 0, 0, 0, 11, 29147, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Cast ''Tunnel Bore Passive'''),
(28083, 0, 20, 21, 61, 0, 100, 0, 0, 0, 0, 0, 14, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Set All Threat 100-0'),
(28083, 0, 21, 0, 61, 0, 100, 0, 0, 0, 0, 0, 18, 33554434, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - On Evade - Set Flags Not Attackable & Not Selectable'),
(28083, 0, 22, 0, 6, 0, 100, 0, 0, 0, 0, 0, 45, 1, 2, 0, 0, 0, 0, 9, 28215, 0, 50, 0, 0, 0, 0, 'Serfex the Reaver - On Just Died - Set Data 1 2'),
(28083, 0, 23, 0, 9, 0, 100, 0, 1, 50, 2000, 3500, 11, 31747, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Serfex the Reaver - Within Range 1-50yd - Cast Poison'),
(28111, 0, 0, 0, 0, 0, 100, 0, 4000, 11000, 7000, 11000, 11, 54429, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sparktouched Warrior - In Combat - Cast Lightning Whirl'),
(28111, 0, 1, 0, 0, 0, 100, 0, 8000, 20000, 15000, 31000, 11, 18501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sparktouched Warrior - In Combat - Cast Enrage'),
(29033, 0, 0, 0, 0, 0, 100, 0, 8000, 13000, 12000, 16000, 11, 21790, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Spirit of Atha - In Combat - Cast ''Aqua Jet'' (Phase 4)'),
(29033, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Spirit of Atha - In Combat - Cast ''Thrash'' (Phase 4)'),
(29684, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 54955, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ticking Bomb - On Respawn - Cast ''Ticking Bomb'' (Phase 1)'),
(29684, 0, 1, 0, 0, 0, 100, 1, 5500, 5500, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ticking Bomb - In Combat - Despawn Instant (Phase 1)'),
(29684, 0, 2, 3, 11, 0, 100, 1, 0, 0, 0, 0, 11, 54954, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ticking Bomb - On Respawn - Cast ''Explosion'' (No Repeat)'),
(29684, 0, 3, 0, 61, 0, 100, 1, 0, 0, 0, 0, 41, 2000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ticking Bomb - On Respawn - Despawn In 2000 ms (No Repeat)'),
(32517, 0, 0, 0, 0, 0, 100, 0, 3000, 7000, 12000, 15000, 11, 61184, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Loque''nahak - In Combat - Cast ''Pounce'''),
(32517, 0, 1, 0, 0, 0, 100, 0, 5000, 9000, 8000, 13000, 11, 61186, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Loque''nahak - In Combat - Cast ''Frozen Bite''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '28186';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(28186, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 36228, 0, 0, 2, 'Foreman Swindlegrin - In Combat - Cast ''Chainsaw Blade''');
