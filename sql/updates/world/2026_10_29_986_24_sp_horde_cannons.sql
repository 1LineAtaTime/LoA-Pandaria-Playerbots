-- ported from PR 15 2026_10_27_01_world.sql (#986)
-- Silverpine Forest, The Waters Run Red...: let players board the Horde Cannons (45263) at the Decrepit Fields. The cannon is a
-- vehicle with one seat that the rider controls, and Rocket Blast on its bar, but it had no spellclick, so clicking it did nothing.
-- No spellclick was sniffed for it; it uses the generic Ride Vehicle Hardcoded.
UPDATE `creature_template` SET `npcflag` = 16777216 WHERE `entry` = 45263;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 45263;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(45263, 46598, 1, 0);

-- Only for The Waters Run Red...: it can be boarded while the quest is in progress, and throws the player out once it is done.
-- Having a vehicle condition also keeps the cannon in place: a vehicle without one despawns when its rider leaves it.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 45263 AND `SourceEntry` = 46598;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 16 AND `SourceEntry` = 45263;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 45263, 46598, 0, 0, 9, 0, 27232, 0, 0, 0, 0, 0, '', 'Horde Cannon - Board it only while The Waters Run Red... is in progress'),
(16, 0, 45263, 0, 0, 9, 0, 27232, 0, 0, 0, 0, 0, '', 'Horde Cannon - Leave it once The Waters Run Red... is no longer in progress');
