-- Skitterweb Matriarch: lurks on the ceiling of the Skittering Dark and drops on whoever engages her (npc_silverpine_skitterweb_matriarch).
-- Her SmartAI left her a ground creature in mid-air, out of line of sight from the cave floor.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_skitterweb_matriarch', `flags_extra` = 512 WHERE `entry` = 44906;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 44906 AND `source_type` = 0;

-- Skitterweb Web: the strands run to the Skitterweb Invisible Stalkers above her
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 83827;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83827, 0, 0, 31, 0, 3, 44908, 0, 0, 0, 0, '', 'Skitterweb Web - Target Skitterweb Invisible Stalker');

-- The stalkers hang in the air: no wandering
UPDATE `creature` SET `wander_distance` = 0, `MovementType` = 0 WHERE `id` = 44908;
