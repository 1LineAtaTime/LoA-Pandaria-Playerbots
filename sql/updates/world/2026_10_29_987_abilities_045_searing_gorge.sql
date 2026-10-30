-- #987 wave 1 (pure additions): Searing Gorge creature abilities, imported from DeltaTho PR 15 file 2026_10_24_50_world.sql.
-- 21 creatures, 42 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '5855,5857,8277,8278,8279,8280,8281,8282,8283,14621,14622,14636,26214,47271,47275,47276,47434,47460,47462,47463,47470';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5855, 0, 0, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 11, 11970, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Magma Elemental - Within 0-8 Range - Cast ''Fire Nova'''),
(5857, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 4167, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Searing Lava Spider - On Aggro - Cast ''Web'' (No Repeat)'),
(8277, 0, 0, 0, 0, 0, 100, 0, 2000, 3500, 16000, 18000, 11, 3583, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rekk''tilac - In Combat - Cast ''Deadly Poison'''),
(8278, 0, 0, 0, 0, 0, 100, 0, 3500, 4500, 17500, 19500, 11, 5213, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Smoldar - In Combat - Cast ''Molten Metal'''),
(8279, 0, 0, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 9576, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Faulty War Golem - Between 30-60% Health - Cast ''Lock Down'' (No Repeat)'),
(8280, 0, 0, 0, 0, 0, 100, 0, 4000, 4500, 12000, 13000, 11, 13321, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shleipnarr - In Combat - Cast ''Mana Burn'''),
(8280, 0, 1, 0, 0, 0, 100, 0, 7000, 10000, 25000, 32000, 11, 7399, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shleipnarr - In Combat - Cast ''Terrify'''),
(8281, 0, 0, 0, 0, 0, 100, 0, 3000, 4500, 15000, 16000, 11, 11962, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scald - In Combat - Cast ''Immolate'''),
(8282, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Highlord Mastrogonde - In Combat - Cast ''Shadow Bolt'''),
(8282, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 25000, 28000, 11, 13323, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Highlord Mastrogonde - In Combat - Cast ''Polymorph'''),
(8282, 0, 2, 0, 0, 0, 100, 0, 4000, 12000, 16000, 22000, 11, 11639, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Highlord Mastrogonde - In Combat - Cast ''Shadow Word: Pain'''),
(8282, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 11974, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Highlord Mastrogonde - Between 0-30% Health - Cast ''Power Word: Shield'' (No Repeat)'),
(8283, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Slave Master Blackheart - In Combat - Cast ''Shoot'''),
(8283, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Slave Master Blackheart - Between 0-15% Health - Flee For Assist (No Repeat)'),
(8283, 0, 2, 0, 9, 0, 100, 0, 0, 20, 8000, 9000, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Slave Master Blackheart - Within 0-20 Range - Cast ''Net'''),
(14621, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Overseer Maltorius - In Combat - Cast ''Shadow Bolt'''),
(14621, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 25000, 28000, 11, 13323, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Overseer Maltorius - In Combat - Cast ''Polymorph'''),
(14621, 0, 2, 0, 0, 0, 100, 0, 4000, 12000, 16000, 22000, 11, 11639, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Overseer Maltorius - In Combat - Cast ''Shadow Word: Pain'''),
(14621, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 11974, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Overseer Maltorius - Between 0-30% Health - Cast ''Power Word: Shield'' (No Repeat)'),
(14622, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thorium Brotherhood Lookout - In Combat - Cast ''Shoot'''),
(14622, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Thorium Brotherhood Lookout - Between 0-15% Health - Flee For Assist (No Repeat)'),
(14622, 0, 2, 0, 9, 0, 100, 0, 0, 20, 8000, 9000, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thorium Brotherhood Lookout - Within 0-20 Range - Cast ''Net'''),
(14636, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 23416, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Chambermaid Pillaclencher - On Aggro - Cast ''Pillow Fight'' (No Repeat)'),
(14636, 0, 1, 0, 0, 0, 100, 0, 4000, 4500, 14000, 16500, 11, 23417, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Chambermaid Pillaclencher - In Combat - Cast ''Smother'''),
(26214, 0, 0, 0, 0, 0, 100, 0, 1000, 5000, 10000, 11000, 11, 3131, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frigid Lieutenant - IC - Cast Frost Breath'),
(26214, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 1, 0, 2000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frigid Lieutenant - On aggro - Say text 0'),
(47271, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Dig-Boss Dinwhisker - On Aggro - Cast ''Charge'' (No Repeat)'),
(47271, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 120000, 125000, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dig-Boss Dinwhisker - In Combat - Cast ''Battle Shout'''),
(47275, 0, 0, 0, 9, 0, 100, 0, 0, 5, 11000, 12000, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Footman - Within 0-5 Range - Cast ''Strike'''),
(47276, 0, 0, 0, 0, 0, 100, 0, 2000, 4500, 9000, 12000, 11, 9053, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Excavator - In Combat - Cast ''Fireball'''),
(47276, 0, 1, 0, 9, 0, 100, 0, 5, 30, 13000, 14500, 11, 8858, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Excavator - Within 5-30 Range - Cast ''Bomb'''),
(47434, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Iron Summit Guard - In Combat - Cast ''Shoot'''),
(47434, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Iron Summit Guard - Between 0-15% Health - Flee For Assist (No Repeat)'),
(47434, 0, 2, 0, 9, 0, 100, 0, 0, 20, 8000, 9000, 11, 12024, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Iron Summit Guard - Within 0-20 Range - Cast ''Net'''),
(47460, 0, 0, 0, 0, 0, 100, 0, 2000, 3000, 12000, 13000, 11, 80561, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight-Lord Arkkus - In Combat - Cast ''Flame Edge'''),
(47460, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 18968, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Twilight-Lord Arkkus - On Aggro - Cast ''Fire Shield'' (No Repeat)'),
(47462, 0, 0, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 11, 11970, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Archduke Calcinder - Within 0-8 Range - Cast ''Fire Nova'''),
(47462, 0, 1, 0, 0, 0, 100, 0, 8000, 9000, 22000, 26000, 11, 80549, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Archduke Calcinder - In Combat - Cast ''Gout of Flame'''),
(47462, 0, 2, 0, 0, 0, 100, 0, 12000, 14000, 33000, 35000, 11, 75068, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Archduke Calcinder - In Combat - Cast ''Lava Burst'''),
(47462, 0, 3, 0, 0, 0, 100, 0, 2000, 4000, 13000, 16000, 11, 35377, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Archduke Calcinder - In Combat - Cast ''Scorch'''),
(47463, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 16500, 22000, 11, 10733, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Searing Flamewraith - In Combat - Cast ''Flame Spray'''),
(47470, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cauldron Defender - In Combat - Cast ''Shoot''');
