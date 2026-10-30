-- creature_classlevelstats.damage_exp4 becomes damage per second, like damage_base .. damage_exp3.
-- Goes together with the core change in Creature::UpdateDamagePhysical (StatSystem.cpp) that multiplies the creature weapon
-- damage by the base attack time, as TrinityCore and AzerothCore do (Creature::CalculateMinMaxDamage).
--
-- Units before this change:
--   * damage_base .. damage_exp3 are per-second curves (TrinityCore/AzerothCore semantics), but this core added them per swing
--     with no attack-time factor, so expansion 0-3 creatures hit about a third below AzerothCore (2.0 s swing: 0.57-0.66 of
--     AzerothCore's per-swing damage for the same template at the same level; with the core change 1.00).
--   * damage_exp4 is per swing for a 2.0 s attack: levels 1-19 are the client game table NpcDamageByClassExp4 (MoP Classic
--     5.5.4, damage per second) x 2 (level 6: 4.254545 x 2 = 8.5091), levels 20+ the per-level minimum mindmg of the old
--     expansion-4 templates (2.0 s swings).
-- Halving damage_exp4 puts it on the per-second scale (levels 1-19 become exactly the client table), so every 2.0 s
-- expansion-4 creature keeps exactly the damage it has today; 1.0 s / 1.5 s / 2.5 s expansion-4 creatures get 0.5x / 0.75x /
-- 1.25x their current per-swing damage (the same damage per second as a 2.0 s creature).
--
-- Retail checks (per swing):
--   * Classic Beast Lore damage ranges (public: cmangos/classic-db Updates/4834_beast_lore_cls.sql): at the same level the
--     per-swing damage is proportional to the attack time (level 10: 2.0 s 13.2-17.3, 1.5 s 9.9-13.0, 1.3 s 8.5-11.3,
--     1.2 s 7.9-10.4). Young Fleshripper 199 (level 10, 2.0 s) 13.2-17.3: 7.8-9.5 before, 11.1-14.4 after.
--     Rabid Shardtooth 7446 (level 60) 103.9-137.9: 65.0-81.9 before, 90.0-119.4 after (its swing here is 1.743 s).
--   * MoP Classic 5.5.4.70449 sniffs (public: svenbledt/trinitycore-mop-classic
--     sql/updates/world/mop_classic/2026_10_19_29_world.sql, median non-crit melee hit): 2.0 s expansion-4 creatures are
--     unchanged by this update (Plump Virmen 55483, level 6: retail 10, ours 11.2-15.5).
--
-- Applied alone, without the core change, this would halve every expansion-4 creature's melee. The update runs once: it only
-- applies while damage_exp4 still holds the per-swing values (level 90 warrior: 14666 per swing, 7333 per second).
UPDATE `creature_classlevelstats` SET `damage_exp4` = `damage_exp4` / 2
WHERE (SELECT `x`.`d` FROM (SELECT `damage_exp4` AS `d` FROM `creature_classlevelstats` WHERE `level` = 90 AND `class` = 1) AS `x`) > 10000;

-- Check: level 6 / 7 / 8 warrior 4.25455 / 4.5818 / 4.9091, level 90 warrior 7333.
SELECT `level`, `class`, `damage_exp4` FROM `creature_classlevelstats` WHERE `class` = 1 AND `level` IN (6, 7, 8, 90);

-- Undo (together with reverting the core change; halving and doubling a float is exact):
-- UPDATE `creature_classlevelstats` SET `damage_exp4` = `damage_exp4` * 2
-- WHERE (SELECT `x`.`d` FROM (SELECT `damage_exp4` AS `d` FROM `creature_classlevelstats` WHERE `level` = 90 AND `class` = 1) AS `x`) < 10000;
