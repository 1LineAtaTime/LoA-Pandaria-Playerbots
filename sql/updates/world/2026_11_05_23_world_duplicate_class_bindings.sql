-- Dark Transformation's current native OnHit already removes Dark Infusion
-- from caster and pet; the absent old form handler would duplicate it.
DELETE old FROM spell_script_names old JOIN spell_script_names current
 ON current.spell_id=old.spell_id AND current.ScriptName='spell_dk_dark_transformation'
 WHERE old.spell_id=63560 AND old.ScriptName='spell_dk_dark_transformation_form';
-- Current Stealth's inherited apply/remove callbacks already implement both
-- Nightstalker and Shadow Focus, including delayed Nightstalker removal so
-- the opening attack retains its bonus. Keep that implementation authoritative.
DELETE old FROM spell_script_names old JOIN spell_script_names current
 ON current.spell_id=old.spell_id AND current.ScriptName='spell_rog_stealth'
 WHERE old.spell_id=1784 AND old.ScriptName='spell_rog_nightstalker';
