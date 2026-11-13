-- Preserve actual passive/producer/trigger chains. These are exact erroneous
-- bindings identified from installed 5.4.8 effects and the bound source handler.
-- Sword and Board already binds passive46953; Devastate20243 has no aura.
DELETE FROM spell_script_names WHERE spell_id=20243 AND ScriptName='spell_warr_sword_and_board';
-- 88091/88092 trigger the three Arcane/Fire summons. The selector calls these
-- spells, so attaching it to its own outputs is also a recursion hazard.
-- Preserve selector55342/110621 and the existing valid dummy63093 binding.
DELETE FROM spell_script_names WHERE spell_id IN (88091,88092) AND ScriptName='spell_mage_mirror_image';
-- Glyph108939 is the passive glyph, not Levitate. Preserve the existing correct
-- binding on aura111758, cast by the separate Levitate1706 dummy handler.
DELETE FROM spell_script_names WHERE spell_id=108939 AND ScriptName='spell_pri_glyph_of_levitate';
-- Chi Brew115399 is a direct restore/trigger spell, not the Healing Elixirs
-- passive122280. Preserve that passive and its timer/heal implementations.
DELETE FROM spell_script_names WHERE spell_id=115399 AND ScriptName='spell_monk_healing_elixirs';
-- This obsolete area-target callback only queries an area, discards the result,
-- and returns true. 23218 is a native self speed aura, not an area aura.
DELETE FROM spell_script_names WHERE spell_id=23218 AND ScriptName='spell_dru_shapeshift_move_speed';
-- Mail Specialization86529 is a tooltip/learned dummy, not an aura. Actual
-- specialization passives are 86108(Elemental),86099(Enhancement),86100(Resto)
-- in SpecializationSpells.dbc; their native armor-gated stat auras remain.
DELETE FROM spell_script_names WHERE spell_id=86529 AND ScriptName='spell_sha_mail_specialization';
