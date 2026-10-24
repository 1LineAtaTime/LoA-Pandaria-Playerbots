-- The Storm Peaks: creature abilities
-- Most of the zone's creatures (the iron dwarves and vrykul, the stone giants, the Hyldnir, the elementals and wildlife) had no AI
-- or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing
-- events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted. The creatures no
-- reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '29351,29358,29475,29483,29753,29809,30152,30174,30422,35189';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(29351, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 7000, 9000, 11, 38621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Niffelem Frost Giant - IC - Cast Debilitating Strike'),
(29358, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 7000, 9000, 11, 61572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostworg - IC - Cast Frostbite'),
(29358, 0, 1, 0, 0, 0, 100, 0, 2000, 6000, 6000, 9000, 11, 50075, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostworg - IC - Cast Maim Flesh'),
(29475, 0, 0, 0, 9, 0, 100, 0, 0, 10, 250, 250, 11, 54537, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Improved Land Mine - Victim In Range 0-10 Yards - Cast Spell Detonation'),
(29483, 0, 0, 0, 9, 0, 100, 0, 0, 20, 1000, 1000, 11, 54559, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'K3 Perimeter Turret - Victim In Range 0-20 Yards - Cast ''Energy Surge'''),
(29753, 0, 0, 0, 0, 0, 100, 0, 3000, 3000, 7000, 7000, 11, 57833, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'IC Update - cast spell on victim'),
(29809, 0, 0, 0, 1, 0, 100, 1, 0, 10000, 0, 0, 11, 55291, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Son of Hodir - Out of Combat - Cast Periodic Club Throw'),
(30152, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 53, 0, 30152, 0, 12984, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 'On summoned - Start WP'),
(30152, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 83, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On link - Remove npc flags'),
(30152, 0, 2, 0, 58, 0, 100, 0, 0, 0, 0, 0, 66, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On summoned - Set orintation'),
(30152, 0, 3, 0, 38, 0, 100, 0, 0, 1, 0, 0, 80, 3015200, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On data - Run script'),
(30152, 0, 4, 5, 38, 0, 100, 0, 1, 1, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On data - Despawn'),
(30152, 0, 5, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On link - Talk'),
(30152, 0, 6, 0, 2, 0, 100, 0, 0, 30, 120000, 130000, 11, 56330, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bruor Ironbane - Between 0-30% Health - Cast ''Iron''s Bane'' (No Repeat)'),
(30174, 0, 0, 0, 1, 0, 100, 0, 0, 0, 0, 0, 2, 31, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hyldsmeet Warbear - Out of Combat - Set Faction 31 (No Repeat)'),
(30174, 0, 1, 0, 9, 0, 100, 0, 8, 25, 10000, 12500, 11, 54460, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Brunnhildar Challenger - Within Range 8-25yd - Cast Charge'),
(30174, 0, 2, 0, 0, 0, 100, 0, 2000, 5000, 15000, 22500, 11, 15971, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Brunnhildar Challenger - In Combat - Cast Demoralizing Roar'),
(30174, 0, 3, 0, 0, 0, 100, 0, 7000, 11000, 9000, 19500, 11, 54458, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Brunnhildar Challenger - In Combat - Cast Smash'),
(30422, 0, 0, 1, 2, 0, 100, 1, 0, 10, 0, 0, 11, 56732, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On HP Update - Cast Spell'),
(30422, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 56731, 0, 0, 0, 0, 0, 21, 40, 0, 0, 0, 0, 0, 0, 'On HP Update - Cast Spell'),
(35189, 0, 0, 0, 0, 0, 100, 0, 3000, 7000, 12000, 15000, 11, 61184, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skoll - In Combat - Cast ''Pounce'''),
(35189, 0, 1, 0, 0, 0, 100, 0, 5000, 9000, 8000, 13000, 11, 61186, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skoll - In Combat - Cast ''Frozen Bite''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '30164';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(30164, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 24604, 0, 0, 1, 'Cavedweller Worg - On Aggro - Cast ''Furious Howl'' (No Repeat)');
