-- The Mechanar: creature abilities
-- Most of the dungeon's creatures (the Sunseeker and Bloodwarder blood elves, the Mechanar golems and tinkers) had no AI or ran
-- classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events,
-- actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '19167,19231,19712,19713,20059,20478';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(19167, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 11, 35191, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Slayer - On Spawn - Cast ''Arcane Explosion Proc'''),
(19167, 0, 1, 0, 0, 0, 100, 0, 7200, 10600, 12100, 22900, 11, 35189, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Slayer - In Combat - Cast ''Solar Strike'' (Phase 1, 2)'),
(19167, 0, 2, 0, 0, 0, 100, 0, 7700, 7700, 10900, 21700, 11, 15708, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Slayer - In Combat - Cast ''Mortal Strike'' (Phase 3, 4)'),
(19167, 0, 3, 0, 0, 0, 100, 0, 9800, 22900, 10900, 26500, 11, 13736, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Slayer - In Combat - Cast ''Whirlwind'''),
(19231, 0, 0, 0, 0, 0, 100, 0, 11100, 19300, 22900, 28900, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Crusher - In Combat - Cast ''Glob of Machine Fluid'''),
(19231, 0, 1, 0, 0, 0, 100, 0, 7800, 13200, 13300, 21200, 11, 35055, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Crusher - In Combat - Cast ''The Claw'''),
(19712, 0, 0, 0, 0, 0, 100, 0, 13300, 37400, 21700, 33800, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Driller - In Combat - Cast ''Glob of Machine Fluid'''),
(19712, 0, 1, 0, 0, 0, 100, 0, 4700, 18100, 7200, 16900, 11, 35047, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Driller - In Combat - Cast ''Drill Armor'''),
(19713, 0, 0, 0, 0, 0, 100, 0, 15200, 27700, 21700, 36800, 11, 35056, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Wrecker - In Combat - Cast ''Glob of Machine Fluid'''),
(19713, 0, 1, 0, 0, 0, 100, 0, 15700, 22500, 17300, 26500, 11, 35049, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mechanar Wrecker - In Combat - Cast ''Pound'''),
(20059, 0, 0, 0, 0, 0, 100, 0, 12100, 19300, 10800, 25300, 11, 35243, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Starfire'''),
(20059, 0, 1, 0, 9, 0, 100, 0, 0, 8, 10800, 25300, 11, 35261, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - Within 0-8 Range - Cast ''Arcane Nova'''),
(20059, 0, 2, 0, 0, 0, 100, 0, 5000, 5000, 10000, 10000, 11, 17201, 0, 0, 0, 0, 0, 26, 30, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Dispel Magic'''),
(20059, 0, 3, 4, 0, 0, 100, 0, 14100, 18900, 63200, 68100, 11, 35251, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Summon Arcane Golem'''),
(20059, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 35260, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Netherbinder - In Combat - Cast ''Summon Arcane Golem'''),
(20478, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 11, 35259, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Arcane Servant - On Just Summoned - Cast ''Spotlight'''),
(20478, 0, 1, 0, 0, 0, 100, 0, 3600, 10800, 16900, 26500, 11, 35255, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcane Servant - In Combat - Cast ''Arcane Volley'''),
(20478, 0, 2, 0, 54, 0, 100, 0, 0, 0, 0, 0, 11, 22271, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Arcane Servant - On Just Summoned - Cast ''Arcane Explosion''');
