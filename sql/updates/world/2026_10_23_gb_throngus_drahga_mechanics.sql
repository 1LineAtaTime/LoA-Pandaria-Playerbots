-- #688 Grim Batol (map 670): spell script bindings + one target condition for the mechanics the fork's scripts now run
-- (boss_forgemaster_throngus.cpp, boss_drahga_shadowburner.cpp).
-- DRAFT - not applied. The PM applies it to the test `world` DB with a backup; needs a worldserver restart (spell_script_names is read at
-- startup) and the matching build (the script names below exist only from that build on).
--
-- 74984 Mighty Stomp (Throngus): EFFECT_0 FORCE_CAST 74985 on TARGET_UNIT_SRC_AREA_ENEMY 100 yd. Spell::EffectForceCast makes the hit
--   player cast Cave In 74985 (SUMMON 40228 at TARGET_DEST_DEST) with the boss as its unit target, so the stalker would not land on the
--   player. spell_throngus_mighty_stomp keeps one random player and has the boss summon the Cave In Stalker on that player's spot
--   (4.3.4: The-Cataclysm-Preservation-Project TrinityCore boss_forgemaster_throngus.cpp spell_throngus_mighty_stomp casts it on the
--   player itself). Without this row the boss's AI still casts 74984 (max 1 target) and the default force cast runs.
-- 90949 Devouring Flames damage (Valiona, heroic): spell_drahga_devouring_flames takes 3% off per yard from Valiona (CPP
--   spell_drahga_devouring_flames; Wowpedia: "less damage ... the farther away they are positioned from her").
-- 90950 Devouring Flames channel (Valiona, heroic): EFFECT_0 targets TARGET_UNIT_NEARBY_ENTRY with no `conditions` row (here or in
--   the MoP 5.4.8 data), so SearchNearbyTarget takes any nearby unit; the row limits it to the Devouring Flames stalker 48798 that the
--   script summons on a random player (CPP JustSummoned casts it at that stalker).
-- Before: no rows for 74984 / 90949 in spell_script_names and no conditions rows for 90950 (world, checked 2026-10-23).
-- Undo: DELETE FROM `spell_script_names` WHERE `spell_id` IN (74984, 90949);
--       DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 90950;
DELETE FROM `spell_script_names` WHERE `spell_id` IN (74984, 90949);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(74984, 'spell_throngus_mighty_stomp'),
(90949, 'spell_drahga_devouring_flames');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 90950;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`,
  `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 90950, 0, 0, 31, 0, 3, 48798, 0, 0, 0, 0, '', 'Valiona - Devouring Flames targets Devouring Flames stalker');
