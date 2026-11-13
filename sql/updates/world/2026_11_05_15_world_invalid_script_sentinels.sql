-- Literal0 is not a condition script; use the empty no-script sentinel.
-- Preserve every built-in condition value and its normal evaluation.
UPDATE conditions SET ScriptName='' WHERE ScriptName='0';
-- Malformed legacy binding points at a disabled, entirely empty callback.
-- The registered npc_howling_gale AI already schedules all three gale spells
-- every2500ms and pauses them on damage. Preserve that actual implementation
-- and the native85084 aura; do not replace it with an inert registered stub.
DELETE FROM spell_script_names WHERE spell_id=85084
 AND ScriptName='85084spell_howling_gale_howling_gale';
