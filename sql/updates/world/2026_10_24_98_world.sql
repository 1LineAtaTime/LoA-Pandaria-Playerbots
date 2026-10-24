-- Grizzly Hills: creature abilities
-- Most of the zone's creatures (the Drakkari, the Iron Dwarves, the Venture Co., the worgen, furbolgs and wildlife) had no AI or
-- ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing
-- events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted. The creatures no
-- reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '26261,26347,26417,26544,27018,27259,27547,27593,27959';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26261, 0, 0, 0, 8, 0, 100, 0, 47394, 0, 60000, 60000, 33, 26261, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Grizzly Hills Giant - On SpellHit - Kill Credit'),
(26261, 0, 1, 0, 1, 0, 100, 0, 2000, 5000, 4000, 6000, 11, 46815, 0, 0, 0, 0, 0, 19, 26264, 85, 0, 0, 0, 0, 0, 'Grizzly Hills Giant - Out of Combat - Cast ''Toss Boulder'''),
(26261, 0, 2, 0, 1, 0, 100, 0, 3000, 9000, 7000, 13000, 11, 46815, 0, 0, 0, 0, 0, 19, 26417, 100, 0, 0, 0, 0, 0, 'Grizzly Hills Giant - Out of Combat - Cast ''Toss Boulder'''),
(26347, 0, 0, 0, 0, 0, 100, 0, 8000, 12000, 20000, 30000, 11, 52702, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Runic War Golem - In Combat - Cast ''Rune Punch'''),
(26417, 0, 0, 0, 8, 0, 25, 0, 47604, 0, 10000, 10000, 80, 2641700, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Runed Giant - On Spell Hit - Start Script for Free At Last'),
(26417, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 47329, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On Reset - Cast Spell'),
(26417, 0, 2, 0, 0, 0, 100, 1, 10000, 10000, 0, 0, 11, 49717, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'IC - cast spell'),
(26417, 0, 3, 0, 0, 0, 100, 0, 5000, 6000, 15000, 16000, 11, 55196, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'IC - cast spell'),
(26544, 0, 0, 0, 0, 0, 100, 0, 30000, 30000, 30000, 30000, 11, 3551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Warlord Zim''bo - In Combat - Cast ''Skull Crack'''),
(26544, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 2000, 4000, 11, 52283, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Warlord Zim''bo - In Combat - Cast ''Warlord Roar'''),
(27018, 0, 0, 0, 1, 0, 100, 0, 1000, 1000, 20000, 20000, 11, 48185, 0, 0, 0, 0, 0, 19, 27024, 20, 0, 0, 0, 0, 0, 'Shade of Arugal - Out of Combat - Cast ''Arugal Channel'''),
(27018, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 2000, 3000, 11, 53086, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shade of Arugal - In Combat - Cast ''Shadow Bolt'''),
(27259, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - Out of Combat - Set Event Phase 0'),
(27259, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 52209, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - On Aggro - Cast ''Incite Flames'''),
(27259, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - On Aggro - Increment Phase By 1'),
(27259, 0, 3, 0, 9, 1, 100, 0, 0, 40, 3400, 4800, 11, 52209, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - Within 0-40 Range - Cast ''Incite Flames'''),
(27259, 0, 4, 0, 3, 2, 100, 0, 15, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - Between 15-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat)'),
(27259, 0, 5, 0, 0, 0, 100, 0, 8000, 12000, 16000, 19000, 11, 52208, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - In Combat - Cast ''Flame Patch'' (Phase 1) (No Repeat)'),
(27259, 0, 6, 0, 7, 0, 100, 1, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - On Evade - Set Event Phase 0 (Phase 1) (No Repeat)'),
(27259, 0, 7, 0, 1, 0, 100, 0, 5000, 8000, 5000, 8000, 11, 48213, 0, 0, 0, 0, 0, 19, 26785, 25, 0, 0, 0, 0, 0, 'Dragonflayer Flamebinder - Out of Combat - Cast ''Dragonflayer Crone Fire'''),
(27547, 0, 0, 1, 2, 0, 100, 0, 0, 40, 9000, 9000, 11, 32714, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vladek - In HPCT - Cast ''Enrage'''),
(27547, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 47457, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vladek - In HPCT - Cast Worgen Transform - Male'),
(27593, 0, 0, 0, 27, 0, 100, 0, 0, 0, 0, 0, 11, 49107, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Passenger Boarded - Cast ''Vehicle: Warhead Fuse'''),
(27593, 0, 1, 3, 8, 0, 100, 0, 49372, 0, 0, 0, 11, 49510, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Spellhit ''Horde Boat to Torpedo'' - Cast ''Alliance Kill Credit Torpedo'''),
(27593, 0, 2, 3, 8, 0, 100, 0, 49257, 0, 0, 0, 11, 49340, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Spellhit ''Alliance Boat to Torpedo'' - Cast ''Horde Kill Credit Torpedo'''),
(27593, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 49250, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rocket Propelled Warhead - On Event Link - Cast ''Detonate'''),
(27959, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 11, 41408, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Cast ''Shadowform'''),
(27959, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 85, 50218, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Invoker Cast ''The Cleansing: Your Inner Turmoil''s Mirror Image Aura'''),
(27959, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 85, 41054, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Invoker Cast ''Copy Weapon'''),
(27959, 0, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 85, 41055, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Invoker Cast ''Copy Weapon'''),
(27959, 0, 4, 5, 61, 0, 100, 0, 0, 0, 0, 0, 85, 45205, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Invoker Cast ''Copy Offhand Weapon'''),
(27959, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 85, 45206, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Invoker Cast ''Copy Off-hand Weapon'''),
(27959, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 80, 2795900, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Summoned - Run Script'),
(27959, 0, 7, 0, 2, 0, 100, 1, 0, 50, 0, 0, 1, 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - Between 0-50% Health - Say Line 2'),
(27959, 0, 8, 9, 6, 0, 100, 1, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Say Line 3 (No Repeat)'),
(27959, 0, 9, 0, 61, 0, 100, 1, 0, 0, 0, 0, 3, 0, 11686, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Morph To Model 11686 (No Repeat)'),
(27959, 0, 10, 0, 2, 0, 100, 1, 0, 50, 0, 0, 11, 50222, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - Between 0-50% Health - Cast ''The Cleansing: Your Inner Turmoil`s Whisper to Controller - On Health 50%'' (No Repeat)'),
(27959, 0, 11, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 50223, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Cast ''The Cleansing: Your Inner Turmoil`s Whisper to Controller - On Death'''),
(27959, 0, 12, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 50238, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Your Inner Turmoil - On Just Died - Cast ''The Cleansing: Your Inner Turmoil`s On Death Cast on Master''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '23740,23745,24026,26410,26582,26583,27462,27467,27501,27549,27758,38453,51891,51893,51894';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(23740, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 29577, 0, 0, 2, 'Frosthorn Ram - In Combat - Cast ''Hoof Strike'''),
(23745, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 53824, 0, 0, 2, 'Garg - In Combat - Cast ''Throw'''),
(24026, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 50075, 0, 0, 2, 'Fanggore Worg - In Combat - Cast ''Maim Flesh'''),
(26410, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 35000, 40000, 11, 52710, 32, 0, 1, 'Rune-Smith Kathorn - In Combat - Cast ''Lightning Charged'''),
(26582, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 7, 32, 0, 1, 'Horrified Drakkari Warrior - In Combat - Cast ''Suicide'''),
(26583, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 7, 32, 0, 1, 'Horrified Drakkari Shaman - In Combat - Cast ''Suicide'''),
(27462, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 38861, 64, 0, 2, 'Westfall Brigade Hunter - In Combat - Cast ''Aimed Shot'''),
(27467, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 48815, 0, 0, 2, 'Amberpine Hunter - In Combat - Cast ''Shoot'''),
(27501, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 52719, 0, 0, 2, 'Westfall Brigade Marine - In Combat - Cast ''Concussion Blow'''),
(27501, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 50370, 0, 0, 2, 'Westfall Brigade Marine - In Combat - Cast ''Sunder Armor'''),
(27549, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 9080, 0, 0, 2, 'Westfall Brigade Elite - In Combat - Cast ''Hamstring'''),
(27549, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 11976, 0, 0, 2, 'Westfall Brigade Elite - In Combat - Cast ''Strike'''),
(27758, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 14516, 0, 0, 2, 'Westfall Brigade Defender - In Combat - Cast ''Strike'''),
(38453, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 61184, 0, 0, 2, 'Arcturis - In Combat - Cast ''Pounce'''),
(51891, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 2, 'Amberpine Footman - In Combat - Cast ''Shoot'''),
(51893, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 2, 'Westfall Brigade Footman - In Combat - Cast ''Shoot'''),
(51894, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 2, 'Camp Oneqwah Brave - In Combat - Cast ''Shoot''');
