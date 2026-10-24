-- The Forge of Souls: creature abilities
-- Most of the dungeon's creatures (the Scourge and the soul forges' servants) had no AI or ran classic rows: port their abilities
-- from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks
-- are left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags. The creatures no
-- reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '37094';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37094, 0, 0, 1, 8, 0, 0, 0, 69922, 0, 0, 0, 11, 70663, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Crucible of Souls - On Spellhit ''Temper Quel''Delar'' - Cast ''Shadow Nova'''),
(37094, 0, 1, 0, 61, 0, 0, 0, 69922, 0, 0, 0, 11, 69956, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Crucible of Souls - On Spellhit ''Temper Quel''Delar'' - Cast ''Return Tempered Quel''Delar'''),
(37094, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 69859, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On Reset - Aura self');
