-- Match the actual constructor registration names of these exact C++ classes.
-- Source: greenstone_village.cpp and scholomance.cpp. Preserve other templates,
-- SmartAI data and the potion's use settings.
UPDATE `creature_template` SET ScriptName='npc_greenstone_terror'
WHERE entry=61183 AND ScriptName='npc_greenstone_belligerent_blossom';
UPDATE `creature_template` SET ScriptName='npc_greenstone_cursed_brew'
WHERE entry=61174 AND ScriptName='npc_greenstone_cursed_jade';
UPDATE `gameobject_template` SET ScriptName='go_alchemy_bottle_white'
WHERE entry=211669 AND ScriptName='go_polyformic_acid_potion';

-- boss_tayak.cpp's spell_unseen_strike_aura constructor registers the name
-- spell_unseen_strike. The valid binding is already present here; avoid double
-- registration or replacing it with another handler.
INSERT IGNORE INTO `spell_script_names` (spell_id,ScriptName)
SELECT spell_id,'spell_unseen_strike' FROM `spell_script_names`
WHERE spell_id=122982 AND ScriptName='spell_unseen_strike_aura';
DELETE old_alias FROM `spell_script_names` old_alias
JOIN `spell_script_names` registered ON registered.spell_id=old_alias.spell_id
    AND registered.ScriptName='spell_unseen_strike'
WHERE old_alias.spell_id=122982 AND old_alias.ScriptName='spell_unseen_strike_aura';
