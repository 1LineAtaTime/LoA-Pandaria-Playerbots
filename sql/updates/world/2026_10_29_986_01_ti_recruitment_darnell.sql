-- ported from PR 15 2026_10_23_00_world.sql (#986)
-- Recruitment (26800): pointing Darnell (49337) at a Scarlet Corpse (49340)

-- Right-clicking a corpse makes the player cast Select Scarlet Corpse (91942) on it (the corpse was the caster and the player the
-- target). spell_select_scarlet_corpse makes the player point and sends their Darnell to the corpse.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 49340;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(49340, 91942, 1, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 91942;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(91942, 'spell_select_scarlet_corpse');

-- Only while Recruitment is in the quest log, and a corpse is only picked up by a Darnell that was sent to it
-- (the selection is Select Scarlet Corpse's aura on the corpse)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 49340 AND `SourceEntry` = 91942;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 1 AND `SourceEntry` = 49340 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 49340, 91942, 0, 0, 9, 0, 26800, 0, 0, 0, 0, 0, '', 'Scarlet Corpse - Spellclick only while quest 26800 is taken'),
(22, 1, 49340, 0, 0, 1, 1, 91942, 1, 0, 0, 0, 0, '', 'Scarlet Corpse - Only picked up when selected by the player');

-- Scarlet Corpse: Darnell is a personal summon, which "closest creature" lookups skip, so the corpse finds him with a range lookup.
-- Darnell casts Reverse Cast Ride Vehicle on it, which gives the kill credit to his summoner, and spell_reverse_cast_ride_vehicle_darnell
-- makes the corpse ride on him (his vehicle has a seat for each of the 6 corpses). Saltain despawns the corpses on turn-in.
UPDATE `smart_scripts` SET `link` = 0, `action_param3` = 9, `action_param4` = 49337, `action_param5` = 0, `action_param6` = 5,
`comment` = 'Scarlet Corpse - On Darnell In Range - Darnell Cast Reverse Cast Ride Vehicle'
WHERE `entryorguid` = 49340 AND `source_type` = 0 AND `id` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 49340 AND `source_type` = 0 AND `id` = 1;

DELETE FROM `spell_script_names` WHERE `spell_id` = 91945;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(91945, 'spell_reverse_cast_ride_vehicle_darnell');

-- Darnell: once at the corpse, wait a moment and follow the player again
DELETE FROM `smart_scripts` WHERE `entryorguid` = 49337 AND `source_type` = 0 AND `id` = 7;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 4933703 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(49337, 0, 7, 0, 34, 0, 100, 0, 8, 49340, 0, 0, 80, 4933703, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darnell - On Point 49340 Reached (Scarlet Corpse) - Start Script'),
(4933703, 9, 0, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 29, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Darnell - On Script - Set Follow Owner');

-- Deathguard Saltain: on turning in Beyond the Graves or Recruitment, Darnell goes away. He is a personal summon that "closest
-- creature" cannot find, so use a range lookup (this also reaches other players' Darnells within 20 yards).
UPDATE `smart_scripts` SET `target_type` = 9, `target_param1` = 49337, `target_param2` = 0, `target_param3` = 20
WHERE (`entryorguid` = 1740 AND `source_type` = 0 AND `id` = 2) OR (`entryorguid` = 174001 AND `source_type` = 9 AND `id` = 0);

-- Summon Darnell (91938): spell_area casts it again on login or area change while Beyond the Graves or Recruitment is in the quest
-- log (the aura is no longer saved, so Darnell comes back after a relog) and removes it on an area change once the quest is not.
-- spell_summon_darnell unsummons Darnell when the aura goes away. Deathknell is its own zone (6454) inside Tirisfal Glades (85), and
-- the quests go between both; without a fitting row the summon on quest accept fails with "wrong zone".
DELETE FROM `spell_area` WHERE `spell` = 91938;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(91938, 85, 25089, 0, -8326, 0, 2, 1, (1 << 1) | (1 << 3), 0),
(91938, 85, 26800, 0, -8326, 0, 2, 1, (1 << 1) | (1 << 3), 0),
(91938, 6454, 25089, 0, -8326, 0, 2, 1, (1 << 1) | (1 << 3), 0),
(91938, 6454, 26800, 0, -8326, 0, 2, 1, (1 << 1) | (1 << 3), 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 91938;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(91938, 'spell_summon_darnell');

-- Scarlet Corpses are no quest givers (Recruitment is started by Deathguard Saltain)
DELETE FROM `creature_queststarter` WHERE `id` = 49340 AND `quest` = 26800;
UPDATE `creature_template` SET `npcflag` = 0x01000000 WHERE `entry` = 49340;
