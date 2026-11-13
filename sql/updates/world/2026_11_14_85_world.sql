-- Eastern Plaguelands, Soft Landing (27460): the Crusader's Flare goes up when the quest is handed in at it.
-- Sourced: the quest is handed in at the Crusader's Flare (object 205875) to signal the attack; spell 85889 "Argent Flare"
-- (1.5 s, periodic trigger of 85888 "Argent Flare Visual") sits in the id block of the chain's portal spells 85891 / 85892, and
-- both TDBs spawn a Generic Bunny 44775 on the flare (guid 173885 here). Nothing cast it.
-- Ours: the wiring. The flare object gets a script: when Soft Landing is rewarded at it, the bunny beside it casts Argent Flare
-- (once per hand-in; every player near the flare sees it, the object and the bunny are unphased). The bunny also stops
-- wandering 3 yd around the flare.
-- What the player sees: one flare launch from the Crusader's Flare at the moment of the turn-in.
UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI' WHERE `entry` = 205875;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 205875 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(205875, 1, 0, 0, 20, 0, 100, 0, 27460, 0, 0, 0, 0, 86, 85889, 2, 19, 44775, 10, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Crusader''s Flare - On Quest Soft Landing Rewarded - Cross Cast Argent Flare from the Generic Bunny');
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` = 173885 AND `id` = 44775;
