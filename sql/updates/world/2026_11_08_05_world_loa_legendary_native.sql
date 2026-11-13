-- Explicit entry-matched native credit handler; do not replace existing AI.
UPDATE creature_template SET ScriptName='npc_tarecgosa_52835' WHERE entry=52835 AND AIName='' AND ScriptName='';
