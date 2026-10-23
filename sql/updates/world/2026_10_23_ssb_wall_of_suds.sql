-- #690 Stormstout Brewery: Yan-Zhu's Wall of Suds is back (boss_yanzhu_the_uncasked.cpp, Sudsy brew). The Wall of Suds damage spell
-- 114466 ("A wall of suds moves across the room, inflicting 100000 Frost damage", Wowhead tooltip API 2026-10-22; 114467 is the
-- instant visual aura the wall carries via creature_template_addon) hooks target type 130 (TARGET_UNIT_CONE_UNK_130) in
-- spell_wall_of_suds, which had no GetSpellScript, no AddSC line and no spell_script_names row, so the wall never filtered itself out.
-- Verify with: .botscenario - spellwatch 114466,114467,115003,114386,116162 on  (which Carbonation id actually carries the AoE).
DELETE FROM `spell_script_names` WHERE `spell_id` = 114466 AND `ScriptName` = 'spell_wall_of_suds';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (114466, 'spell_wall_of_suds');
