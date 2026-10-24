-- Krasarang Wilds: creature abilities
-- No reference scripts the zone's creatures (the saurok, the mogu, the Sha, the Horde and Alliance soldiers and the wildlife), and
-- most had no AI: they cast their sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '58941,59306,59404,59714,59715,60198,60358,60602,60603,60774,61027,61091,63767,64034,65024,65820,66729,66748,66753,67197';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(58941, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 118823, 32, 0, 1, 'Seadragon - In Combat - Cast ''Zin Spin'''),
(59306, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 129793, 0, 0, 2, 'Ripple of Despair - In Combat - Cast ''Languor'''),
(59404, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 126922, 0, 0, 2, 'Knifetooth Swarmer - In Combat - Cast ''Razor Teeth'''),
(59714, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 79877, 32, 0, 1, 'Riverblade Raider - In Combat - Cast ''Bloodthirst'''),
(59714, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 2, 'Riverblade Raider - In Combat - Cast ''Cleave'''),
(59715, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 18266, 0, 0, 2, 'Riverblade Chieftain - In Combat - Cast ''Curse of Agony'''),
(59715, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 84533, 0, 0, 2, 'Riverblade Chieftain - In Combat - Cast ''Drain Life'''),
(59715, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 2, 'Riverblade Chieftain - In Combat - Cast ''Shadow Bolt'''),
(60198, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 129135, 32, 0, 1, 'Crimson Lory - In Combat - Cast ''Fan of Feathers'''),
(60198, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 119594, 0, 0, 2, 'Crimson Lory - In Combat - Cast ''Peck'''),
(60358, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 119683, 32, 0, 1, 'Unga Villager - In Combat - Cast ''Ook the Village!'''),
(60358, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 119681, 0, 0, 2, 'Unga Villager - In Combat - Cast ''Ooked in the Dook!'''),
(60602, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 129113, 0, 0, 2, 'Student of Chi-Ji - Within 0-5 Range - Cast ''Blackout Kick'''),
(60602, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 119500, 0, 0, 2, 'Student of Chi-Ji - Within 8-25 Range - Cast ''Flying Serpent Kick'''),
(60602, 0, 2, 0, 0, 0, 100, 0, 3000, 6000, 25000, 30000, 11, 129929, 32, 0, 1, 'Student of Chi-Ji - In Combat - Cast ''Gift of Chi-Ji'''),
(60602, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 119007, 64, 0, 2, 'Student of Chi-Ji - In Combat - Cast ''Jab'''),
(60603, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 25000, 30000, 11, 129929, 32, 0, 1, 'Student of Chi-Ji - In Combat - Cast ''Gift of Chi-Ji'''),
(60603, 0, 1, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 129636, 0, 0, 2, 'Student of Chi-Ji - Within 0-5 Range - Cast ''Mighty Chop'''),
(60774, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 79877, 32, 0, 1, 'Anglers Expedition Defender - In Combat - Cast ''Bloodthirst'''),
(60774, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 2, 'Anglers Expedition Defender - In Combat - Cast ''Cleave'''),
(61027, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 81173, 0, 0, 1, 'Cobalt Hammerhead - Between 0-30% Health - Cast ''Frenzy'' (No Repeat)'),
(61027, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 69203, 0, 0, 2, 'Cobalt Hammerhead - In Combat - Cast ''Vicious Bite'''),
(61091, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 129427, 32, 0, 1, 'Elder Viseclaw - In Combat - Cast ''Rage of Elders'''),
(61091, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 117380, 32, 0, 1, 'Elder Viseclaw - In Combat - Cast ''Scuttle!'''),
(61091, 0, 2, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 129438, 0, 0, 2, 'Elder Viseclaw - Within 0-5 Range - Cast ''Vise Claw'''),
(63767, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 75008, 0, 0, 2, 'Chasheen - In Combat - Cast ''Pounce'''),
(63767, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 24331, 0, 0, 2, 'Chasheen - In Combat - Cast ''Rake'''),
(63767, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 93435, 0, 0, 1, 'Chasheen - On Aggro - Cast ''Roar of Courage'' (No Repeat)'),
(63767, 0, 3, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 111844, 32, 0, 1, 'Chasheen - Out of Combat - Cast ''Vaeldrin''s Mind Vision'''),
(64034, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 79877, 32, 0, 1, 'Riverblade Thief - In Combat - Cast ''Bloodthirst'''),
(64034, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 2, 'Riverblade Thief - In Combat - Cast ''Cleave'''),
(65024, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 49758, 0, 0, 2, 'Terracotta Warrior - Within 8-25 Range - Cast ''Charge'''),
(65024, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 85625, 0, 0, 2, 'Terracotta Warrior - In Combat - Cast ''Cleave'''),
(65820, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 129462, 64, 0, 2, 'Yellowback Water Moccasin - In Combat - Cast ''Poison-Tipped Fangs'''),
(66729, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 131103, 0, 0, 2, 'Krasarang Glider - In Combat - Cast ''Bite'''),
(66729, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 5416, 0, 0, 2, 'Krasarang Glider - In Combat - Cast ''Venom Sting'''),
(66748, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 129427, 32, 0, 1, 'Venerable Viseclaw - In Combat - Cast ''Rage of Elders'''),
(66748, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 117380, 32, 0, 1, 'Venerable Viseclaw - In Combat - Cast ''Scuttle!'''),
(66748, 0, 2, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 129438, 0, 0, 2, 'Venerable Viseclaw - Within 0-5 Range - Cast ''Vise Claw'''),
(66753, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 131103, 0, 0, 2, 'Shrouded Carcharodon - In Combat - Cast ''Bite'''),
(67197, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 2, 'Anglers Wharf Defender - In Combat - Cast ''Shoot''');
