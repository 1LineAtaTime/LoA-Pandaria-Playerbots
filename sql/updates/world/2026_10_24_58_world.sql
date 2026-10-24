-- Plaguelands: The Scarlet Enclave: creature abilities
-- The Scarlet Crusade of New Avalon (Scarlet Commanders, Preachers, Fleet Guardians, Champions, Crusaders, Peasants) and the other
-- foes of the death knight starting zone had little or no AI. Cataclysm left the zone unchanged, so port their abilities from the
-- classic (WotLK) data first, falling back to the Cataclysm data; rows that need action lists or actions LoA lacks are left out.
-- The creatures neither scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '28488,28576,28577,28642,28683,28834,28850,28934,28964,29030,29031,29102,29103,29104,29106,29115,29136';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28488, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 15043, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Frostbolt'),
(28576, 0, 0, 0, 1, 0, 30, 0, 5000, 20000, 5000, 20000, 11, 52149, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Citizen of Havenshire - Out of Combat - Cast ''Rain of Darkness'''),
(28577, 0, 0, 0, 1, 0, 30, 0, 5000, 20000, 5000, 20000, 11, 52149, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Citizen of Havenshire - Out of Combat - Cast ''Rain of Darkness'''),
(28642, 0, 0, 0, 1, 0, 100, 0, 5000, 20000, 5000, 20000, 11, 52147, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scourge Sky Darkener - Out of Combat - Cast ''Sky Darkener Assault'''),
(28683, 0, 0, 1, 4, 0, 100, 0, 0, 0, 0, 0, 11, 52265, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Stable Master Kitrik - On Aggro - Cast ''Repo'''),
(28683, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stable Master Kitrik- On Aggro - Say Line 0'),
(28683, 0, 2, 0, 5, 0, 100, 0, 5000, 5000, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stable Master Kitrik - On Player Kill - Say Line 1'),
(28834, 0, 0, 0, 0, 0, 100, 0, 1000, 5000, 5000, 8000, 11, 52566, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Fleet Defender - In Combat - Cast ''Shoot'''),
(28850, 0, 0, 0, 0, 0, 100, 0, 1000, 1000, 4000, 5000, 11, 52539, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Land Cannon - In Combat - Cast ''Cannonball'''),
(28934, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 11, 52372, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Icy Touch'''),
(28934, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 8000, 12000, 11, 52373, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Plague Strike'''),
(28934, 0, 2, 0, 0, 0, 100, 0, 4000, 8000, 4000, 8000, 11, 52374, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Blood Strike'''),
(28934, 0, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 52375, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Death Coil'''),
(28964, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 8000, 12000, 11, 52835, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Lord Jesseriah McCree - In Combat - Cast ''Cleave'''),
(28964, 0, 1, 0, 0, 0, 100, 0, 12000, 18000, 30000, 35000, 11, 52836, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Lord Jesseriah McCree - In Combat - Cast ''Holy Wrath'''),
(29030, 0, 0, 0, 1, 0, 100, 0, 4000, 10000, 8000, 14000, 11, 52953, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - Out of Combat - Cast ''Torch'''),
(29030, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 11, 52372, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Icy Touch'''),
(29030, 0, 2, 0, 0, 0, 100, 0, 3000, 5000, 8000, 12000, 11, 52373, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Plague Strike'''),
(29030, 0, 3, 0, 0, 0, 100, 0, 4000, 8000, 4000, 8000, 11, 52374, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Blood Strike'''),
(29031, 0, 0, 0, 1, 0, 100, 0, 4000, 10000, 8000, 14000, 11, 52953, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - Out of Combat - Cast ''Torch'''),
(29031, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 11, 52372, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Icy Touch'''),
(29031, 0, 2, 0, 0, 0, 100, 0, 3000, 5000, 8000, 12000, 11, 52373, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Plague Strike'''),
(29031, 0, 3, 0, 0, 0, 100, 0, 4000, 8000, 4000, 8000, 11, 52374, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight - In Combat - Cast ''Blood Strike'''),
(29102, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hearthglen Crusader - On Reset - Set Event Phase 1'),
(29102, 0, 1, 0, 24, 1, 100, 0, 52196, 1, 1000, 2000, 11, 53348, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hearthglen Crusader - On Target Buffed With ''Frostbrood Vanquisher'' - Cast ''Arrow Assault'' (Phase 1)'),
(29102, 0, 2, 0, 9, 1, 100, 0, 40, 150, 4000, 6000, 11, 53345, 64, 0, 1, 0, 0, 9, 0, 40, 150, 0, 0, 0, 0, 'Hearthglen Crusader - Within 40-150 Range - Cast ''Arrow Assault'' (Phase 1)'),
(29102, 0, 3, 0, 8, 0, 100, 0, 53110, 1, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hearthglen Crusader - On Spellhit ''Devour Humanoid'' - Set Event Phase 2'),
(29103, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Tirisfal Crusader - On Reset - Set Event Phase 1'),
(29103, 0, 1, 0, 24, 1, 100, 0, 52196, 1, 1000, 2000, 11, 53348, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tirisfal Crusader - On Target Buffed With ''Frostbrood Vanquisher'' - Cast ''Arrow Assault'' (Phase 1)'),
(29103, 0, 2, 0, 9, 1, 100, 0, 40, 150, 4000, 6000, 11, 53345, 64, 0, 1, 0, 0, 9, 0, 40, 150, 0, 0, 0, 0, 'Tirisfal Crusader - Within 40-150 Range - Cast ''Arrow Assault'' (Phase 1)'),
(29103, 0, 3, 0, 8, 0, 100, 0, 53110, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Tirisfal Crusader - On Spellhit ''Devour Humanoid'' - Set Event Phase 2'),
(29104, 0, 0, 0, 0, 0, 100, 0, 2000, 3000, 4000, 5000, 11, 53117, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Ballista - In Combat - Cast ''Ballista Assault'''),
(29106, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 11, 52372, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight Champion - In Combat - Cast ''Icy Touch'''),
(29106, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 8000, 12000, 11, 52373, 32, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight Champion - In Combat - Cast ''Plague Strike'''),
(29106, 0, 2, 0, 0, 0, 100, 0, 4000, 8000, 4000, 8000, 11, 52374, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight Champion - In Combat - Cast ''Blood Strike'''),
(29106, 0, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 52375, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Death Knight Champion - In Combat - Cast ''Death Coil'''),
(29115, 0, 0, 0, 0, 0, 100, 0, 8000, 12000, 8000, 12000, 11, 53633, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rampaging Abomination - In Combat - Cast ''Cleave'''),
(29115, 0, 1, 0, 9, 0, 100, 0, 8, 40, 8000, 12000, 11, 50335, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rampaging Abomination - Within 8-40 Range - Cast ''Scourge Hook'''),
(29136, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 53278, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Volatile Ghoul - On Just Died - Cast ''Ghoulplosion''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '28530,28594,28856,29080';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(28530, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Scarlet Commander - In Combat - Cast ''Shoot'''),
(28594, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 15498, 64, 0, 2, 'Scarlet Preacher - In Combat - Cast ''Holy Smite'''),
(28594, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 19725, 0, 0, 2, 'Scarlet Preacher - In Combat - Cast ''Turn Undead'''),
(28856, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 25710, 0, 0, 2, 'Scarlet Fleet Guardian - In Combat - Cast ''Heroic Strike'''),
(28856, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 52566, 0, 0, 2, 'Scarlet Fleet Guardian - In Combat - Cast ''Shoot'''),
(29080, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 25710, 0, 0, 2, 'Scarlet Champion - In Combat - Cast ''Heroic Strike'''),
(29080, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 52566, 0, 0, 2, 'Scarlet Champion - In Combat - Cast ''Shoot''');
