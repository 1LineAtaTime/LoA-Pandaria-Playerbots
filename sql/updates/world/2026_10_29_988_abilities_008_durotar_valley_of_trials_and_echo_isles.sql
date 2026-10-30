-- #988 wave 2 (merge): Durotar, Valley of Trials and Echo Isles creature abilities from DeltaTho PR 15 file 2026_10_24_11_world.sql, ADDED to creatures that already have smart_scripts.
-- 28 creatures, 51 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3099:1 3106:1 3107:1 3108:1 3111:3 3112:2 3114:2 3115:1 3116:2 3117:1 3118:2 3123:1 3125:3 3127:1 3183:2 3196:1 3197:1 3198:1 3199:2 3281:1 5809:4 5822:2 5823:1 5824:3 5826:4 10676:3 39072:2 39317:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3099 AND `id` >= 1)
  OR (`entryorguid` = 3106 AND `id` >= 1)
  OR (`entryorguid` = 3107 AND `id` >= 1)
  OR (`entryorguid` = 3108 AND `id` >= 1)
  OR (`entryorguid` = 3111 AND `id` >= 3)
  OR (`entryorguid` = 3112 AND `id` >= 2)
  OR (`entryorguid` = 3114 AND `id` >= 2)
  OR (`entryorguid` = 3115 AND `id` >= 1)
  OR (`entryorguid` = 3116 AND `id` >= 2)
  OR (`entryorguid` = 3117 AND `id` >= 1)
  OR (`entryorguid` = 3118 AND `id` >= 2)
  OR (`entryorguid` = 3123 AND `id` >= 1)
  OR (`entryorguid` = 3125 AND `id` >= 3)
  OR (`entryorguid` = 3127 AND `id` >= 1)
  OR (`entryorguid` = 3183 AND `id` >= 2)
  OR (`entryorguid` = 3196 AND `id` >= 1)
  OR (`entryorguid` = 3197 AND `id` >= 1)
  OR (`entryorguid` = 3198 AND `id` >= 1)
  OR (`entryorguid` = 3199 AND `id` >= 2)
  OR (`entryorguid` = 3281 AND `id` >= 1)
  OR (`entryorguid` = 5809 AND `id` >= 4)
  OR (`entryorguid` = 5822 AND `id` >= 2)
  OR (`entryorguid` = 5823 AND `id` >= 1)
  OR (`entryorguid` = 5824 AND `id` >= 3)
  OR (`entryorguid` = 5826 AND `id` >= 4)
  OR (`entryorguid` = 10676 AND `id` >= 3)
  OR (`entryorguid` = 39072 AND `id` >= 2)
  OR (`entryorguid` = 39317 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3099, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dire Mottled Boar - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(3106, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 17000, 22000, 0, 11, 78744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Surf Crawler - In Combat - Cast ''Bubblebeam'''),
(3107, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 17000, 22000, 0, 11, 78744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Surf Crawler - In Combat - Cast ''Bubblebeam'''),
(3108, 0, 1, 0, 2, 0, 100, 0, 0, 50, 24000, 26000, 0, 11, 79840, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Encrusted Surf Crawler - Between 0-50% Health - Cast ''Harden'''),
(3111, 0, 3, 0, 0, 0, 100, 0, 4000, 4500, 17000, 24500, 0, 11, 80146, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Quilboar - In Combat - Cast ''Bonk'''),
(3112, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 80149, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Scout - On Aggro - Cast ''Throw Axe'' (No Repeat)'),
(3112, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 300000, 300000, 0, 11, 5280, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Scout - Out of Combat - Cast ''Razor Mane'''),
(3114, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Battleguard - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(3114, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 300000, 300000, 0, 11, 5280, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Battleguard - Out of Combat - Cast ''Razor Mane'''),
(3114, 0, 4, 0, 0, 0, 100, 0, 4000, 6000, 25000, 31000, 0, 11, 77558, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Battleguard - In Combat - Cast ''Bloody Strike'''),
(3114, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Battleguard - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3115, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 12000, 19000, 0, 11, 77522, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Harpy - In Combat - Cast ''Swoop'''),
(3115, 0, 2, 0, 2, 0, 100, 0, 0, 50, 13000, 24000, 0, 11, 80215, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Harpy - Between 0-50% Health - Cast ''Dustwind'''),
(3116, 0, 2, 0, 2, 0, 100, 0, 0, 50, 13000, 24000, 0, 11, 80215, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Pillager - Between 0-50% Health - Cast ''Dustwind'''),
(3117, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 18000, 22000, 0, 11, 75930, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Savage - In Combat - Cast ''Mangle'''),
(3117, 0, 2, 0, 2, 0, 100, 0, 0, 50, 13000, 24000, 0, 11, 80215, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Savage - Between 0-50% Health - Cast ''Dustwind'''),
(3118, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77160, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Storm Witch - In Combat - Cast ''Nimbus Bolt'''),
(3118, 0, 3, 0, 2, 0, 100, 0, 0, 50, 13000, 24000, 0, 11, 80215, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Storm Witch - Between 0-50% Health - Cast ''Dustwind'''),
(3118, 0, 4, 0, 0, 0, 100, 0, 7000, 9000, 24000, 26000, 0, 11, 12058, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Dustwind Storm Witch - In Combat - Cast ''Chain Lightning'''),
(3123, 0, 1, 0, 0, 0, 100, 0, 3000, 4500, 24000, 25000, 0, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bloodtalon Scythemaw - In Combat - Cast ''Rend'''),
(3125, 0, 3, 0, 0, 0, 100, 0, 4000, 4500, 32000, 38000, 0, 11, 79687, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Clattering Scorpid - In Combat - Cast Poison (No Repeat)'),
(3127, 0, 1, 0, 0, 0, 100, 0, 3500, 4500, 15000, 24000, 0, 11, 79607, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Venomtail Scorpid - In Combat - Cast ''Venom Splash'''),
(3183, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 35913, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Yarrog Baneshadow - In Combat - Cast ''Fel Fireball'''),
(3183, 0, 3, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 11, 51733, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Yarrog Baneshadow - Out of Combat - Cast ''Shadow Channelling'' (No Repeat)'),
(3183, 0, 4, 0, 0, 0, 100, 0, 8000, 12000, 22000, 29000, 0, 11, 37628, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Yarrog Baneshadow - In Combat - Cast ''Fel Immolate'''),
(3196, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79932, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Neophyte - In Combat - Cast ''Shadow Bolt'''),
(3196, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79954, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Neophyte - Out of Combat - Cast ''Fel Armor'''),
(3196, 0, 3, 0, 0, 0, 100, 0, 6000, 8000, 60000, 65000, 0, 11, 80174, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Neophyte - In Combat - Cast ''Felblood'''),
(3197, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 60000, 65000, 0, 11, 80174, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Fanatic - In Combat - Cast ''Felblood'''),
(3198, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 77721, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Apprentice - In Combat - Cast ''Shadow Weave'''),
(3198, 0, 2, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 12746, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Apprentice - Out of Combat - Cast ''Summon Voidwalker'' (No Repeat)'),
(3198, 0, 3, 0, 0, 0, 100, 0, 6000, 8000, 60000, 65000, 0, 11, 80174, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Apprentice - In Combat - Cast ''Felblood'''),
(3199, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79938, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Cultist - In Combat - Cast ''Incinerate'''),
(3199, 0, 3, 0, 0, 0, 100, 0, 6000, 8000, 60000, 65000, 0, 11, 80174, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Cultist - In Combat - Cast ''Felblood'''),
(3281, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 18000, 25000, 0, 11, 79607, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sarkoth - In Combat - Cast ''Venom Splash'''),
(5809, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 78894, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sergeant Curtis - On Aggro - Cast ''Leaping Cleave'' (No Repeat)'),
(5809, 0, 5, 0, 0, 0, 100, 0, 7000, 9000, 22000, 27000, 0, 11, 79726, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sergeant Curtis - In Combat - Cast ''Ticking Time Bomb'''),
(5822, 0, 2, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 30842, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felweaver Scornn - Out of Combat - Cast ''Summon Felhunter'' (No Repeat)'),
(5822, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felweaver Scornn - Out of Combat - Cast ''Demon Armor'''),
(5822, 0, 4, 0, 2, 0, 100, 0, 0, 40, 16000, 23000, 0, 11, 79933, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 'Felweaver Scornn - Between 0-40% Health - Cast ''Drain Life'''),
(5823, 0, 1, 0, 2, 0, 100, 0, 0, 50, 24000, 26000, 0, 11, 79840, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Death Flayer - Between 0-50% Health - Cast ''Harden'''),
(5823, 0, 2, 0, 0, 0, 100, 0, 3500, 4500, 15000, 24000, 0, 11, 79607, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Death Flayer - In Combat - Cast ''Venom Splash'''),
(5824, 0, 3, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 5280, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captain Flat Tusk - On Respawn - Cast ''Razor Mane'' (No Repeat)'),
(5824, 0, 4, 0, 9, 0, 100, 0, 0, 5, 7000, 12000, 0, 11, 77558, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Captain Flat Tusk - Within 0-5 Range - Cast ''Bloody Strike'''),
(5824, 0, 5, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 79733, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captain Flat Tusk - On Aggro - Cast ''Commanding Shout'' (No Repeat)'),
(5826, 0, 4, 0, 0, 0, 100, 0, 5000, 6000, 16000, 18000, 0, 11, 80117, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Geolord Mottle - In Combat - Cast ''Earth Spike'''),
(5826, 0, 5, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79927, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Geolord Mottle - Out of Combat - Cast ''Earth Shield'''),
(10676, 0, 3, 0, 9, 0, 100, 0, 0, 20, 7000, 9000, 0, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - Within 0-20 Range - Cast ''Net'''),
(39072, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 24000, 32000, 0, 11, 79782, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Naj''tess - In Combat - Cast ''Orb of Corruption'''),
(39317, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Scout - In Combat - Cast ''Shoot'''),
(39317, 0, 2, 0, 9, 0, 100, 0, 0, 10, 15000, 22000, 0, 11, 79584, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Scout - Within 0-10 Range - Cast ''Fan of Knives''');
