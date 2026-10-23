-- Refs #439, #481. Quest 24616 "Losing Your Tail" (Stormglen, map 654, player phase 131072).
--
-- Intended (Wowpedia "Losing Your Tail", MediaWiki API read 2026-10-04): walk into the Dark Scout's Freezing Trap
-- on the road north of Belysra; the scout appears ("Such easy prey. Sylvanas will be most pleased!"), a boss emote
-- tells the player to use the talisman, she winds up an aimed shot that one-shots players of that level; Belysra's
-- Talisman (item 49944, spell 70797) breaks the player free ("How did you--?!") and the player kills her.
-- Fork today: Dark Scout 37953 is a static spawn (guid 222093) with no AI, so the quest is "kill a mob" and the
-- talisman (TARGET_UNIT_NEARBY_ENTRY, no conditions row in the fork) has nothing to hit.
--
-- Source: MoP 5.4.8 world data:
--   * smart_scripts -219500 (trigger 35374 at the road): OOC_LOS(noHostile 1, 25 yd, cd 1 s) -> INVOKER_CAST 70794
--     (triggered) on the invoker, conditions 22/1/-219500: QUEST_TAKEN 24616 and NOT near 37953 within 30 yd.
--   * smart_scripts 37953 (12 rows), conditions 13/2/70797 -> creature 37953. MoP 5.4.8 has no static 37953 spawn.
-- DBC: 70794 "Freezing Trap Effect" = stun 20 s + trigger 70795
-- "Summon Dark Scout" (37953 in front of the player, 300 s) + 95845 tracker; 70796 "Aimed Shot" 10 s cast, 75% max
-- health; 70797 talisman has SPELL_ATTR5_USABLE_WHILE_STUNNED, effect 1 = dummy on nearby entry.
-- Fork deltas against MoP 5.4.8:
--   * trigger guid 219500 phaseMask 65535 -> 196607: in Stormglen the player is in phase 131072 only
--     (PhaseData::GetPhaseMaskForSpawn, PhaseMgr.cpp:244), so a 65535 trigger never sees him (no MoveInLineOfSight).
--   * the scout targets her summoner (SMART_TARGET_OWNER_OR_SUMMONER) instead of "closest player", so two players
--     on the quest each get their own scout and shot.
--   * The MoP talk groups are off by one (spellhit said the ambush line); here group 1 = ambush line (BT 37672),
--     group 2 = talisman boss emote (BT 37674), group 0 (fork row, BT 37673) = "How did you--?!" on the talisman.
--   * on the talisman hit the scout also removes 70794 from the player (the break-free the quest text describes;
--     70797's own aura is a charm-mechanic immunity and does not drop the stun).
--
-- Rollback:
--   UPDATE `creature` SET `phaseMask` = 65535 WHERE `guid` = 219500;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (35374, 37953);
--   DELETE FROM `smart_scripts` WHERE (`entryorguid` = -219500 OR `entryorguid` = 37953) AND `source_type` = 0;
--   DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500) OR (`SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 70797);
--   DELETE FROM `creature_text` WHERE `CreatureID` = 37953 AND `GroupID` IN (1, 2);
--   INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `curhealth`, `curmana`, `MovementType`) VALUES
--   (222093, 37953, 654, 4714, 4730, 1, 131072, 21746, -2226.2, 1444.82, -25.4718, 4.31105, 300, 3, 160, 350, 1);
-- Re-runnable: UPDATE + DELETE-then-INSERT.

-- The ambush trigger must share the Stormglen phase.
UPDATE `creature` SET `phaseMask` = 196607 WHERE `guid` = 219500 AND `id` = 35374;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (35374, 37953) AND `ScriptName` = '';

-- The scout only exists as the ambush summon.
DELETE FROM `creature` WHERE `guid` = 222093 AND `id` = 37953;
DELETE FROM `creature_addon` WHERE `guid` = 222093;

DELETE FROM `smart_scripts` WHERE (`entryorguid` = -219500 OR `entryorguid` = 37953) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-219500, 0, 0, 0, 10, 0, 100, 0, 1, 25, 1000, 1000, 1, 85, 70794, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Losing Your Tail trap trigger - OOC LOS player (quest 24616 taken, no scout near) - Invoker Cast Freezing Trap Effect'),
(37953, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 59, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Set Run'),
(37953, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Say Line 1 (Such easy prey)'),
(37953, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Boss Emote Line 2 (use the talisman)'),
(37953, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 18, 898, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Set Unit Flags (non attackable, immune)'),
(37953, 0, 4, 0, 60, 0, 100, 1, 3500, 3500, 0, 0, 0, 11, 70796, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - 3.5 s after summon - Cast Aimed Shot on summoner'),
(37953, 0, 5, 6, 8, 0, 100, 1, 70797, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Say Line 0 (How did you--?!)'),
(37953, 0, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 0, 92, 0, 70796, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Interrupt Aimed Shot'),
(37953, 0, 7, 8, 61, 0, 100, 0, 0, 0, 0, 0, 0, 19, 898, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Remove Unit Flags'),
(37953, 0, 8, 9, 61, 0, 100, 0, 0, 0, 0, 0, 0, 28, 70794, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Free summoner from Freezing Trap'),
(37953, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Attack summoner'),
(37953, 0, 10, 0, 7, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Evade - Despawn');

DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500) OR (`SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 70797);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, -219500, 0, 0, 9, 0, 24616, 0, 0, 0, 0, 0, '', 'Losing Your Tail trap - invoker has quest 24616 taken'),
(22, 1, -219500, 0, 0, 29, 0, 37953, 30, 0, 1, 0, 0, '', 'Losing Your Tail trap - no Dark Scout within 30 yd of the invoker'),
(13, 2, 70797, 0, 0, 31, 0, 3, 37953, 0, 0, 0, 0, '', 'Belysra''s Talisman effect 1 - targets Dark Scout');

DELETE FROM `creature_text` WHERE `CreatureID` = 37953 AND `GroupID` IN (1, 2);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37953, 1, 0, 'Such easy prey.  Sylvanas will be most pleased!', 12, 0, 100, 0, 0, 0, 0, 37672, 0, 'Dark Scout - ambush'),
(37953, 2, 0, 'Use Belrysa''s Talisman to escape before the Dark Scout shoots you!', 41, 0, 100, 0, 0, 0, 0, 37674, 0, 'Dark Scout - talisman hint');
