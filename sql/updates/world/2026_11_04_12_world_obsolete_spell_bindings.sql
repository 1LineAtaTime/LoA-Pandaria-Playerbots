-- Scoped subset of 2026_09_08_00_world.sql, verified against this 5.4.8 DBC.
-- Do not apply that older migration wholesale: other hooks need source repairs.
-- 1463 already has spell_mage_incanters_ward and 118859 its cooldown script;
-- 11426 already has spell_mage_ice_barrier. The old absorption scripts refer
-- to absent 44394/44413. 144318 is a flat modifier, not a mana shield aura.
DELETE FROM spell_script_names WHERE spell_id IN (1463,144318)
 AND ScriptName='spell_mage_incanters_absorbtion_manashield';
DELETE FROM spell_script_names WHERE spell_id=11426
 AND ScriptName='spell_mage_incanters_absorbtion_absorb';
-- 137587 is the native cast-while-moving passive (aura 330). This old active
-- cooldown script expects dummy aura 4 and casts absent passive 108507.
DELETE FROM spell_script_names WHERE spell_id=137587
 AND ScriptName='spell_warl_kil_jaedens_cunning';
-- 140149 is a self marker. The target-death reset belongs to 137619, whose
-- existing binding and dummy aura remain untouched.
DELETE FROM spell_script_names WHERE spell_id=140149
 AND ScriptName='spell_rog_marked_for_death';
-- 147997 natively heals 25% and applies periodic 1%-max-health damage.
-- The unused periodic-dummy script instead changed maximum health to a
-- percentage interpreted as absolute HP. Keep native heal/burn effects.
DELETE FROM spell_script_names WHERE spell_id=147997
 AND ScriptName='spell_timeless_isle_cauterize';
