-- Zul'Drak: creature abilities
-- Most of the zone's creatures (the Drakkari trolls, the Scourge, the Argent Crusade's foes, the wildlife) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted. The creatures no reference scripts
-- cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '28145,28221,28369,28416,28603,28779,29436,29698';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28145, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 17000, 27000, 11, 54470, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'In Combat - Cast Venemous Bite'),
(28221, 0, 0, 0, 1, 0, 100, 0, 20000, 30000, 20000, 30000, 11, 50981, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapdoor Crawler - Out of Combat - Cast ''Burrow'''),
(28221, 0, 1, 0, 0, 0, 100, 0, 2000, 5000, 4000, 8000, 11, 11918, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Trapdoor Crawler - In Combat - Cast ''Poison'''),
(28369, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 11, 40148, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Burning Skimmer - On Respawn - Cast ''Immolation'''),
(28416, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 4000, 8000, 11, 34298, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rhunok - In Combat - Cast ''Maul'''),
(28603, 0, 0, 0, 0, 0, 100, 0, 6000, 6000, 10000, 10000, 11, 33914, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blightguard - In Combat - Cast ''Shadowstrike'''),
(28779, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 54582, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blood of Mam''toth - On Respawn - Cast ''Plasma'''),
(28779, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 11000, 11, 54580, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blood of Mam''toth - IC - Cast ''Coagulate'''),
(29436, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 12000, 24000, 11, 55216, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Icetouched Earthrager - In Combat - Cast ''Avalanche'''),
(29698, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 54487, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Drakuru Raptor - On aggro - Cast Jump Attack');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '26582,28022,28023,28034,28255,28257,28258,28402,28442,28477,28564,28759,28784,28803,28879,28916,28918,29211,29437,29438,29439,29444,29449,29450,29452,29453,29699';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(26582, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 7, 32, 0, 1, 'Horrified Drakkari Warrior - In Combat - Cast ''Suicide'''),
(28022, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 54387, 0, 0, 2, 'Carrion Eater - In Combat - Cast ''Ravenous Claw'''),
(28023, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 2, 'Rotting Abomination - In Combat - Cast ''Cleave'''),
(28023, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 50335, 0, 0, 2, 'Rotting Abomination - In Combat - Cast ''Scourge Hook'''),
(28034, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 54482, 32, 0, 1, 'Drakkari Snake Handler - Out of Combat - Cast ''Blood of Sseratus'''),
(28034, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Drakkari Snake Handler - In Combat - Cast ''Shoot'''),
(28034, 0, 2, 0, 0, 0, 100, 0, 2000, 5000, 20000, 25000, 11, 54484, 0, 0, 1, 'Drakkari Snake Handler - In Combat - Cast ''Summon Drakkari Snake'''),
(28255, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 57854, 0, 0, 1, 'Malas the Corrupter - Within 0-8 Range - Cast ''Raging Shadows'''),
(28255, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 51676, 64, 0, 2, 'Malas the Corrupter - In Combat - Cast ''Wavering Will'''),
(28257, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 54491, 64, 0, 2, 'Hath''ar Necromagus - In Combat - Cast ''Conversion Beam'''),
(28258, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 54487, 0, 0, 2, 'Hath''ar Skimmer - Within 8-25 Range - Cast ''Jump Attack'''),
(28402, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 54615, 64, 0, 2, 'Claw of Har''koa - In Combat - Cast ''Aimed Shot'''),
(28442, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 49758, 0, 0, 2, 'Prophet of Rhunok - Within 8-25 Range - Cast ''Charge'''),
(28442, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16095, 0, 0, 2, 'Prophet of Rhunok - In Combat - Cast ''Vicious Rend'''),
(28477, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 38193, 64, 0, 2, 'Scion of Quetz''lun - In Combat - Cast ''Lightning Breath'''),
(28477, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 54526, 0, 0, 2, 'Scion of Quetz''lun - In Combat - Cast ''Torment'''),
(28564, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 54340, 0, 0, 2, 'Putrid Abomination - In Combat - Cast ''Vile Vomit'''),
(28759, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 16564, 0, 0, 2, 'Flying Fiend - In Combat - Cast ''Gargoyle Strike'''),
(28784, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 54588, 0, 0, 2, 'Altar Warden - In Combat - Cast ''Gust of Wind'''),
(28784, 0, 1, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 54589, 32, 0, 1, 'Altar Warden - Out of Combat - Cast ''Whirling Winds'''),
(28784, 0, 2, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 50215, 0, 0, 2, 'Altar Warden - In Combat - Cast ''Zephyr'''),
(28803, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 50420, 0, 0, 1, 'Drakuru''s Guard - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(28803, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 54345, 0, 0, 2, 'Drakuru''s Guard - In Combat - Cast ''Withering Strike'''),
(28879, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 49980, 0, 0, 2, 'Shalewing - In Combat - Cast ''Ravenous Pathogen'''),
(28916, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 52719, 64, 0, 2, 'Tiri - In Combat - Cast ''Concussion Blow'''),
(28916, 0, 1, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 25058, 0, 0, 1, 'Tiri - Between 0-40% Health - Cast ''Renew'''),
(28918, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 1, 'Drek''Maz - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(28918, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16856, 0, 0, 2, 'Drek''Maz - In Combat - Cast ''Mortal Strike'''),
(28918, 0, 2, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 15578, 0, 0, 1, 'Drek''Maz - Within 0-8 Range - Cast ''Whirlwind'''),
(29211, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 42332, 0, 0, 2, 'Drakkari Native - In Combat - Cast ''Throw'''),
(29211, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 35000, 40000, 11, 51307, 32, 0, 1, 'Drakkari Native - In Combat - Cast ''Unwavering Will'''),
(29437, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 50420, 0, 0, 1, 'Rageclaw Berserker - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(29438, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 50420, 0, 0, 1, 'Rageclaw Primalist - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(29438, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 12548, 64, 0, 2, 'Rageclaw Primalist - In Combat - Cast ''Frost Shock'''),
(29438, 0, 2, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 11986, 0, 0, 1, 'Rageclaw Primalist - Between 0-40% Health - Cast ''Healing Wave'''),
(29438, 0, 3, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 12550, 32, 0, 1, 'Rageclaw Primalist - Out of Combat - Cast ''Lightning Shield'''),
(29439, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 50420, 0, 0, 1, 'Rageclaw Hunter - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(29439, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 18651, 64, 0, 2, 'Rageclaw Hunter - In Combat - Cast ''Multi-Shot'''),
(29439, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 15547, 0, 0, 2, 'Rageclaw Hunter - In Combat - Cast ''Shoot'''),
(29444, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25809, 64, 0, 2, 'Drakkari Snake - In Combat - Cast ''Crippling Poison'''),
(29444, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 25810, 0, 0, 2, 'Drakkari Snake - In Combat - Cast ''Mind-numbing Poison'''),
(29449, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 56038, 64, 0, 2, 'Vargul Deathwaker - In Combat - Cast ''Plaguebolt'''),
(29450, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 54512, 32, 0, 1, 'Vargul Runelord - Out of Combat - Cast ''Plague Shield'''),
(29450, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 56036, 64, 0, 2, 'Vargul Runelord - In Combat - Cast ''Rune of Destruction'''),
(29452, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 24604, 0, 0, 1, 'Vargul Blighthound - On Aggro - Cast ''Furious Howl'' (No Repeat)'),
(29452, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 50046, 0, 0, 2, 'Vargul Blighthound - In Combat - Cast ''Gnaw Bone'''),
(29453, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 55079, 0, 0, 2, 'Vargul Plaguetalon - In Combat - Cast ''Swoop'''),
(29699, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 15547, 64, 0, 2, 'Drakuru Raptor Rider - In Combat - Cast ''Shoot''');
