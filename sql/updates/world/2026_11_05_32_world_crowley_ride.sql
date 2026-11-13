-- Requires the native Crowley questgiver/horse/mounted-actor handlers from
-- this source batch. Apply offline after backing up creature_template and
-- smart_scripts. Preserve the local33-point path and all quest credit data.
-- Mounted Crowley must not run the legacy nearest-horse SmartAI while the
-- native horse explicitly assigns its own actor to seat1.
UPDATE creature_template SET AIName='',ScriptName='npc_crowley_mounted'
 WHERE entry=35230 AND AIName='SmartAI' AND ScriptName='';
DELETE s FROM smart_scripts s INNER JOIN creature_template t ON t.entry=s.entryorguid
 WHERE t.entry=35230 AND t.AIName='' AND t.ScriptName='npc_crowley_mounted'
 AND s.source_type=0 AND s.id=0 AND s.event_type=54 AND s.action_type=80 AND s.action_param1=3523000;
-- Ordinary questgiver UI is retained; gossip adds a retry after dismount/logout.
UPDATE creature_template SET npcflag=3
 WHERE entry=35552 AND npcflag=2 AND AIName='' AND ScriptName='npc_lord_darius_crowley_c2';
