-- The Nexus: creature abilities
-- Most of the dungeon's creatures (the Blue Dragonflight, the Kirin Tor rebels and the crystalline creatures) had no AI or ran
-- classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events,
-- actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '26727,26730,26734,26746';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26727, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 30, 1, 2, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - On Reset - Set Random Event Phase'),
(26727, 0, 1, 0, 0, 1, 100, 2, 5000, 10000, 60000, 60000, 11, 50182, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Aura of Arcane Haste'),
(26727, 0, 2, 0, 0, 1, 100, 4, 5000, 10000, 60000, 60000, 11, 56827, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Aura of Arcane Haste'),
(26727, 0, 3, 0, 0, 1, 100, 1, 0, 0, 0, 0, 11, 35191, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Arcane Explosion Proc'),
(26727, 0, 4, 0, 0, 1, 100, 0, 3000, 6000, 30000, 40000, 11, 47789, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Arcane Trap'),
(26727, 0, 5, 0, 0, 1, 100, 0, 3000, 6000, 5000, 10000, 11, 13323, 0, 0, 0, 0, 0, 6, 30, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Polymorph'),
(26727, 0, 6, 0, 0, 2, 100, 2, 0, 1000, 3000, 3500, 11, 12737, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Frostbolt'),
(26727, 0, 7, 0, 0, 2, 100, 4, 0, 1000, 3000, 3500, 11, 56837, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Frostbolt'),
(26727, 0, 8, 0, 0, 2, 100, 2, 4000, 9000, 12000, 12500, 11, 15244, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Cone of Cold'),
(26727, 0, 9, 0, 0, 2, 100, 4, 4000, 9000, 12000, 12500, 11, 38384, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Cone of Cold'),
(26727, 0, 10, 0, 0, 2, 100, 0, 3000, 6000, 30000, 40000, 11, 55040, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Freezing Trap'),
(26727, 0, 11, 0, 0, 4, 100, 2, 0, 1000, 3000, 3500, 11, 12466, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Fireball'),
(26727, 0, 12, 0, 0, 4, 100, 4, 0, 1000, 3000, 3500, 11, 17290, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Fireball'),
(26727, 0, 13, 0, 0, 4, 100, 2, 4000, 9000, 22000, 25500, 11, 36808, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Rain of Fire'),
(26727, 0, 14, 0, 0, 4, 100, 4, 4000, 9000, 22000, 25500, 11, 39376, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Rain of Fire'),
(26727, 0, 15, 0, 0, 4, 100, 0, 3000, 6000, 30000, 40000, 11, 47784, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Hunter Ascendant - In Combat - Cast Immolation Trap'),
(26730, 0, 1, 0, 0, 0, 100, 6, 8000, 10000, 9500, 15000, 11, 30849, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Spell Lock'),
(26730, 0, 2, 0, 0, 0, 100, 0, 2000, 10000, 15000, 25000, 11, 50131, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mage Slayer - In Combat - Cast Draw Magic'),
(26734, 0, 0, 0, 0, 0, 100, 6, 3000, 4000, 12000, 19000, 11, 39171, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Azure Enforcer - In Combat - Cast ''Mortal Strike'' (Phase 4) (No Repeat) (Dungeon)'),
(26734, 0, 1, 0, 0, 0, 100, 6, 6300, 8100, 9500, 15000, 11, 15284, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Azure Enforcer - In Combat - Cast ''Cleave'' (Phase 4) (No Repeat) (Dungeon)'),
(26734, 0, 2, 0, 0, 0, 100, 0, 2000, 10000, 10000, 15000, 11, 58460, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Azure Enforcer - In Combat - Cast Brutal Strike'),
(26746, 0, 0, 0, 0, 0, 100, 6, 3000, 4000, 1000, 1500, 11, 33833, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crazed Mana-Wraith - In Combat - Cast Arcane Missiles'),
(26746, 0, 1, 0, 0, 0, 100, 0, 0, 5000, 10000, 10000, 11, 33832, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crazed Mana-Wraith - In Combat - Cast Arcane Missiles'),
(26746, 0, 2, 0, 54, 0, 100, 0, 0, 0, 0, 0, 11, 51347, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Crazed Mana-Wraith - Is Summoned By - Cast Teleport Visual');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '53472';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(53472, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 99494, 64, 0, 2, 'Thyrinar - In Combat - Cast ''Shadow Beam'''),
(53472, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 99501, 0, 0, 2, 'Thyrinar - In Combat - Cast ''Summon Beam Stalker'''),
(53472, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 99148, 0, 0, 2, 'Thyrinar - In Combat - Cast ''Twilight Blast''');
