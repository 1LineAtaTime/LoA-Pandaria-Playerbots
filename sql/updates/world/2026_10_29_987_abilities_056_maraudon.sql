-- #987 wave 1 (pure additions): Maraudon creature abilities, imported from DeltaTho PR 15 file 2026_10_24_61_world.sql.
-- 22 creatures, 39 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '11783,11784,11790,11791,11792,11793,11794,12206,12207,12216,12217,12218,12219,12220,12221,12222,12223,12224,13141,13323,13533,13743';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11783, 0, 0, 0, 0, 0, 100, 0, 1000, 6000, 5000, 11000, 11, 13584, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Theradrim Guardian - In Combat - Cast Strike'),
(11784, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 21057, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Theradrim Guardian - On Just Died - Cast ''Summon Theradrim Shardling'''),
(11784, 0, 1, 0, 0, 0, 100, 0, 1000, 6000, 8000, 14000, 11, 16790, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Theradrim Guardian - In Combat - Cast Knockdown'),
(11790, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 21061, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Satyr - On Reset - Cast Putrid Breath'),
(11790, 0, 1, 0, 0, 0, 100, 0, 1000, 5000, 5000, 7000, 11, 14873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Putridus Satyr - In Combat - Cast Sinister Strike'),
(11790, 0, 2, 0, 0, 0, 100, 0, 4000, 15000, 22500, 30000, 11, 12540, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Putridus Satyr - In Combat - Cast Gouge'),
(11791, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 21061, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Trickster - On Reset - Cast Putrid Breath'),
(11791, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Trickster - On Reset - Cast Thrash'),
(11791, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 13299, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Trickster - On Reset - Cast Poison Proc'),
(11791, 0, 3, 0, 67, 0, 100, 0, 5000, 5000, 0, 0, 11, 15657, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Putridus Trickster - Behind Target - Cast Backstab'),
(11792, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 21061, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Shadowstalker - On Reset - Cast Putrid Breath'),
(11792, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Shadowstalker - On Reset - Cast Sneak'),
(11792, 0, 2, 0, 0, 0, 100, 0, 1000, 5000, 7000, 10000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Putridus Shadowstalker - In Combat - Cast Hamstring'),
(11792, 0, 3, 0, 0, 0, 100, 0, 4000, 15000, 22500, 30000, 11, 15087, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Putridus Shadowstalker - In Combat - Cast Evasion'),
(11793, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8601, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Celebrian Dryad - On Reset - Cast Slowing Poison'),
(11794, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 2000, 2000, 11, 15795, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sister of Celebras - In Combat - Cast Throw'),
(11794, 0, 1, 0, 0, 0, 100, 0, 1000, 5000, 5000, 8000, 11, 14516, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sister of Celebras - In Combat - Cast Strike'),
(12206, 0, 0, 0, 0, 0, 100, 0, 1000, 5000, 8000, 14000, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primordial Behemoth - In Combat - Cast Trample'),
(12206, 0, 1, 0, 0, 0, 100, 0, 7000, 8000, 11000, 16000, 11, 21071, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primordial Behemoth - In Combat - Cast Boulder'),
(12207, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 21788, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thessala Hydra - On Reset - Cast Deadly Poison'),
(12207, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thessala Hydra - On Reset - Cast Thrash'),
(12207, 0, 2, 0, 0, 0, 100, 0, 1000, 9000, 11000, 20000, 11, 21790, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thessala Hydra - In Combat - Cast Aqua Jet'),
(12216, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 2500, 3000, 11, 21067, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Poison Sprite - In Combat - Cast Poison Bolt'),
(12217, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 2500, 3500, 11, 21068, 96, 1, 0, 0, 0, 24, 30, 0, 0, 0, 0, 0, 0, 'Corruptor - In Combat - Cast Corruption'),
(12217, 0, 1, 0, 0, 0, 100, 0, 4000, 11000, 10000, 29000, 11, 5413, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 'Corruptor - In Combat - Cast Noxious Catalyst'),
(12218, 0, 0, 0, 0, 0, 100, 0, 4000, 17000, 10000, 29000, 11, 21069, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vile Larva - In Combat - Cast Larva Goo'),
(12219, 0, 0, 0, 0, 0, 100, 0, 0, 10000, 13000, 24000, 11, 21749, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Barbed Lasher - In Combat - Cast Thorn Volley'),
(12220, 0, 0, 0, 0, 0, 100, 0, 0, 15000, 13000, 24000, 11, 11922, 32, 1, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 'Constrictor Vine - In Combat - Cast Entangling Roots'),
(12221, 0, 0, 0, 0, 0, 100, 0, 1000, 7000, 10000, 15000, 11, 21070, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Noxious Slime - In Combat - Cast Noxious Cloud'),
(12222, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 7000, 10000, 11, 26419, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Creeping Sludge - IC - Cast Acid Spray'),
(12222, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 75, 22638, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Creeping Sludge - On reset - Add Poison Shock'),
(12223, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 8000, 13000, 11, 11428, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cavern Lurker - In Combat - Cast Knockdown'),
(12224, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 16000, 16000, 11, 7948, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cavern Shambler - In Combat - Cast Wild Regeneration'),
(12224, 0, 1, 0, 0, 0, 100, 0, 7000, 11000, 11500, 20000, 11, 16790, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cavern Shambler - In Combat - Cast Knockdown'),
(13141, 0, 0, 0, 0, 0, 100, 0, 1000, 5000, 5000, 7000, 11, 13446, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deeprot Stomper - In Combat - Cast Strike'),
(13141, 0, 1, 0, 0, 0, 100, 0, 4000, 15000, 22500, 30000, 11, 11876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Deeprot Stomper - In Combat - Cast War Stomp'),
(13323, 0, 0, 0, 0, 0, 100, 0, 1000, 10000, 10000, 20000, 11, 8281, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Subterranean Diemetradon - In Combat - Cast Sonic Burst'),
(13533, 0, 0, 0, 0, 0, 100, 0, 6000, 13000, 20000, 29000, 11, 5413, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spewed Larva - In Combat - Cast Noxious Catalyst'),
(13743, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25025, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Corrupt Force of Nature - In Combat - Cast ''Earth Shock''');
