-- #987 wave 1 (pure additions): Grizzly Hills creature abilities, imported from DeltaTho PR 15 file 2026_10_24_98_world.sql.
-- 19 creatures, 27 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '23740,23745,24026,26347,26410,26544,26583,27018,27462,27467,27501,27547,27549,27593,27758,38453,51891,51893,51894';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(23740, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 29577, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frosthorn Ram - In Combat - Cast ''Hoof Strike'''),
(23745, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 53824, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Garg - In Combat - Cast ''Throw'''),
(24026, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 50075, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fanggore Worg - In Combat - Cast ''Maim Flesh'''),
(26347, 0, 0, 0, 0, 0, 100, 0, 8000, 12000, 20000, 30000, 11, 52702, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Runic War Golem - In Combat - Cast ''Rune Punch'''),
(26410, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 35000, 40000, 11, 52710, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rune-Smith Kathorn - In Combat - Cast ''Lightning Charged'''),
(26544, 0, 0, 0, 0, 0, 100, 0, 30000, 30000, 30000, 30000, 11, 3551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Warlord Zim''bo - In Combat - Cast ''Skull Crack'''),
(26544, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 2000, 4000, 11, 52283, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Warlord Zim''bo - In Combat - Cast ''Warlord Roar'''),
(26583, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 7, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Horrified Drakkari Shaman - In Combat - Cast ''Suicide'''),
(27018, 0, 0, 0, 1, 0, 100, 0, 1000, 1000, 20000, 20000, 11, 48185, 0, 0, 0, 0, 0, 19, 27024, 20, 0, 0, 0, 0, 0, 'Shade of Arugal - Out of Combat - Cast ''Arugal Channel'''),
(27018, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 2000, 3000, 11, 53086, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shade of Arugal - In Combat - Cast ''Shadow Bolt'''),
(27462, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 38861, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Hunter - In Combat - Cast ''Aimed Shot'''),
(27467, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 48815, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Amberpine Hunter - In Combat - Cast ''Shoot'''),
(27501, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 52719, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Marine - In Combat - Cast ''Concussion Blow'''),
(27501, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 50370, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Marine - In Combat - Cast ''Sunder Armor'''),
(27547, 0, 0, 1, 2, 0, 100, 0, 0, 40, 9000, 9000, 11, 32714, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vladek - In HPCT - Cast ''Enrage'''),
(27547, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 47457, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vladek - In HPCT - Cast Worgen Transform - Male'),
(27549, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Elite - In Combat - Cast ''Hamstring'''),
(27549, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Elite - In Combat - Cast ''Strike'''),
(27593, 0, 0, 0, 27, 0, 100, 0, 0, 0, 0, 0, 11, 49107, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Passenger Boarded - Cast ''Vehicle: Warhead Fuse'''),
(27593, 0, 1, 3, 8, 0, 100, 0, 49372, 0, 0, 0, 11, 49510, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Spellhit ''Horde Boat to Torpedo'' - Cast ''Alliance Kill Credit Torpedo'''),
(27593, 0, 2, 3, 8, 0, 100, 0, 49257, 0, 0, 0, 11, 49340, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Spellhit ''Alliance Boat to Torpedo'' - Cast ''Horde Kill Credit Torpedo'''),
(27593, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 49250, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Event Link - Cast ''Detonate'''),
(27758, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 14516, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Defender - In Combat - Cast ''Strike'''),
(38453, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 61184, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcturis - In Combat - Cast ''Pounce'''),
(51891, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Amberpine Footman - In Combat - Cast ''Shoot'''),
(51893, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Footman - In Combat - Cast ''Shoot'''),
(51894, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Camp Oneqwah Brave - In Combat - Cast ''Shoot''');
