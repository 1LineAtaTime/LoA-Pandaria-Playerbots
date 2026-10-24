-- The Lost Isles: creature abilities
-- None of the zone's fighters had AI and neither the Cataclysm nor the classic data scripts them, but their sniffed spell lists are in
-- creature_template spell1-8. Cast those: cast-time spells as the main nuke, shocks and DoTs on timers, melee abilities staggered,
-- self buffs out of combat, heals below 40% health, charges at range. Taunts, movement spells, the town and quest NPCs and the
-- scripted encounters (Thrall, Gallywix, the plane gun, Yngwie and his illusion) are left out, as are the Oomlot zombie
-- transformations, which belong to the quest.

SET @ENTRIES := '34763,35812,35838,35891,35894,35896,35897,35904,36092,36149,36153,36346,36681,36719,36735,36740,38187,38359,38360,38575,38808,38809,38810,38811,38845,38851,39042,39069,39141,39142,39143,39193,39354,39376,39426,39449,39589,40064,40067,40068';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
-- Oomlot Shaman keeps its quest rows (ids 201 and up: Enveloping Winds on the Goblin Captive)
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38644;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38644 AND `id` < 200;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(34763, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 69203, 0, 0, 2, 'Daggertooth Shark - In Combat - Cast ''Vicious Bite'''),
(35812, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 38560, 64, 0, 2, 'Smart Mining Monkey - In Combat - Cast ''Throw'''),
(35838, 0, 0, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 11986, 0, 0, 1, 'Pygmy Witchdoctor - Between 0-40% Health - Cast ''Healing Wave'''),
(35838, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 25000, 30000, 11, 18503, 0, 0, 2, 'Pygmy Witchdoctor - In Combat - Cast ''Hex'''),
(35838, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 2, 'Pygmy Witchdoctor - In Combat - Cast ''Shadow Bolt'''),
(35891, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15691, 0, 0, 2, 'SI:7 Infiltrator - In Combat - Cast ''Eviscerate'''),
(35894, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 15620, 64, 0, 2, 'Orc Scout - In Combat - Cast ''Shoot'''),
(35896, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 68207, 64, 0, 2, 'Poison Spitter - In Combat - Cast ''Poison Spit'''),
(35896, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 68208, 0, 0, 2, 'Poison Spitter - In Combat - Cast ''Shoot Thorns'''),
(35897, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 68209, 64, 0, 2, 'Freezya - In Combat - Cast ''Frostbolt'''),
(35904, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 31289, 0, 0, 2, 'Jungle Cat - In Combat - Cast ''Claw'''),
(36092, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 14873, 0, 0, 2, 'SI:7 Assassin - In Combat - Cast ''Sinister Strike'''),
(36149, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11976, 0, 0, 2, 'Alliance Sailor - In Combat - Cast ''Strike'''),
(36153, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 13730, 0, 0, 1, 'Alliance Captain - Within 0-8 Range - Cast ''Demoralizing Shout'''),
(36346, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15691, 0, 0, 2, 'SI:7 Operative - In Combat - Cast ''Eviscerate'''),
(36681, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 69203, 0, 0, 2, 'Ravenous Lurker - In Combat - Cast ''Vicious Bite'''),
(36719, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 31273, 0, 0, 1, 'Pterrordax Scavenger - Within 0-8 Range - Cast ''Screech'''),
(36735, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 35321, 0, 0, 2, 'Teraptor Matriarch - In Combat - Cast ''Gushing Wound'''),
(36735, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 36629, 0, 0, 1, 'Teraptor Matriarch - Within 0-8 Range - Cast ''Terrifying Roar'''),
(36740, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 6268, 0, 0, 2, 'Teraptor Hatchling - Within 8-25 Range - Cast ''Rushing Charge'''),
(38187, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16827, 0, 0, 2, 'Spiny Raptor - In Combat - Cast ''Claw'''),
(38187, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 42320, 0, 0, 2, 'Spiny Raptor - In Combat - Cast ''Head Butt'''),
(38359, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 1, 'Vashj''elan Warrior - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(38359, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 57846, 0, 0, 2, 'Vashj''elan Warrior - In Combat - Cast ''Heroic Strike'''),
(38360, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 11831, 0, 0, 1, 'Vashj''elan Siren - Within 0-8 Range - Cast ''Frost Nova'''),
(38360, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9672, 64, 0, 2, 'Vashj''elan Siren - In Combat - Cast ''Frostbolt'''),
(38575, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 74613, 64, 0, 2, 'Oomlot Tribesman - In Combat - Cast ''Shoot'''),
(38644, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 9000, 12000, 11, 13281, 0, 0, 2, 'Oomlot Shaman - In Combat - Cast ''Earth Shock'''),
(38644, 0, 1, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 12550, 32, 0, 1, 'Oomlot Shaman - Out of Combat - Cast ''Lightning Shield'''),
(38808, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 9000, 12000, 11, 12548, 0, 0, 2, 'Gaahl - In Combat - Cast ''Frost Shock'''),
(38809, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 75945, 0, 0, 2, 'Malmo - In Combat - Cast ''Fury of the Storm'''),
(38809, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 57780, 64, 0, 2, 'Malmo - In Combat - Cast ''Lightning Bolt'''),
(38810, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 14000, 16000, 11, 75946, 0, 0, 2, 'Teloch - In Combat - Cast ''Fire and the Flames'''),
(38810, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 39591, 0, 0, 1, 'Teloch - On Aggro - Cast ''Searing Totem'' (No Repeat)'),
(38811, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 84837, 64, 0, 2, 'Oostan Headhunter - In Combat - Cast ''Shoot'''),
(38845, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 8873, 0, 0, 2, 'Child of Volcanoth - Within 0-5 Range - Cast ''Flame Breath'''),
(38851, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 72991, 64, 0, 2, 'Volcanoth Priest - In Combat - Cast ''Fireball'''),
(39042, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Alliance Paratrooper - In Combat - Cast ''Shoot'''),
(39069, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Alliance Paratrooper - In Combat - Cast ''Shoot'''),
(39141, 0, 0, 0, 2, 0, 100, 0, 0, 40, 30000, 35000, 11, 79853, 0, 0, 1, 'Commander Arrington - Between 0-40% Health - Cast ''Evasion'''),
(39141, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79852, 0, 0, 2, 'Commander Arrington - In Combat - Cast ''Eviscerate'''),
(39141, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 60842, 0, 0, 2, 'Commander Arrington - In Combat - Cast ''Expose Armor'''),
(39141, 0, 3, 0, 9, 0, 100, 0, 0, 10, 12000, 15000, 11, 79851, 0, 0, 1, 'Commander Arrington - Within 0-10 Range - Cast ''Fan of Knives'''),
(39142, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 60850, 0, 0, 2, 'Darkblade Cyn - In Combat - Cast ''Mutilate'''),
(39142, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 60195, 0, 0, 2, 'Darkblade Cyn - In Combat - Cast ''Sinister Strike'''),
(39143, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79862, 0, 0, 2, 'Alexi Silenthowl - In Combat - Cast ''Backstab'''),
(39143, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 79866, 0, 0, 2, 'Alexi Silenthowl - In Combat - Cast ''Deadly Throw'''),
(39143, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 79863, 0, 0, 2, 'Alexi Silenthowl - In Combat - Cast ''Hemorrhage'''),
(39193, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 76136, 32, 0, 1, 'Brute Overseer - Out of Combat - Cast ''Acidic Sweat'''),
(39193, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 3551, 0, 0, 2, 'Brute Overseer - In Combat - Cast ''Skull Crack'''),
(39354, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 32735, 64, 0, 2, 'Steamwheedle Shark - In Combat - Cast ''Saw Blade'''),
(39354, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 75962, 0, 0, 2, 'Steamwheedle Shark - In Combat - Cast ''Shred Armor'''),
(39376, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 73867, 64, 0, 2, 'KTC Oil Bot - In Combat - Cast ''Cutting Laser'''),
(39426, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 75924, 64, 0, 2, 'Candy Cane - In Combat - Cast ''Heartbreaker'''),
(39449, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 75361, 0, 0, 2, 'Southsea Mercenary - In Combat - Cast ''Swashbuckling Slice'''),
(39589, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 76136, 32, 0, 1, 'Brute Bodyguard - Out of Combat - Cast ''Acidic Sweat'''),
(39589, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 10000, 14000, 11, 10966, 0, 0, 2, 'Brute Bodyguard - In Combat - Cast ''Uppercut'''),
(40064, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16827, 0, 0, 2, 'Jungle Panther - In Combat - Cast ''Claw'''),
(40064, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 75002, 0, 0, 2, 'Jungle Panther - Within 8-25 Range - Cast ''Leaping Rush'''),
(40067, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 74724, 64, 0, 2, 'Iceeya - In Combat - Cast ''Frostbolt'''),
(40068, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 74726, 64, 0, 2, 'Venom Spitter - In Combat - Cast ''Shoot Thorns'''),
(40068, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 74725, 0, 0, 2, 'Venom Spitter - In Combat - Cast ''Venom Spit''');
