-- #988 wave 2 (merge): Blasted Lands creature abilities from DeltaTho PR 15 file 2026_10_24_55_world.sql, ADDED to creatures that already have smart_scripts.
-- 25 creatures, 64 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 5975:2 5977:1 5978:2 5983:1 5990:1 5996:2 5997:3 5998:2 6004:2 6005:3 6006:2 6007:1 6008:4 6009:3 6010:1 8296:2 8297:4 8298:3 8299:2 8300:2 8301:2 8302:2 8303:3 42235:2 42334:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 5975 AND `id` >= 2)
  OR (`entryorguid` = 5977 AND `id` >= 1)
  OR (`entryorguid` = 5978 AND `id` >= 2)
  OR (`entryorguid` = 5983 AND `id` >= 1)
  OR (`entryorguid` = 5990 AND `id` >= 1)
  OR (`entryorguid` = 5996 AND `id` >= 2)
  OR (`entryorguid` = 5997 AND `id` >= 3)
  OR (`entryorguid` = 5998 AND `id` >= 2)
  OR (`entryorguid` = 6004 AND `id` >= 2)
  OR (`entryorguid` = 6005 AND `id` >= 3)
  OR (`entryorguid` = 6006 AND `id` >= 2)
  OR (`entryorguid` = 6007 AND `id` >= 1)
  OR (`entryorguid` = 6008 AND `id` >= 4)
  OR (`entryorguid` = 6009 AND `id` >= 3)
  OR (`entryorguid` = 6010 AND `id` >= 1)
  OR (`entryorguid` = 8296 AND `id` >= 2)
  OR (`entryorguid` = 8297 AND `id` >= 4)
  OR (`entryorguid` = 8298 AND `id` >= 3)
  OR (`entryorguid` = 8299 AND `id` >= 2)
  OR (`entryorguid` = 8300 AND `id` >= 2)
  OR (`entryorguid` = 8301 AND `id` >= 2)
  OR (`entryorguid` = 8302 AND `id` >= 2)
  OR (`entryorguid` = 8303 AND `id` >= 3)
  OR (`entryorguid` = 42235 AND `id` >= 2)
  OR (`entryorguid` = 42334 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5975, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79858, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Ogre Mage - In Combat - Cast ''Frostbolt'''),
(5975, 0, 3, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 0, 11, 79850, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Ogre Mage - Within 0-8 Range - Cast ''Frost Nova'''),
(5975, 0, 4, 0, 0, 0, 100, 0, 8000, 9000, 15000, 16000, 0, 11, 79859, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Ogre Mage - In Combat - Cast ''Ice Lance'''),
(5975, 0, 5, 0, 0, 0, 100, 0, 9000, 14000, 25000, 28000, 0, 11, 84333, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Ogre Mage - In Combat - Cast ''Summon Dreadmaul'''),
(5977, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 20000, 22000, 0, 11, 81140, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Mauler - In Combat - Cast ''Thunderclap'''),
(5977, 0, 2, 0, 0, 0, 100, 0, 2000, 5000, 35000, 41000, 0, 11, 11960, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Mauler - In Combat - Cast ''Curse of the Dreadmaul'''),
(5977, 0, 3, 0, 0, 0, 100, 0, 9000, 14000, 25000, 28000, 0, 11, 84333, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Mauler - In Combat - Cast ''Summon Dreadmaul'''),
(5977, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Mauler - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(5977, 0, 5, 0, 9, 0, 100, 0, 0, 5, 17000, 18000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Mauler - Within 0-5 Range - Cast ''Uppercut'''),
(5978, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79938, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Warlock - In Combat - Cast ''Incinerate'''),
(5978, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 44977, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Warlock - Out of Combat - Cast ''Fel Armor'''),
(5978, 0, 4, 0, 0, 0, 100, 0, 8000, 9000, 17000, 17500, 0, 11, 79937, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Warlock - In Combat - Cast ''Immolate'''),
(5978, 0, 5, 0, 0, 0, 100, 0, 9000, 14000, 25000, 28000, 0, 11, 84333, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaul Warlock - In Combat - Cast ''Summon Dreadmaul'''),
(5983, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 12000, 14000, 0, 11, 37945, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bonepicker Felfeeder - In Combat - Cast ''Fel Fireball'''),
(5990, 0, 1, 0, 0, 0, 100, 0, 3000, 4500, 14000, 16000, 0, 11, 83657, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Redstone Basilisk - In Combat - Cast ''Hellgaze'''),
(5990, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 22000, 24000, 0, 11, 77576, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Redstone Basilisk - In Combat - Cast ''Stone Breath'''),
(5996, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 38557, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Miner - In Combat - Cast ''Throw'''),
(5996, 0, 3, 0, 0, 0, 100, 0, 6000, 9000, 21000, 26000, 0, 11, 80382, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Miner - In Combat - Cast ''Dirt Toss'''),
(5996, 0, 4, 0, 9, 0, 100, 0, 0, 5, 12000, 13000, 0, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Miner - Within 0-5 Range - Cast ''Pierce Armor'''),
(5997, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 12000, 15000, 0, 11, 79721, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Engineer - In Combat - Cast ''Explosive Shot'''),
(5997, 0, 4, 0, 0, 0, 100, 0, 8000, 12000, 22000, 26000, 0, 11, 79726, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Engineer - In Combat - Cast ''Ticking Time Bomb'''),
(5998, 0, 2, 0, 9, 0, 100, 0, 0, 8, 14000, 15000, 0, 11, 81140, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Nethergarde Foreman - Within 0-8 Range - Cast ''Thunderclap'''),
(6004, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 84325, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Ritualist - Between 0-30% Health - Cast ''Fel-Infused'' (No Repeat)'),
(6005, 0, 3, 0, 9, 0, 100, 0, 0, 5, 18000, 22000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Thug - Within 0-5 Range - Cast ''Uppercut'''),
(6005, 0, 4, 0, 0, 0, 100, 0, 2000, 4500, 15000, 15500, 0, 11, 3148, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Thug - In Combat - Cast ''Head Crack'''),
(6006, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77721, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Adept - In Combat - Cast ''Shadow Weave'''),
(6006, 0, 3, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 0, 11, 75073, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Adept - Within 0-8 Range - Cast ''Shadow Nova'''),
(6006, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 77471, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Adept - Between 0-30% Health - Cast ''Shadow Shield'' (No Repeat)'),
(6007, 0, 1, 0, 9, 0, 100, 0, 0, 8, 14000, 16000, 0, 11, 35373, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Enforcer - Within 0-8 Range - Cast ''Shadowfury'''),
(6007, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 13000, 0, 11, 32916, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Enforcer - In Combat - Cast ''Shadow Cleave'''),
(6008, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79939, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Warlock - In Combat - Cast ''Chaos Bolt'''),
(6008, 0, 5, 0, 0, 0, 100, 0, 8000, 9000, 22000, 27000, 0, 11, 79955, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Warlock - In Combat - Cast ''Rain of Fire'''),
(6009, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77722, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Dreadweaver - In Combat - Cast ''Void Whip'''),
(6009, 0, 4, 0, 0, 0, 100, 0, 6000, 9000, 18000, 22000, 0, 11, 79930, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowsworn Dreadweaver - In Combat - Cast ''Corruption'''),
(6010, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 15000, 17000, 0, 11, 81082, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felhound - In Combat - Cast ''Noxious Breath'''),
(6010, 0, 2, 0, 2, 0, 100, 0, 0, 55, 14000, 21000, 0, 11, 79616, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felhound - Between 0-55% Health - Cast ''Drain Life'''),
(8296, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79954, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mojo the Twisted - Out of Combat - Cast ''Fel Armor'''),
(8296, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 31976, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mojo the Twisted - Between 0-30% Health - Cast ''Shadow Shield'' (No Repeat)'),
(8297, 0, 4, 0, 9, 0, 100, 0, 0, 5, 17000, 18000, 0, 11, 10966, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Magronos the Unyielding - Within 0-5 Range - Cast ''Uppercut'''),
(8297, 0, 5, 0, 0, 0, 100, 0, 6000, 14000, 19000, 22500, 0, 11, 22859, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Magronos the Unyielding - In Combat - Cast ''Mortal Cleave'''),
(8297, 0, 6, 0, 0, 0, 100, 0, 14000, 18000, 35000, 36000, 0, 11, 78827, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Magronos the Unyielding - In Combat - Cast ''Bladestorm'''),
(8298, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79892, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Akubar the Seer - Out of Combat - Cast ''Water Shield'''),
(8298, 0, 4, 0, 9, 0, 100, 0, 0, 8, 14000, 14500, 0, 11, 79890, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Akubar the Seer - Within 0-8 Range - Cast ''Flame Shock'''),
(8298, 0, 5, 0, 0, 0, 100, 0, 5000, 7000, 19000, 22000, 0, 11, 39591, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Akubar the Seer - In Combat - Cast ''Searing Totem'''),
(8298, 0, 6, 0, 0, 0, 100, 0, 12000, 15000, 25000, 31000, 0, 11, 79886, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Akubar the Seer - In Combat - Cast ''Lava Burst'''),
(8299, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Spiteflayer - On Aggro - Cast ''Sprint'' (No Repeat)'),
(8299, 0, 3, 0, 0, 0, 100, 0, 2000, 4500, 12000, 14000, 0, 11, 37945, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Spiteflayer - In Combat - Cast ''Fel Fireball'''),
(8300, 0, 2, 0, 9, 0, 100, 1, 0, 20, 0, 0, 0, 11, 82797, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ravage - Within 0-20 Range - Cast ''Leaping Bite'' (No Repeat)'),
(8300, 0, 3, 0, 0, 0, 50, 0, 7000, 8000, 10000, 25000, 0, 11, 13445, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ravage - In Combat - Cast ''Rend'''),
(8300, 0, 4, 0, 0, 0, 80, 0, 5000, 9000, 18000, 21000, 0, 11, 3149, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ravage - In Combat - Cast ''Furious Howl'''),
(8301, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 11000, 12000, 0, 11, 21067, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Clack the Reaver - In Combat - Cast ''Poison Bolt'''),
(8301, 0, 3, 0, 0, 0, 100, 0, 6000, 9000, 18000, 22000, 0, 11, 79607, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Clack the Reaver - In Combat - Cast ''Venom Splash'''),
(8301, 0, 4, 0, 2, 0, 100, 0, 0, 40, 15000, 16000, 0, 11, 79840, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Clack the Reaver - Between 0-40% Health - Cast ''Harden'''),
(8302, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 15000, 0, 11, 83657, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Deatheye - In Combat - Cast ''Hellgaze'''),
(8302, 0, 3, 0, 0, 0, 100, 0, 6000, 9000, 24000, 27000, 0, 11, 77576, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Deatheye - In Combat - Cast ''Stone Breath'''),
(8302, 0, 4, 0, 2, 0, 100, 0, 0, 40, 9000, 9000, 0, 11, 54663, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Deatheye - Between 0-40% Health - Cast ''Fatal Bite'''),
(8303, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Grunter - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(8303, 0, 4, 0, 0, 0, 100, 0, 2000, 9000, 9000, 25000, 0, 11, 79581, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grunter - In Combat - Cast ''Angry Snort'''),
(42235, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Darktail Bonepicker - On Aggro - Cast ''Sprint'' (No Repeat)'),
(42235, 0, 3, 0, 0, 0, 100, 0, 3000, 5000, 22000, 25000, 0, 11, 8139, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Darktail Bonepicker - In Combat - Cast ''Fevered Fatigue'''),
(42334, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marl Wormthorn - In Combat - Cast ''Wrath'''),
(42334, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79833, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Marl Wormthorn - Out of Combat - Cast ''Mark of the Wild'''),
(42334, 0, 3, 0, 0, 0, 100, 0, 7000, 9000, 14000, 20000, 0, 11, 33844, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Marl Wormthorn - In Combat - Cast ''Entangling Roots'''),
(42334, 0, 4, 0, 2, 0, 100, 0, 0, 40, 19000, 27000, 0, 11, 12160, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Marl Wormthorn - Between 0-40% Health - Cast ''Rejuvenation''');
