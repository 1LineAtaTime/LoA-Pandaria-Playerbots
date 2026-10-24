-- Quest turn-in scripts that still started on the pre-Cataclysm version of their quest, so they never ran

-- Johaan's Experiment (24977): feeding the Laced Pumpkin to the Captured Scarlet Zealot (1931)
-- His script still started on the pre-Cataclysm quest Fields of Grief (407) and walked him around the Gallow's End cellar. Since
-- Cataclysm he is held outside by the gazebo and the quest is 24977: on turn-in he speaks, turns into a ghoul (Ghoul Form, 3287),
-- staggers around, rots and dies, then despawns and respawns quickly for the next player.
UPDATE `creature` SET `spawntimesecs` = 1 WHERE `guid` = 193727;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 1931 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1931, 0, 0, 0, 20, 0, 100, 0, 24977, 0, 0, 0, 0, 80, 193100, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Quest \'Johaan\'s Experiment\' Finished - Run Script'),
(1931, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 41, 3000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Just Died - Despawn In 3000 ms');

-- No turn-ins to him while he is turning; the quest giver flag comes back when he respawns
DELETE FROM `smart_scripts` WHERE `entryorguid` = 193100 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(193100, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 83, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Remove Npc Flag Questgiver'),
(193100, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Say Line 0'),
(193100, 9, 2, 0, 0, 0, 100, 0, 3000, 3000, 0, 0, 0, 11, 3287, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Cast \'Ghoul Form\''),
(193100, 9, 3, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Say Line 1'),
(193100, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 89, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Start Random Movement'),
(193100, 9, 5, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 0, 37, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Captured Scarlet Zealot - On Script - Kill Self');

-- Sarkoth (25129, was 790): Hana'zua (3287), lying wounded, sits up, kneels and sends the player off, then lies back down
UPDATE `smart_scripts` SET `event_param1` = 25129, `comment` = 'Hana''zua - On Quest ''Sarkoth'' Finished - Remove Bytes 1 Dead'
WHERE `entryorguid` = 3287 AND `source_type` = 0 AND `id` = 0;

-- Samophlange (29024, was 902) and Nugget Slugs (29027, was 3922): Sputtervalve (3442) and Wenikee Boltbucket (9316) try to fix the
-- Samophlange and break it. Their actionlists also respawned the Ratchet Samophlange (now a copper vein's guid); the ones at their
-- camps are always spawned, and setting a respawn time on those would despawn them, so that step is gone.
UPDATE `smart_scripts` SET `event_param1` = 29024, `comment` = 'Sputtervalve - On Quest ''Samophlange'' Finished - Run Script'
WHERE `entryorguid` = 3442 AND `source_type` = 0 AND `id` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 344200 AND `source_type` = 9 AND `id` = 1;
UPDATE `smart_scripts` SET `event_param1` = 2000, `event_param2` = 2000 WHERE `entryorguid` = 344200 AND `source_type` = 9 AND `id` = 2;

UPDATE `smart_scripts` SET `event_param1` = 29027, `comment` = 'Wenikee Boltbucket - On Quest ''Nugget Slugs'' Finished - Run Script'
WHERE `entryorguid` = 9316 AND `source_type` = 0 AND `id` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 931600 AND `source_type` = 9 AND `id` = 1;
-- He opened with his closing line ("... oops.") instead of "Ok, let's get started."
UPDATE `smart_scripts` SET `action_param1` = 0, `comment` = 'Wenikee Boltbucket - On Script - Say Line 0'
WHERE `entryorguid` = 931600 AND `source_type` = 9 AND `id` = 0;
UPDATE `smart_scripts` SET `event_param1` = 2000, `event_param2` = 2000 WHERE `entryorguid` = 931600 AND `source_type` = 9 AND `id` = 2;
