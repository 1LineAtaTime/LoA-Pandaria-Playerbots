-- Eastern Plaguelands, Tyr's Hand: the Scarlet healers heal their friends.
-- Sourced: both TDBs' scripts: Scarlet Cleric 9449 casts Heal 15586 on a friend below 50 %, Scarlet Curate 9450 Renew 25058 and
-- Scarlet Warder 9447 Holy Light 15493 on a friend below 40 %, every 15-18 s, within 40 yd; Wowhead comments on 9447 and 9449
-- ("heal themselves and comrades", "Clerics heal").
-- Before: the Cleric's row cast Mind Blast and the Curate's Dispel Magic, the Warder had no such row, and the two rows fired
-- only for a creature standing inside the caster: they were "friendly health percent" events (74) with a creature-range
-- target of 0 yd, and this core's event 74 looks for the friend in its target list and has no invoker mode (the TDB rows use
-- target 7, which SmartScript::ProcessEvent refuses for that event here).
-- Ours: the same behaviour through event 14 "friendly health" (missing health in points, radius): 650 of the Cleric's 1268-1321
-- for 50 %, 760 for the 40 % of the other two. The friend must be in combat and in the healer's own phase.
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` = 9449 AND `id` = 1) OR (`entryorguid` = 9450 AND `id` = 2) OR (`entryorguid` = 9447 AND `id` = 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(9447, 0, 2, 0, 14, 0, 100, 0, 760, 40, 15000, 18000, 0, 11, 15493, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Warder - Friendly Missing 760 Health - Cast ''Holy Light'''),
(9449, 0, 1, 0, 14, 0, 100, 0, 650, 40, 15000, 18000, 0, 11, 15586, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Cleric - Friendly Missing 650 Health - Cast ''Heal'''),
(9450, 0, 2, 0, 14, 0, 100, 0, 760, 40, 15000, 18000, 0, 11, 25058, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Curate - Friendly Missing 760 Health - Cast ''Renew''');
