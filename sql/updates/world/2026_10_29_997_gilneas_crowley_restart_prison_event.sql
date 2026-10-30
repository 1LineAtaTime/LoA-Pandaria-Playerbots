-- #997: Lord Darius Crowley (35077) starts the "By the Skin of His Teeth" (14154) hold-out scene only when the
-- quest is accepted (SAI event 19). If the scene is lost (death or a combat logout removes auras 66894/68218 and
-- the worgen waves), the quest stays incomplete forever. Talking to Crowley again while the quest is taken and the
-- 2 minute aura 68218 is gone now re-casts the same forcecast (66914) and so restarts the scene.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35077 AND `source_type` = 0 AND `id` = 3;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35077, 0, 3, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 11, 66914, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Darius Crowley - On Gossip Hello (quest 14154 taken, scene aura 68218 gone) - Cast Spell ''Gilneas - Quest - Gilneas Prison Periodic Forcecast''');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 4 AND `SourceEntry` = 35077 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 4, 35077, 0, 0, 9, 0, 14154, 0, 0, 0, 0, 0, '', 'Crowley restart scene - player has quest 14154 taken'),
(22, 4, 35077, 0, 0, 1, 0, 68218, 0, 0, 1, 0, 0, '', 'Crowley restart scene - player does not have aura 68218');
