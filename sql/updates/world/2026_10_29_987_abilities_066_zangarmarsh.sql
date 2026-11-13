-- #987 wave 1 (pure additions): Zangarmarsh creature abilities, imported from DeltaTho PR 15 file 2026_10_24_72_world.sql.
-- 24 creatures, 51 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '18046,18122,18201,18212,18214,18340,18680,18681,18682,18909,18910,18912,18922,18992,19734,20090,20283,20290,20291,20293,20442,21894,51878,51880';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18046, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 35472, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rajah Haghazed - On Aggro - Cast ''Shield Charge'' (No Repeat)'),
(18046, 0, 1, 0, 0, 0, 100, 0, 2500, 5000, 10000, 15000, 11, 35473, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rajah Haghazed - In Combat - Cast ''Forceful Cleave'''),
(18046, 0, 2, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rajah Haghazed - Between 20-80% Health - Cast ''Mortal Strike'' (No Repeat)'),
(18046, 0, 3, 0, 2, 0, 100, 1, 5, 30, 0, 0, 11, 15062, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rajah Haghazed - Between 5-30% Health - Cast ''Shield Wall'' (No Repeat)'),
(18122, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 41363, 0, 0, 0, 0, 0, 19, 18122, 5, 0, 0, 0, 0, 0, 'Dreghood Drudge - On Aggro - Cast ''Shared Bonds'' (No Repeat)'),
(18201, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 35336, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tamed Sporebat - Between 5-15% Health - Cast ''Energizing Spores'' (No Repeat)'),
(18212, 0, 0, 0, 2, 0, 100, 1, 5, 10, 0, 0, 11, 3019, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mudfin Frenzy - Between 5-10% Health - Cast ''Frenzy'' (No Repeat)'),
(18214, 0, 0, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 3391, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fenclaw Thrasher - Between 20-80% Health - Cast ''Thrash'' (No Repeat)'),
(18340, 0, 0, 0, 2, 0, 100, 0, 0, 90, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-90% Health - Cast ''Toughen'' I'),
(18340, 0, 1, 0, 2, 0, 100, 0, 0, 80, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-80% Health - Cast ''Toughen'' II'),
(18340, 0, 2, 0, 2, 0, 100, 0, 0, 70, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-70% Health - Cast ''Toughen'' III'),
(18340, 0, 3, 0, 2, 0, 100, 0, 0, 60, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-60% Health - Cast ''Toughen'' IV'),
(18340, 0, 4, 0, 2, 0, 100, 0, 0, 50, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-50% Health - Cast ''Toughen'' V'),
(18340, 0, 5, 0, 2, 0, 100, 0, 0, 40, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-40% Health - Cast ''Toughen'' VI'),
(18340, 0, 6, 0, 2, 0, 100, 0, 0, 30, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-30% Health - Cast ''Toughen'' VII'),
(18340, 0, 7, 0, 2, 0, 100, 0, 0, 20, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-20% Health - Cast ''Toughen'' VIII'),
(18340, 0, 8, 0, 2, 0, 100, 0, 0, 10, 0, 0, 11, 33962, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Steam Pump Overseer - Between 0-10% Health - Cast ''Toughen'' IX'),
(18680, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 32039, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marticar - On Aggro - Cast ''Magnetic Pull'''),
(18680, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 16000, 11, 35493, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marticar - Within 0-8 Range - Cast ''Forked Lightning Tether'''),
(18680, 0, 2, 0, 9, 0, 100, 0, 10, 40, 12000, 16000, 11, 32039, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Marticar - Within 10-40 Range - Cast ''Magnetic Pull'''),
(18681, 0, 0, 0, 9, 0, 100, 0, 0, 10, 8000, 15000, 11, 33860, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Emissary - Within 0-10 Range - Cast ''Arcane Explosion'''),
(18681, 0, 1, 0, 0, 0, 100, 0, 6000, 10000, 14000, 21000, 11, 39207, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Emissary - In Combat - Cast ''Water Spout'''),
(18681, 0, 2, 0, 9, 0, 100, 0, 0, 10, 12000, 16000, 11, 11831, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Emissary - Within 0-10 Range - Cast ''Frost Nova'''),
(18681, 0, 3, 0, 9, 0, 100, 0, 0, 40, 3400, 4800, 11, 20297, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Emissary - Within 0-40 Range - Cast ''Frostbolt'''),
(18682, 0, 0, 0, 9, 0, 100, 0, 0, 5, 12000, 16000, 11, 35238, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Bog Lurker - Within 0-5 Range - Cast ''War Stomp'''),
(18682, 0, 1, 0, 2, 0, 100, 0, 0, 50, 22000, 26000, 11, 34163, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bog Lurker - Between 0-50% Health - Cast ''Fungal Regrowth'''),
(18909, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zabra''jin Guard - In Combat - Cast ''Shield Block'''),
(18910, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Swamprat Guard - In Combat - Cast ''Revenge'''),
(18910, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Swamprat Guard - In Combat - Cast ''Shield Block'''),
(18912, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 35230, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sporelok - Out of Combat - Cast ''Sporeskin'''),
(18922, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Telredor Guard - In Combat - Cast ''Revenge'''),
(18922, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Telredor Guard - In Combat - Cast ''Shield Block'''),
(18992, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 35491, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Krosh - In Combat - Cast ''Furious Rage'''),
(19734, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 35273, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fungal Giant - In Combat - Cast ''Unstable Mushroom Primer'''),
(20090, 0, 0, 0, 2, 0, 100, 1, 5, 15, 0, 0, 11, 11831, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodscale Sentry - Between 5-15% Health - Cast ''Frost Nova'' (No Repeat)'),
(20283, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 15000, 30000, 11, 12612, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshrock Stomper - In Combat - Cast ''Stomp'''),
(20283, 0, 1, 0, 2, 0, 30, 0, 0, 30, 0, 0, 11, 28728, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Marshrock Threshalisk - Between 0-30% Health - Cast ''Feign Death'''),
(20283, 0, 2, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 11, 35385, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshrock Threshalisk - In Combat - Cast ''Threshalisk Charge'' (Phase 1)'),
(20290, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 35319, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lagoon Eel - On Aggro - Cast ''Electric Skin'' (No Repeat)'),
(20291, 0, 0, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 6870, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lagoon Walker - Between 20-80% Health - Cast ''Moss Covered Feet'' (No Repeat)'),
(20291, 0, 1, 0, 2, 0, 100, 1, 10, 40, 0, 0, 11, 7948, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lagoon Walker - Between 10-40% Health - Cast ''Wild Regeneration'' (No Repeat)'),
(20293, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 3248, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bogstrok Clacker - On Aggro - Cast ''Improved Blocking'' (No Repeat)'),
(20442, 0, 0, 0, 2, 0, 100, 1, 10, 30, 0, 0, 11, 35491, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captain Bo''kar - Between 10-30% Health - Cast ''Furious Rage'' (No Repeat)'),
(21894, 0, 0, 0, 0, 0, 100, 0, 0, 0, 14000, 16000, 11, 36414, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Xeleth - In Combat - Cast ''Focused Bursts'''),
(21894, 0, 1, 0, 0, 0, 75, 0, 8000, 14000, 8000, 14000, 11, 36398, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Xeleth - In Combat - Cast ''Tongue Lash'' (75%)'),
(51878, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Telredor Guard - In Combat - Cast ''Revenge'''),
(51878, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Telredor Guard - In Combat - Cast ''Shield Block'''),
(51878, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Telredor Guard - In Combat - Cast ''Shoot'''),
(51880, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zabra''jin Guard - In Combat - Cast ''Revenge'''),
(51880, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Zabra''jin Guard - In Combat - Cast ''Shield Block'''),
(51880, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zabra''jin Guard - In Combat - Cast ''Shoot''');
