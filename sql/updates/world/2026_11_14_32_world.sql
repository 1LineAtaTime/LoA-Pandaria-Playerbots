-- Eastern Plaguelands, Catalysm (27453): the plague elementals of Lake Mereldar shrank for good and vanished after a sample.
--
-- Before: Blighted Surge 8519 and Plague Ravager 8520 wore Shrink 85178 (scale -50 %, permanent) from their template addon, so
-- they were half size from spawn and after every evade; a sample (Gather Liquid Sample 85144 from the flask) set SmartAI phase
-- 1, cast Flow of Gratitude on the sampler and despawned the elemental 30 s later, mid-fight, with its loot and XP; once reset
-- it could not be sampled again until it respawned. Blighted Horror 8521 and Plague Monstrosity 8522 cast Flow of Gratitude
-- on themselves on aggro.
--
-- What the player should see, sourced:
--  * Elementals are full size; a sampled one shrinks, fights on, and "grows back to its normal size" after a reset, when it
--    can be sampled again (comments 2011-10-31 [+11], 2010-12-04 [+4], 2014-05-24 [+4]; [+199] 2010-11-25: sample, then kill).
--  * Killing one gives Flow of Gratitude, +70 % swim speed for 30 s (reply [+1] "when you kill one you get the buff"; the
--    Wowhead abilities tab of 8519 and 8520 lists only 16555 and 85873, so it is the elementals' own spell). 85873 has
--    CASTABLE_WHILE_DEAD and hits units within 10 yd of its caster (DEST_CASTER + UNIT_DEST_AREA_ENTRY).
-- Ours:
--  * A sample: the elemental attacks the sampler and casts Shrink on itself. A further sample is refused while it is shrunk
--    (condition: target has no 85178), replacing mop's SmartAI-phase conditions (the only use of phases on these entries).
--  * An evade row removes Shrink: SmartAI::EnterEvadeMode keeps auras the creature cast on itself (SmartAI.cpp RemoveAuras),
--    so without it the shrink would survive the reset.
--  * The dying elemental casts Flow of Gratitude on itself, triggered; condition: it lands on players only, so the players
--    within 10 yd of the corpse get it (the killer's pet or a nearby mob does not). The same for 8521 / 8522 (on death instead
--    of on aggro; the spawns dossier's S2-7 drafted this move, handed to this section).
--  * The template aura Thrash 3417 on 8520 stays (not certain either way; neither TDB has it, Wowhead's tab would not show a
--    passive proc).

UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` = 8519 AND `auras` = '85178';
UPDATE `creature_template_addon` SET `auras` = '3417' WHERE `entry` = 8520 AND `auras` = '3417 85178';

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceEntry` = 85144;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(17, 0, 85144, 0, 0, 31, 1, 3, 8519, 0, 0, 0, 0, '', 'Gather Liquid Sample - target Blighted Surge'),
(17, 0, 85144, 0, 0, 1, 1, 85178, 0, 0, 1, 0, 0, '', 'Gather Liquid Sample - target not shrunk (sampled) yet'),
(17, 0, 85144, 0, 1, 31, 1, 3, 8520, 0, 0, 0, 0, '', 'Gather Liquid Sample - target Plague Ravager'),
(17, 0, 85144, 0, 1, 1, 1, 85178, 0, 0, 1, 0, 0, '', 'Gather Liquid Sample - target not shrunk (sampled) yet');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85873;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 85873, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Flow of Gratitude - players only');

-- 8519: rows 0 and 3-7 replaced; 1 (Toxic Bolt), 2 (Aspect of Neptulon), 8 (Drowning Death) kept
DELETE FROM `smart_scripts` WHERE `entryorguid` = 8519 AND `source_type` = 0 AND `id` IN (0, 3, 4, 5, 6, 7);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8519, 0, 3, 4, 8, 0, 100, 0, 85144, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Blighted Surge - On Spellhit Gather Liquid Sample - Attack Invoker'),
(8519, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85178, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Blighted Surge - Linked - Cast Shrink'),
(8519, 0, 5, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85873, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Blighted Surge - On Death - Cast Flow of Gratitude (10 yd, players only)'),
(8519, 0, 6, 0, 7, 0, 100, 0, 0, 0, 0, 0, 0, 28, 85178, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Blighted Surge - On Evade - Remove Shrink');

-- 8520: rows 0 and 2-6 replaced; 1 (Aspect of Neptulon), 7 (Drowning Death), 8 (Mighty Blow) kept
DELETE FROM `smart_scripts` WHERE `entryorguid` = 8520 AND `source_type` = 0 AND `id` IN (0, 2, 3, 4, 5, 6);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8520, 0, 2, 3, 8, 0, 100, 0, 85144, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Ravager - On Spellhit Gather Liquid Sample - Attack Invoker'),
(8520, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85178, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Ravager - Linked - Cast Shrink'),
(8520, 0, 4, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85873, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Ravager - On Death - Cast Flow of Gratitude (10 yd, players only)'),
(8520, 0, 5, 0, 7, 0, 100, 0, 0, 0, 0, 0, 0, 28, 85178, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Ravager - On Evade - Remove Shrink');

-- 8521 / 8522: Flow of Gratitude on death instead of on aggro (row 3)
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (8521, 8522) AND `source_type` = 0 AND `id` = 3;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8521, 0, 3, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85873, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Blighted Horror - On Death - Cast Flow of Gratitude (10 yd, players only)'),
(8522, 0, 3, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85873, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Monstrosity - On Death - Cast Flow of Gratitude (10 yd, players only)');
