-- #643: every DUNGEON_NORMAL row in creature_template_difficulty has health_mod = 1.0 (21,906 rows; 15,552 of them for
-- creatures whose creature_template.Health_mod is not 1). 2026_09_06_00_world.sql part B (#425) created the rows with
-- health_mod = 1.0, and Creature::SelectLevel (Creature.cpp:1211-1218) uses difficultyInfo->HealthMod instead of the
-- template's Health_mod whenever the map difficulty is above REGULAR - a normal 5-man is DUNGEON_DIFFICULTY_NORMAL (1).
-- Result: every normal-dungeon creature spawned with 1/Health_mod of its HP (Hungarfen 5,914 vs 65,054 on AC,
-- Krystallus 12,175 vs 286,113, Bronjahm 13,481 vs 539,240, Ymiron 13,481 vs 404,430) and the bosses died in 2-19 s.
-- Fix: restore the template's Health_mod on those rows (the pre-#425 HP, equal to AC's HealthModifier for all 4 bosses).
-- Other difficulties are not touched: their rows carry real per-difficulty values (DUNGEON_HEROIC 1473 of 1602 differ
-- from 1; the 42 heroic rows at 1.0 need a separate check against the heroic templates).
-- Known secondary difference, NOT changed here: the same migration carried dmg_multiplier into DamageModifier, so boss
-- damage multipliers differ from AC (see #643).
-- Run with the worldserver STOPPED (creature templates load at boot). Re-runnable: a second run matches nothing.
-- ROLLBACK:
--   UPDATE creature_template_difficulty d JOIN creature_template_difficulty_bak_643 b
--     ON b.id = d.id AND b.difficulty = d.difficulty SET d.health_mod = b.health_mod;
--   DROP TABLE creature_template_difficulty_bak_643;

CREATE TABLE IF NOT EXISTS creature_template_difficulty_bak_643 AS
SELECT d.id, d.difficulty, d.health_mod
FROM creature_template_difficulty d
JOIN creature_template t ON t.entry = d.id
WHERE d.difficulty = 'DUNGEON_NORMAL' AND d.health_mod = 1 AND t.Health_mod <> 1;

UPDATE creature_template_difficulty d
JOIN creature_template t ON t.entry = d.id
SET d.health_mod = t.Health_mod
WHERE d.difficulty = 'DUNGEON_NORMAL' AND d.health_mod = 1 AND t.Health_mod <> 1;
