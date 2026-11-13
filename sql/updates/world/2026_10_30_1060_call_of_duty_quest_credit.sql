-- Call of Duty 25924 (Horde): Erunak Stonespeaker (41618) gives the quest's own credit.
--
-- The quest's only objective (266797) is a kill credit of the Submerge Bunny (36901, amount 1); nothing gave it, so
-- Erunak's "area explored or event happens" (SmartAI action 15) only set the explored flag and the quest could never
-- complete. He now gives the 36901 credit first, then the event, on the same line-of-sight event (30 yd) as before.
-- (14482, Alliance, has an event objective - type AREATRIGGER, ObjectID -1 - which the core now counts in
-- Player::AreaExploredOrEventHappens, so Erunak 36915 needs no change.) The ship scene gives the same credit to the
-- riders (transport_ship_to_vashjir); this covers players who reach Erunak any other way.
--
-- Backup (state before this file): `smart_scripts` 41618 source 0 rows
--   (41618, 0, 0, 0, 10, 0, 100, 0, 0, 30, 1100, 1100, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker'),
--   (41618, 0, 1, 0, 10, 0, 100, 0, 1, 30, 1100, 1100, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker').
-- Undo:
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 41618 AND `source_type` = 0;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (41618, 0, 0, 0, 10, 0, 100, 0, 0, 30, 1100, 1100, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker'),
--   (41618, 0, 1, 0, 10, 0, 100, 0, 1, 30, 1100, 1100, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker');
-- Takes effect at the next worldserver start (or .reload smart_scripts). Re-runnable.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 41618 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41618, 0, 0, 2, 10, 0, 100, 0, 0, 30, 1100, 1100, 0, 33, 36901, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker - OOC LOS - Kill credit Submerge Bunny (Call of Duty 25924)'),
(41618, 0, 1, 3, 10, 0, 100, 0, 1, 30, 1100, 1100, 0, 33, 36901, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker - OOC LOS - Kill credit Submerge Bunny (Call of Duty 25924)'),
(41618, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker - Linked - Quest event happens (Call of Duty 25924)'),
(41618, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 15, 25924, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Erunak Stonespeaker - Linked - Quest event happens (Call of Duty 25924)');
