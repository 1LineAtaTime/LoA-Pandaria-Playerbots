-- The Blood Furnace: creature abilities
-- Most of the dungeon's creatures (the Shadowmoon and Laughing Skull orcs, the felguards and hellhounds) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '17371,17397,17398,17400,17477,17626';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17371, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Out of Combat - Disable Combat Movement'),
(17371, 0, 7, 8, 3, 0, 100, 0, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Between 0-15% Mana - Enable Combat Movement'),
(17371, 0, 8, 0, 61, 0, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Between 0-15% Mana - Increment Phase By 1'),
(17371, 0, 9, 0, 9, 0, 100, 7, 35, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Within 35-80 Range - Enable Combat Movement'),
(17371, 0, 10, 0, 9, 0, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Within 5-15 Range - Disable Combat Movement'),
(17371, 0, 11, 0, 9, 0, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Within 0-5 Range - Enable Combat Movement'),
(17371, 0, 12, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Between 30-100% Mana - Decrement Phase By 1'),
(17371, 0, 13, 0, 0, 0, 100, 2, 400, 4750, 3600, 6300, 11, 12739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Shadow Bolt'' (Normal Dungeon)'),
(17371, 0, 14, 0, 0, 0, 100, 4, 400, 4750, 3600, 6300, 11, 15472, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Shadow Bolt'' (Heroic Dungeon)'),
(17371, 0, 15, 0, 0, 0, 100, 0, 600, 8500, 12150, 21750, 11, 32197, 32, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Corruption'''),
(17371, 0, 16, 0, 0, 0, 100, 0, 6600, 10700, 14900, 14900, 11, 13338, 32, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Curse of Tongues'''),
(17371, 0, 17, 0, 0, 0, 100, 4, 2300, 16600, 21850, 43750, 11, 32863, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - In Combat - Cast ''Seed of Corruption'' (Heroic Dungeon)'),
(17371, 0, 18, 0, 1, 0, 100, 0, 12600, 13700, 21900, 24900, 11, 33111, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Warlock - Out of Combat - Cast ''Fel Power'''),
(17397, 0, 0, 0, 4, 0, 10, 7, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Adept - On Aggro - Say Line 0'),
(17397, 0, 1, 0, 2, 0, 100, 7, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Adept - Between 0-15% Health - Flee For Assist'),
(17397, 0, 2, 0, 13, 0, 100, 0, 9600, 11400, 0, 0, 11, 11978, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Adept - In Combat - Cast Kick'),
(17397, 0, 3, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 31059, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowmoon Adept - Out Of Combat - Cast Hellfire Channeling'),
(17398, 0, 0, 0, 0, 0, 100, 0, 6000, 18500, 1200, 27950, 11, 22427, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nascent Fel Orc - In Combat - Cast ''Concussion Blow'''),
(17398, 0, 1, 0, 0, 0, 100, 0, 1200, 12150, 12150, 27950, 11, 31900, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nascent Fel Orc - In Combat - Cast ''Stomp'''),
(17400, 0, 0, 0, 9, 0, 100, 4, 8, 25, 8700, 14700, 11, 27577, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Felguard Annihilator - Within 8-25 Range - Cast ''Intercept'' (Heroic Dungeon)'),
(17400, 0, 1, 0, 0, 0, 100, 0, 3000, 10950, 8500, 20650, 11, 18072, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Felguard Annihilator - In Combat - Cast ''Uppercut'''),
(17400, 0, 2, 0, 13, 0, 100, 0, 6900, 9500, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Felguard Annihilator - On Victim Casting - Cast ''Pummel'''),
(17477, 0, 0, 1, 1, 0, 100, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Out of Combat - Disable Combat Movement'),
(17477, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Out of Combat - Set Event Phase 0'),
(17477, 0, 8, 9, 3, 0, 100, 0, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Between 0-15% Mana - Enable Combat Movement'),
(17477, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Between 0-15% Mana - Increment Phase By 1'),
(17477, 0, 10, 0, 9, 0, 100, 7, 35, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Within 35-80 Range - Enable Combat Movement'),
(17477, 0, 11, 0, 9, 0, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Within 5-15 Range - Disable Combat Movement'),
(17477, 0, 12, 0, 9, 0, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Within 0-5 Range - Enable Combat Movement'),
(17477, 0, 13, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Between 30-100% Mana - Decrement Phase By 1'),
(17477, 0, 14, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 30540, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - Out of Combat - Cast ''Summon Visual'' (No Repeat)'),
(17477, 0, 15, 0, 0, 0, 100, 2, 200, 2400, 3600, 4800, 11, 15242, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - In Combat - Cast ''Fireball'' (Normal Dungeon)'),
(17477, 0, 16, 0, 0, 0, 100, 4, 200, 2400, 3600, 4800, 11, 17290, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - In Combat - Cast ''Fireball'' (Heroic Dungeon)'),
(17477, 0, 17, 0, 0, 0, 100, 4, 8500, 14800, 6050, 17750, 11, 16144, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Imp - In Combat - Cast ''Fire Blast'' (Heroic Dungeon)'),
(17626, 0, 0, 0, 0, 0, 100, 0, 9300, 18200, 10950, 23100, 11, 10966, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Laughing Skull Legionnaire - In Combat - Cast Uppercut'),
(17626, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Laughing Skull Legionnaire - At 30% HP - Cast Enrage'),
(17626, 0, 2, 0, 0, 0, 100, 0, 5000, 12000, 25000, 35000, 11, 18765, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Laughing Skull Legionnaire - In Combat - Cast Sweeping Strikes');
