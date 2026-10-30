-- Issue #962: runtime/boot errors from the mop1k-v4-1028 run.

-- (a) "HandleEmoteStateCommand cant't be used for oneshot emotes. Emote 4, creature (Entry: 59577)" x7.
-- Farmstead Slave (59577) "set emote quand death" used SMART_ACTION_SET_EMOTE_STATE (17) with emote 4, a oneshot emote.
-- Oneshot emotes are played with SMART_ACTION_PLAY_EMOTE (5), same param1.
UPDATE `smart_scripts` SET `action_type` = 5 WHERE `entryorguid` = 59577 AND `source_type` = 0 AND `id` = 0 AND `action_type` = 17 AND `action_param1` = 4;

-- (b) "Spell::EffectForceCast of spell 88445: triggering unknown spell id 88444" (steady at runtime).
-- Dark Iron Marksman (8338) "In Combat - Cast 'Shoot'" cast 88445, a Cataclysm Shoot whose force-cast target 88444 does not
-- exist in the 5.4.8 data (neither SpellEffect.dbc nor spell_dbc). The other ~120 Shoot users of this table use 6660.
UPDATE `smart_scripts` SET `action_param1` = 6660 WHERE `entryorguid` = 8338 AND `source_type` = 0 AND `id` = 0 AND `action_type` = 11 AND `action_param1` = 88445;

-- (c) "ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (18) for achievement criteria 17826/17831 for non-dungeon/non-raid map 1".
-- Both are plain KILL_CREATURE criteria for Millagazor (52649), a Mount Hyjal world rare. The type 18 row (value 0, no script)
-- is an instance-script requirement that can never be met on a world map (the check always failed); drop it so the kill counts.
DELETE FROM `achievement_criteria_data` WHERE `criteria_id` IN (17826, 17831) AND `type` = 18;
