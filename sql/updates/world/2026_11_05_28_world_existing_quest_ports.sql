-- Activate reviewed native ports already in this source tree. Back up first.
-- Adapt 2026_09_28_21 and 2026_10_09_12 to actual current values; preserve other
-- spell handlers and authored text. Native build carrying the corrected pillory
-- text groups and the original complete Kezan handlers is required.
UPDATE creature_template SET ScriptName='npc_hot_rod_rolling_my_homies' WHERE entry=34840 AND ScriptName='npc_original_hot_rod' AND AIName='';
UPDATE creature_template SET ScriptName='npc_homies' WHERE entry=34890 AND ScriptName='npc_friend_gob' AND AIName='';
UPDATE creature_template SET ScriptName='npc_homies',AIName='' WHERE entry IN (34892,34954) AND ScriptName='' AND AIName='SmartAI';
UPDATE creature_template SET ScriptName='npc_homies_summon' WHERE entry IN (34957,34958,34959) AND ScriptName='npc_friend_gob' AND AIName='';
UPDATE creature_template SET ScriptName='npc_krennan_aranas_pillory' WHERE entry=36331 AND ScriptName='npc_trigger_event_first_c3' AND AIName='';
UPDATE quest_template_addon SET ScriptName='quest_rolling_with_my_homies' WHERE ID=14071 AND ScriptName='';
-- Retire only the two exact automatic-on-foot credit rows replaced by native
-- vehicle pickup. Preserve any unrelated/custom SmartAI.
DELETE FROM smart_scripts WHERE source_type=0 AND id=0 AND event_type=10 AND action_type=33 AND target_type=7
 AND ((entryorguid=34892 AND action_param1=34957 AND comment='Ace') OR (entryorguid=34954 AND action_param1=34958 AND comment='Gobber'));
-- 61178 already has its complete Wintergrasp/Kezan dispatcher. This is an
-- obsolete duplicate name, not removal of the horn/passenger behavior.
DELETE old FROM spell_script_names old INNER JOIN spell_script_names active
 ON active.spell_id=old.spell_id AND active.ScriptName='spell_wintergrasp_grab_passenger'
 WHERE old.spell_id=61178 AND old.ScriptName='spell_klaxon';
INSERT INTO spell_script_names (spell_id,ScriptName)
 SELECT 68639,'spell_gilneas_forcecast_worgen_intro_completion' WHERE NOT EXISTS
 (SELECT 1 FROM spell_script_names WHERE spell_id=68639 AND ScriptName='spell_gilneas_forcecast_worgen_intro_completion');
-- Existing static Genn14375 giver at spawn375757 is already in phase4097.
-- The scene lasts only until14375 is rewarded; reward68639 removes the curse.
INSERT INTO spell_area (spell,area,quest_start,quest_end,aura_spell,racemask,gender,autocast,quest_start_status,quest_end_status)
 SELECT 68630,4786,14222,14375,0,0,2,1,64,11 WHERE NOT EXISTS
 (SELECT 1 FROM spell_area WHERE spell=68630 AND area=4786);
