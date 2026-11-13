-- ported from PR 15 2026_10_25_45_world.sql (#986)
-- A Daughter's Embrace (25046): Summon Lilian Voss (Guardian) (73306) is cast again on login or on entering the Scarlet
-- Watch Post, and spell_summon_lilian_voss_guardian only summons her when she is not still following the player.
-- Only while the quest is incomplete: once High Priest Voss is dealt with she has left and must not return to replay it.
DELETE FROM `spell_area` WHERE `spell` = 73306;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(73306, 459, 25046, 0, 0, 0, 2, 1, (1 << 3), 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 73306;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(73306, 'spell_summon_lilian_voss_guardian');
