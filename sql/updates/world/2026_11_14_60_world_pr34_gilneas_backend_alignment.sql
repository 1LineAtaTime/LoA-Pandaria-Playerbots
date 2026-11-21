-- Follow-up to SQL30: engine-load comparison exposed historical backend
-- assumptions absent in this database. Apply both migrations offline.
-- Crowley's Horse's old native name is unimplemented here. The historical
-- boarding bridge alone is not a complete ride (missing route/jump/ejection
-- callbacks). Defer it, preserving the existing script name and unbound TAL,
-- rather than enabling an incomplete SmartAI or fabricating quest completion.
DELETE s FROM smart_scripts s INNER JOIN creature_template t ON t.entry=s.entryorguid
 WHERE t.entry=35231 AND t.AIName='' AND t.ScriptName='npc_crowley_horse'
 AND s.source_type=0 AND s.id=8 AND s.event_type=27 AND s.action_type=80 AND s.action_param1=3523100
 AND s.comment='Crowley''s Horse - On Player Boarded (static spawn) - Start Timed Action Script 3523100';
DELETE c FROM conditions c INNER JOIN creature_template t ON t.entry=35231
 WHERE t.AIName='' AND t.ScriptName='npc_crowley_horse'
 AND c.SourceTypeOrReferenceId=22 AND c.SourceGroup=9 AND c.SourceEntry=35231
 AND c.ConditionTypeOrReference=31 AND c.ConditionValue1=4
 AND c.Comment='Crowley''s Horse - start the run only when a player boards';
-- The reviewed native survivor now owns spawn/combat/rescue handling. Its old
-- SmartAI is no longer a second valid backend; keep unrelated timed lists.
DELETE s FROM smart_scripts s INNER JOIN creature_template t ON t.entry=s.entryorguid
 WHERE t.entry=37067 AND t.ScriptName='npc_crash_survivor' AND t.AIName='' AND s.source_type=0;
-- Lorna's former C2 script is missing. Install the actual reviewed quest-accept
-- handoff as SmartAI while retaining ordinary questgiver behavior. Krennan's
-- existing group1 is exactly the authored "time to join the fray" line.
UPDATE creature_template SET AIName='SmartAI',ScriptName=''
 WHERE entry=37783 AND AIName='' AND ScriptName='npc_lorna_crowley_c2';
UPDATE smart_scripts SET action_param1=1
 WHERE entryorguid=37783 AND source_type=0 AND id=0 AND event_type=19 AND event_param1=24904
 AND action_type=1 AND action_param1=0 AND target_type=19 AND target_param1=38553;
