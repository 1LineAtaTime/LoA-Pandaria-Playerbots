-- Register Anduin's Decision gossip with the existing native implementation.
-- zone_the_jade_forest.cpp: quest29901, creature56434, summon-Anduin option.
UPDATE creature_template SET ScriptName='npc_prince_anduin_decision_questgiver'
WHERE entry=56434 AND ScriptName='npc_prince_anduin_decisionquestGiver';

-- These obsolete spell names coexist with the canonical binding on the SAME
-- spell ID. Remove only the redundant alias after proving its replacement is
-- present. Never clear a genuinely missing implementation to silence the log.
DELETE old_binding FROM spell_script_names old_binding
JOIN spell_script_names replacement ON replacement.spell_id=old_binding.spell_id
WHERE (old_binding.ScriptName='spell_hunt_steady_shot' AND replacement.ScriptName='spell_hun_steady_shot')
   OR (old_binding.ScriptName='spell_auriaya_strenght_of_the_pack' AND replacement.ScriptName='spell_auriaya_strength_of_the_pack')
   OR (old_binding.ScriptName='spell_igb_incinerating_blast' AND replacement.ScriptName='spell_gb_incinerating_blast')
   OR (old_binding.ScriptName='spell_igb_rocket_artillery' AND replacement.ScriptName='spell_gb_rocket_artillery')
   OR (old_binding.ScriptName='spell_igb_burning_pitch' AND replacement.ScriptName='spell_gb_burning_pitch')
   OR (old_binding.ScriptName='spell_rog_assassasins_resolve' AND replacement.ScriptName='spell_rog_assassins_resolve');
