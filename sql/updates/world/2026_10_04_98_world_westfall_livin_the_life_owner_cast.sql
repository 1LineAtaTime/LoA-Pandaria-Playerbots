-- #485 (epic #441): quest 26228 "Livin' the Life" - make the Jangolode scene's General Trigger spells reach the actors.
--
-- Root cause (t441 r5 diagnostic, [J441] lines from spell_westfall_jangolode_general_trigger):
--   Two-Shoed Lou's Old House 42500 casts all 8 General Trigger spells on time, but every area search comes back with
--   0 targets and "nearest 42492/42515: none within 100 yd", with the house's private owner Empty.
--   * The actors are PRIVATE objects: Glubtok 42492 (79263, SummonProperties 2978) and the Shadowy Figure 42515
--     (79265, SummonProperties 2979) have flags 0x2D18, which include SUMMON_PROP_FLAG_PERSONAL_SPAWN 0x10
--     (DBCEnums.h:589), so Spell::EffectSummonType makes the summoning player their private owner (SpellEffects.cpp:2521).
--   * The house is NOT private: SummonProperties 2977 has flags 0x2908 (no 0x10).
--   * WorldObject::CheckPrivateObjectOwnerVisibility (Object.cpp:2219) lets only the owner, another private object of
--     the same owner, or the owner's group see a private object. SpellInfo::CheckTarget (SpellInfo.cpp:2041) and the
--     grid searchers (GridNotifiers.h:1223) apply it, so the house can never target the actors. No SPELLHIT reaction
--     fires, the Figure never casts the credit spell 79275, and the quest never completes.
-- Same rule in TrinityCore master / cata_classic (Object.cpp CheckPrivateObjectOwnerVisibility, identical).
--
-- Fix, 1:1 with TrinityCore's rework of this quest (TrinityCore/TrinityCore
-- sql/old/11.x/world/25081_2025_10_29/2025_10_19_12_world.sql, timed list 4250000): the house's owner/summoner - the
-- player riding it - casts each trigger on itself (TC: action 85 SELF_CAST, target 23 OWNER_OR_SUMMONER). The player
-- owns the actors, so the area search (TARGET_UNIT_SRC_AREA_ENTRY + the existing type-13 entry conditions) finds them.
-- LoA's action 85 is INVOKER_CAST, so the same thing is written as SMART_ACTION_CROSS_CAST (86): caster target 23
-- (owner/summoner, SmartScript.cpp:2891 - charmer/owner, else summoner, else creator) casts on target 23 (itself),
-- SMARTCAST_TRIGGERED (0x2) so it ignores the vehicle-seat cast restriction (Spell.cpp:6381), like TC's
-- TRIGGERED_FULL_MASK spells in this quest. Delays are unchanged (9.3/6.5/8/4/16/4.1/6.4/11.4 s).
-- The despawn path needs no change: on passenger removed the house casts 79274 (FORCE_CAST) on the player, who then
-- casts 79273 itself (as in TC).
-- Rollback: re-run 2026_10_04_75_world_westfall_livin_the_life.sql (it rewrites 4250000 with the old self-casts).

DELETE FROM `smart_scripts` WHERE `entryorguid` = 4250000 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4250000, 9, 0, 0, 0, 0, 100, 0, 9300, 9300, 0, 0, 86, 79290, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 1: Glubtok'' (triggered)'),
(4250000, 9, 1, 0, 0, 0, 100, 0, 6500, 6500, 0, 0, 86, 79279, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 1: Figure'' (triggered)'),
(4250000, 9, 2, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 86, 79292, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 2: Glubtok'' (triggered)'),
(4250000, 9, 3, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 86, 79283, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 2: Figure'' (triggered)'),
(4250000, 9, 4, 0, 0, 0, 100, 0, 16000, 16000, 0, 0, 86, 79294, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 3: Glubtok'' (triggered)'),
(4250000, 9, 5, 0, 0, 0, 100, 0, 4100, 4100, 0, 0, 86, 79284, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 3: Figure'' (triggered)'),
(4250000, 9, 6, 0, 0, 0, 100, 0, 6400, 6400, 0, 0, 86, 79297, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 4: Glubtok'' (triggered)'),
(4250000, 9, 7, 0, 0, 0, 100, 0, 11400, 11400, 0, 0, 86, 79287, 2, 23, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Two-Shoed Lou''s Old House - On Script - Owner/Summoner Casts ''General Trigger 4: Figure'' (triggered)');
