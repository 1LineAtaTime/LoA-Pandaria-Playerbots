-- #1003 abilities: drop #987-imported casts whose spell has no effect other than triggering a spell missing from the 5.4.8 Spell.dbc.
-- Scan: every smart_scripts cast (actions 11/85/86/75), creature_template spell1-8 and spell_linked_spell, following
--   FORCE_CAST 140, TRIGGER_SPELL 64, TRIGGER_MISSILE 32 and aura PERIODIC_TRIGGER_SPELL 23 (enum values from SharedDefines.h / SpellAuraDefines.h)
--   through Spell.dbc + SpellEffect.dbc + spell_dbc/spelleffect_dbc (up to 3 levels). Details: (not shipped)
-- Rows dropped (all from #987 files; each spell's ONLY effect is the missing trigger):
--   19734 id 0  Fungal Giant       cast 35273 Unstable Mushroom Primer -> TRIGGER_SPELL 35256 (missing)   [2026_10_29_987_abilities_066_zangarmarsh.sql]
--   62755 id 0  Kor'thik Resonator cast 128381 Sonic Field -> TRIGGER_MISSILE 128383 (missing)            [2026_10_29_987_abilities_113_dread_wastes.sql]
--   66275 id 0  Bellandra Felstorm cast 131446 Fel Energies -> periodic trigger 131448 (missing)          [2026_10_29_987_abilities_108_the_jade_forest.sql]
--   Sources: the reference DB and AC give no working replacement (the reference DB 66275 has only Felbolt 131390 + Fel Storm 131447, both already present; AC 19734 casts 35256, absent from 5.4.8).
-- Idempotent: the deletes match on entry, source_type, id AND the action spell id, so re-running is a no-op.
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 19734 AND `id` = 0 AND `action_type` = 11 AND `action_param1` = 35273;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 62755 AND `id` = 0 AND `action_type` = 11 AND `action_param1` = 128381;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 66275 AND `id` = 0 AND `action_type` = 11 AND `action_param1` = 131446;
-- 19734 had AIName '' before #987 (backup (not shipped) and now has no rows left.
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 19734 AND `AIName` = 'SmartAI' AND `ScriptName` = ''
  AND NOT EXISTS (SELECT 1 FROM `smart_scripts` s WHERE s.`entryorguid` = 19734 AND s.`source_type` = 0);
-- Undo (full row backups in (not shipped):
-- UPDATE creature_template SET AIName='SmartAI' WHERE entry=19734;
-- INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (19734,0,0,0,0,0,100,0,0,0,2300,3900,0,11,35273,64,0,0,0,0,2,0,0,0,0,0,0,0,0,''Fungal Giant - In Combat - Cast \''Unstable Mushroom Primer\'''');
-- INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (62755,0,0,0,0,0,100,0,6000,9000,15000,20000,0,11,128381,0,0,0,0,0,2,0,0,0,0,0,0,0,0,''Kor\''thik Resonator - In Combat - Cast \''Sonic Field\'''');
-- INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (66275,0,0,0,1,0,100,0,1000,2000,30000,30000,0,11,131446,32,0,0,0,0,1,0,0,0,0,0,0,0,0,''Bellandra Felstorm - Out of Combat - Cast \''Fel Energies\'''');
