-- [DEPRECATED] Elixir of Agony 515: remove the starter row from Master Apothecary Faranell 2055.
-- Sourced: TDB 4.4.2 and 1210 have no starter for it. In mop it needs 513 rewarded, and 513 has no starter or ender, so nobody can be
-- offered it; 515 has no ender here either. Faranell keeps his other starter row (29317).
DELETE FROM `creature_queststarter` WHERE `id`=2055 AND `quest`=515;
