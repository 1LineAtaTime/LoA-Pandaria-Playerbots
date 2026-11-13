-- Quest29409: use one native encounter, not competing SmartAI/native owners.
-- Deploy the matching boss_jaomin_ro / npc_jaomin_ro_hawk native build first.
-- Preserve all creature_text, quest, spawn, phase and personal character data.
CREATE TEMPORARY TABLE IF NOT EXISTS jaomin_native_repair (entry INT PRIMARY KEY);
DELETE FROM jaomin_native_repair;
INSERT INTO jaomin_native_repair SELECT entry FROM creature_template
 WHERE (entry=54611 AND ((AIName='SmartAI' AND ScriptName='') OR (AIName='' AND ScriptName='boss_jaomin_ro')))
 OR (entry=57750 AND ((AIName='SmartAI' AND ScriptName='') OR (AIName='' AND ScriptName='npc_jaomin_ro_hawk')));
-- Treat boss and vehicle as one unit; preserve the entire subsystem if either
-- template has a different custom binding.
SET @jaomin_repair_eligible = (SELECT COUNT(*)=2 FROM jaomin_native_repair);
DELETE FROM jaomin_native_repair WHERE @jaomin_repair_eligible=0;
UPDATE creature_template c JOIN jaomin_native_repair g ON g.entry=c.entry
 SET c.AIName='',c.ScriptName=IF(c.entry=54611,'boss_jaomin_ro','npc_jaomin_ro_hawk'),
 c.faction_A=2357,c.faction_H=2357;
DELETE old FROM smart_scripts old JOIN jaomin_native_repair g ON g.entry=old.entryorguid
 WHERE old.source_type=0;
-- These two conditions gated the retired SmartAI SetData event. The native
-- script checks pending quest eligibility directly and never bypasses phases.
DELETE old FROM conditions old JOIN jaomin_native_repair g ON g.entry=old.SourceEntry
 WHERE old.SourceTypeOrReferenceId=22 AND old.SourceGroup=7 AND old.SourceEntry=54611
 AND old.SourceId=0 AND old.ElseGroup=0 AND old.ConditionTypeOrReference=101
 AND old.ConditionTarget=1 AND old.ConditionValue1 IN (1,2)
 AND old.ConditionValue2=0 AND old.ConditionValue3=0 AND old.NegativeCondition=1
 AND old.ErrorType=0 AND old.ErrorTextId=0 AND old.ScriptName='';
DROP TEMPORARY TABLE IF EXISTS jaomin_native_repair;
