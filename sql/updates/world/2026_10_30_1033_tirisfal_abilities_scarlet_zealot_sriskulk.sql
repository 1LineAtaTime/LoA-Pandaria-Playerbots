-- Tirisfal Glades: abilities of Scarlet Zealot (1537) and Sri'skulk (10359), from DeltaTho's Tirisfal Glades fixes (2026_10_26_04_world.sql).
-- Both had no AI and only melee'd. The earlier import of that zone's abilities skipped them by mistake, so they are added here.
-- Scarlet Zealot shouts a Crusade line on aggro and casts Zealotry (84535); Sri'skulk, the rare spider of Venomweb Vale, casts Deadly Poison (3583).
-- Both abilities as listed by Wowpedia (Scarlet Zealot, Sri'skulk).
-- Idempotent. AIName is set only where the creature has no AI of its own.
-- Undo: DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid IN (1537, 10359); UPDATE creature_template SET AIName = '' WHERE entry IN (1537, 10359);

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND `entry` IN (1537, 10359);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (1537, 10359);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(1537, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 1, 'Scarlet Zealot - On Aggro - Say Line 0'),
(1537, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 5000, 5000, 11, 84535, 0, 2, 'Scarlet Zealot - In Combat - Cast ''Zealotry'''),
(10359, 0, 0, 0, 0, 0, 100, 0, 4000, 4000, 15000, 15000, 11, 3583, 0, 2, 'Sri''skulk - In Combat - Cast ''Deadly Poison''');
