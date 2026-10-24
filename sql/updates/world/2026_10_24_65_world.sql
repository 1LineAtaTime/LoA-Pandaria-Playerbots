-- Razorfen Downs: creature abilities
-- Most of the dungeon's creatures (the Razorfen and Death's Head quilboar, the Scourge and the boars) had no AI or ran classic rows. Cataclysm left these classic dungeons
-- largely as they were, so port their abilities from the classic (AzerothCore) data first, with its dungeon flags and spell-set
-- phases, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '7332,7341,7347,7348,7352,7353,7357,8567,14686';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7332, 0, 0, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Withered Spearhide - Between 0-15% Health - Flee For Assist (No Repeat) (Normal Dungeon)'),
(7332, 0, 1, 0, 0, 0, 100, 0, 0, 1000, 2000, 2000, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Withered Spearhide - In Combat - Cast Shoot'),
(7332, 0, 2, 0, 0, 0, 100, 0, 4000, 9000, 12000, 20000, 11, 11397, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Withered Spearhide - In Combat - Cast Diseased Shot'),
(7332, 0, 3, 0, 2, 0, 100, 1, 0, 25, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Withered Spearhide - In Combat - Between Health 0-25% - Cast Enrage'),
(7341, 0, 0, 0, 0, 0, 100, 2, 0, 0, 2400, 3800, 11, 9672, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skeletal Frostweaver - In Combat CMC - Cast ''Frostbolt'''),
(7347, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12539, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Boneflayer Ghoul - On Reset - Cast Ghoul Rot'),
(7347, 0, 1, 0, 0, 0, 100, 0, 3000, 7000, 7000, 11000, 11, 40504, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Boneflayer Ghoul - In Combat - Cast Cleave'),
(7348, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12539, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thorn Eater Ghoul - On Reset - Cast Ghoul Rot'),
(7348, 0, 1, 0, 0, 0, 100, 0, 3000, 7000, 7000, 11000, 11, 12538, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thorn Eater Ghoul - In Combat - Cast Ravenous Claw'),
(7352, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12529, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Soul - On Reset - Cast Chilling Touch'),
(7352, 0, 1, 0, 0, 0, 100, 0, 3000, 13000, 15000, 31000, 11, 12528, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Soul - In Combat - Cast Silence'),
(7353, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12529, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Freezing Spirit - On Reset - Cast Chilling Touch'),
(7353, 0, 1, 0, 9, 0, 100, 0, 0, 10, 15000, 31000, 11, 15532, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Freezing Spirit - In Combat - Cast Frost Nova'),
(7357, 0, 0, 0, 0, 0, 100, 0, 0, 1000, 3000, 3500, 11, 12466, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mordresh Fire Eye - In Combat - Cast Fireball'),
(7357, 0, 1, 0, 9, 0, 100, 0, 0, 9, 10000, 16000, 11, 12470, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mordresh Fire Eye - Within Range 0-9yd - Cast Fire Nova'),
(8567, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12627, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glutton - On Reset - Cast Disease Cloud'),
(14686, 0, 0, 0, 37, 0, 100, 0, 0, 0, 0, 0, 11, 28533, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lady Falther''ess - On Initialize - Cast ''Transform'' (Salma Saldean)'),
(14686, 0, 1, 0, 0, 0, 100, 0, 2500, 8000, 10000, 18000, 11, 22743, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lady Falther''ess - In Combat - Cast ''Ribbon of Souls'''),
(14686, 0, 2, 0, 0, 0, 100, 0, 17500, 20000, 19000, 22000, 11, 17105, 0, 0, 0, 0, 0, 5, 30, 1, 2, 0, 0, 0, 0, 'Lady Falther''ess - In Combat - Cast ''Banshee Curse''');
