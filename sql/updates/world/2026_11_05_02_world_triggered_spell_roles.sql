-- Attach scripts to the effects they implement, preserving native trigger auras.
-- The quadrant selector handles only 77939/77942/77943/77944. 77827 is a
-- separate cone damage/stun spell; 77836 retains the quadrant trigger script.
DELETE FROM spell_script_names WHERE spell_id=77827 AND ScriptName='spell_onyxia_lightning_discharge';
-- 111698 periodically triggers 112060. Only 112060 has the filtered area
-- absorb shield implemented by this combined SpellScript/AuraScript.
DELETE FROM spell_script_names WHERE spell_id=111698 AND ScriptName='spell_shadopan_apparitions';
-- 116267 is Incanter's Ward's damage modifier, not an absorb shield. Its
-- native modifier and the correct Incanter's Ward scripts remain intact.
DELETE FROM spell_script_names WHERE spell_id=116267 AND ScriptName='spell_mage_incanters_absorbtion_absorb';
-- Tranquility (Symbiosis): 113277 is the periodic producer; 113278 is the
-- area heal/HoT that needs the smart-raid-target filter named in its source.
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName)
 SELECT 113278,'spell_common_smart_heal_raid_25' FROM spell_script_names
 WHERE spell_id=113277 AND ScriptName='spell_common_smart_heal_raid_25';
DELETE FROM spell_script_names WHERE spell_id=113277 AND ScriptName='spell_common_smart_heal_raid_25';
