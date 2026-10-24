-- Vashj'ir: creature abilities
-- Most of the zone's creatures (the naga and the sea creatures of the Vashj'ir waters) had no AI or ran classic rows: port their
-- abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest scripts already
-- in place; gossip and quest reward scripts of the references are left out. The creatures no reference scripts cast their sniffed
-- spells (creature_template spell1-8).

SET @ENTRIES := '39418,39638,40200,40649,40728,40889,40911,40912,41249,41250';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39418, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 79175, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Slap & Chop'),
(39418, 0, 1, 0, 0, 0, 100, 0, 4000, 12000, 10000, 22000, 11, 79174, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Sand Spray'),
(39638, 0, 0, 0, 9, 0, 100, 0, 0, 5, 15000, 25000, 11, 31553, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Hamstring on Close'),
(39638, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 11, 79444, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Impale'),
(39638, 0, 2, 0, 13, 0, 100, 0, 5000, 8000, 20000, 30000, 11, 8242, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shield Slam on Player Spell Cast'),
(40200, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 79417, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Toxic Bite'),
(40649, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 35201, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Paralytic Poison'),
(40728, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 79449, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Body Slam on Aggro'),
(40728, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 79490, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Diseased Bite'),
(40889, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 12000, 13000, 11, 79443, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Sand Step'),
(40911, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 11, 38556, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Throw'),
(40911, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Flee at 15% HP'),
(40912, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 79442, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Snapjaw'),
(41249, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 11, 61168, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Throw'),
(41250, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 34447, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Arcane Missiles'),
(41250, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Flee at 15% HP'),
(41250, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 12544, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Frost Armor on Spawn'),
(41250, 0, 3, 0, 0, 0, 100, 0, 10000, 10000, 15000, 20000, 11, 79539, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Cast Crashing Wave'),
(41250, 0, 4, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 79232, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Hymn of Sira''kess at 40% HP');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '42424,42726,50052,50575';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(42424, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 57790, 0, 0, 2, 'Stygian Bounty Deckhand - In Combat - Cast ''Bloodthirst'''),
(42424, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 61490, 0, 0, 2, 'Stygian Bounty Deckhand - Within 8-25 Range - Cast ''Intercept'''),
(42726, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 78146, 0, 0, 2, 'Earthwatcher Shaman - In Combat - Cast ''Lava Lash'''),
(42726, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 78144, 0, 0, 2, 'Earthwatcher Shaman - In Combat - Cast ''Stormstrike'''),
(50052, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79414, 0, 0, 2, 'Burgy Blackheart - In Combat - Cast ''Brittle Touch'''),
(50052, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 75380, 0, 0, 2, 'Burgy Blackheart - In Combat - Cast ''Parrrley!'''),
(50052, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 75361, 0, 0, 2, 'Burgy Blackheart - In Combat - Cast ''Swashbuckling Slice'''),
(50575, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 57790, 0, 0, 2, 'Iron Reaper Deckhand - In Combat - Cast ''Bloodthirst'''),
(50575, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 61490, 0, 0, 2, 'Iron Reaper Deckhand - Within 8-25 Range - Cast ''Intercept''');
