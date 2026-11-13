-- Use the implemented Geist glyph handler, now owner-safe on aura removal.
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName)
 SELECT 58640,'aura_glyph_of_the_geist' FROM spell_script_names
 WHERE spell_id=58640 AND ScriptName='spell_dk_glyph_of_the_geist';
DELETE FROM spell_script_names WHERE spell_id=58640 AND ScriptName='spell_dk_glyph_of_the_geist';
-- The Isle of Conquest handler already covers periodic and removal parachutes.
-- Remove the obsolete duplicate only while the actual handler remains bound.
DELETE old FROM spell_script_names old JOIN spell_script_names live
 ON live.spell_id=old.spell_id AND live.ScriptName='spell_ioc_parachute_ic'
 WHERE old.spell_id=66656 AND old.ScriptName='spell_gen_parachute_ic';
