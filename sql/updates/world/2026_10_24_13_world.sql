-- Eversong Woods and Sunstrider Isle: creature abilities
-- Most of the zone is scripted, but the Mana Wyrms never cast Faerie Fire and several creatures had no AI. Port their abilities from
-- the Cataclysm data, and from the classic data where Cataclysm has none (Eversong Tender, the dragonhawks, Thaelis the Hungerer,
-- Ether Fiend, Tregla). The Mana Wyrms keep their Arcane Torrent quest credit.

SET @ENTRIES := '15274,15494,15635,15649,15650,15949,15967,16855';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(15274, 0, 0, 0, 8, 0, 100, 1, 28730, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit ''Arcane Torrent'' - Cast ''Quest Credit 8346'' (Normal Dungeon)'),
(15274, 0, 1, 0, 8, 0, 100, 1, 25046, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit ''Arcane Torrent'' - Cast ''Quest Credit 8346'' (Normal Dungeon)'),
(15274, 0, 2, 0, 8, 0, 100, 1, 69179, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit Arcane Torrent - Quest Credit'),
(15274, 0, 3, 0, 8, 0, 100, 1, 80483, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit Arcane Torrent - Quest Credit'),
(15274, 0, 4, 0, 8, 0, 100, 1, 50613, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit Arcane Torrent - Quest Credit'),
(15274, 0, 5, 0, 8, 0, 100, 1, 129597, 0, 0, 0, 11, 61314, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - On Spellhit Arcane Torrent - Quest Credit'),
(15274, 0, 6, 0, 0, 0, 100, 0, 4000, 8000, 8000, 12000, 11, 25602, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Mana Wyrm - In Combat - Cast ''Faerie Fire'''),
(15494, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 11, 11939, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Yasmine Teli/Larien - On Just Summoned - Summon Imp'),
(15635, 0, 0, 0, 2, 0, 100, 1, 10, 30, 0, 0, 11, 12160, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eversong Tender - Between 10-30% Health - Cast ''Rejuvenation'' (No Repeat)'),
(15649, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 29117, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Feral Dragonhawk Hatchling - Between 5-15% Health - Cast ''Feather Burst'' (No Repeat)'),
(15650, 0, 0, 0, 0, 0, 100, 0, 3100, 3100, 22800, 22800, 11, 29117, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crazed Dragonhawk - In Combat - Cast ''Feather Burst'''),
(15949, 0, 0, 0, 0, 0, 100, 0, 2500, 3000, 9500, 10000, 11, 6818, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thaelis the Hungerer - In Combat - Cast ''Corrupted Intellect'''),
(15967, 0, 0, 0, 2, 0, 100, 1, 20, 40, 0, 0, 11, 7994, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ether Fiend - Between 20-40% Health - Cast ''Nullify Mana'' (No Repeat)'),
(16855, 0, 0, 0, 0, 0, 100, 0, 4500, 6000, 25000, 26000, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tregla - In Combat - Cast ''Pierce Armor''');
