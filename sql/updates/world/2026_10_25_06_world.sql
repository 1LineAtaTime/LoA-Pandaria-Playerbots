-- Drak'Tharon Keep: creature abilities
-- Most of the dungeon's creatures (the Scourge and the Drakkari trolls) had no AI or ran classic rows: port their abilities from
-- the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are
-- left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags. The creatures no
-- reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '26620,26621,26623,26635,26636,27431,27909';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26620, 0, 0, 0, 25, 0, 100, 6, 0, 0, 0, 0, 2, 1965, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - On Reset - Set Faction 1965'),
(26620, 0, 1, 0, 1, 0, 100, 6, 500, 500, 1000, 1000, 49, 0, 0, 0, 0, 0, 0, 25, 20, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - Out Of Combat - Start Attacking'),
(26620, 0, 2, 0, 2, 0, 100, 6, 0, 40, 5000, 10000, 11, 47668, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - Between 0-40% Health - Cast "Heal"'),
(26620, 0, 3, 0, 1, 0, 100, 6, 1000, 1000, 1000, 1000, 28, 49852, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - Out Of Combat - Remove Aura "Bloody Explosion"'),
(26620, 0, 4, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26623, 30, 0, 0, 0, 0, 0, 'Drakkari Guardian - In Combat - Reduce Threat'),
(26620, 0, 5, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26635, 30, 0, 0, 0, 0, 0, 'Drakkari Guardian - In Combat - Reduce Threat'),
(26620, 0, 6, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26636, 30, 0, 0, 0, 0, 0, 'Drakkari Guardian - In Combat - Reduce Threat'),
(26620, 0, 7, 0, 0, 0, 100, 0, 1000, 6000, 7000, 12000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - In Combat - Cast Hamstring'),
(26620, 0, 8, 0, 13, 0, 100, 0, 8000, 9000, 0, 0, 11, 11972, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Guardian - Victim Casting - Cast Shield Bash'),
(26621, 0, 0, 0, 0, 0, 100, 6, 3000, 6000, 6000, 9000, 11, 51917, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghoul Tormentor - In Combat - Cast Cleave'),
(26621, 0, 1, 0, 0, 0, 100, 2, 6000, 9000, 9000, 12000, 11, 51917, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghoul Tormentor - In Combat - Cast Cleave'),
(26621, 0, 2, 0, 0, 0, 100, 4, 6000, 9000, 9000, 12000, 11, 59007, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghoul Tormentor - In Combat - Cast Flesh Rot'),
(26621, 0, 3, 0, 1, 0, 100, 6, 2000, 2000, 2000, 2000, 86, 49852, 34, 19, 26620, 5, 0, 19, 26620, 5, 0, 0, 0, 0, 0, 'Ghoul Tormentor - Out Of Combat - Cast "Bloody Explosion"'),
(26621, 0, 4, 0, 0, 0, 100, 2, 6000, 9000, 9000, 12000, 11, 49678, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghoul Tormentor - In Combat - Cast Flesh Rot'),
(26623, 0, 0, 0, 25, 0, 100, 6, 0, 0, 0, 0, 2, 1814, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scourge Brute - On Reset - Set Faction 1814'),
(26623, 0, 1, 0, 1, 0, 100, 6, 500, 500, 1000, 1000, 49, 0, 0, 0, 0, 0, 0, 25, 20, 0, 0, 0, 0, 0, 0, 'Scourge Brute - Out Of Combat - Start Attacking'),
(26623, 0, 2, 0, 2, 0, 100, 6, 0, 40, 5000, 10000, 11, 47668, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scourge Brute - Between 0-40% Health - Cast "Heal"'),
(26623, 0, 3, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26620, 30, 0, 0, 0, 0, 0, 'Scourge Brute - In Combat - Reduce Threat'),
(26623, 0, 4, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26639, 30, 0, 0, 0, 0, 0, 'Scourge Brute - In Combat - Reduce Threat'),
(26623, 0, 5, 0, 0, 0, 100, 0, 1000, 6000, 7000, 12000, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scourge Brute - In Combat - Cast Mortal Strike'),
(26623, 0, 6, 0, 0, 0, 100, 0, 10000, 12000, 13000, 15000, 11, 35011, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scourge Brute - In Combat - Cast Knockdown'),
(26635, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 2, 1814, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - On Reset - Set Faction 1814'),
(26635, 0, 1, 0, 1, 0, 100, 6, 500, 500, 1000, 1000, 49, 0, 0, 0, 0, 0, 0, 25, 20, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - Out Of Combat - Start Attacking'),
(26635, 0, 2, 0, 2, 0, 100, 0, 0, 40, 5000, 10000, 11, 47668, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - Between 0-40% Health - Cast "Heal"'),
(26635, 0, 3, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26620, 30, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - In Combat - Reduce Threat'),
(26635, 0, 4, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26639, 30, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - In Combat - Reduce Threat'),
(26635, 0, 5, 0, 0, 0, 100, 0, 1000, 3000, 5000, 9500, 11, 36093, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - In Combat - Cast Ghost Strike'),
(26635, 0, 6, 0, 0, 0, 100, 0, 1000, 3000, 7250, 10000, 11, 33661, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - In Combat - Cast Crush Armor'),
(26635, 0, 7, 0, 54, 0, 100, 0, 0, 0, 0, 0, 11, 48624, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Warrior - In Summoned By - Cast Birth Dead Visual'),
(26636, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 2, 1814, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - On Reset - Set Faction 1814'),
(26636, 0, 1, 0, 1, 0, 100, 6, 500, 500, 1000, 1000, 49, 0, 0, 0, 0, 0, 0, 25, 20, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - Out Of Combat - Start Attacking'),
(26636, 0, 2, 0, 2, 0, 100, 0, 0, 40, 5000, 10000, 11, 47668, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - Between 0-40% Health - Cast "Heal"'),
(26636, 0, 3, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26620, 30, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Reduce Threat'),
(26636, 0, 4, 0, 0, 0, 100, 6, 1000, 1000, 1000, 1000, 13, 0, 100, 0, 0, 0, 0, 11, 26639, 30, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Reduce Threat'),
(26636, 0, 5, 0, 0, 0, 100, 2, 1000, 6000, 7000, 12000, 11, 49696, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Cast Shadow Blast'),
(26636, 0, 6, 0, 0, 0, 100, 4, 1000, 5000, 7000, 12000, 11, 59013, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Cast Shadow Blast'),
(26636, 0, 7, 0, 0, 0, 100, 0, 3000, 6000, 8900, 13500, 11, 35011, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Cast Knockdown'),
(26636, 0, 8, 0, 0, 0, 50, 2, 2900, 6600, 10000, 12000, 11, 55847, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Cast Shadow Void'),
(26636, 0, 9, 0, 0, 0, 50, 4, 2900, 6600, 10000, 12000, 11, 59014, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - In Combat - Cast Shadow Void'),
(26636, 0, 10, 0, 2, 0, 100, 2, 0, 75, 4000, 9000, 11, 49701, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - Between 0-75% Health - Cast Blood Siphon'),
(26636, 0, 11, 0, 2, 0, 100, 4, 0, 75, 4000, 9000, 11, 59015, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Risen Drakkari Soulmage - Between 0-75% Health - Cast Blood Siphon'),
(27431, 0, 0, 0, 0, 0, 100, 0, 0, 0, 30000, 30000, 11, 49724, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Battle Shout'),
(27431, 0, 1, 0, 0, 0, 100, 2, 4000, 7000, 8000, 10000, 11, 49807, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Whirlwind'),
(27431, 0, 2, 0, 0, 0, 100, 4, 4000, 7000, 8000, 10000, 11, 24236, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Whirlwind'),
(27431, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - Between Health 0-30% - Cast Frenzy'),
(27909, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 42, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darkweb Victim - On Spawn - Set Invincibility Hp'),
(27909, 0, 1, 0, 2, 0, 100, 1, 0, 1, 0, 0, 87, 2790900, 2790901, 2790902, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darkweb Victim - On 1% HP - Set Random script'),
(27909, 0, 2, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 49960, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darkweb Victim - On Death - Cast Summon Random Drakkari');
