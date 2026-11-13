-- Restore published Graveyard Rat torch response (historical SF548 SmartAI).
UPDATE creature_template SET AIName='SmartAI' WHERE entry=37889 AND ScriptName='';
DELETE FROM smart_scripts WHERE entryorguid=37889 AND source_type=0 AND event_type=8 AND event_param1=70631;
INSERT INTO smart_scripts (entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,action_type,action_param1,action_param2,action_param3,action_param4,action_param5,action_param6,target_type,target_param1,target_param2,target_param3,target_x,target_y,target_z,target_o,comment)
VALUES (37889,0,0,0,8,0,100,0,70631,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Graveyard Rat - Half-Burnt Torch hit - Remove rat threat');
-- Spell 70631 uses an entry-filtered area target. Affect these rats only.
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=70631;
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,ConditionTypeOrReference,ConditionValue1,ConditionValue2,ConditionValue3,Comment)
VALUES (13,1,70631,31,3,37889,0,'Half-Burnt Torch: Graveyard Rats only');
-- This periodic dummy script belongs to triggered spell 62791, already supplied
-- by spelleffect_dbc. Base aura 62789 triggers it; do not bind a SpellScript to it.
DELETE FROM spell_script_names WHERE spell_id=62789 AND ScriptName='spell_xt002_heart_overload_periodic';
INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT 62791,'spell_xt002_heart_overload_periodic' WHERE NOT EXISTS
 (SELECT 1 FROM spell_script_names WHERE spell_id=62791 AND ScriptName='spell_xt002_heart_overload_periodic');
-- Native GetAI factories take priority. Preserve ScriptName; install a SmartAI
-- fallback only where scripts exist and the fallback AIName is currently blank.
UPDATE creature_template t JOIN (SELECT DISTINCT entryorguid FROM smart_scripts WHERE source_type=0 AND entryorguid>0) s ON s.entryorguid=t.entry
SET t.AIName='SmartAI' WHERE t.AIName='';
UPDATE gameobject_template t JOIN (SELECT DISTINCT entryorguid FROM smart_scripts WHERE source_type=1 AND entryorguid>0) s ON s.entryorguid=t.entry
SET t.AIName='SmartGameObjectAI' WHERE t.AIName='';
