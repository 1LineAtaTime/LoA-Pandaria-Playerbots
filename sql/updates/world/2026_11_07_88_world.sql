-- Stormpike Engineer 49116 (Hillsbrad Foothills, Sludge Fields): Heavy Dynamite 32191 every 17-20 s instead of every 3.5-4.1 s, and melee between throws.
-- Sourced: the timers (first throw 3-8 s into the fight, then every 16.8-19.9 s) are both TDBs' (442 and 1210) Schnottz Overseer 48205/51753 rows for
-- the same spell, i.e. from other users of 32191, not from a sniff of the engineers. Both TDBs give 49116 itself 0-0 / 3.5-4.1 s with cast flag 64,
-- which is what mop had.
-- Ours: cast flag 64 (SMARTCAST_COMBAT_MOVE) dropped. With it the engineer stopped combat movement after its first throw and stood at range; with 0
-- it chases and melees, and the instant 0-30 yd throw lands whenever its timer comes up with the target in range. The flee at 15% (id 1), the
-- template emote 133 and the 60 s respawn are unchanged.
DELETE FROM `smart_scripts` WHERE `entryorguid`=49116 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(49116, 0, 0, 0, 0, 0, 100, 0, 3000, 8000, 16800, 19900, 11, 32191, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stormpike Engineer - In Combat - Cast ''Heavy Dynamite''');
