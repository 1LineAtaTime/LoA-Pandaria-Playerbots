-- It Does You No Good In The Keg (30624 Alliance, 30653 Horde): beer is delivered to Zhu's Wardens (Koro Mistwalker 59846 gives
-- credit 59878), the Shado-Pan (Fo Sho Knucklebump 59829, credit 59880), the Crane Wing priests (credit 59879) and the faction's
-- settlers: the night elves (Lyalia 59849, credit 59881 "Beer Credit: Alliance") for the Alliance, the tauren (Sunwalker Dezco
-- 59851, credit 59882 "Beer Credit: Horde") for the Horde, as both world databases have the objectives. Lyalia gave the Horde
-- credit and Dezco the Crane Wing priests', so the Alliance copy could never be finished (it was handed in unfinished through the
-- core's old auto-complete loophole). Lyalia now gives 59881 and Dezco 59882. Until the Crane Wing priests' place is known (none
-- stand near the camps), each settler leader also gives the priests' credit, so each faction gets it from its own, friendly
-- camp; the credit only counts for players on the quest.
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (59849, 59851) AND `source_type` = 0 AND `action_type` = 33;
DELETE FROM `smart_scripts` WHERE (`entryorguid` = 59851 AND `source_type` = 0 AND `id` = 5) OR (`entryorguid` = 59849 AND `source_type` = 0 AND `id` = 4);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(59849, 0, 3, 0, 60, 0, 100, 0, 1000, 1000, 1000, 1000, 0, 33, 59881, 0, 0, 0, 0, 0, 18, 10, 0, 0, 0, 0, 0, 0, 0, 'Lyalia - On Update - Quest Credit ''Beer Credit: Alliance'' to players within 10 yards'),
(59849, 0, 4, 0, 60, 0, 100, 0, 1000, 1000, 1000, 1000, 0, 33, 59879, 0, 0, 0, 0, 0, 18, 10, 0, 0, 0, 0, 0, 0, 0, 'Lyalia - On Update - Quest Credit ''Beer Credit: Redwing Priests'' to players within 10 yards (stand-in)'),
(59851, 0, 4, 0, 60, 0, 100, 0, 1000, 1000, 1000, 1000, 0, 33, 59882, 0, 0, 0, 0, 0, 18, 10, 0, 0, 0, 0, 0, 0, 0, 'Sunwalker Dezco - On Update - Quest Credit ''Beer Credit: Horde'' to players within 10 yards'),
(59851, 0, 5, 0, 60, 0, 100, 0, 1000, 1000, 1000, 1000, 0, 33, 59879, 0, 0, 0, 0, 0, 18, 10, 0, 0, 0, 0, 0, 0, 0, 'Sunwalker Dezco - On Update - Quest Credit ''Beer Credit: Redwing Priests'' to players within 10 yards (stand-in)');
